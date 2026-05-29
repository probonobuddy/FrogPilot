#!/usr/bin/env python3
import json
import math

from dataclasses import asdict, dataclass
from datetime import datetime, timedelta
from pathlib import Path
from uuid import uuid4

# Import sentry first to break the latent frogpilot_variables -> car_helpers -> sentry cycle, matching
# stats_service.py and lib/onroad.py: a cold import of frogpilot_variables raises unless sentry leads.
import openpilot.system.sentry as sentry  # noqa: F401

from openpilot.common.conversions import Conversions as CV

import openpilot.frogpilot.system.the_pond.services.routes_service as routes_service

# Per-route stats are reconstructed from the footage the device still has on disk and cached so the
# heavy log parse runs once per route.
DRIVE_STATS_CACHE_PATH = Path("/data/drive_stats_cache")

# How many of the most recent drives the dashboard feed shows, and how many days "this week" spans.
RECENT_DRIVES_LIMIT = 8
WEEK_DAYS = 7
LOCATION_SOURCE_PRIORITY = ("gpsLocationExternal", "gpsLocation", "liveLocationKalman")
MAX_ROUTE_PATH_POINTS = 96
MAX_ROUTE_POINT_JUMP_M = 2000.0
MIN_ROUTE_POINT_DISTANCE_M = 8.0
EARTH_METERS_PER_DEGREE = 111320.0

# Integrating vEgo over the qlog's own timestamps reconstructs distance; cap the gap between two samples
# so a logging hiccup (or the seam between segments) cannot inflate the integral. qlog carState lands at
# ~10-20Hz, so a one-second cap never clips a real sample step.
MAX_SAMPLE_GAP_SECONDS = 1.0
NANOS_PER_SECOND = 1e9

# A segment still being written holds this lock; parsing it would race the recorder, so it is skipped
# (same guard routes_service uses before serving a clip).
RECORDING_LOCK_NAME = "rlog.lock"


@dataclass(frozen=True)
class RouteStats:
  date: str | None
  distance_m: float
  duration_s: float
  enabled_samples: int
  epoch: float | None
  name: str
  segments: int
  total_samples: int
  path: dict | None = None
  path_attempted: bool = False

  @property
  def engagement(self):
    return (100.0 * self.enabled_samples / self.total_samples) if self.total_samples else 0.0

  @property
  def avg_speed_ms(self):
    return (self.distance_m / self.duration_s) if self.duration_s else 0.0


def drive_history(now=None):
  # The whole dashboard "driving" payload, assembled from retained footage. Everything degrades to an
  # empty list / None when there is no footage, so the frontend simply shows its empty states.
  now = now or datetime.now()
  is_metric = metric()

  all_stats = sorted(every_route_stats(), key=lambda stats: stats.epoch or 0)

  return {
    "lastDrive": last_drive(all_stats, is_metric),
    "records": records(all_stats, is_metric),
    "recentDrives": recent_drives(all_stats, is_metric),
    "thisWeek": this_week(all_stats, is_metric, now),
  }


def every_route_stats():
  cache = DriveStatsCache()
  stats = []
  routes_by_name = routes_service.grouped_segments()
  latest_name = latest_route_name(routes_by_name)
  for name, segment_dirs in routes_by_name.items():
    try:
      stats.append(cache.stats_for(name, segment_dirs, include_path=name == latest_name))
    except OSError as error:
      print(f"Skipping unreadable route {name}: {error}")

  return stats


def latest_route_name(routes_by_name):
  if not routes_by_name:
    return None

  name, _ = max(routes_by_name.items(), key=lambda item: (route_epoch(item[1][0]) or 0, item[0]))
  return name


# --- assembly (pure: turns RouteStats into the frontend's shapes) -----------------------------------

def last_drive(all_stats, is_metric):
  if not all_stats:
    return None

  stats = all_stats[-1]
  return {
    "avgSpeed": round(speed_value(stats.avg_speed_ms, is_metric), 0),
    "distance": round(distance_value(stats.distance_m, is_metric), 1),
    "distanceUnit": distance_unit(is_metric),
    "durationMin": round(stats.duration_s / 60),
    "engagement": round(stats.engagement),
    "path": stats.path,
    "segments": stats.segments,
    "speedUnit": speed_unit(is_metric),
    "when": friendly_when(stats),
  }


def recent_drives(all_stats, is_metric):
  drives = []
  for stats in reversed(all_stats[-RECENT_DRIVES_LIMIT:]):
    drives.append({
      "distance": round(distance_value(stats.distance_m, is_metric), 1),
      "distanceUnit": distance_unit(is_metric),
      "duration": format_duration(stats.duration_s),
      "engagement": round(stats.engagement),
      "id": stats.name,
      "segments": stats.segments,
      "when": friendly_when(stats),
    })

  return drives


def records(all_stats, is_metric):
  driven = [stats for stats in all_stats if stats.distance_m > 0 and stats.epoch]
  if not driven:
    return {}

  by_day = group_by_day(driven)
  by_week = group_by_week(driven)

  longest = max(driven, key=lambda stats: stats.distance_m)
  most_engaged = max(by_day.values(), key=lambda day: day_engagement(day))
  best_week = max(by_week.values(), key=lambda week: sum(stats.distance_m for stats in week))

  unit = distance_unit(is_metric)
  return {
    "bestWeek": {
      "value": f"{round(distance_value(sum(stats.distance_m for stats in best_week), is_metric))} {unit}",
      "detail": week_label(best_week[0].epoch),
    },
    "highestStreak": {
      "value": streak_label(highest_streak(by_day)),
      "detail": None,
    },
    "longestDrive": {
      "value": f"{round(distance_value(longest.distance_m, is_metric), 1)} {unit}",
      "detail": day_label(longest.epoch),
    },
    "mostEngagedDay": {
      "value": f"{round(day_engagement(most_engaged))}%",
      "detail": day_label(most_engaged[0].epoch),
    },
  }


def this_week(all_stats, is_metric, now):
  start = (now - timedelta(days=WEEK_DAYS - 1)).date()
  week_stats = [stats for stats in all_stats if stats.epoch and datetime.fromtimestamp(stats.epoch).date() >= start]

  per_day = []
  today = now.date()
  for offset in range(WEEK_DAYS):
    day = start + timedelta(days=offset)
    day_stats = [stats for stats in week_stats if datetime.fromtimestamp(stats.epoch).date() == day]
    per_day.append({
      "label": day.strftime("%a"),
      "miles": round(distance_value(sum(stats.distance_m for stats in day_stats), is_metric), 1),
      "today": day == today,
    })

  if not week_stats:
    return {"perDay": per_day, "engagement": 0, "totals": None}

  total_samples = sum(stats.total_samples for stats in week_stats)
  enabled_samples = sum(stats.enabled_samples for stats in week_stats)
  return {
    "engagement": round(100.0 * enabled_samples / total_samples) if total_samples else 0,
    "perDay": per_day,
    "totals": {
      "distance": round(distance_value(sum(stats.distance_m for stats in week_stats), is_metric), 1),
      "distanceUnit": distance_unit(is_metric),
      "drives": len(week_stats),
      "hours": round(sum(stats.duration_s for stats in week_stats) / 3600, 1),
    },
  }


# --- per-route reconstruction (the heavy, cached half) ----------------------------------------------

class DriveStatsCache:
  def __init__(self):
    self.cache_path = DRIVE_STATS_CACHE_PATH
    self.cache_path.mkdir(parents=True, exist_ok=True)

  def stats_for(self, name, segment_dirs, include_path=False):
    source_mtime = max((segment_dir.stat().st_mtime for segment_dir in segment_dirs), default=0)
    cached = self.read(name, source_mtime)
    if cached is not None and (not include_path or cached.path_attempted):
      return cached

    stats = reconstruct_route_stats(name, segment_dirs, include_path=include_path)
    self.write(name, stats)
    return stats

  def path_for(self, name):
    return self.cache_path / f"{name}.json"

  def read(self, name, source_mtime):
    path = self.path_for(name)
    if not path.is_file() or path.stat().st_mtime < source_mtime:
      return None

    try:
      return RouteStats(**json.loads(path.read_text()))
    except (TypeError, ValueError):
      # A cache file written by an older shape (or a partial write) is simply recomputed.
      return None

  def write(self, name, stats):
    # Write to a unique temp then atomically replace so a concurrent writer for the same route never
    # exposes a torn cache file.
    path = self.path_for(name)
    staged_path = path.with_name(f"{name}.{uuid4().hex}.partial.json")
    staged_path.write_text(json.dumps(asdict(stats)))
    staged_path.replace(path)


def reconstruct_route_stats(name, segment_dirs, include_path=False):
  # Distance and engagement come from one qlog pass per segment; duration comes from the qcamera track
  # via routes_service (the same probe the routes page uses), because qlog logMonoTime is cumulative over
  # the whole drive, not per-segment, so a min/max span over it would over-count badly. Segments are
  # tolerated individually: a partial or message-sparse qlog contributes whatever it has rather than
  # failing the route (verified on-device: some segments log no carState at all).
  distance_m = duration_s = 0.0
  enabled_samples = total_samples = 0
  route_points = []
  route_path_source = None

  for segment_dir in segment_dirs:
    if (segment_dir / RECORDING_LOCK_NAME).exists():
      continue

    duration_s += routes_service.segment_duration(segment_dir)
    segment = parse_segment_qlog(segment_dir / "qlog", include_path=include_path)
    distance_m += segment["distance_m"]
    enabled_samples += segment["enabled_samples"]
    total_samples += segment["total_samples"]
    if include_path and segment["path_points"]:
      route_points.extend(segment["path_points"])
      route_path_source = route_path_source or segment["path_source"]

  if include_path and len(route_points) < 2:
    route_points, route_path_source = route_path_from_rlogs(segment_dirs)

  return RouteStats(
    date=routes_service.route_date(segment_dirs[0]),
    distance_m=distance_m,
    duration_s=duration_s,
    enabled_samples=enabled_samples,
    epoch=route_epoch(segment_dirs[0]),
    name=name,
    path=build_route_path(route_points, route_path_source) if include_path else None,
    path_attempted=include_path,
    segments=len(segment_dirs),
    total_samples=total_samples,
  )


def parse_segment_qlog(qlog_path, include_path=False):
  # Distance is the integral of vEgo over the gaps between consecutive carState samples (each ~0.1s,
  # capped so a logging hiccup cannot inflate it); engagement is the fraction of controlsState samples
  # that were enabled. Duration is NOT taken here — see reconstruct_route_stats.
  empty = {"distance_m": 0.0, "enabled_samples": 0, "path_points": [], "path_source": None, "total_samples": 0}
  if not qlog_path.is_file():
    return empty

  # Imported lazily so the rest of the module (and its assembly half) does not pull in capnp until a
  # real parse happens.
  from openpilot.tools.lib.logreader import LogReader

  distance_m = 0.0
  enabled_samples = total_samples = 0
  path_candidates = {source: [] for source in LOCATION_SOURCE_PRIORITY}
  previous_mono = None

  try:
    for message in LogReader(str(qlog_path)):
      which = message.which()
      if which == "carState":
        mono = message.logMonoTime
        if previous_mono is not None:
          gap = min((mono - previous_mono) / NANOS_PER_SECOND, MAX_SAMPLE_GAP_SECONDS)
          distance_m += max(message.carState.vEgo, 0.0) * max(gap, 0.0)
        previous_mono = mono
      elif which == "controlsState":
        total_samples += 1
        if message.controlsState.enabled:
          enabled_samples += 1
      elif include_path and which in path_candidates:
        point = location_point_from_message(message, which)
        if point:
          path_candidates[which].append((message.logMonoTime, point[0], point[1]))
  except Exception as error:
    # A truncated/corrupt qlog yields what was read before the failure rather than sinking the route.
    print(f"Partial qlog {qlog_path}: {error}")

  path_points, path_source = preferred_location_points(path_candidates)

  return {
    "distance_m": distance_m,
    "enabled_samples": enabled_samples,
    "path_points": path_points,
    "path_source": path_source,
    "total_samples": total_samples,
  }


def route_path_from_rlogs(segment_dirs):
  route_points = []
  route_path_source = None

  for segment_dir in segment_dirs:
    if (segment_dir / RECORDING_LOCK_NAME).exists():
      continue

    points, source = parse_path_log(segment_dir / "rlog")
    if points:
      route_points.extend(points)
      route_path_source = route_path_source or source

  return route_points, route_path_source


def parse_path_log(log_path):
  if not log_path.is_file():
    return [], None

  from openpilot.tools.lib.logreader import LogReader

  path_candidates = {source: [] for source in LOCATION_SOURCE_PRIORITY}
  try:
    for message in LogReader(str(log_path)):
      which = message.which()
      if which not in path_candidates:
        continue

      point = location_point_from_message(message, which)
      if point:
        path_candidates[which].append((message.logMonoTime, point[0], point[1]))
  except Exception as error:
    print(f"Partial path log {log_path}: {error}")

  return preferred_location_points(path_candidates)


def location_point_from_message(message, which):
  if which in ("gpsLocationExternal", "gpsLocation"):
    location = getattr(message, which)
    if not location.hasFix:
      return None

    accuracy = finite_float(getattr(location, "horizontalAccuracy", None))
    if accuracy and accuracy > 100:
      return None

    return valid_lat_lon(location.latitude, location.longitude)

  if which == "liveLocationKalman":
    location = message.liveLocationKalman
    if not location.gpsOK or str(location.status) != "valid":
      return None

    position = location.positionGeodetic
    if not position.valid or len(position.value) < 2:
      return None

    return valid_lat_lon(position.value[0], position.value[1])

  return None


def preferred_location_points(path_candidates):
  for source in LOCATION_SOURCE_PRIORITY:
    points = sorted(path_candidates[source], key=lambda point: point[0])
    if len(points) >= 2:
      return [(lat, lon) for _, lat, lon in points], source

  return [], None


def valid_lat_lon(latitude, longitude):
  lat = finite_float(latitude)
  lon = finite_float(longitude)
  if lat is None or lon is None:
    return None
  if not -90 <= lat <= 90 or not -180 <= lon <= 180:
    return None
  if abs(lat) < 0.000001 and abs(lon) < 0.000001:
    return None

  return lat, lon


def finite_float(value):
  try:
    parsed = float(value)
  except (TypeError, ValueError):
    return None

  return parsed if math.isfinite(parsed) else None


def build_route_path(points, source):
  cleaned = filter_route_points(points)
  normalized = normalize_route_points(sample_route_points(cleaned))
  if not normalized:
    return None

  return {
    "pointCount": len(cleaned),
    "points": normalized,
    "source": source or "unknown",
  }


def filter_route_points(points):
  filtered = []
  for point in points:
    if valid_lat_lon(point[0], point[1]) is None:
      continue
    if filtered:
      gap = distance_between_points(filtered[-1], point)
      if gap < MIN_ROUTE_POINT_DISTANCE_M or gap > MAX_ROUTE_POINT_JUMP_M:
        continue

    filtered.append((float(point[0]), float(point[1])))

  return filtered


def sample_route_points(points):
  if len(points) <= MAX_ROUTE_PATH_POINTS:
    return points

  max_index = len(points) - 1
  return [points[round(index * max_index / (MAX_ROUTE_PATH_POINTS - 1))] for index in range(MAX_ROUTE_PATH_POINTS)]


def normalize_route_points(points):
  if len(points) < 2:
    return None

  origin_lat = sum(point[0] for point in points) / len(points)
  origin_lon = points[0][1]
  mean_lat = math.radians(origin_lat)
  projected = [
    (
      (lon - origin_lon) * EARTH_METERS_PER_DEGREE * math.cos(mean_lat),
      (lat - origin_lat) * EARTH_METERS_PER_DEGREE,
    )
    for lat, lon in points
  ]

  xs = [point[0] for point in projected]
  ys = [point[1] for point in projected]
  min_x, max_x = min(xs), max(xs)
  min_y, max_y = min(ys), max(ys)
  span_x = max_x - min_x
  span_y = max_y - min_y
  if span_x < 1 and span_y < 1:
    return None

  padding = 0.08
  scale = 1 - (padding * 2)
  normalized = []
  for x, y in projected:
    mapped_x = 0.5 if span_x < 1 else padding + ((x - min_x) / span_x * scale)
    mapped_y = 0.5 if span_y < 1 else padding + ((max_y - y) / span_y * scale)
    normalized.append([round(mapped_x, 4), round(mapped_y, 4)])

  return normalized


def distance_between_points(first, second):
  mean_lat = math.radians((first[0] + second[0]) / 2)
  dx = (second[1] - first[1]) * EARTH_METERS_PER_DEGREE * math.cos(mean_lat)
  dy = (second[0] - first[0]) * EARTH_METERS_PER_DEGREE
  return math.hypot(dx, dy)


# --- small helpers ----------------------------------------------------------------------------------

def day_engagement(day_stats):
  total = sum(stats.total_samples for stats in day_stats)
  return (100.0 * sum(stats.enabled_samples for stats in day_stats) / total) if total else 0.0


def day_key(epoch):
  return datetime.fromtimestamp(epoch).date()


def day_label(epoch):
  if not epoch:
    return None

  # Built without strftime's "%-d" because that flag is glibc-only and raises on the PC/debug host.
  moment = datetime.fromtimestamp(epoch)
  return f"{moment.strftime('%b')} {moment.day}"


def distance_unit(is_metric):
  return "kilometers" if is_metric else "miles"


def distance_value(meters, is_metric):
  return meters * (0.001 if is_metric else CV.METER_TO_MILE)


def format_duration(seconds):
  # Rounded to the nearest minute so it agrees with last_drive's durationMin for the same route.
  minutes = round(seconds / 60)
  if minutes < 60:
    return f"{minutes}m"

  return f"{minutes // 60}h {minutes % 60:02d}m"


def friendly_when(stats):
  # A user-set custom route name wins; otherwise the recorded start time in a short, locale-free form
  # ("%-I"/"%-d" are glibc-only and raise on the PC/debug host, so the 12-hour clock is built by hand).
  if stats.epoch is None:
    return stats.date

  moment = datetime.fromtimestamp(stats.epoch)
  if stats.date and stats.date != moment.isoformat():
    return stats.date

  hour = moment.hour % 12 or 12
  return f"{moment.strftime('%b')} {moment.day}, {hour}:{moment.minute:02d} {moment.strftime('%p')}"


def group_by_day(driven):
  days = {}
  for stats in driven:
    days.setdefault(day_key(stats.epoch), []).append(stats)

  return days


def group_by_week(driven):
  weeks = {}
  for stats in driven:
    iso = datetime.fromtimestamp(stats.epoch).isocalendar()
    weeks.setdefault((iso[0], iso[1]), []).append(stats)

  return weeks


def highest_streak(by_day):
  # Longest run of consecutive calendar days that each have at least one drive, over retained footage.
  days = sorted(by_day)
  best = run = 0
  previous = None
  for day in days:
    run = run + 1 if previous is not None and (day - previous).days == 1 else 1
    best = max(best, run)
    previous = day

  return best


def metric():
  from openpilot.frogpilot.common.frogpilot_variables import params
  try:
    return params.get_bool("IsMetric")
  except Exception:
    return False


def route_epoch(segment_dir):
  rlog_path = segment_dir / "rlog"
  if rlog_path.exists():
    return rlog_path.stat().st_ctime

  return None


def speed_unit(is_metric):
  return "km/h" if is_metric else "mph"


def speed_value(speed_ms, is_metric):
  return speed_ms * (CV.MS_TO_KPH if is_metric else CV.MS_TO_MPH)


def streak_label(days):
  return f"{days} day" if days == 1 else f"{days} days"


def week_label(epoch):
  if not epoch:
    return None

  start = day_key(epoch) - timedelta(days=datetime.fromtimestamp(epoch).weekday())
  return f"week of {start.strftime('%b')} {start.day}"

import { useCallback, useEffect, useState } from "preact/hooks";

import { Icon } from "../../components/Icon.js";
import { html } from "../../lib/html.js";
import { http } from "../../lib/http.js";
import { strings } from "../../lib/strings.js";

const AUTO_REFRESH_MS = 120000;
const ROUTE_HISTORY_PATH = "/dashcam_routes";
const SOURCE_DEFS = [
  { key: "stats", label: "Stats", url: "/api/stats" },
  { key: "update", label: "Update", url: "/api/update/fast/status" },
  { key: "vehicle", label: "Vehicle", url: "/api/car_features_check" },
];
const SOURCE_LABELS = Object.fromEntries(SOURCE_DEFS.map((source) => [source.key, source.label]));
const VEHICLE_LOGO_LABELS = {
  acura: "Acura",
  audi: "Audi",
  buick: "Buick",
  cadillac: "Cadillac",
  chevrolet: "Chevrolet",
  chrysler: "Chrysler",
  dodge: "Dodge",
  ford: "Ford",
  genesis: "Genesis",
  gm: "GM",
  gmc: "GMC",
  holden: "Holden",
  honda: "Honda",
  hyundai: "Hyundai",
  infiniti: "Infiniti",
  jeep: "Jeep",
  kia: "Kia",
  lexus: "Lexus",
  man: "MAN",
  mazda: "Mazda",
  nissan: "Nissan",
  ram: "Ram",
  seat: "SEAT",
  skoda: "Skoda",
  subaru: "Subaru",
  tesla: "Tesla",
  toyota: "Toyota",
  volkswagen: "Volkswagen",
};
const VEHICLE_LOGO_FILES = {
  acura: "acura.svg",
  audi: "audi.svg",
  buick: "buick.png",
  cadillac: "cadillac.svg",
  chevrolet: "chevrolet.svg",
  chrysler: "chrysler.svg",
  dodge: "dodge.png",
  ford: "ford.svg",
  genesis: "genesis.png",
  gm: "generalmotors.svg",
  gmc: "gmc.png",
  holden: "holden.png",
  honda: "honda.svg",
  hyundai: "hyundai.svg",
  infiniti: "infiniti.svg",
  jeep: "jeep.svg",
  kia: "kia.svg",
  lexus: "lexus.svg",
  man: "man.svg",
  mazda: "mazda.svg",
  nissan: "nissan.svg",
  ram: "ram.svg",
  seat: "seat.svg",
  skoda: "skoda.svg",
  subaru: "subaru.svg",
  tesla: "tesla.svg",
  toyota: "toyota.svg",
  volkswagen: "volkswagen.svg",
};
const VEHICLE_LOGO_PREFIXES = [
  ["ACURA_", "acura"],
  ["AUDI_", "audi"],
  ["BUICK_", "buick"],
  ["CADILLAC_", "cadillac"],
  ["CHEVROLET_", "chevrolet"],
  ["CHRYSLER_", "chrysler"],
  ["DODGE_", "dodge"],
  ["FORD_", "ford"],
  ["GENESIS_", "genesis"],
  ["GMC_", "gmc"],
  ["HOLDEN_", "holden"],
  ["HONDA_", "honda"],
  ["HYUNDAI_", "hyundai"],
  ["INFINITI_", "infiniti"],
  ["JEEP_", "jeep"],
  ["KIA_", "kia"],
  ["LEXUS_", "lexus"],
  ["MAN_", "man"],
  ["MAZDA_", "mazda"],
  ["NISSAN_", "nissan"],
  ["RAM_", "ram"],
  ["SEAT_", "seat"],
  ["SKODA_", "skoda"],
  ["SUBARU_", "subaru"],
  ["TESLA_", "tesla"],
  ["TOYOTA_", "toyota"],
  ["VOLKSWAGEN_", "volkswagen"],
];
const VEHICLE_LOGO_ROOT = "/assets/images/vehicle_logos";
const FROGPILOT_MARK = "/assets/images/frogpilot_frog.png";
const OPENPILOT_MARK = "/assets/images/openpilot.png";

let cachedHomeSnapshot = null;

function sourceHadProblem(snapshot, keys) {
  return keys.some((key) => {
    const state = snapshot?.sources?.[key]?.state;
    return state === "failed" || state === "stale";
  });
}

function finiteNumber(value) {
  return typeof value === "number" && Number.isFinite(value) ? value : 0;
}

function rounded(value) {
  return Math.round(finiteNumber(value)).toLocaleString("en-US");
}

function decimal(value) {
  return finiteNumber(value).toLocaleString("en-US", { maximumFractionDigits: 1 });
}

function valueOrUnknown(value) {
  return value === null || value === undefined || value === "" ? strings.home.unknown : value;
}

function formatPercent(value) {
  if (typeof value === "number" && Number.isFinite(value)) {
    return `${decimal(value)}%`;
  }
  if (typeof value === "string" && value.trim()) {
    const parsed = Number.parseFloat(value.replace("%", ""));
    if (Number.isFinite(parsed)) {
      return `${decimal(parsed)}%`;
    }
    return value;
  }
  return strings.home.unknown;
}

// Display-only: render a bare Celsius reading ("54C") with a degree symbol ("54°C") to match
// mockup A. Leaves the underlying value untouched and passes through anything not in that shape.
function formatTemp(value) {
  if (typeof value === "string" && /^\s*-?\d+(\.\d+)?\s*C\s*$/i.test(value)) {
    return value.trim().replace(/\s*C$/i, "°C");
  }
  return value;
}

function percentNumber(value) {
  const parsed = Number.parseFloat(String(value ?? "").replace("%", ""));
  if (!Number.isFinite(parsed)) {
    return 0;
  }
  return Math.max(0, Math.min(100, parsed));
}

function formatRefreshTime(value) {
  if (!value) {
    return strings.home.unknown;
  }
  return new Date(value).toLocaleTimeString("en-US", { hour: "numeric", minute: "2-digit" });
}

function formatPlatform(value) {
  return (value || "")
    .split("_")
    .filter(Boolean)
    .map((part) => part.charAt(0).toUpperCase() + part.slice(1).toLowerCase())
    .join(" ");
}

function titleCase(value) {
  return (value || "")
    .split(/\s+/)
    .filter(Boolean)
    .map((part) => part.charAt(0).toUpperCase() + part.slice(1).toLowerCase())
    .join(" ");
}

function normalizedVehicleMake(value) {
  return (value || "")
    .normalize("NFD")
    .replace(/[\u0300-\u036f]/g, "")
    .toLowerCase()
    .replace(/[^a-z0-9]/g, "");
}

function vehicleLogoMake(vehicle) {
  const fingerprint = (vehicle?.carFingerprint || "").toUpperCase();

  for (const [prefix, make] of VEHICLE_LOGO_PREFIXES) {
    if (fingerprint.startsWith(prefix)) {
      return make;
    }
  }

  const make = normalizedVehicleMake(vehicle?.carMake);
  const aliases = {
    acura: "acura",
    audi: "audi",
    buick: "buick",
    cadillac: "cadillac",
    chevrolet: "chevrolet",
    chrysler: "chrysler",
    dodge: "dodge",
    ford: "ford",
    genesis: "genesis",
    generalmotors: "gm",
    gm: "gm",
    gmc: "gmc",
    holden: "holden",
    honda: "honda",
    hyundai: "hyundai",
    infiniti: "infiniti",
    jeep: "jeep",
    kia: "kia",
    lexus: "lexus",
    man: "man",
    mazda: "mazda",
    nissan: "nissan",
    ram: "ram",
    seat: "seat",
    skoda: "skoda",
    subaru: "subaru",
    tesla: "tesla",
    toyota: "toyota",
    volkswagen: "volkswagen",
    vw: "volkswagen",
  };

  return aliases[make] || make;
}

function vehicleLogoLabel(make, vehicle) {
  const label = VEHICLE_LOGO_LABELS[make] || titleCase(vehicle?.carMake);
  return label ? `${label} logo` : strings.home.vehicle;
}

function vehicleLabel(vehicle) {
  if (!vehicle) {
    return strings.home.unknownVehicle;
  }
  if (vehicle.carModelName) {
    return vehicle.carModelName;
  }
  if (vehicle.carMake && vehicle.carFingerprint) {
    return `${titleCase(vehicle.carMake)} ${formatPlatform(vehicle.carFingerprint)}`;
  }
  if (vehicle.carFingerprint) {
    return formatPlatform(vehicle.carFingerprint);
  }
  if (vehicle.carMake) {
    return titleCase(vehicle.carMake);
  }
  return strings.home.unknownVehicle;
}

// When the make logo is shown, drop a leading make word from the visible name ("Toyota RAV4
// Hybrid" -> "RAV4 Hybrid") to match mockup A; the make stays in the logo alt + the cell aria-label.
function stripLeadingMake(name, make) {
  const label = VEHICLE_LOGO_LABELS[make];
  if (!name || !label) {
    return name;
  }
  const stripped = name.replace(new RegExp(`^${label}\\s+`, "i"), "");
  return stripped || name;
}

function VehicleLogo({ label, make }) {
  const fileName = VEHICLE_LOGO_FILES[make];
  if (fileName) {
    return html`
      <img
        class=${`home-vehicle-logo home-vehicle-logo-image home-vehicle-logo-${make}`}
        src=${`${VEHICLE_LOGO_ROOT}/${fileName}`}
        alt=${label}
        decoding="async"
        loading="eager"
      />
    `;
  }

  const fallback = (VEHICLE_LOGO_LABELS[make] || make || "").slice(0, 3).toUpperCase();
  if (fallback) {
    return html`
      <span class="home-vehicle-logo home-vehicle-logo-fallback" aria-label=${label} role="img">${fallback}</span>
    `;
  }

  return html`<${Icon} name="car" size=${38} />`;
}

function updateAvailableFact(snapshot) {
  const update = snapshot?.raw?.update;
  const source = snapshot?.sources?.update;
  if (!snapshot || !source || (!update && source.state === "failed")) {
    return {
      detail: source?.state === "failed" ? strings.home.updateUnknownDetail : strings.home.updateCheckingDetail,
      tone: "unknown",
      value: source?.state === "failed" ? strings.home.unknown : strings.home.updateChecking,
    };
  }

  if (!update) {
    return {
      detail: strings.home.updateUnknownDetail,
      tone: "unknown",
      value: strings.home.unknown,
    };
  }

  const staleDetail = source.state === "stale" ? strings.home.updateLastKnownDetail : null;
  const updaterState = update?.updaterState ?? "";
  if (updaterState && updaterState !== "idle") {
    return {
      detail: staleDetail || strings.home.updateBusyDetail,
      tone: "busy",
      value: strings.home.updaterActive,
    };
  }

  if (update?.updateAvailable || update?.fetchAvailable) {
    return {
      detail: staleDetail || strings.home.updateReadyDetail,
      tone: "available",
      value: strings.home.updateReady,
    };
  }

  return {
    detail: staleDetail || strings.home.updateCurrentDetail,
    tone: "ok",
    value: strings.home.updateUpToDate,
  };
}

function buildSourceState(results, previous) {
  const raw = {};
  const sources = {};
  let hasFreshData = false;
  let refreshFailed = false;

  for (const source of SOURCE_DEFS) {
    const result = results.find((entry) => entry.key === source.key);
    const previousValue = previous?.raw?.[source.key] ?? null;
    if (result?.ok) {
      raw[source.key] = result.data;
      sources[source.key] = { error: null, label: source.label, state: "fresh" };
      hasFreshData = true;
      continue;
    }

    refreshFailed = true;
    raw[source.key] = previousValue;
    sources[source.key] = {
      error: result?.error || strings.home.sourceFailed(source.label),
      label: source.label,
      state: previousValue ? "stale" : "failed",
    };
  }

  return {
    raw,
    refreshFailed,
    sources,
    updatedAt: hasFreshData ? Date.now() : (previous?.updatedAt ?? null),
  };
}

async function fetchSource(source, signal) {
  try {
    return { data: await http.get(source.url, { signal }), key: source.key, ok: true };
  } catch (error) {
    return { error: error?.message || strings.home.sourceFailed(source.label), key: source.key, ok: false };
  }
}

async function fetchHomeSnapshot(previous, signal) {
  const results = await Promise.all(SOURCE_DEFS.map((source) => fetchSource(source, signal)));
  return buildSourceState(results, previous);
}

function sourceNotes(snapshot, keys) {
  return keys
    .map((key) => {
      const source = snapshot?.sources?.[key];
      if (!source || source.state === "fresh") {
        return null;
      }
      const label = SOURCE_LABELS[key] || key;
      return source.state === "stale" ? `${label}: ${strings.home.staleSource}` : strings.home.sourceFailed(label);
    })
    .filter(Boolean);
}

function Widget({ action, children, className = "", icon, notes = [], title }) {
  return html`
    <section class=${`home-widget ${className}`}>
      <header class="home-widget-header">
        <div class="home-widget-heading">
          ${icon && html`<span class="home-widget-icon"><${Icon} name=${icon} size=${18} /></span>`}
          <h2 class="home-widget-title">${title}</h2>
        </div>
        ${action && html`<a class="home-action" href=${action.href}>${action.label}</a>`}
      </header>
      ${notes.length > 0 && html`<${SourceNotes} notes=${notes} />`}
      ${children}
    </section>
  `;
}

function SourceNotes({ notes }) {
  return html`
    <ul class="home-source-notes" role="status">
      ${notes.map((note) => html`<li key=${note}>${note}</li>`)}
    </ul>
  `;
}

function Fact({ label, value }) {
  return html`
    <div class="home-fact">
      <dt>${label}</dt>
      <dd>${valueOrUnknown(value)}</dd>
    </div>
  `;
}

function glanceModel(snapshot) {
  const stats = snapshot?.raw?.stats ?? {};
  const vehicle = snapshot?.raw?.vehicle;
  const software = stats.softwareInfo ?? {};
  const vitals = stats.vitals ?? {};
  const update = snapshot?.raw?.update ?? {};
  const branch = software.branchName || update.currentBranch;
  const updateFact = updateAvailableFact(snapshot);
  const logoMake = vehicleLogoMake(vehicle);
  const vehicleName = vehicleLabel(vehicle);

  return {
    branch,
    cpuTemp: vitals.cpuTemp,
    maintainer: software.forkMaintainer,
    network: vitals.network,
    notes: sourceNotes(snapshot, ["stats", "vehicle", "update"]),
    repoName: software.repoName,
    repoUrl: software.repoUrl,
    updateFact,
    uptime: vitals.uptime,
    vehicleLogoLabel: vehicleLogoLabel(logoMake, vehicle),
    vehicleLogoMake: logoMake,
    vehicleName,
    // Visible name drops the make (carried by the logo); full name stays for the aria-label.
    vehicleNameDisplay: stripLeadingMake(vehicleName, logoMake),
  };
}

// GitHub usernames are 1-39 chars of [A-Za-z0-9] with non-leading/trailing single hyphens; reject
// anything else so a malformed/untrusted maintainer value can never build an arbitrary external URL.
const GITHUB_USERNAME_RE = /^[a-z\d](?:[a-z\d]|-(?=[a-z\d])){0,38}$/i;
const GITHUB_REPO_RE = /^[a-z\d._-]+$/i;

function validGithubUsername(username) {
  return typeof username === "string" && GITHUB_USERNAME_RE.test(username.trim());
}

function validGithubRepoName(repo) {
  return typeof repo === "string" && GITHUB_REPO_RE.test(repo.trim());
}

function githubAvatarUrl(username) {
  return `https://github.com/${encodeURIComponent(username.trim())}.png?size=80`;
}

function githubProfileUrl(username) {
  return validGithubUsername(username) ? `https://github.com/${encodeURIComponent(username.trim())}` : "";
}

function normalizedGithubRepoUrl(url) {
  const value = typeof url === "string" ? url.trim() : "";
  const match = /^https:\/\/github\.com\/([^/?#]+)\/([^/?#]+)\/?$/i.exec(value);
  if (!match || !validGithubUsername(match[1]) || !validGithubRepoName(match[2])) {
    return "";
  }
  return `https://github.com/${encodeURIComponent(match[1])}/${encodeURIComponent(match[2])}`;
}

function githubRepoUrl(owner, repo) {
  if (!validGithubUsername(owner) || !validGithubRepoName(repo)) {
    return "";
  }
  return `https://github.com/${encodeURIComponent(owner.trim())}/${encodeURIComponent(repo.trim())}`;
}

function githubBranchUrl({ branch, owner, repo, repoUrl }) {
  const branchName = typeof branch === "string" ? branch.trim() : "";
  if (!branchName) {
    return "";
  }

  const baseUrl = normalizedGithubRepoUrl(repoUrl) || githubRepoUrl(owner, repo || "FrogPilot");
  return baseUrl ? `${baseUrl}/tree/${encodeURIComponent(branchName)}` : "";
}

function pluralized(value, singular, plural = `${singular}s`) {
  return `${value} ${value === 1 ? singular : plural}`;
}

function formatUptimeDisplay(value) {
  const text = typeof value === "string" ? value.trim() : "";
  if (!text) {
    return `${strings.home.uptime}: ${strings.home.unknown}`;
  }

  const days = Number.parseInt(/(\d+)\s*d/i.exec(text)?.[1] ?? "", 10);
  const hours = Number.parseInt(/(\d+)\s*h/i.exec(text)?.[1] ?? "", 10);
  const minutes = Number.parseInt(/(\d+)\s*m/i.exec(text)?.[1] ?? "", 10);
  const parts = [];

  if (Number.isFinite(days)) {
    parts.push(pluralized(days, "day"));
  }
  if (Number.isFinite(hours) && (!Number.isFinite(days) || hours > 0)) {
    parts.push(pluralized(hours, "hour"));
  }
  if (!Number.isFinite(days) && Number.isFinite(minutes)) {
    parts.push(pluralized(minutes, "minute"));
  }

  return `${strings.home.uptime}: ${parts.length > 0 ? parts.join(" ") : text}`;
}

// Real GitHub profile image for the maintainer, with a neutral person-glyph fallback when the
// username is unknown/invalid or the image fails to load — so the rail never breaks layout.
function MaintainerAvatar({ username }) {
  const [failed, setFailed] = useState(false);
  const valid = validGithubUsername(username);

  useEffect(() => {
    setFailed(false);
  }, [username]);

  if (!valid || failed) {
    return html`<span class="home-badge-avatar home-badge-avatar-fallback"><${Icon} name="user" size=${16} /></span>`;
  }

  return html`
    <img
      class="home-badge-avatar"
      src=${githubAvatarUrl(username)}
      alt=""
      decoding="async"
      loading="lazy"
      referrerpolicy="no-referrer"
      onError=${() => setFailed(true)}
    />
  `;
}

function BadgeStat({ ariaLabel, className = "", icon, label, tone = "neutral", value }) {
  const display = valueOrUnknown(value);

  return html`
    <div class=${["home-badge-cell", "home-badge-stat", `home-badge-cell-${tone}`, className].filter(Boolean).join(" ")} aria-label=${ariaLabel || `${label}: ${display}`}>
      <span class="home-badge-icon"><${Icon} name=${icon} size=${18} /></span>
      <span class="home-badge-value">${display}</span>
    </div>
  `;
}

function BadgeRail({ model }) {
  const branch = valueOrUnknown(model.branch);
  const maintainer = valueOrUnknown(model.maintainer);
  const branchUrl = githubBranchUrl({
    branch: model.branch,
    owner: model.maintainer,
    repo: model.repoName,
    repoUrl: model.repoUrl,
  });
  const maintainerUrl = githubProfileUrl(model.maintainer);
  const uptime = formatUptimeDisplay(model.uptime);

  return html`
    <section class="home-badge" aria-label=${strings.home.atAGlance}>
      <div class="home-badge-cell home-badge-identity" aria-label=${`${strings.home.vehicle}: ${model.vehicleName}`}>
        <span class="home-badge-logo">
          <${VehicleLogo} make=${model.vehicleLogoMake} label=${model.vehicleLogoLabel} />
        </span>
        <span class="home-badge-vehicle">${model.vehicleNameDisplay}</span>
      </div>

      <div class="home-badge-cell home-badge-stat home-badge-cell-accent" aria-label=${`${strings.home.branch}: ${branch}`}>
        ${branchUrl
          ? html`
              <a class="home-badge-link home-badge-link-content" href=${branchUrl} target="_blank" rel="noreferrer">
                <span class="home-badge-icon"><${Icon} name="branch" size=${18} /></span>
                <span class="home-badge-value">${branch}</span>
              </a>
            `
          : html`
              <span class="home-badge-link-content">
                <span class="home-badge-icon"><${Icon} name="branch" size=${18} /></span>
                <span class="home-badge-value">${branch}</span>
              </span>
            `}
      </div>

      <div class="home-badge-cell home-badge-stat home-badge-cell-accent" aria-label=${`${strings.home.maintainer}: ${maintainer}`}>
        ${maintainerUrl
          ? html`
              <a class="home-badge-link home-badge-link-content" href=${maintainerUrl} target="_blank" rel="noreferrer">
                <${MaintainerAvatar} username=${model.maintainer} />
                <span class="home-badge-value">${maintainer}</span>
              </a>
            `
          : html`
              <span class="home-badge-link-content">
                <${MaintainerAvatar} username=${model.maintainer} />
                <span class="home-badge-value">${maintainer}</span>
              </span>
            `}
      </div>

      <${BadgeStat}
        className="home-badge-uptime"
        icon="clock"
        label=${strings.home.uptime}
        value=${uptime}
        ariaLabel=${uptime}
        tone=${model.uptime ? "neutral" : "unknown"}
      />
      <${BadgeStat} icon="wifi" label=${strings.home.network} value=${model.network} tone=${model.network ? "neutral" : "unknown"} />
      <${BadgeStat} icon="thermometer" label=${strings.home.cpuTemp} value=${formatTemp(model.cpuTemp)} tone=${model.cpuTemp ? "info" : "unknown"} />
      <${BadgeStat}
        icon=${model.updateFact.tone === "available" ? "download" : "check-circle"}
        label=${strings.home.updateStatus}
        value=${model.updateFact.value}
        tone=${model.updateFact.tone}
      />
    </section>
  `;
}

function AtAGlance({ snapshot }) {
  const model = glanceModel(snapshot);

  return html`
    <div class="home-glance-stack" aria-label=${strings.home.atAGlance}>
      ${model.notes.length > 0 && html`<${SourceNotes} notes=${model.notes} />`}
      <${BadgeRail} model=${model} />
    </div>
  `;
}

function StorageFootage({ snapshot }) {
  const stats = snapshot?.raw?.stats ?? {};
  const disk = Array.isArray(stats.diskUsage) ? stats.diskUsage[0] : null;
  const roots = Array.isArray(stats.footageUsage) ? stats.footageUsage : [];
  const segments = stats.firehoseStats?.segments;
  const usedPercent = percentNumber(disk?.usedPercentage);

  return html`
    <${Widget}
      title=${strings.home.storageFootage}
      icon="video"
      className="home-storage"
      action=${{ href: ROUTE_HISTORY_PATH, label: strings.home.reviewFootage }}
      notes=${sourceNotes(snapshot, ["stats"])}
    >
      <div class="home-storage-capacity">
        <div
          class="home-storage-dial"
          style=${`--home-storage-used: ${usedPercent}%`}
          aria-label=${`${strings.home.usedPercent}: ${formatPercent(disk?.usedPercentage)}`}
        >
          <div class="home-storage-dial-core">
            <strong>${formatPercent(disk?.usedPercentage)}</strong>
            <span>${strings.home.used.toLowerCase()}</span>
          </div>
        </div>
        <div class="home-storage-side">
          <div class="home-storage-stat-grid">
            <div class="home-storage-stat">
              <span>${strings.home.used}</span>
              <strong>${disk?.used || strings.home.unknown}</strong>
            </div>
            <div class="home-storage-stat">
              <span>${strings.home.free}</span>
              <strong>${disk?.free || strings.home.unknown}</strong>
            </div>
            <div class="home-storage-stat">
              <span>${strings.home.total}</span>
              <strong>${disk?.size || strings.home.unknown}</strong>
            </div>
            <div class="home-storage-stat">
              <span>${strings.home.totalSegments}</span>
              <strong>${segments === undefined ? strings.home.unknown : rounded(segments)}</strong>
            </div>
          </div>
          ${roots.length
            ? html`
                <ul class="home-breakdown home-storage-roots">
                  ${roots.map(
                    (root) => html`
                      <li key=${root.path || root.label}>
                        <span>${root.label}</span>
                        <strong>${rounded(root.segments)} ${strings.home.segments.toLowerCase()}</strong>
                      </li>
                    `,
                  )}
                </ul>
              `
            : html`<p class="home-empty">${strings.home.storageRootsEmpty}</p>`}
        </div>
      </div>
    <//>`;
}

function validRoutePoints(path) {
  const points = Array.isArray(path?.points) ? path.points : [];
  return points.filter(
    (point) =>
      Array.isArray(point) &&
      point.length >= 2 &&
      Number.isFinite(point[0]) &&
      Number.isFinite(point[1]) &&
      point[0] >= 0 &&
      point[0] <= 1 &&
      point[1] >= 0 &&
      point[1] <= 1,
  );
}

function svgPoint(point) {
  return { x: Number((point[0] * 100).toFixed(2)), y: Number((point[1] * 100).toFixed(2)) };
}

function routePolyline(points) {
  return points.map((point) => {
    const mapped = svgPoint(point);
    return `${mapped.x},${mapped.y}`;
  }).join(" ");
}

function RouteMap({ path }) {
  const points = validRoutePoints(path);
  const hasPath = points.length >= 2;
  const first = hasPath ? svgPoint(points[0]) : null;
  const last = hasPath ? svgPoint(points[points.length - 1]) : null;

  return html`
    <div class=${`home-route-map ${hasPath ? "has-path" : "is-empty"}`} aria-label=${strings.home.routePath}>
      <svg viewBox="0 0 100 100" preserveAspectRatio="none" aria-hidden="true">
        <path class="home-map-road home-map-road-main" d="M6 72 C22 62 28 44 45 40 C65 35 72 18 94 14"></path>
        <path class="home-map-road home-map-road-secondary" d="M10 20 C28 26 35 34 45 52 C54 68 68 78 90 84"></path>
        <path class="home-map-road home-map-road-minor" d="M5 48 H28 C40 48 51 58 62 58 H95"></path>
        <path class="home-map-road home-map-road-minor" d="M32 8 V35 C32 55 22 65 18 94"></path>
        <path class="home-map-road home-map-road-minor" d="M72 6 V32 C72 52 82 62 84 96"></path>
        ${hasPath &&
        html`
          <polyline class="home-route-line" points=${routePolyline(points)}></polyline>
          <circle class="home-route-point home-route-point-start" cx=${first.x} cy=${first.y} r="2.4"></circle>
          <circle class="home-route-point home-route-point-end" cx=${last.x} cy=${last.y} r="2.8"></circle>
        `}
      </svg>
      ${!hasPath && html`<span class="home-map-empty">${strings.home.routePathUnavailable}</span>`}
    </div>
  `;
}

function formatDriveDuration(minutes) {
  const value = Number(minutes);
  if (!Number.isFinite(value)) {
    return strings.home.unknown;
  }

  const totalMinutes = Math.max(0, Math.round(value));
  const hours = Math.floor(totalMinutes / 60);
  const mins = totalMinutes % 60;
  if (hours > 0 && mins > 0) {
    return `${hours}h ${mins}m`;
  }
  if (hours > 0) {
    return `${hours}h`;
  }
  return `${mins} min`;
}

function LatestDrive({ snapshot }) {
  const lastDrive = snapshot?.raw?.stats?.lastDrive;
  const distance = lastDrive ? `${decimal(lastDrive.distance)} ${lastDrive.distanceUnit}` : strings.home.unknown;
  const durationLabel = strings.home.duration || "Duration";
  const whenLabel = strings.home.when || "When";

  return html`
    <${Widget}
      title=${strings.home.latestDrive}
      icon="route"
      className="home-latest"
      action=${{ href: ROUTE_HISTORY_PATH, label: strings.home.reviewFootage }}
      notes=${sourceNotes(snapshot, ["stats"])}
    >
      ${lastDrive
        ? html`
            <div class="home-latest-strip-map">
              <${RouteMap} path=${lastDrive.path} />
            </div>
            <dl class="home-drive-stats home-drive-stats-strip">
              <${Fact} label=${whenLabel} value=${lastDrive.when} />
              <${Fact} label=${strings.home.distance} value=${distance} />
              <${Fact} label=${durationLabel} value=${formatDriveDuration(lastDrive.durationMin)} />
              <${Fact} label=${strings.home.avgSpeed} value=${`${rounded(lastDrive.avgSpeed)} ${lastDrive.speedUnit}`} />
              <${Fact} label=${strings.home.engagement} value=${`${rounded(lastDrive.engagement)}%`} />
            </dl>
          `
        : html`<p class="home-empty">${strings.home.latestDriveEmpty}</p>`}
    <//>`;
}

function unitAbbrev(unit) {
  const normalized = (unit || "").toLowerCase();
  if (normalized === "mile" || normalized === "miles") {
    return "miles";
  }
  if (normalized === "kilometer" || normalized === "kilometers") {
    return "kilometers";
  }
  return unit || "";
}

function TotalSourceMark({ tone }) {
  if (tone === "frogpilot") {
    return html`
      <span class="home-total-source-mark home-total-source-mark-frogpilot">
        <img src=${FROGPILOT_MARK} alt="" decoding="async" loading="eager" />
      </span>
    `;
  }

  return html`
    <span class="home-total-source-mark home-total-source-mark-openpilot">
      <img src=${OPENPILOT_MARK} alt="" decoding="async" loading="eager" />
    </span>
  `;
}

function TotalsStat({ icon, label, unit = "", value }) {
  const accessibleValue = `${label}: ${value}${unit ? ` ${unit}` : ""}`;

  return html`
    <div class="home-total-stat" aria-label=${accessibleValue}>
      <span class="home-total-stat-icon"><${Icon} name=${icon} size=${20} /></span>
      <strong>${value}</strong>
      ${unit && html`<span class="home-total-stat-unit">${unit}</span>`}
    </div>
  `;
}

function totalsDistance(totals) {
  return totals ? rounded(totals.distance) : strings.home.unknown;
}

function TotalsRail({ label, totals, tone }) {
  const distanceUnit = totals ? unitAbbrev(totals.unit) : "";

  return html`
    <div class=${`home-total-rail home-total-rail-${tone}`}>
      <div class="home-total-source">
        <${TotalSourceMark} tone=${tone} />
        <h3>${label}</h3>
      </div>
      <${TotalsStat} icon="road" label=${strings.home.distance} value=${totalsDistance(totals)} unit=${distanceUnit} />
      <${TotalsStat} icon="steering-wheel" label=${strings.home.drives} value=${totals ? rounded(totals.drives) : strings.home.unknown} unit=${strings.home.drives} />
      <${TotalsStat} icon="clock" label=${strings.home.hours} value=${totals ? rounded(totals.hours) : strings.home.unknown} unit=${strings.home.hours} />
    </div>
  `;
}

function YourTotals({ snapshot }) {
  const totals = snapshot?.raw?.stats?.driveStats;
  const notes = sourceNotes(snapshot, ["stats"]);

  return html`
    <section class="home-widget home-totals-widget" aria-label=${strings.home.yourTotals}>
      ${notes.length > 0 && html`<${SourceNotes} notes=${notes} />`}
      <div class="home-totals-ledger">
        <${TotalsRail} label=${strings.home.frogpilotTotals} totals=${totals?.frogpilot} tone="frogpilot" />
        <${TotalsRail} label=${strings.home.overallTotals} totals=${totals?.all} tone="openpilot" />
      </div>
    </section>
  `;
}

export function Home() {
  const [snapshot, setSnapshot] = useState(() => cachedHomeSnapshot);
  const [loading, setLoading] = useState(() => !cachedHomeSnapshot);
  const [refreshing, setRefreshing] = useState(false);
  const [reloadKey, setReloadKey] = useState(0);
  const refresh = useCallback(() => setReloadKey((key) => key + 1), []);

  useEffect(() => {
    const timer = setInterval(refresh, AUTO_REFRESH_MS);
    return () => clearInterval(timer);
  }, [refresh]);

  useEffect(() => {
    let mounted = true;
    const controller = new AbortController();

    setLoading(!cachedHomeSnapshot);
    setRefreshing(Boolean(cachedHomeSnapshot));

    fetchHomeSnapshot(cachedHomeSnapshot, controller.signal)
      .then((next) => {
        if (!mounted) {
          return;
        }
        cachedHomeSnapshot = next;
        setSnapshot(next);
        setLoading(false);
        setRefreshing(false);
      })
      .catch(() => {
        if (mounted) {
          setLoading(false);
          setRefreshing(false);
        }
      });

    return () => {
      mounted = false;
      controller.abort();
    };
  }, [reloadKey]);

  const loadingMessage = loading ? strings.home.loading : refreshing ? strings.home.refreshing : null;
  const hasRefreshProblem = snapshot?.refreshFailed || sourceHadProblem(snapshot, SOURCE_DEFS.map((source) => source.key));

  return html`
    <section class="home" aria-label=${strings.home.title}>
      <header class="home-header">
        <div>
          <h1 class="home-title">${strings.home.title}</h1>
          <p class="home-subtitle">${loadingMessage || strings.home.updatedAt(formatRefreshTime(snapshot?.updatedAt))}</p>
        </div>
      </header>

      ${hasRefreshProblem && html`<p class="home-refresh-note">${strings.home.refreshFailed}</p>`}

      <div class="home-console">
        <div class="home-main-grid">
          <${AtAGlance} snapshot=${snapshot} />
          <${YourTotals} snapshot=${snapshot} />
          <div class="home-primary-grid">
            <${LatestDrive} snapshot=${snapshot} />
            <${StorageFootage} snapshot=${snapshot} />
          </div>
        </div>
      </div>
    </section>
  `;
}

# Vision Turn Controller Branch Reconstruction

Recorded: June 7, 2026  
Repository: `probonobuddy/FrogPilot`  
Branch: `codex/vision-turn-controller`  
Known-good branch tip before this document: `09bce69d7ffbfab01a4787feff7ef8246ca56bfb`

## Purpose

This branch replaces FrogPilot's learned Curve Speed Controller calibration with
the older deterministic Vision Turn Controller style. The driver directly tunes
curve response using `CurveSensitivity` and `TurnAggressiveness`, each ranging
from 50% to 150% with 100% as the historical default. This branch successfully
booted and was tested on the comma 3X, including starting the car after the final
compatibility fix.

## Starting Point

This branch changes Python, Cython Params definitions, and C++/Qt settings code.
It therefore started from the uncompiled source parent of FrogPilot-Staging and
was rebuilt for ARM64.

```text
2c0a11e0605989f04d04225556ad6161154cbd70  February 28th, 2026 Patch
63b1beb44e729c023b0b7068d1ca91607fcf78b0  Compile FrogPilot
```

For future staging, locate the new compiled staging tip and use its source parent
as the reconstruction base.

## Controller Logic Port

Initial port:

```text
7d8629d18  Restore manual vision curve speed controls
```

### FrogPilot variables

In `frogpilot/common/frogpilot_variables.py`, add defaults:

```text
CurveSensitivity = 100
TurnAggressiveness = 100
```

Both use tuning level 2, stock value 100, and controller multipliers clipped to
`0.5` through `1.5`.

### Curve controller

In `frogpilot/controls/lib/curve_speed_controller.py`, remove learned calibration,
curvature training data, calibration progress, percentile lateral acceleration,
and training timers. Use:

```python
lateral_acceleration = DEFAULT_LATERAL_ACCELERATION * turn_aggressiveness
adjusted_curvature = max(abs(road_curvature) * curve_sensitivity, 1e-6)
target = max(sqrt(lateral_acceleration / adjusted_curvature), CRUISING_SPEED)
```

Retain the weather reduction. Retain the minimum-speed gate: calculations run
only above `CRUISING_SPEED`, and target is floored there. In this source,
`CRUISING_SPEED` is `5 m/s`, about `11.2 mph`.

Add compatibility state:

```python
self.enable_training = False
```

### Vcruise integration

In `frogpilot/controls/lib/frogpilot_vcruise.py`, call:

```python
self.csc.update_target(frogpilot_toggles)
```

Remove obsolete learned-controller logging and `target_set` reset behavior while
retaining normal enable, curvature, and minimum-speed gates.

### User interface

Change:

```text
frogpilot/ui/layouts/settings/toggle_metadata.py
frogpilot/ui/qt/offroad/longitudinal_settings.cc
frogpilot/ui/qt/offroad/longitudinal_settings.h
```

Replace calibration display/reset controls with numeric 50%-150% controls for
`CurveSensitivity` and `TurnAggressiveness`.

### Tests

Update `frogpilot/controls/lib/tests/test_curve_speed_controller.py` to cover the
historical default formula, conservative/aggressive settings, weather reduction,
minimum target floor, and non-training compatibility attribute.

## Params Registration Required for Boot

The first compiled build failed with:

```text
UnknownKeyName: b'CurveSensitivity'
```

Register `CurveSensitivity` and `TurnAggressiveness` in `common/params.cc` with:

```cpp
PERSISTENT | FROGPILOT_STORAGE | FROGPILOT_CONTROLS
```

Restore flag definitions in `common/params.h` and `common/params_pyx.pyx`:

```text
FROGPILOT_CONTROLS = 0x80
FROGPILOT_STORAGE  = 0x800
```

Rebuild:

```bash
scons -j4 common/params_pyx.so
```

Include generated `common/params_pyx.cpp` and ARM64 `common/params_pyx.so` in the
compiled release. The final audit confirmed combined flags `0x882`, read/write,
and registration of all FrogPilot default Params.

```text
d5a62caad  Register Vision Turn Controller parameters
6b8efaa1d  Restore Vision parameter flag definitions
120bd1fde  Rebuild Params extension for Vision controls
```

## Full comma 3X Build

Build in an isolated directory, leaving active `/data/openpilot` untouched:

```text
/data/vision-turn-build
/data/vision-turn-source-files
/data/vision-turn-build.log
```

Copy the release manifest:

```bash
python3 ./release/release_files.py |
  xargs -d '\n' cp -pR --parents -t /data/vision-turn-build/
```

Use the installed comma environment because `uv 0.11.19` rejected the older
Poetry metadata:

```bash
export PATH="/usr/local/pyenv/shims:/usr/local/pyenv/bin:$PATH"
export PYTHONPATH=/data/vision-turn-build
python3.11 --version
scons --version
scons -j"$(nproc)" --minimal
scons -j"$(nproc)" panda/
```

SCons also required the official source file:

```text
tinygrad_repo/examples/openpilot/compile3.py
```

Restore it from the retained source tree if the release copy omits it.

After compilation, perform normal release cleanup of object files, tests, docs,
developer directories, caches, and unsupported architectures. Create `prebuilt`
and commit the compiled tree:

```text
2bfcbc6e0  Compile Vision Turn Controller
```

## Critical Packaging Lesson: Keep tools/lib

The first cleanup incorrectly removed `tools/lib/`, causing:

```text
ModuleNotFoundError: No module named 'openpilot.tools.lib'
```

The Pond imports `openpilot.tools.lib.route.SegmentName`. The official release
manifest includes `tools/lib`, so future cleanup must not delete it. Verify:

```bash
python3 -c "from openpilot.tools.lib.route import SegmentName"
```

Also import The Pond utility and entry modules before deployment.

```text
1f2bec60f  Restore tools library required by The Pond
```

## Planner Compatibility Lesson

`frogpilot_planner.py` publishes:

```python
frogpilotPlan.cscTraining = self.frogpilot_vcruise.csc.enable_training
```

Without the compatibility attribute, starting the car caused:

```text
AttributeError: 'CurveSpeedController' object has no attribute 'enable_training'
```

Add `self.enable_training = False` and audit all external CSC references. The
expected public interface is `enable_training`, `update_target(...)`, and
`target`.

```text
09bce69d7  Expose deterministic CSC training state
```

## Transfer and Installation

Export the compiled branch from the comma:

```bash
git bundle create /data/vision-turn-controller.bundle \
  codex/vision-turn-controller
```

Transfer with SCP, import locally, and push to `probonobuddy/FrogPilot`. Point the
comma updater at the branch, signal it, verify the staged commit, then reboot.
The FrogPilot selector can switch between:

```text
codex/vision-turn-controller
csc-2ms2-cap
```

## Final Commit Sequence

```text
7d8629d18  Restore manual vision curve speed controls
2bfcbc6e0  Compile Vision Turn Controller
3f06148a4  Add personal branch selector and recovery guide
d5a62caad  Register Vision Turn Controller parameters
6b8efaa1d  Restore Vision parameter flag definitions
120bd1fde  Rebuild Params extension for Vision controls
1f2bec60f  Restore tools library required by The Pond
09bce69d7  Expose deterministic CSC training state
```

## Future Staging Reconstruction Checklist

1. Identify the newest staging compiled tip and source parent.
2. Review changes to CSC, vcruise, Params, manager startup, UI, release files,
   The Pond, and planner publication.
3. Create the source branch from the new staging source parent.
4. Port the deterministic formula and manual toggles to current interfaces.
5. Preserve the minimum-speed gate and weather behavior unless deliberately changed.
6. Register both Params with the three historical flags.
7. Confirm current flag values and conventions.
8. Preserve `enable_training = False` or adapt every current consumer.
9. Update Python and C++/Qt settings UI and controller tests.
10. Build `common/params_pyx.so`, then full ARM64 and Panda artifacts.
11. Preserve `tools/lib` and required tinygrad compile sources.
12. Verify Params, imports, manager startup, UI, and final commit.
13. Push, select, stage, verify, and reboot.
14. Test parked first, then start the car and road-test cautiously.

## Mandatory Review Warning

Do not blindly replay these commits. A clean cherry-pick can still compile but
crash at manager startup or only when the car starts. Search the new tree for
every CSC object reference, both Param names, old calibration fields, UI
controls, and release cleanup paths. Rebuild every native artifact affected by
current source changes.

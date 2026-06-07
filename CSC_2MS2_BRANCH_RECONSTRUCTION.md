# CSC 2.0 m/s^2 Cap Branch Reconstruction

Recorded: June 7, 2026  
Repository: `probonobuddy/FrogPilot`  
Branch: `csc-2ms2-cap`  
Known-good branch tip before this document: `9cb91949e59a423ce79e997add858cf3437f0d53`

## Purpose

This branch keeps the normal FrogPilot-Staging learned Curve Speed Controller
(CSC), but prevents its learned lateral-acceleration calibration from exceeding
`2.0 m/s^2`. It retains FrogPilot's normal learning behavior while placing a
firm upper bound on the learned value used to calculate curve speed.

The cap limits the controller's requested curve-speed target. It does not
physically guarantee measured lateral acceleration can never exceed `2.0 m/s^2`,
because steering response, road geometry, vehicle dynamics, braking delay,
grade, and tire conditions remain outside this calculation.

## Starting Point

The branch was created directly from compiled FrogPilot-Staging commit:

```text
63b1beb44e729c023b0b7068d1ca91607fcf78b0  Compile FrogPilot
```

This was possible because the functional change is Python-only. The existing
compiled staging artifacts and `prebuilt` marker were retained; no comma 3X
ARM64 rebuild was required.

For future reconstruction, first identify the newest tested
`FrogPilot-Staging` compiled commit. Do not blindly reuse the commit above.
Review upstream CSC changes before applying the patch.

## Functional Code Change

File: `frogpilot/controls/lib/curve_speed_controller.py`

Add:

```python
MAX_CALIBRATED_LATERAL_ACCELERATION = 2.0
```

Cap newly observed training samples before they enter calibration data:

```python
lateral_acceleration = min(
  abs(v_ego**2 * self.curvature),
  MAX_CALIBRATED_LATERAL_ACCELERATION,
)
```

Cap loaded or percentile-derived calibration after calculation:

```python
self.lateral_acceleration = min(
  self.lateral_acceleration,
  MAX_CALIBRATED_LATERAL_ACCELERATION,
)
```

The second check protects devices containing historical calibration data above
the new cap. The normal target relationship remains:

```text
target speed = sqrt(calibrated lateral acceleration / curvature)
```

## Regression Tests

File: `frogpilot/controls/lib/tests/test_curve_speed_controller.py`

The tests use lightweight module and Params stubs. They verify:

1. Existing calibration above `2.0 m/s^2` is clamped.
2. A new training sample above `2.0 m/s^2` is clamped before storage.

Run:

```bash
python3 -m unittest frogpilot.controls.lib.tests.test_curve_speed_controller
python3 -m py_compile \
  frogpilot/controls/lib/curve_speed_controller.py \
  frogpilot/controls/lib/tests/test_curve_speed_controller.py
```

## Construction Sequence

Functional commits:

```text
4feb337a3  Cap curve calibration lateral acceleration at 2.0
ccd56d149  Add curve calibration cap regression tests
```

Documentation commits:

```text
8f49493f9  Add installation and rollback guide
4841474e2  Personalize comma installation instructions
f9cb3ff26  Add personal FrogPilot staging update guide
8dd4c99f9  Link detailed staging update guide
7e61ba195  Document both comma 3X controller branches
6b5d7fd6c  Correct branch selector and SSH instructions
9cb91949e  Update personal selector and recovery guide
```

Important branch documents:

```text
README.md
FROGPILOT_2MS2_GUIDE.md
UPDATING_FROGPILOT_STAGING.md
```

## Future Staging Reconstruction Checklist

1. Fetch the latest `FrogPilot-Staging` and identify its compiled tip.
2. Inspect upstream changes to `curve_speed_controller.py`.
3. Confirm it still learns lateral acceleration and uses it in the target-speed formula.
4. Create a new branch from the latest compiled staging tip.
5. Add the constant and both clamps at equivalent current locations.
6. Port the regression tests to the current CSC interfaces.
7. Run focused tests and syntax checks.
8. Confirm the diff contains no unrelated staging changes.
9. Push the refreshed branch to `probonobuddy/FrogPilot`.
10. Select it through FrogPilot's selector and let the updater stage it.
11. Test parked first, then perform a cautious road test.

## Mandatory Review Warning

Do not mechanically cherry-pick the old commit if FrogPilot changed the CSC.
Check for renamed calibration fields, changed formulas or units, new smoothing,
weather modifiers, and stored calibration Params. A clean merge does not prove
the safety behavior is still correct.

Because this branch starts at compiled staging and changes only Python, it
normally needs no full comma 3X rebuild. If future changes include Cython, C++,
Qt, cereal schemas, or Params registry code, use the full source-build process
documented for the Vision Turn Controller branch.

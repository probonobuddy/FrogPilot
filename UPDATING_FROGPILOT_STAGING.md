# Updating My Fork After a FrogPilot Staging Release

This guide is only for my personal fork:

- Repository: `https://github.com/probonobuddy/FrogPilot`
- Custom branch: `csc-2ms2-cap`
- Upstream repository: `https://github.com/FrogAi/FrogPilot`
- Upstream branch: `FrogPilot-Staging`
- comma 3X checkout: `/data/openpilot`

My custom behavior is implemented in:

```text
frogpilot/controls/lib/curve_speed_controller.py
```

Never assume a new staging release is compatible merely because Git reports no merge conflict. Before updating, check whether FrogPilot changed the controller or the nearby integration code.

The immediate files to inspect are:

```text
frogpilot/controls/lib/curve_speed_controller.py
frogpilot/controls/lib/frogpilot_vcruise.py
frogpilot/common/frogpilot_variables.py
```

If upstream changed any of these files, stop and review the changes before merging. Ask Codex to reassess the `2.0 m/s^2` cap against the new implementation. Do not blindly resolve a conflict by choosing either the whole upstream file or the whole custom file.

## Recommended: ask Codex to update the branch

Use this prompt in the Codex thread connected to `probonobuddy/FrogPilot`:

```text
Update my personal branch probonobuddy/FrogPilot:csc-2ms2-cap from
FrogAi/FrogPilot:FrogPilot-Staging.

Before making changes, compare the new staging branch against the merge base and
carefully inspect:

- frogpilot/controls/lib/curve_speed_controller.py
- frogpilot/controls/lib/frogpilot_vcruise.py
- frogpilot/common/frogpilot_variables.py

My intended customization is a hard 2.0 m/s^2 maximum for both newly learned
curve samples and the final CalibratedLateralAcceleration value. Do not modify
panda, opendbc safety, steering torque, or driver monitoring.

If upstream changed the Curve Speed Controller or its integration in a way that
could affect this customization, stop and explain the changes without merging.
If there are no relevant breaking changes, update csc-2ms2-cap, preserve the
cap, run the focused tests, and verify that the final controller diff against
FrogPilot-Staging contains only the intended cap.
```

After Codex completes the update, inspect the branch:

```text
https://github.com/probonobuddy/FrogPilot/tree/csc-2ms2-cap
```

Then follow **Update the comma 3X** below.

## Manual Git workflow

Perform this work on a computer in Git Bash, WSL, Linux, or macOS. Do not perform the merge directly in `/data/openpilot`; update GitHub first, then deploy the verified branch to the comma.

### 1. Clone my fork

Use a new directory to avoid mixing the update with another checkout:

```bash
git clone https://github.com/probonobuddy/FrogPilot.git FrogPilot-personal
cd FrogPilot-personal
git remote add upstream https://github.com/FrogAi/FrogPilot.git
```

If the directory already exists:

```bash
cd FrogPilot-personal
git remote set-url origin https://github.com/probonobuddy/FrogPilot.git
git remote remove upstream 2>/dev/null || true
git remote add upstream https://github.com/FrogAi/FrogPilot.git
```

Confirm the remotes:

```bash
git remote -v
```

Expected URLs:

```text
origin    https://github.com/probonobuddy/FrogPilot.git
upstream  https://github.com/FrogAi/FrogPilot.git
```

### 2. Fetch both branches

```bash
git fetch origin csc-2ms2-cap
git fetch upstream FrogPilot-Staging
git checkout -B csc-2ms2-cap origin/csc-2ms2-cap
```

Confirm the starting point:

```bash
git branch --show-current
git status --short
git log -1 --oneline
```

The branch must be `csc-2ms2-cap`, and `git status --short` should print nothing.

### 3. Create and publish a backup branch

```bash
BACKUP_BRANCH="backup-csc-2ms2-cap-$(date +%Y%m%d-%H%M)"
git branch "$BACKUP_BRANCH"
git push origin "$BACKUP_BRANCH"
echo "$BACKUP_BRANCH"
```

Keep the printed branch name until the updated build is working on the comma.

### 4. Find the old common staging commit

```bash
BASE_COMMIT=$(git merge-base csc-2ms2-cap upstream/FrogPilot-Staging)
if [ -z "$BASE_COMMIT" ]; then
  echo "STOP: no common staging commit was found."
  exit 1
fi
echo "$BASE_COMMIT"
git show -s --oneline "$BASE_COMMIT"
```

`BASE_COMMIT` is the staging commit from which the current custom branch diverged. It lets me inspect what upstream changed since the previous update.

### 5. Check the controller and integration files

Run:

```bash
git diff "$BASE_COMMIT"..upstream/FrogPilot-Staging -- \
  frogpilot/controls/lib/curve_speed_controller.py \
  frogpilot/controls/lib/frogpilot_vcruise.py \
  frogpilot/common/frogpilot_variables.py
```

Interpretation:

- No output means upstream did not change these files since the previous common staging commit.
- Any output means upstream changed relevant code. Stop and review it before merging.
- A clean Git merge is not proof that the customization is still correct.

Also review the complete incoming update:

```bash
git log --oneline --decorate "$BASE_COMMIT"..upstream/FrogPilot-Staging
git diff --stat "$BASE_COMMIT"..upstream/FrogPilot-Staging
```

### 6. Confirm my cap exists before merging

```bash
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" \
  frogpilot/controls/lib/curve_speed_controller.py
```

Expected output includes:

```text
MAX_CALIBRATED_LATERAL_ACCELERATION = 2.0
```

### 7. Merge the new staging branch

Only continue if the inspection above found no relevant breaking changes:

```bash
git merge --no-edit upstream/FrogPilot-Staging
```

If Git reports a conflict, stop. Do not use `git checkout --ours`, `git checkout --theirs`, or delete conflict markers without reviewing the new controller design.

### 8. Verify the final customization

The most important check is the final custom branch compared with the newly merged staging branch:

```bash
git diff upstream/FrogPilot-Staging..HEAD -- \
  frogpilot/controls/lib/curve_speed_controller.py
```

The intended production differences are:

1. `MAX_CALIBRATED_LATERAL_ACCELERATION = 2.0`
2. Newly logged lateral acceleration is limited with `min(...)`.
3. The final `self.lateral_acceleration` is limited before writing `CalibratedLateralAcceleration`.

Check the exact locations:

```bash
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" \
  frogpilot/controls/lib/curve_speed_controller.py
grep -n "CalibratedLateralAcceleration" \
  frogpilot/controls/lib/curve_speed_controller.py
```

### 9. Run focused verification

```bash
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
python3 -m unittest frogpilot.controls.lib.tests.test_curve_speed_controller -v
```

Expected test result:

```text
Ran 2 tests
OK
```

Also check that no unexpected personal files changed:

```bash
git status --short
git diff --stat origin/csc-2ms2-cap..HEAD
```

### 10. Push the updated personal branch

```bash
git push origin csc-2ms2-cap
```

Verify the result on GitHub:

```text
https://github.com/probonobuddy/FrogPilot/tree/csc-2ms2-cap
```

## Update the comma 3X

Only do this after the updated GitHub branch has passed the checks above. Keep the vehicle parked.

SSH into the comma and run:

```bash
cd /data/openpilot
git remote set-url myfork https://github.com/probonobuddy/FrogPilot.git
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" \
  frogpilot/controls/lib/curve_speed_controller.py
sudo reboot
```

If `myfork` does not exist yet:

```bash
cd /data/openpilot
git remote add myfork https://github.com/probonobuddy/FrogPilot.git
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
sudo reboot
```

## Verify the comma after reboot

Reconnect over SSH:

```bash
cd /data/openpilot
git branch --show-current
git log -1 --oneline
git remote -v
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" \
  frogpilot/controls/lib/curve_speed_controller.py
```

Expected results:

- Branch is `csc-2ms2-cap`.
- `myfork` points to `https://github.com/probonobuddy/FrogPilot.git`.
- The cap is still `2.0`.
- FrogPilot starts normally.

Use a familiar, low-traffic route for the first drive after every staging update. Stay ready to take over and verify curve entry behavior before using more demanding roads.

## Roll back the GitHub branch

Use the backup branch name created earlier:

```bash
git checkout csc-2ms2-cap
git reset --hard "$BACKUP_BRANCH"
git push --force-with-lease origin csc-2ms2-cap
```

If using a new terminal, replace `$BACKUP_BRANCH` with the exact saved branch name.

## Roll back the comma

After restoring the GitHub branch:

```bash
cd /data/openpilot
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
sudo reboot
```

To return to official staging instead:

```bash
cd /data/openpilot
git remote remove upstream-frogpilot 2>/dev/null || true
git remote add upstream-frogpilot https://github.com/FrogAi/FrogPilot.git
git fetch upstream-frogpilot FrogPilot-Staging
git checkout -B FrogPilot-Staging upstream-frogpilot/FrogPilot-Staging
sudo reboot
```

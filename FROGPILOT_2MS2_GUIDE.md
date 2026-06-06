# My FrogPilot 2.0 m/s^2 Curve Calibration Cap

This is my personal FrogPilot build:

- Repository: `https://github.com/probonobuddy/FrogPilot`
- Branch: `csc-2ms2-cap`
- Base: `FrogAi/FrogPilot:FrogPilot-Staging`

It caps newly learned curve samples and the final `CalibratedLateralAcceleration` value at `2.0 m/s^2`. Steering torque, panda safety, driver monitoring, and vehicle safety limits are unchanged.

## Install on my comma 3X

Keep the vehicle parked throughout the update.

1. Connect the comma 3X to Wi-Fi.
2. Enable SSH in the device settings and configure my GitHub username and SSH key.
3. Find the comma 3X IP address in its network settings.
4. From the computer containing the matching private key, connect:

```bash
ssh comma@DEVICE_IP -i PATH_TO_PRIVATE_KEY
```

5. Save the current branch and commit for rollback:

```bash
cd /data/openpilot
git branch --show-current
git rev-parse HEAD
git remote -v
```

6. Add my GitHub repository as a remote and install the branch:

```bash
git remote add myfork https://github.com/probonobuddy/FrogPilot.git
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
sudo reboot
```

`myfork` is only a local alias. It must be created with `git remote add` before `git fetch myfork` will work.

If `git remote add` reports that `myfork` already exists, use:

```bash
git remote set-url myfork https://github.com/probonobuddy/FrogPilot.git
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
sudo reboot
```

## Verify after reboot

Reconnect over SSH and run:

```bash
cd /data/openpilot
git branch --show-current
git log -1 --oneline
git remote -v
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" frogpilot/controls/lib/curve_speed_controller.py
```

Expected results:

- Current branch: `csc-2ms2-cap`
- `myfork` points to `https://github.com/probonobuddy/FrogPilot.git`
- The controller contains `MAX_CALIBRATED_LATERAL_ACCELERATION = 2.0`
- FrogPilot starts normally
- `Calibrated Lateral Acceleration` never displays more than `2.00 m/s^2`

Using `Reset Curve Data` once is optional. Existing values above the cap are already limited by the code.

## Update my comma later

After I update the GitHub branch, run this while parked:

```bash
cd /data/openpilot
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
python3 -m py_compile frogpilot/controls/lib/curve_speed_controller.py
sudo reboot
```

## Roll back

Return to official FrogPilot staging:

```bash
cd /data/openpilot
git remote remove upstream-frogpilot 2>/dev/null || true
git remote add upstream-frogpilot https://github.com/FrogAi/FrogPilot.git
git fetch upstream-frogpilot FrogPilot-Staging
git checkout -B FrogPilot-Staging upstream-frogpilot/FrogPilot-Staging
sudo reboot
```

A clean reinstall of official staging can also use `staging.frogpilot.download` from the comma setup screen.

## First road test

Use a familiar, low-traffic route in good weather. Stay fully attentive and ready to take over immediately. Confirm that curve entry speeds are more conservative before trying demanding roads.

This change lowers the curve-speed calibration ceiling. It does not increase steering authority or make the vehicle capable of taking curves beyond its existing actuator limits.

## Updating after a new staging release

Use the separate, detailed update guide:

```text
UPDATING_FROGPILOT_STAGING.md
```

It requires checking upstream changes to the Curve Speed Controller and its integration before merging.

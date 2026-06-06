# FrogPilot 2.0 m/s^2 Curve Calibration Cap

Repository: `probonobuddy/FrogPilot`

Branch: `csc-2ms2-cap`

This branch starts from `FrogAi/FrogPilot:FrogPilot-Staging` and makes one driving-behavior change:

- Newly learned curve samples are capped at `2.0 m/s^2`.
- The final `CalibratedLateralAcceleration` is capped at `2.0 m/s^2`, including when old stored calibration data contains higher values.
- Steering torque, panda safety, driver monitoring, and vehicle safety limits are unchanged.

## Install on a comma 3X

Use SSH for this FrogPilot-named fork. Keep the vehicle parked throughout the update.

1. Connect the comma 3X to Wi-Fi.
2. Enable SSH in the device settings and configure your GitHub username/key.
3. Find the comma 3X IP address in its network settings.
4. From a computer containing the matching private key, connect with:

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

6. Add the personal fork and switch branches:

```bash
git remote remove myfork 2>/dev/null || true
git remote add myfork https://github.com/probonobuddy/FrogPilot.git
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
grep -n "MAX_CALIBRATED_LATERAL_ACCELERATION" frogpilot/controls/lib/curve_speed_controller.py
```

Expected results:

- Current branch: `csc-2ms2-cap`
- The controller contains `MAX_CALIBRATED_LATERAL_ACCELERATION = 2.0`
- FrogPilot starts normally
- `Calibrated Lateral Acceleration` never displays more than `2.00 m/s^2`

Using `Reset Curve Data` once is optional. Existing values above the cap are already limited by the code.

## First road test

1. Use a familiar, low-traffic route in good weather.
2. Stay fully attentive and ready to take over immediately.
3. Begin with simple curves at moderate speed.
4. Confirm curve entry speeds are more conservative before trying demanding roads.

This change lowers the curve-speed calibration ceiling. It does not increase steering authority or make the vehicle capable of taking curves beyond its existing actuator limits.

## Update this branch later

FrogPilot staging changes frequently. Rebase and test on a computer before updating the comma device:

```bash
git remote add upstream https://github.com/FrogAi/FrogPilot.git
git fetch upstream FrogPilot-Staging
git switch csc-2ms2-cap
git rebase upstream/FrogPilot-Staging
python3 -m unittest frogpilot.controls.lib.tests.test_curve_speed_controller
git push --force-with-lease
```

Then, while parked, update the comma 3X:

```bash
cd /data/openpilot
git fetch myfork csc-2ms2-cap
git checkout -B csc-2ms2-cap myfork/csc-2ms2-cap
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

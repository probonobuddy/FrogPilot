# My comma 3X FrogPilot branches

This is my personal FrogPilot fork. It is configured so the FrogPilot
touchscreen branch selector can switch between my two custom branches without
requiring SSH for every change.

| Branch | Controller |
| --- | --- |
| `csc-2ms2-cap` | Current learned Curve Speed Controller with a hard 2.0 m/s2 lateral-acceleration speed cap. This is the known-working custom branch. |
| `codex/vision-turn-controller` | Deterministic Vision Turn Controller restored from the older FrogPilot design, with manual 50%-150% curve sensitivity and turn aggressiveness settings. |

The retired branch `codex/manual-vision-turn-controller` is not the branch to
use. It was source-only, failed on the comma 3X, and caused a loading-screen
boot loop. The replacement is `codex/vision-turn-controller`.

> [!WARNING]
> `codex/vision-turn-controller` was source-tested and compiled successfully on
> the comma 3X on June 7, 2026. Its compiled commit is
> `2bfcbc6e08417335021c7f485b2543976c12c4e5`. It has not yet completed a real
> vehicle drive test. Test cautiously, remain ready to take control, and keep
> the working `csc-2ms2-cap` backup.

## SSH from my Windows computer

SSH is useful for the one-time selector setup, verification, and recovery.
Publishing `id_ed25519.pub` is safe and expected. Never publish or share the
private file `C:\Users\mabra\.ssh\id_ed25519`.

On the comma 3X:

1. Park the vehicle and connect the comma to the same Wi-Fi as the computer.
2. Open **Settings > Network > SSH**.
3. Enable SSH and enter the GitHub username `probonobuddy`.
4. Note the comma's current IP address. It was `192.168.1.99` during setup, but
   DHCP can change it.

The public key registered with GitHub is displayed in Windows PowerShell with:

```powershell
Get-Content C:\Users\$env:USERNAME\.ssh\id_ed25519.pub
```

Connect with:

```powershell
ssh comma@192.168.1.99 -i "$HOME\.ssh\id_ed25519"
```

Replace the IP address if the comma shows a different one. A successful login
shows a prompt similar to `comma@comma-74d4d7eb:~$`.

## One-time touchscreen selector setup

Run these commands over SSH while parked and connected to reliable Wi-Fi. The
working FrogPilot installation is expected at `/data/openpilot`.

First verify that directory before changing any Git configuration:

```bash
test -x /data/openpilot/launch_openpilot.sh &&
test -d /data/openpilot/.git &&
echo "OPENPILOT FOUND"
```

Configure my fork as `origin`, because FrogPilot's selector reads branches from
that remote:

```bash
cd /data/openpilot

git config --global --add safe.directory /data/openpilot
git remote set-url origin https://github.com/probonobuddy/FrogPilot.git
git config --replace-all remote.origin.fetch \
  '+refs/heads/*:refs/remotes/origin/*'
git fetch --prune origin
```

Verify the exact remote and both usable branches:

```bash
git remote -v
git branch -r --list \
  origin/csc-2ms2-cap \
  origin/codex/vision-turn-controller
```

The branch list must include:

```text
origin/codex/vision-turn-controller
origin/csc-2ms2-cap
```

Reboot once so FrogPilot recreates its updater state:

```bash
sudo reboot
```

## Switch branches from the comma screen

1. Park the vehicle and connect the comma to reliable Wi-Fi.
2. Open **Settings > Software**.
3. Open **Target Branch > Select**.
4. Select either `csc-2ms2-cap` or `codex/vision-turn-controller`.
5. Confirm the download.
6. Do not reboot or remove power while it downloads and finalizes.
7. Press **Install Update** when offered.
8. Let the comma reboot fully.

Only one branch is active at a time. The branches remain available in the
selector because both are stored in `probonobuddy/FrogPilot`.

After switching, verify over SSH:

```bash
git -C /data/openpilot branch --show-current
git -C /data/openpilot log -1 --oneline
test -f /data/openpilot/prebuilt && echo "PREBUILT PRESENT"
test -x /data/openpilot/selfdrive/ui/ui && echo "UI BINARY PRESENT"
```

Expected Vision branch commit after its first compiled release:

```text
2bfcbc6e0 Compile Vision Turn Controller
```

## Updating my fork for a new FrogPilot staging release

Do not directly merge a new staging release into either compiled custom branch
and assume it is safe. FrogPilot staging updates can change the controller
interfaces, longitudinal planner, model data, parameter names, settings UI,
build files, or updater packaging even when Git reports no conflicts.

Before rebuilding either custom branch:

1. Compare the new upstream `FrogPilot-Staging` source with the source base used
   by the custom branch.
2. Specifically inspect:
   `frogpilot/controls/lib/curve_speed_controller.py`,
   `frogpilot/controls/lib/frogpilot_vcruise.py`,
   `frogpilot/common/frogpilot_variables.py`, the settings metadata and C++ UI
   files, longitudinal planning code, model messages, and `release/`.
3. Confirm that controller inputs, units, model array lengths, parameter names,
   and the vCruise integration point have not changed.
4. Reapply the custom controller to the new uncompiled staging source.
5. Run controller unit tests and Python syntax checks.
6. Compile on ARM64 using the comma 3X build environment.
7. Verify `prebuilt`, `/selfdrive/ui/ui`, and the controller files exist in the
   compiled result.
8. Publish the compiled commit to the same branch name only after those checks.
9. Keep the previous automatic backup until the new branch boots and completes
   a cautious test drive.

The Vision branch must remain deterministic and manually adjustable. The
`csc-2ms2-cap` branch must retain its explicit 2.0 m/s2 lateral-acceleration
speed cap. If upstream changes either behavior or makes the integration
ambiguous, stop rather than performing a blind merge.

## Recovery if FrogPilot will not load

Do not repeatedly reboot, delete `/data/backups`, or overwrite a working
backup. Connect with SSH first.

Inspect the launcher, installation, and backups:

```bash
sudo cat /data/continue.sh
sudo ls -la /data/openpilot
sudo find /data/backups -maxdepth 2 -type f -printf '%s %p\n'
df -h /data
```

The launcher should normally contain:

```bash
cd /data/openpilot
exec ./launch_openpilot.sh
```

Automatic backups previously used on this device include:

```text
/data/backups/FrogPilot-Staging_2026-02-28_auto.tar.zst
/data/backups/csc-2ms2-cap_2026-06-05_auto.tar.zst
```

Do not assume those exact files still exist; list `/data/backups` first. Verify
a selected archive before restoring it:

```bash
zstd -t /data/backups/csc-2ms2-cap_2026-06-05_auto.tar.zst
```

If Git reports "dubious ownership" after a restore, do not restore again. Mark
the restored repository safe:

```bash
git config --global --add safe.directory /data/openpilot
git -C /data/openpilot branch --show-current
git -C /data/openpilot log -1 --oneline
```

Official comma SSH documentation:
<https://docs.comma.ai/how-to/connect-to-comma/>

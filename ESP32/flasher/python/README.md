# TCode ESP32 Flasher (Python)

Cross-platform, single-file flasher for the SR6PCB and SSR1PCB controllers.
Bundled releases ship as one self-contained executable per OS — no Python or
PlatformIO install required on the user's machine.

## Download

End users do **not** need Python. Grab the prebuilt binary for your OS from
the latest [GitHub Release](https://github.com/jcfain/TCodeESP32/releases/latest):

| OS      | File                                          | After download                                      |
| ------- | --------------------------------------------- | --------------------------------------------------- |
| Windows | `TCodeFlasher-windows-x86_64.zip`             | Unzip and double-click `TCodeFlasher-windows-x86_64.exe`. SmartScreen may warn — click *More info* → *Run anyway*. |
| macOS   | `TCodeFlasher-macos-universal.zip`            | Unzip → drag `TCodeFlasher.app` to Applications. First launch: right-click → *Open* to bypass Gatekeeper. |
| Linux   | `TCodeFlasher-linux-x86_64.tar.gz`            | `tar -xzf TCodeFlasher-linux-x86_64.tar.gz && ./TCodeFlasher-linux-x86_64`. Add your user to `dialout` (or `uucp` on Arch) for serial access. |

Every binary has the latest firmware **and** filesystem image baked in, so
the first run is fully offline. The "Check for update" button optionally
queries GitHub Releases for newer firmware after install.

## What it does

1. Auto-detects an SR6PCB / SSR1PCB plugged in over USB.
2. Reads the firmware version from the boot banner.
3. Backs up the LittleFS settings partition (so user pin maps, wifi creds and
   board configuration survive a re-flash).
4. Flashes the latest firmware + filesystem image (bundled in the exe; can
   also fetch the latest GitHub release if "Check for update" is clicked).
5. Restores the saved settings partition.
6. Optionally pushes Wi-Fi credentials to the device over the serial console.
7. Shows a "Done" screen with the device's IP once it joins the network.

## Quick start (developers)

```powershell
cd flasher/python
python -m venv .venv
. .venv/Scripts/Activate.ps1     # macOS/Linux: source .venv/bin/activate
pip install -e .[dev]

# Run the GUI wizard:
python -m tcode_flasher

# Or the CLI:
python -m tcode_flasher --cli --ssid "MyWifi" --password "secret"
```

## Building a release exe

```powershell
pip install pyinstaller
pyinstaller flasher.spec
# dist/TCodeFlasher.exe (or .app / ELF) is fully standalone.
```

The PyInstaller spec bundles `firmware/<board>/*.bin` so the exe carries the
matching firmware payload for every supported board.

## Adding a new firmware bundle

Drop the four esptool artefacts into `firmware/<board>/`:

```
firmware/sr6_pcb/
  bootloader.bin
  partitions.bin
  firmware.bin
  littlefs.bin
  manifest.json
```

`manifest.json` records flash addresses, board id, and the firmware version
string the on-device boot banner advertises. See
[manifest.py](src/tcode_flasher/manifest.py) for the schema.

## CI

Built on every push via `.github/workflows/flasher.yml` — matrix of
`windows-latest`, `ubuntu-latest`, `macos-latest`. Artefacts are attached to
the workflow run and uploaded to the Release page on tag pushes.

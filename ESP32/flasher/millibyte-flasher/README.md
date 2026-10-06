# Millibyte Flasher

A single-binary, zero-dependency flasher for the Millibyte TCode ESP32
controllers (SR6PCB, SSR1PCB). Written in Rust, statically links the
[`espflash`](https://crates.io/crates/espflash) library — no Python, no
PlatformIO, no esptool.exe needed at runtime.

Same binary is **GUI when invoked with no arguments** and **CLI when invoked
with arguments**:

```text
# Double-click the .exe / run from a launcher  →  GUI wizard
millibyte-flasher

# Any flag/subcommand  →  CLI
millibyte-flasher --port COM5 --board sr6_pcb flash
millibyte-flasher list-ports
millibyte-flasher --board sr6_pcb --ssid MyWifi --password "secret" flash
```

## Distribution layout

The release zip ships:

```text
TCodeFlasher-<os>/
  millibyte-flasher.exe   (or millibyte-flasher on macOS/Linux)
  firmware/
    sr6_pcb/
      bootloader.bin
      partitions.bin
      boot_app0.bin
      firmware.bin
      littlefs.bin
      manifest.json
    ssr1_pcb/
      ...
  README.txt
```

The flasher discovers `firmware/<board>/manifest.json` next to the executable
and offers each bundle in the GUI / `--board` flag.

## Manifest schema

`firmware/<board>/manifest.json`:

```json
{
    "board_id":         "sr6_pcb",
    "display_name":     "SR6 PCB (ESP32-S3)",
    "chip":             "esp32s3",
    "firmware_version": "0.483b",
    "flash_mode":       "dio",
    "flash_freq":       "80MHz",
    "flash_size":       "4MB",
    "usb_vid_pid":      ["303a:1001", "303a:0002"],
    "files": [
        { "offset": "0x0000",   "name": "bootloader.bin" },
        { "offset": "0x8000",   "name": "partitions.bin" },
        { "offset": "0xe000",   "name": "boot_app0.bin"  },
        { "offset": "0x10000",  "name": "firmware.bin"   },
        { "offset": "0x290000", "name": "littlefs.bin"   }
    ]
}
```

Boot offsets follow the ESP-IDF/Arduino layout used by `flash_sr6_pcb.ps1`.

## Build

```powershell
cd flasher/millibyte-flasher
cargo build --release
# target/release/millibyte-flasher.exe
```

CI matrix (windows-latest, macos-latest, ubuntu-latest) packages the binary
plus the `firmware/<board>/` tree from the latest `.pio/build/<env>/` output.

## CLI reference

```text
millibyte-flasher [OPTIONS] [COMMAND]

Commands:
  flash         Flash firmware + filesystem to a connected device  (default)
  list-ports    Print serial ports that look like an ESP32
  list-boards   Print every firmware bundle the flasher can see
  configure     Push Wi-Fi credentials over serial without re-flashing

Options:
  -p, --port <PORT>          Serial port (auto-detect if omitted)
  -b, --board <BOARD>        Board id (auto-detect if a single bundle exists)
      --ssid <SSID>          Wi-Fi SSID to push after flashing
      --password <PASSWORD>  Wi-Fi password (used with --ssid)
      --no-fs                Skip the filesystem image
      --no-wifi              Skip the Wi-Fi configure step
      --baud <BAUD>          Flash baud rate (default: 921_600)
  -v, --verbose              Increase log verbosity
```

Running with no arguments at all launches the GUI wizard.

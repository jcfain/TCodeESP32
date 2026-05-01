# Firmware payload directory

Each subfolder is one bundled firmware target: drop the four bin files
plus a `manifest.json` here. The PyInstaller spec
([`flasher.spec`](../flasher.spec)) bundles every subfolder into the
final executable.

## Layout

```
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
```

## manifest.json schema

```json
{
  "board_id": "sr6_pcb",
  "display_name": "SR6 PCB (ESP32-S3)",
  "chip": "esp32s3",
  "firmware_version": "0.483b",
  "flash_mode": "dio",
  "flash_freq": "80m",
  "flash_size": "4MB",
  "usb_vid_pid": ["303a:1001", "303a:0002", "10c4:ea60"],
  "fs_partition": "spiffs",
  "files": [
    { "offset": "0x0000",   "name": "bootloader.bin" },
    { "offset": "0x8000",   "name": "partitions.bin" },
    { "offset": "0xe000",   "name": "boot_app0.bin"  },
    { "offset": "0x10000",  "name": "firmware.bin"   },
    { "offset": "0x290000", "name": "littlefs.bin"   }
  ]
}
```

`offset` accepts decimal or `0x...` hex.

## Producing the bin files from PlatformIO

```powershell
# in ESP32/
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e sr6_pcb
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -t buildfs -e sr6_pcb

# Copy outputs:
Copy-Item .pio/build/sr6_pcb/bootloader.bin   flasher/python/firmware/sr6_pcb/
Copy-Item .pio/build/sr6_pcb/partitions.bin   flasher/python/firmware/sr6_pcb/
Copy-Item .pio/build/sr6_pcb/firmware.bin     flasher/python/firmware/sr6_pcb/
Copy-Item .pio/build/sr6_pcb/littlefs.bin     flasher/python/firmware/sr6_pcb/
Copy-Item ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin `
          flasher/python/firmware/sr6_pcb/
```

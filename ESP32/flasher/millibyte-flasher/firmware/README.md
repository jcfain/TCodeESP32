Firmware payload directory.

Each subfolder is a board id (e.g. `sr6_pcb`, `ssr1_pcb`). At release time
the CI pipeline copies the four artefacts produced by `pio run` (and `pio run
-t buildfs`) into the matching folder:

    sr6_pcb/
        bootloader.bin
        partitions.bin
        boot_app0.bin
        firmware.bin
        littlefs.bin
        manifest.json   <- offsets, chip, version

The flasher reads `firmware/<board>/manifest.json` next to the executable and
streams every payload at the declared offset.

A template `sr6_pcb/manifest.json` is checked in; the .bin files are NOT
checked in (see `../.gitignore`).

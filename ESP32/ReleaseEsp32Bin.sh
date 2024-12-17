#!/bin/bash

# This script requires the following:
# ESPTool both windows and linux versions
# https://github.com/espressif/esptool
# 7Zip linux only
# https://www.7-zip.org/download.html

version="UNKNOWN"
BUILD_MODIFIER="esp32doit-devkit-v1"
MODULE_TYPE=0
ESPTOOLDIR="./bin/esptool-linux-amd64/"
ESPTOOLDIRWIN="./bin/esptool-win64/"
esp32ArduinoCoredir="framework-arduinoespressif32@src-a01c93a63f3ed4184a2ede3960108545"

echo "Enter a version (example: 0.255b):"
read version

# echo 1 - ESP32 Devkit V1 Base
# echo 2 - ESP32 Devkit V1 bldc
# echo 3 - ESP32 Devkit V4 Base
# echo 4 - ESP32 Devkit V4 bldc
# echo 5 - S3 ZERO Base
# echo 6 - S3 ZERO bldc
# echo 7 - S3 DevkitC 1 N8R8 Base
# echo 8 - S3 DevkitC 1 N8R8 bldc
# # echo 9 - Devkit V1 Debug
# echo 9 - EXIT
# echo.

function printMenu
{
	echo
	echo
	echo "-----------------------------------------------"
	echo "Select the build to deploy, or 9 to EXIT."
	echo "-----------------------------------------------"
	echo
	local builds=(
		"esp32doit-devkit-v1" 
		"esp32doit-devkit-v1-bldc" 
		"esp32-devkit-v4" 
		"esp32-devkit-v4-bldc" 
		"esp32-s3-zero" 
		"esp32-s3-zero-bldc" 
		"esp32-s3-devkitc-1-N8R8" 
		"esp32-s3-devkitc-1-N8R8-bldc" 
		"EXIT"
	)
	select build in "${builds[@]}"

	do
	  case $build in
		"esp32doit-devkit-v1")
			BUILD_MODIFIER=$build
			MODULE_TYPE=0
			buildAndDeploy
			break
		  ;;
		"esp32doit-devkit-v1-bldc")
			BUILD_MODIFIER=$build
			MODULE_TYPE=0
			buildAndDeploy
			break
		  ;;
		"esp32-devkit-v4")
			BUILD_MODIFIER=$build
			MODULE_TYPE=0
			buildAndDeploy
			break
		  ;;
		"esp32-devkit-v4-bldc")
			BUILD_MODIFIER=$build
			MODULE_TYPE=0
			buildAndDeploy
			break
		  ;;
		"esp32-s3-zero")
			BUILD_MODIFIER=$build
			MODULE_TYPE=1
			buildAndDeploy
			break
		  ;;
		"esp32-s3-zero-bldc")
			BUILD_MODIFIER=$build
			MODULE_TYPE=1
			buildAndDeploy
			break
		  ;;
		"esp32-s3-devkitc-1-N8R8")
			BUILD_MODIFIER=$build
			MODULE_TYPE=2
			buildAndDeploy
			break
		  ;;
		"esp32-s3-devkitc-1-N8R8-bldc")
			BUILD_MODIFIER=$build
			MODULE_TYPE=2
			buildAndDeploy
			break
		  ;;
		"EXIT")
		  echo "EXIT"
		  exit
		  ;;
		*) 
		  echo "Invalid choice"
			break
		  ;;
	  esac
	done
}

function buildAndDeploy
{
	local BUILD_DIR="$(pwd)/.pio/build/$BUILD_MODIFIER"
	local OUTDIR="$(pwd)/bin/Release-$BUILD_MODIFIER"
	mkdir -p $OUTDIR
	local releaseZip="$(pwd)/bin/TCode_ESP32_Release_v$version-$BUILD_MODIFIER.zip"
	
	~/.platformio/penv/bin/platformio run --environment $BUILD_MODIFIER
	if [ $? -ne 0 ]; then 
		echo "Error building firmware"
		return
	fi
	~/.platformio/penv/bin/platformio run --target buildfs --environment $BUILD_MODIFIER
	if [ $? -ne 0 ]; then
		echo "Error building filesystem"
		return
	fi
	cp -u "../How to upload binaries.pdf" "$OUTDIR/How to upload binaries.pdf"
	cp -u "./command example.txt" "$OUTDIR/command example.txt"
	cp -u "./Flash.exe" "$OUTDIR/flash.exe"
	mkdir -p "$OUTDIR/esptool/linux/esptool"
	cp -u "$ESPTOOLDIR/esptool" "$OUTDIR/esptool/linux/esptool"
	cp -u "$ESPTOOLDIR/LICENSE" "$OUTDIR/esptool/linux/LICENSE"
	cp -u "$ESPTOOLDIR/README.md" "$OUTDIR/esptool/linux/README.md"
	cp -u "$ESPTOOLDIRWIN/esptool.exe" "$OUTDIR/esptool/esptool.exe"
	cp -u "$ESPTOOLDIRWIN/LICENSE" "$OUTDIR/esptool/LICENSE"
	cp -u "$ESPTOOLDIRWIN/README.md" "$OUTDIR/esptool/README.md"
	if [ $? -ne 0 ]; then 
		echo "Error copying files"
		return
	fi
	# ESP32
	if [ $MODULE_TYPE -eq 0 ]; then
		$ESPTOOLDIR/esptool --chip esp32 merge_bin -o $OUTDIR/release.bin --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 $BUILD_DIR/bootloader.bin 0x8000 $BUILD_DIR/partitions.bin 0xe000 ~/.platformio/packages/$esp32ArduinoCoredir/tools/partitions/boot_app0.bin 0x10000 $BUILD_DIR/firmware.bin 0x310000 $BUILD_DIR/littlefs.bin
	
	# S3 ZERO
	elif [ $MODULE_TYPE -eq 1 ]; then
		$ESPTOOLDIR/esptool --chip esp32 merge_bin -o $OUTDIR/release.bin --flash_mode qio --flash_freq 80m --flash_size 4MB 0x1000 $BUILD_DIR/bootloader.bin 0x8000 $BUILD_DIR/partitions.bin 0xe000 ~/.platformio/packages/$esp32ArduinoCoredir/tools/partitions/boot_app0.bin 0x10000 $BUILD_DIR/firmware.bin 0x310000 $BUILD_DIR/littlefs.bin
	
	# S3 N8 8mb
	elif [ $MODULE_TYPE -eq 2 ]; then
		$ESPTOOLDIR/esptool --chip esp32 merge_bin -o $OUTDIR/release.bin --flash_mode qio --flash_freq 80m --flash_size 8MB 0x0000 $BUILD_DIR/bootloader.bin 0x8000 $BUILD_DIR/partitions.bin 0xe000 ~/.platformio/packages/$esp32ArduinoCoredir/tools/partitions/boot_app0.bin 0x10000 $BUILD_DIR/firmware.bin 0x00670000 $BUILD_DIR/littlefs.bin
	fi
	if [ $? -ne 0 ]; then 
		echo "Error merging bin"
		return
	fi

    rm -f "$releaseZip"

	"7z" a "$releaseZip" "$OUTDIR"
}

while true
do
	printMenu
done


# "~/.platformio/penv/Scripts/python" 
# "~/.platformio/packages/tool-esptoolpy/esptool.py" 
# --chip esp32s3 --port "COM14" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB 
# 0x0000 ~/git/TCodeESP32/ESP32/.pio/build/esp32-s3-devkitc-1-N8R8/bootloader.bin 
# 0x8000 ~/git/TCodeESP32/ESP32/.pio/build/esp32-s3-devkitc-1-N8R8/partitions.bin 
# 0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin 
# 0x10000 .pio/build/esp32-s3-devkitc-1-N8R8/firmware.bin
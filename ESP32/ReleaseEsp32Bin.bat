REM Requires
REM https://github.com/espressif/esptool
REM https://www.7-zip.org/download.html

@ECHO OFF

SET /P version=Enter a version (example: 0.255b):
IF NOT DEFINED version SET "version=UNKNOWN"

ECHO.
ECHO.
rem ECHO. [43mWARNING: This will NOT build the project.[0m
rem ECHO. [43mYou need to do that manually in platformIO first[0m
ECHO ...............................................
ECHO Select the build to deploy, or 9 to EXIT.
ECHO ...............................................
ECHO.

:MENU

ECHO 1 - ESP32 Devkit V1 Base
ECHO 2 - ESP32 Devkit V1 bldc
ECHO 3 - ESP32 Devkit V4 Base
ECHO 4 - ESP32 Devkit V4 bldc
ECHO 5 - S3 ZERO Base
ECHO 6 - S3 ZERO bldc
ECHO 7 - S3 DevkitC 1 N8R8 Base
ECHO 8 - S3 DevkitC 1 N8R8 bldc
rem ECHO 9 - Devkit V1 Debug
ECHO 9 - EXIT
ECHO.

SET /P M=Build (1):
IF NOT DEFINED M SET "M=1"

IF %M%==1 (
	SET BUILD_MODIFIER=esp32doit-devkit-v1
	SET MODULE_TYPE=0
)
IF %M%==2 (
	SET BUILD_MODIFIER=esp32doit-devkit-v1-bldc
	SET MODULE_TYPE=0
)
IF %M%==3 (
	SET BUILD_MODIFIER=esp32-devkit-v4
	SET MODULE_TYPE=0
)
IF %M%==4 (
	SET BUILD_MODIFIER=esp32-devkit-v4-bldc
	SET MODULE_TYPE=0
)
IF %M%==5 (
	SET BUILD_MODIFIER=esp32-s3-zero
	SET MODULE_TYPE=1
)
IF %M%==6 (
	SET BUILD_MODIFIER=esp32-s3-zero-bldc
	SET MODULE_TYPE=1
)
IF %M%==7 (
	SET BUILD_MODIFIER=esp32-s3-devkitc-1-N8R8
	SET MODULE_TYPE=2
)
IF %M%==8 (
	SET BUILD_MODIFIER=esp32-s3-devkitc-1-N8R8-bldc
	SET MODULE_TYPE=2
)
rem IF %M%==9 SET BUILD_MODIFIER=-esp32doit-devkit-v1-debug
IF %M%==9 GOTO EOF

SET ESPTOOLDIR="%~dp0bin\esptool-v4.8.1-win64\"

SET BUILD_DIR=.pio\build\%BUILD_MODIFIER%
SET OUTDIR=%~dp0bin\Release-%BUILD_MODIFIER%
SET esp32ArduinoCoredir=framework-arduinoespressif32

%userprofile%\.platformio\penv\Scripts\platformio.exe run --environment %BUILD_MODIFIER%
if %ERRORLEVEL% NEQ 0  ( 
	echo Error building firmware
	GOTO MENU 
)
platformio.exe run --target buildfs --environment %BUILD_MODIFIER%
if %ERRORLEVEL% NEQ 0  ( 
	echo Error building filesystem
	GOTO MENU 
)
XCOPY "..\How to upload binaries.pdf" "%OUTDIR%\How to upload binaries.pdf" /d /Y
XCOPY "%~dp0command example.txt" "%OUTDIR%\command example.txt" /d /Y
XCOPY "%~dp0Flash.exe" "%OUTDIR%\flash.exe" /d /Y
XCOPY %ESPTOOLDIR%esptool.exe "%OUTDIR%\esptool\esptool.exe" /d /Y
XCOPY %ESPTOOLDIR%LICENSE "%OUTDIR%\esptool\LICENSE" /d /Y
XCOPY %ESPTOOLDIR%README.md "%OUTDIR%\esptool\README.md" /d /Y
if %ERRORLEVEL% NEQ 0  ( 
	echo Error copying files
	GOTO MENU 
)
REM ESP32
If %MODULE_TYPE%==0 (
	%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%OUTDIR%\release.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x0000 %BUILD_DIR%\bootloader.bin 0x8000 %BUILD_DIR%\partitions.bin 0xe000 "%userprofile%\.platformio\packages\%esp32ArduinoCoredir%\tools\partitions\boot_app0.bin" 0x10000 %BUILD_DIR%\firmware.bin 0x310000 %BUILD_DIR%\littlefs.bin
)
REM S3 ZERO
If %MODULE_TYPE%==1 (
	%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%OUTDIR%\release.bin" --flash_mode qio --flash_freq 80m --flash_size 4MB 0x0000 %BUILD_DIR%\bootloader.bin 0x8000 %BUILD_DIR%\partitions.bin 0xe000 "%userprofile%\.platformio\packages\%esp32ArduinoCoredir%\tools\partitions\boot_app0.bin" 0x10000 %BUILD_DIR%\firmware.bin 0x310000 %BUILD_DIR%\littlefs.bin
)
REM S3 N8 8mb
If %MODULE_TYPE%==2 (
	%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%OUTDIR%\release.bin" --flash_mode qio --flash_freq 80m --flash_size 8MB 0x0000 %BUILD_DIR%\bootloader.bin 0x8000 %BUILD_DIR%\partitions.bin 0xe000 "%userprofile%\.platformio\packages\%esp32ArduinoCoredir%\tools\partitions\boot_app0.bin" 0x10000 %BUILD_DIR%\firmware.bin 0x00670000 %BUILD_DIR%\littlefs.bin
)

SET releaseZip="%~dp0bin\TCode_ESP32_Release_v%version%-%BUILD_MODIFIER%.zip"

IF EXIST %releaseZip% DEL /F %releaseZip%

"C:\Program Files\7-Zip\7z" a "%releaseZip%" "%OUTDIR%"

GOTO MENU


REM "%userprofile%.platformio\penv\Scripts\python.exe" 
REM "%userprofile%.platformio\packages\tool-esptoolpy\esptool.py" 
REM --chip esp32s3 --port "COM14" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB 
REM 0x0000 %userprofile%git\TCodeESP32\ESP32\.pio\build\esp32-s3-devkitc-1-N8R8\bootloader.bin 
REM 0x8000 C%userprofile%git\TCodeESP32\ESP32\.pio\build\esp32-s3-devkitc-1-N8R8\partitions.bin 
REM 0xe000 %userprofile%.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin 
REM 0x10000 .pio\build\esp32-s3-devkitc-1-N8R8\firmware.bin
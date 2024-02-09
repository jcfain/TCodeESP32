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

ECHO 1 - Base
ECHO 2 - bldc
ECHO 3 - Display
ECHO 4 - Temp
ECHO 5 - BlueTooth
ECHO 6 - Debug
ECHO 7 - EXIT
ECHO.

SET /P M=Build (1):
IF NOT DEFINED M SET "M=1"

IF %M%==1 SET BUILD_MODIFIER=
IF %M%==2 SET BUILD_MODIFIER=-bldc
IF %M%==3 SET BUILD_MODIFIER=-display
IF %M%==4 SET BUILD_MODIFIER=-temperature
IF %M%==5 SET BUILD_MODIFIER=-bluetooth
IF %M%==6 SET BUILD_MODIFIER=-debug
IF %M%==7 GOTO EOF

SET ESPTOOLDIR="%~dp0bin\esptool-v4.5.1-win64\"

SET BUILD=esp32doit-devkit-v1%BUILD_MODIFIER%
SET OUTDIR=%~dp0bin\Release%BUILD_MODIFIER%

%userprofile%\.platformio\penv\Scripts\platformio.exe run --environment %BUILD%
if %ERRORLEVEL% NEQ 0  ( 
	echo Error building firmware
	GOTO MENU 
)
platformio.exe run --target buildfs --environment %BUILD%
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


%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%OUTDIR%\release.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 .pio\build\%BUILD%\bootloader.bin 0x8000 .pio\build\%BUILD%\partitions.bin 0xe000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin" 0x10000 .pio\build\%BUILD%\firmware.bin 0x310000 .pio\build\%BUILD%\spiffs.bin

SET releaseZip="%~dp0bin\TCode_ESP32_Release_v%version%%BUILD_MODIFIER%.zip"

IF EXIST %releaseZip% DEL /F %releaseZip%

"C:\Program Files\7-Zip\7z" a "%releaseZip%" "%OUTDIR%"

GOTO MENU
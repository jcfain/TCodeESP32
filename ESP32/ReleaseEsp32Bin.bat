REM Requires
REM https://github.com/espressif/esptool
REM https://www.7-zip.org/download.html

@ECHO OFF

SET /P version=Enter a version (example: 0.255b):
IF NOT DEFINED version SET "version=UNKNOWN"

ECHO.
ECHO.
ECHO. [43mWARNING: This will NOT build the project.[0m
ECHO. [43mYou need to do that manually in platformIO first[0m
ECHO ...............................................
ECHO Select the build to deploy, or 9 to EXIT.
ECHO ...............................................
ECHO.

:MENU

ECHO 1 - Base
ECHO 2 - Display Temp
ECHO 3 - Crimzzon
ECHO 4 - Isaac
ECHO 5 - Display
ECHO 6 - Temp
ECHO 7 - BlueTooth
ECHO 8 - Debug
ECHO 9 - EXIT
ECHO.

SET /P M=Build (1):
IF NOT DEFINED M SET "M=1"

IF %M%==1 SET BUILD_MODIFIER=
IF %M%==2 SET BUILD_MODIFIER=-display-temp
IF %M%==3 SET BUILD_MODIFIER=-crimzzon
IF %M%==4 SET BUILD_MODIFIER=-isaac
IF %M%==5 SET BUILD_MODIFIER=-display
IF %M%==6 SET BUILD_MODIFIER=-temperature
IF %M%==7 SET BUILD_MODIFIER=-bluetooth
IF %M%==8 SET BUILD_MODIFIER=-debug
IF %M%==9 GOTO EOF

SET ESPTOOLDIR="%~dp0bin\esptool-v3.3.2-win64\"

SET BUILD=esp32doit-devkit-v1%BUILD_MODIFIER%
SET OUTDIR=%~dp0bin\Release%BUILD_MODIFIER%

XCOPY "..\How to upload binaries.pdf" "%OUTDIR%\How to upload binaries.pdf" /d /Y
XCOPY "%~dp0flash.bat" "%OUTDIR%\flash.bat" /d /Y
XCOPY %ESPTOOLDIR%esptool.exe "%OUTDIR%\esptool\esptool.exe" /d /Y
XCOPY %ESPTOOLDIR%LICENSE "%OUTDIR%\esptool\LICENSE" /d /Y
XCOPY %ESPTOOLDIR%README.md "%OUTDIR%\esptool\README.md" /d /Y

%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%OUTDIR%\release.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dio_40m.bin" 0x8000 .pio\build\%BUILD%\partitions.bin 0x10000 .pio\build\%BUILD%\firmware.bin 0xe000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin" 0x003d0000 .pio\build\%BUILD%\spiffs.bin

SET releaseZip="%~dp0bin\TCode_ESP32_Release_v%version%%BUILD_MODIFIER%.zip"

IF EXIST %releaseZip% DEL /F %releaseZip%

"C:\Program Files\7-Zip\7z" a "%releaseZip%" "%OUTDIR%"

GOTO MENU
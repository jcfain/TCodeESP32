REM Requires
REM https://github.com/espressif/esptool
REM https://www.7-zip.org/download.html

@ECHO OFF

SET /P version=Enter a version (default: UNKNOWN):
IF NOT DEFINED version SET "version=UNKNOWN"

SET ESPTOOLDIR="%~dp0bin\esptool-v3.3.2-win64\"

XCOPY "..\How to upload binaries.pdf" "%~dp0bin\Release\How to upload binaries.pdf" /d /Y
XCOPY ".\flash.bat" "%~dp0bin\Release\flash.bat" /d /Y
XCOPY %ESPTOOLDIR%esptool.exe "%~dp0bin\Release\esptool\esptool.exe" /d /Y
XCOPY %ESPTOOLDIR%LICENSE "%~dp0bin\Release\esptool\LICENSE" /d /Y
XCOPY %ESPTOOLDIR%README.md "%~dp0bin\Release\esptool\README.md" /d /Y

XCOPY "..\How to upload binaries.pdf" "%~dp0bin\Release-display-temp\How to upload binaries.pdf" /d /Y
XCOPY ".\flash.bat" "%~dp0bin\Release-display-temp\flash.bat" /d /Y
XCOPY %ESPTOOLDIR%esptool.exe "%~dp0bin\Release-display-temp\esptool\esptool.exe" /d /Y
XCOPY %ESPTOOLDIR%LICENSE "%~dp0bin\Release-display-temp\esptool\LICENSE" /d /Y
XCOPY %ESPTOOLDIR%README.md "%~dp0bin\Release-display-temp\esptool\README.md" /d /Y

%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%~dp0bin\Release\release.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dio_40m.bin" 0x8000 .pio\build\esp32doit-devkit-v1\partitions.bin 0x10000 .pio\build\esp32doit-devkit-v1\firmware.bin 0xe000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin" 0x003d0000 .pio\build\esp32doit-devkit-v1\spiffs.bin
%ESPTOOLDIR%esptool.exe --chip esp32 merge_bin -o "%~dp0bin\Release-display-temp\release.bin" --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dio_40m.bin" 0x8000 .pio\build\esp32doit-devkit-v1-display-temp\partitions.bin 0x10000 .pio\build\esp32doit-devkit-v1-display-temp\firmware.bin 0xe000 "%userprofile%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin" 0x003d0000 .pio\build\esp32doit-devkit-v1-display-temp\spiffs.bin

SET releaseZip="%~dp0bin\TCode_ESP32_Release_%version%.zip"
SET releaseDisplayTempZip="%~dp0bin\TCode_ESP32_Release_%version%-display-temp.zip"

IF EXIST %releaseZip% DEL /F %releaseZip%
IF EXIST %releaseDisplayTempZip% DEL /F %releaseDisplayTempZip%

"C:\Program Files\7-Zip\7z" a "%releaseZip%" "%~dp0bin\Release"
"C:\Program Files\7-Zip\7z" a "%releaseDisplayTempZip%" "%~dp0bin\Release-display-temp"

pause
REM 0x1000
echo F|XCOPY %userprofile%\.platformio\packages\framework-arduinoespressif32\tools\sdk\bin\bootloader_dio_40m.bin %~dp0bin\Release\a_0x1000.bin /d
REM 0x8000
echo F|XCOPY .pio\build\esp32doit-devkit-v1\partitions.bin %~dp0bin\Release\b_0x8000.bin /d
REM 0x10000 
echo F|XCOPY .pio\build\esp32doit-devkit-v1\firmware.bin %~dp0bin\Release\c_0x10000.bin /d
REM 0xe000 
echo F|XCOPY %userprofile%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin %~dp0bin\Release\d_0xe000.bin /d
REM 0x003d0000
echo F|XCOPY .pio\build\esp32doit-devkit-v1\spiffs.bin %~dp0bin\Release\e_0x003d0000.bin /d

echo F|XCOPY "..\How to upload binaries.pdf" "%~dp0bin\Release\How to upload binaries.pdf" /d

pause
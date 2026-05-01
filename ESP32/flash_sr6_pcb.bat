@echo off
setlocal

if "%~1"=="--skip-wifi" goto run_skip_wifi
if "%~1"=="" (
    echo Usage: %~nx0 ^<ssid^> ^<password^> [COMx]
    echo        %~nx0 --skip-wifi [COMx]
    echo Example: %~nx0 MyWifi MyPass123 COM7
    exit /b 1
)

set "SSID=%~1"
set "PASSWORD=%~2"
set "PORT=%~3"

if "%PASSWORD%"=="" (
    echo Usage: %~nx0 ^<ssid^> ^<password^> [COMx]
    echo        %~nx0 --skip-wifi [COMx]
    exit /b 1
)

if "%PORT%"=="" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0flash_sr6_pcb.ps1" -Ssid "%SSID%" -Password "%PASSWORD%"
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0flash_sr6_pcb.ps1" -Ssid "%SSID%" -Password "%PASSWORD%" -Port "%PORT%"
)
goto :eof

:run_skip_wifi
set "PORT=%~2"
if "%PORT%"=="" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0flash_sr6_pcb.ps1" -SkipWifi
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0flash_sr6_pcb.ps1" -SkipWifi -Port "%PORT%"
)

endlocal

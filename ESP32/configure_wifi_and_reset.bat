@echo off
setlocal

if "%~1"=="" (
    echo Usage: %~nx0 ^<ssid^> ^<password^> [COMx]
    echo Example: %~nx0 MyWifi MyPass123 COM7
    exit /b 1
)

set "SSID=%~1"
set "PASSWORD=%~2"
set "PORT=%~3"

if "%PASSWORD%"=="" (
    echo Usage: %~nx0 ^<ssid^> ^<password^> [COMx]
    exit /b 1
)

if "%PORT%"=="" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0configure_wifi_and_reset.ps1" -Ssid "%SSID%" -Password "%PASSWORD%"
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0configure_wifi_and_reset.ps1" -Ssid "%SSID%" -Password "%PASSWORD%" -Port "%PORT%"
)

endlocal

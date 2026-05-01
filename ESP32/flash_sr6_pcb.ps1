<#
.SYNOPSIS
    One-stop flasher for the sr6_pcb (ESP32-S3) target: builds firmware +
    filesystem, uploads both, then configures Wi-Fi via the serial console.

.DESCRIPTION
    Steps performed (in order):
        1. Build firmware (platformio run -e sr6_pcb)
        2. Build filesystem image (platformio run -t buildfs -e sr6_pcb)
        3. Upload firmware (platformio run -t upload -e sr6_pcb)
        4. Upload filesystem (platformio run -t uploadfs -e sr6_pcb)
        5. Run configure_wifi_and_reset.ps1 with the supplied SSID/password

    Pass -SkipBuild to flash already-built artifacts without recompiling.
    Pass -SkipWifi to flash only and skip the Wi-Fi configuration step.
    Pass -SkipFs to skip the filesystem build/upload steps.

.PARAMETER Ssid
    Wi-Fi SSID to configure on the device.

.PARAMETER Password
    Wi-Fi password.

.PARAMETER Port
    Optional COM port (e.g. COM7). When omitted, platformio and the Wi-Fi
    configurator each auto-detect.

.PARAMETER SkipBuild
    Skip the firmware/filesystem build steps (useful when iterating on
    Wi-Fi config only).

.PARAMETER SkipWifi
    Skip the Wi-Fi configuration step (just flash). When set, Ssid/Password
    are not required.

.PARAMETER SkipFs
    Skip the filesystem build and upload (firmware-only flash).

.EXAMPLE
    .\flash_sr6_pcb.ps1 -Ssid MyWifi -Password 'secret'

.EXAMPLE
    .\flash_sr6_pcb.ps1 -Ssid MyWifi -Password 'secret' -Port COM7

.EXAMPLE
    .\flash_sr6_pcb.ps1 -SkipWifi
#>
[CmdletBinding()]
param(
    [string]$Ssid,
    [string]$Password,
    [string]$Port,
    [switch]$SkipBuild,
    [switch]$SkipWifi,
    [switch]$SkipFs
)

$ErrorActionPreference = "Stop"

if (-not $SkipWifi) {
    if (-not $Ssid -or -not $Password) {
        throw "Ssid and Password are required unless -SkipWifi is set. Use -? for help."
    }
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

$pio = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\platformio.exe"
if (-not (Test-Path $pio)) {
    throw "PlatformIO CLI not found at $pio. Install PlatformIO or update this script."
}

$envName = "sr6_pcb"

function Invoke-Pio {
    param(
        [string[]]$PioArgs,
        [string]$StepName
    )

    Write-Host ""
    Write-Host "=== $StepName ===" -ForegroundColor Cyan
    Write-Host "  $pio $($PioArgs -join ' ')" -ForegroundColor DarkGray
    & $pio @PioArgs
    if ($LASTEXITCODE -ne 0) {
        throw "$StepName failed with exit code $LASTEXITCODE"
    }
}

# 1) Build firmware
if (-not $SkipBuild) {
    Invoke-Pio -StepName "Build firmware ($envName)" -PioArgs @("run", "-e", $envName)
} else {
    Write-Host "[skip] firmware build" -ForegroundColor Yellow
}

# 2) Build filesystem
if (-not $SkipBuild -and -not $SkipFs) {
    Invoke-Pio -StepName "Build filesystem image ($envName)" -PioArgs @("run", "-t", "buildfs", "-e", $envName)
} else {
    Write-Host "[skip] filesystem build" -ForegroundColor Yellow
}

# Build upload args (optional --upload-port).
$uploadCommon = @()
if ($Port) {
    $uploadCommon += @("--upload-port", $Port)
}

# 3) Upload firmware
$uploadArgs = @("run", "-t", "upload", "-e", $envName) + $uploadCommon
Invoke-Pio -StepName "Upload firmware" -PioArgs $uploadArgs

# 4) Upload filesystem
if (-not $SkipFs) {
    $uploadFsArgs = @("run", "-t", "uploadfs", "-e", $envName) + $uploadCommon
    Invoke-Pio -StepName "Upload filesystem image" -PioArgs $uploadFsArgs
} else {
    Write-Host "[skip] filesystem upload" -ForegroundColor Yellow
}

# 5) Configure Wi-Fi
if ($SkipWifi) {
    Write-Host ""
    Write-Host "[skip] Wi-Fi configuration" -ForegroundColor Yellow
    Write-Host "Done. Firmware flashed. Configure Wi-Fi manually with configure_wifi_and_reset.ps1." -ForegroundColor Green
    exit 0
}

# Give the device a moment to finish rebooting after the upload step before
# the serial port is reopened by the Wi-Fi configurator.
Start-Sleep -Seconds 2

$wifiScript = Join-Path $scriptDir "configure_wifi_and_reset.ps1"
if (-not (Test-Path $wifiScript)) {
    throw "configure_wifi_and_reset.ps1 not found next to this script."
}

Write-Host ""
Write-Host "=== Configure Wi-Fi ===" -ForegroundColor Cyan
$wifiArgs = @{ Ssid = $Ssid; Password = $Password }
if ($Port) { $wifiArgs.Port = $Port }
& $wifiScript @wifiArgs

Write-Host ""
Write-Host "All steps complete." -ForegroundColor Green

param(
    [Parameter(Mandatory = $true)]
    [string]$Ssid,

    [Parameter(Mandatory = $true)]
    [string]$Password,

    [string]$Port,

    [int]$Baud = 115200,

    [int]$CommandResponseTimeoutSeconds = 5,

    [int]$IpReadTimeoutSeconds = 30
)

$ErrorActionPreference = "Continue"

function Get-AutoPort {
    $available = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object
    if (-not $available -or $available.Count -eq 0) {
        throw "No serial ports found."
    }

    $preferred = @()
    try {
        $deviceRows = Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match "\(COM\d+\)" }
        foreach ($row in $deviceRows) {
            if ($row.Name -match "\((COM\d+)\)") {
                $com = $Matches[1]
                if (
                    $row.Name -match "ESP32|USB|UART|CP210|CH340|CH910|Silicon Labs|FTDI" -and
                    $available -contains $com
                ) {
                    $preferred += $com
                }
            }
        }
    }
    catch {
        # Fallback to first port when CIM query is unavailable.
    }

    if ($preferred.Count -gt 0) {
        return $preferred[0]
    }

    return $available[0]
}

if (-not $Port) {
    $Port = Get-AutoPort
}

Write-Host "Using serial port: $Port"
Write-Host "Baud: $Baud"

$serial = New-Object System.IO.Ports.SerialPort $Port, $Baud, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
$serial.NewLine = "`n"
$serial.ReadTimeout = 300
$serial.WriteTimeout = 1000
$serial.DtrEnable = $false
$serial.RtsEnable = $false

function Read-Until {
    param(
        [int]$TimeoutSeconds,
        [string[]]$SuccessPatterns,
        [string[]]$FailurePatterns
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    $lines = New-Object System.Collections.Generic.List[string]

    while ((Get-Date) -lt $deadline) {
        try {
            $line = $serial.ReadLine()
            if ($null -eq $line) {
                continue
            }

            $line = $line.Trim()
            if ($line.Length -eq 0) {
                continue
            }

            Write-Host "< $line"
            $lines.Add($line)

            foreach ($failPattern in $FailurePatterns) {
                if ($line -match $failPattern) {
                    throw "Device reported an error: $line"
                }
            }

            foreach ($successPattern in $SuccessPatterns) {
                if ($line -match $successPattern) {
                    return @{ Matched = $true; Lines = $lines }
                }
            }
        }
        catch [System.TimeoutException] {
            Start-Sleep -Milliseconds 80
        }
    }

    return @{ Matched = $false; Lines = $lines }
}

function Send-And-Validate {
    param(
        [string]$Command,
        [string]$Display,
        [string[]]$SuccessPatterns,
        [switch]$RequireMatch
    )

    $failPatterns = @(
        "Unknown command",
        "Unknown save command",
        "Invalid command",
        "Invalid value"
    )

    Write-Host "Sending: $Display"
    $serial.WriteLine($Command)

    $result = Read-Until -TimeoutSeconds $CommandResponseTimeoutSeconds -SuccessPatterns $SuccessPatterns -FailurePatterns $failPatterns

    if ($RequireMatch -and -not $result.Matched) {
        throw "Did not receive expected confirmation for command: $Display"
    }
}

function Read-DeviceIp {
    $ipPatterns = @(
        "IP\s*Address:\s*([0-9]{1,3}(\.[0-9]{1,3}){3})",
        "WiFi\s*connected:\s*([0-9]{1,3}(\.[0-9]{1,3}){3})",
        "Captive\s*portal\s*IP:\s*([0-9]{1,3}(\.[0-9]{1,3}){3})"
    )

    $deadline = (Get-Date).AddSeconds($IpReadTimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        Write-Host "Sending: #ip"
        $serial.WriteLine("#ip")

        $result = Read-Until -TimeoutSeconds 2 -SuccessPatterns $ipPatterns -FailurePatterns @()
        if ($result.Matched) {
            foreach ($line in $result.Lines) {
                foreach ($pattern in $ipPatterns) {
                    if ($line -match $pattern) {
                        $candidate = $Matches[1]
                        if ($candidate -and $candidate -ne "0.0.0.0") {
                            return $candidate
                        }
                    }
                }
            }
        }

        Start-Sleep -Milliseconds 500
    }

    throw "Did not detect device IP via #ip query within timeout."
}

try {
    $serial.Open()
    Start-Sleep -Milliseconds 500

    # Drain boot noise before issuing commands.
    $null = Read-Until -TimeoutSeconds 1 -SuccessPatterns @() -FailurePatterns @()

    Send-And-Validate -Command "#wifi-ssid:$Ssid" -Display "#wifi-ssid:<ssid>" -SuccessPatterns @("Wifi SSID changed to:", "Restart is required after save") -RequireMatch
    Send-And-Validate -Command "#wifi-pass:$Password" -Display "#wifi-pass:<password>" -SuccessPatterns @("Wifi password changed to a value of", "Restart is required after save") -RequireMatch
    Send-And-Validate -Command '$save' -Display '$save' -SuccessPatterns @("Settings saved!") -RequireMatch
    Send-And-Validate -Command "#restart" -Display "#restart" -SuccessPatterns @()

    Write-Host "Waiting for device to reboot and report Wi-Fi IP..."
    $ip = Read-DeviceIp
    Write-Host "Device IP: $ip"
    Write-Host "Done. Validation passed and device restart completed."
}
finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
    $serial.Dispose()
}

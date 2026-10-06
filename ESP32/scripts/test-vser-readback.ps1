param(
    [string]$Host_ = '192.168.2.194'
)
function Connect-Ws {
    param($host_, $seconds)
    $ws = New-Object System.Net.WebSockets.ClientWebSocket
    $cts = New-Object System.Threading.CancellationTokenSource
    $cts.CancelAfter(($seconds + 5) * 1000)
    $uri = [System.Uri]("ws://$host_/ws")
    $ws.ConnectAsync($uri, $cts.Token).Wait()
    return @{ Ws = $ws; Cts = $cts }
}

function Send-Cmd($conn, $cmd, $msg) {
    $payload = '{"command":"' + $cmd + '","message":"' + $msg + '"}'
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($payload)
    $seg = [System.ArraySegment[byte]]::new($bytes)
    $conn.Ws.SendAsync($seg, [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $conn.Cts.Token).Wait()
}

function Read-PowerStatus($host_, $sampleSeconds = 3) {
    $conn = Connect-Ws $host_ $sampleSeconds
    $deadline = (Get-Date).AddSeconds($sampleSeconds)
    $buf = New-Object byte[] 8192
    while ((Get-Date) -lt $deadline) {
        $seg = [System.ArraySegment[byte]]::new($buf)
        $task = $conn.Ws.ReceiveAsync($seg, $conn.Cts.Token)
        $remainingMs = [int]((($deadline - (Get-Date)).TotalMilliseconds))
        if ($remainingMs -le 0) { break }
        if (-not $task.Wait($remainingMs)) { break }
        $r = $task.Result
        if ($r.Count -gt 0) {
            $text = [System.Text.Encoding]::UTF8.GetString($buf, 0, $r.Count)
            if ($text -match 'powerStatus') {
                if ($text -match '"Voltage_Motor":\{[^}]*"railVoltage":([\d\.]+)') { $vmotor = [double]$Matches[1] }
                if ($text -match '"Voltage_Bus":\{[^}]*"railVoltage":([\d\.]+)') { $vbus = [double]$Matches[1] }
                if ($text -match '"servoVoltageEnabled":(true|false)') { $en = $Matches[1] }
                try { $conn.Ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $conn.Cts.Token).Wait(1000) } catch {}
                return [PSCustomObject]@{ VBus = $vbus; VMotor = $vmotor; ServoVoltageEnabled = $en }
            }
        }
    }
    try { $conn.Ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $conn.Cts.Token).Wait(1000) } catch {}
    return $null
}

function Set-Vser($host_, $value) {
    $conn = Connect-Ws $host_ 3
    Send-Cmd $conn 'setServoVoltageEnabled' ($value.ToString().ToLower())
    Start-Sleep -Milliseconds 300
    try { $conn.Ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $conn.Cts.Token).Wait(1000) } catch {}
}

Write-Host "=== Initial reading ==="
$r0 = Read-PowerStatus -host_ $Host_; $r0 | Format-List

Write-Host "`n=== Disabling VSER ==="
Set-Vser -host_ $Host_ -value $false
Start-Sleep -Seconds 1
$r1 = Read-PowerStatus -host_ $Host_; $r1 | Format-List

Write-Host "`n=== Enabling VSER ==="
Set-Vser -host_ $Host_ -value $true
Start-Sleep -Seconds 1
$r2 = Read-PowerStatus -host_ $Host_; $r2 | Format-List

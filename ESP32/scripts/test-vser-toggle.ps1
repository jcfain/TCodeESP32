param(
    [string]$Host_ = '192.168.2.194',
    [string]$Action = 'toggle' # off|on|toggle
)
$ws = New-Object System.Net.WebSockets.ClientWebSocket
$cts = New-Object System.Threading.CancellationTokenSource
$cts.CancelAfter(5000)
$uri = [System.Uri]("ws://$Host_/ws")
try {
    $ws.ConnectAsync($uri, $cts.Token).Wait()
    Write-Host "Connected: state=$($ws.State)"
    function Send-Msg($w, $cts, $cmd, $message) {
        $msg = '{"command":"' + $cmd + '","message":"' + $message + '"}'
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($msg)
        $seg = [System.ArraySegment[byte]]::new($bytes)
        $w.SendAsync($seg, [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $cts.Token).Wait()
        Write-Host "Sent: $msg"
    }
    if ($Action -eq 'toggle' -or $Action -eq 'off') {
        Send-Msg $ws $cts 'setServoVoltageEnabled' 'false'
        Start-Sleep -Seconds 2
    }
    if ($Action -eq 'toggle' -or $Action -eq 'on') {
        Send-Msg $ws $cts 'setServoVoltageEnabled' 'true'
        Start-Sleep -Seconds 1
    }
    $ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $cts.Token).Wait()
    Write-Host 'Closed'
} catch {
    Write-Host "ERR: $_"
}

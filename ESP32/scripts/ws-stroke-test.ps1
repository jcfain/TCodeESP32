param(
    [string]$Host_ = '192.168.2.194',
    [int]$Cycles = 3,
    [int]$GapMs = 1200
)
$ws = New-Object System.Net.WebSockets.ClientWebSocket
$cts = New-Object System.Threading.CancellationTokenSource
$cts.CancelAfter(60000)
$uri = [System.Uri]("ws://$Host_/ws")
$ws.ConnectAsync($uri, $cts.Token).Wait()
Write-Host "Connected: $($ws.State)"

function Send-Text($text) {
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($text)
    $seg = [System.ArraySegment[byte]]::new($bytes)
    $ws.SendAsync($seg, [System.Net.WebSockets.WebSocketMessageType]::Text, $true, $cts.Token).Wait()
}

# Ensure VSER on
Send-Text '{"command":"setServoVoltageEnabled","message":"true"}'
Start-Sleep -Milliseconds 500

$cmds = @('L09999','L00000','L05000')
for ($i = 1; $i -le $Cycles; $i++) {
    Write-Host "--- cycle $i ---"
    foreach ($c in $cmds) {
        Write-Host "Sending $c"
        # TCode parser executes the buffer on '\n' (LF). Without a newline the
        # command never gets dispatched.
        Send-Text ($c + "`n")
        Start-Sleep -Milliseconds $GapMs
    }
}

try { $ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $cts.Token).Wait(2000) } catch {}
Write-Host "DONE"

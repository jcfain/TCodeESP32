param(
    [string]$Host_ = '192.168.2.194',
    [int]$Seconds = 10
)
$ws = New-Object System.Net.WebSockets.ClientWebSocket
$cts = New-Object System.Threading.CancellationTokenSource
$cts.CancelAfter(($Seconds + 5) * 1000)
$uri = [System.Uri]("ws://$Host_/ws")
try {
    $ws.ConnectAsync($uri, $cts.Token).Wait()
    Write-Host "Connected: state=$($ws.State)"
    $deadline = (Get-Date).AddSeconds($Seconds)
    $buf = New-Object byte[] 8192
    $count = 0
    while ((Get-Date) -lt $deadline -and $ws.State -eq 'Open') {
        $seg = [System.ArraySegment[byte]]::new($buf)
        $task = $ws.ReceiveAsync($seg, $cts.Token)
        $remainingMs = [int]((($deadline - (Get-Date)).TotalMilliseconds))
        if ($remainingMs -le 0) { break }
        if (-not $task.Wait($remainingMs)) { break }
        $r = $task.Result
        if ($r.Count -gt 0) {
            $text = [System.Text.Encoding]::UTF8.GetString($buf, 0, $r.Count)
            $count++
            $now = Get-Date -Format 'HH:mm:ss.fff'
            $preview = $text
            Write-Host "[$now] #${count}: $preview"
        }
    }
    Write-Host "Total messages: $count"
    if ($ws.State -eq 'Open') {
        $closeCts = New-Object System.Threading.CancellationTokenSource
        $closeCts.CancelAfter(2000)
        try { $ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'bye', $closeCts.Token).Wait() } catch {}
    }
} catch {
    Write-Host "ERR: $_"
}

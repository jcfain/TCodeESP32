param(
    [string]$ComPort = 'COM4',
    [int]$Baud = 115200,
    [int]$Cycles = 3,
    [int]$GapMs = 1000
)

$port = New-Object System.IO.Ports.SerialPort $ComPort, $Baud, 'None', 8, 'One'
$port.NewLine = "`n"
$port.ReadTimeout = 200
$port.Open()
Start-Sleep -Milliseconds 200
$port.DiscardInBuffer()

function Drain {
    param($p)
    try { while ($true) { $line = $p.ReadLine(); Write-Host ">> $line" } } catch {}
}

$cmds = @(
    @{ name = 'MAX';    cmd = 'L09999' },
    @{ name = 'MIN';    cmd = 'L00000' },
    @{ name = 'CENTER'; cmd = 'L05000' }
)

for ($i = 1; $i -le $Cycles; $i++) {
    Write-Host "--- cycle $i ---"
    foreach ($c in $cmds) {
        Write-Host "Sending $($c.name): $($c.cmd)"
        $port.WriteLine($c.cmd)
        Start-Sleep -Milliseconds $GapMs
        Drain $port
    }
}

$port.Close()
Write-Host "DONE"

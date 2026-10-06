param(
    [string]$ComPort = 'COM5',
    [int]$Baud = 115200,
    [int]$Seconds = 18,
    [string]$OutFile = 'C:\MyTemp\sr6_boot.log'
)
$port = New-Object System.IO.Ports.SerialPort $ComPort, $Baud, 'None', 8, 'One'
$port.NewLine = "`n"
$port.ReadTimeout = 200
$port.DtrEnable = $false
$port.RtsEnable = $false
$port.Open()
Start-Sleep -Milliseconds 100
$port.DiscardInBuffer()
Write-Host "Sending #restart on $ComPort, then capturing for ${Seconds}s..."
$port.WriteLine('#restart')
$deadline = (Get-Date).AddSeconds($Seconds)
$sb = New-Object System.Text.StringBuilder
while ((Get-Date) -lt $deadline -and $port.IsOpen) {
    try {
        $line = $port.ReadLine()
        $stamp = (Get-Date -Format 'HH:mm:ss.fff')
        $entry = "[$stamp] $line"
        Write-Host $entry
        [void]$sb.AppendLine($entry)
    } catch [System.TimeoutException] { }
      catch { break }
}
if ($port.IsOpen) { $port.Close() }
$sb.ToString() | Out-File -FilePath $OutFile -Encoding utf8
Write-Host "Wrote $OutFile (length=$($sb.Length))"

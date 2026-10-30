# Extracts the 256x256 frame from res\app.ico into res\logo.png.
# Browsers cannot show .ico through an <img> tag, so the README uses the PNG.
# Usage: powershell -ExecutionPolicy Bypass -File tools\make-logo.ps1
Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$ico = Join-Path $root 'res\app.ico'
$png = Join-Path $root 'res\logo.png'

$fs = [IO.File]::OpenRead($ico)
$br = New-Object IO.BinaryReader $fs
$null = $br.ReadUInt16()
$null = $br.ReadUInt16()
$count = $br.ReadUInt16()

$best = $null
for ($i = 0; $i -lt $count; $i++) {
    $w = $br.ReadByte(); $h = $br.ReadByte()
    $null = $br.ReadByte(); $null = $br.ReadByte()
    $null = $br.ReadUInt16(); $null = $br.ReadUInt16()
    $size = $br.ReadUInt32(); $offset = $br.ReadUInt32()
    $width = if ($w -eq 0) { 256 } else { [int]$w }
    if ($null -eq $best -or $width -gt $best.Width) {
        $best = [pscustomobject]@{ Width = $width; Size = $size; Offset = $offset }
    }
}

$fs.Position = $best.Offset
$data = $br.ReadBytes([int]$best.Size)
$br.Close(); $fs.Close()

# frames of this size are stored as PNG already
if ($data[0] -eq 0x89 -and $data[1] -eq 0x50) {
    [IO.File]::WriteAllBytes($png, $data)
} else {
    $icon = New-Object System.Drawing.Icon $ico, $best.Width, $best.Width
    $icon.ToBitmap().Save($png, [System.Drawing.Imaging.ImageFormat]::Png)
    $icon.Dispose()
}

$img = [System.Drawing.Image]::FromFile($png)
Write-Host ("Wrote {0}  ({1}x{2}, {3:N0} bytes)" -f $png, $img.Width, $img.Height, (Get-Item $png).Length)
$img.Dispose()

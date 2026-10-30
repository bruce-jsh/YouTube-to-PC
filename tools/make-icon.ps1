# Generates res\app.ico (blue rounded square with a white download arrow).
# Usage: powershell -ExecutionPolicy Bypass -File tools\make-icon.ps1
Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$out = Join-Path $root 'res\app.ico'
New-Item -ItemType Directory -Force (Split-Path $out) | Out-Null

function New-RoundedPath([float]$x, [float]$y, [float]$w, [float]$h, [float]$r) {
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = [Math]::Min($r * 2, [Math]::Min($w, $h))
    $p.AddArc($x, $y, $d, $d, 180, 90)
    $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $p.CloseFigure()
    return $p
}

$sizes = 16, 20, 24, 32, 40, 48, 64, 128, 256
$images = @()
foreach ($s in $sizes) {
    $bmp = New-Object System.Drawing.Bitmap $s, $s, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.Clear([System.Drawing.Color]::Transparent)

    $pad = [Math]::Max(0.5, $s * 0.04)
    $rect = New-Object System.Drawing.RectangleF $pad, $pad, ($s - 2 * $pad), ($s - 2 * $pad)
    $path = New-RoundedPath $rect.X $rect.Y $rect.Width $rect.Height ($s * 0.22)
    $top = [System.Drawing.Color]::FromArgb(255, 59, 130, 246)
    $bottom = [System.Drawing.Color]::FromArgb(255, 29, 78, 216)
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush $rect, $top, $bottom, 90
    $g.FillPath($brush, $path)

    $white = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::White)
    $cx = $s / 2.0
    $shaftW = $s * 0.14
    $headW = $s * 0.44
    $y0 = $s * 0.20
    $headTop = $s * 0.42
    $tip = $s * 0.66
    $pts = @(
        (New-Object System.Drawing.PointF ($cx - $shaftW / 2), $y0),
        (New-Object System.Drawing.PointF ($cx + $shaftW / 2), $y0),
        (New-Object System.Drawing.PointF ($cx + $shaftW / 2), $headTop),
        (New-Object System.Drawing.PointF ($cx + $headW / 2), $headTop),
        (New-Object System.Drawing.PointF $cx, $tip),
        (New-Object System.Drawing.PointF ($cx - $headW / 2), $headTop),
        (New-Object System.Drawing.PointF ($cx - $shaftW / 2), $headTop)
    )
    $g.FillPolygon($white, [System.Drawing.PointF[]]$pts)
    $barH = [Math]::Max(1.5, $s * 0.085)
    $bar = New-RoundedPath ($s * 0.24) ($s * 0.73) ($s * 0.52) $barH ($barH / 2)
    $g.FillPath($white, $bar)

    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $images += , @($s, $ms.ToArray())
    $g.Dispose(); $bmp.Dispose()
}

$fs = [System.IO.File]::Create($out)
$bw = New-Object System.IO.BinaryWriter $fs
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$images.Count)
$offset = 6 + 16 * $images.Count
foreach ($img in $images) {
    $s = $img[0]; $data = $img[1]
    $dim = if ($s -ge 256) { 0 } else { $s }
    $bw.Write([Byte]$dim); $bw.Write([Byte]$dim); $bw.Write([Byte]0); $bw.Write([Byte]0)
    $bw.Write([UInt16]1); $bw.Write([UInt16]32)
    $bw.Write([UInt32]$data.Length); $bw.Write([UInt32]$offset)
    $offset += $data.Length
}
foreach ($img in $images) { $bw.Write([byte[]]$img[1]) }
$bw.Close()
Write-Host "Wrote $out"

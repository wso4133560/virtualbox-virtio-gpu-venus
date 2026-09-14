#requires -Version 7.0
# Download once, retain the verified image in the ignored build cache.
[CmdletBinding()]
param([string]$CacheDirectory)
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$pin = Get-Content (Join-Path $PSScriptRoot 'linux-test-image.json') -Raw | ConvertFrom-Json
if (-not $CacheDirectory) { $CacheDirectory = Join-Path (Split-Path $PSScriptRoot -Parent) '.build\images' }
New-Item -ItemType Directory -Force $CacheDirectory | Out-Null
$target = Join-Path (Resolve-Path $CacheDirectory).Path 'noble-20260911-amd64.img'
function Test-Image([string]$Path) {
    return (Test-Path -LiteralPath $Path -PathType Leaf) -and (Get-Item $Path).Length -eq $pin.bytes -and
           (Get-FileHash $Path -Algorithm SHA256).Hash -eq $pin.sha256
}
if (Test-Image $target) { Write-Output $target; return }
if (Test-Path -LiteralPath $target) { throw "Cached image checksum mismatch: $target" }
$partial = $target + '.partial'
for ($attempt = 1; $attempt -le 20; $attempt++) {
    if (Test-Path -LiteralPath $partial) {
        $bytes = (Get-Item $partial).Length
        if ($bytes -ge $pin.bytes) { break }
    }
    try { Invoke-WebRequest $pin.url -OutFile $partial -Resume -TimeoutSec 120 }
    catch { Write-Warning "Image transfer interrupted (attempt $attempt/20): $($_.Exception.Message)" }
    if (Test-Path -LiteralPath $partial) { Write-Host "Downloaded $((Get-Item $partial).Length)/$($pin.bytes) bytes" }
}
if (-not (Test-Image $partial)) { throw "Incomplete or corrupt download retained at $partial" }
Move-Item -LiteralPath $partial -Destination $target
Write-Host "Ubuntu image SHA256 verified: $($pin.sha256)"
Write-Output $target

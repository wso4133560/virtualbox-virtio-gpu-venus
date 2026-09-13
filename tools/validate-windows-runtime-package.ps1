[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PackagePath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$cleanup = $null

if (Test-Path -LiteralPath $PackagePath -PathType Leaf) {
    if ([IO.Path]::GetExtension($PackagePath) -ne '.zip') {
        throw "PackagePath must be a runtime directory or .zip: $PackagePath"
    }
    $cleanup = Join-Path ([IO.Path]::GetTempPath()) ("vbox-runtime-" + [guid]::NewGuid().ToString('N'))
    Expand-Archive -LiteralPath $PackagePath -DestinationPath $cleanup
    $root = $cleanup
} elseif (Test-Path -LiteralPath $PackagePath -PathType Container) {
    $root = (Resolve-Path -LiteralPath $PackagePath).Path
} else {
    throw "PackagePath does not exist: $PackagePath"
}

try {
    $manifestPath = Join-Path $root 'manifest.json'
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        throw "Missing package manifest: $manifestPath"
    }
    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    if ($manifest.format -ne 1 -or -not $manifest.files) {
        throw 'Unsupported or empty package manifest.'
    }
    $bad = @()
    foreach ($entry in $manifest.files) {
        $path = Join-Path $root ($entry.path -replace '/', '\')
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            $bad += "$($entry.path):missing"
            continue
        }
        $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
        if ($hash -ne $entry.sha256) { $bad += "$($entry.path):hash" }
    }
    if ($bad.Count) { throw "Manifest integrity failed: $($bad -join ', ')" }

    $bin = Join-Path $root 'bin'
    $manage = Join-Path $bin 'VBoxManage.exe'
    $test = Join-Path $bin 'testcase\tstVirtioGPU.exe'
    if (-not (Test-Path -LiteralPath $manage -PathType Leaf)) { throw "Missing VBoxManage.exe" }
    if (-not (Test-Path -LiteralPath $test -PathType Leaf)) { throw "Missing tstVirtioGPU.exe" }

    $version = (& $manage --version 2>&1 | Out-String).Trim()
    if ($version -notmatch '^7\.2\.6r172322$') { throw "Unexpected VBoxManage version: $version" }
    $testOutput = (& $test 2>&1 | Out-String)
    if ($LASTEXITCODE -ne 0 -or $testOutput -notmatch 'tstVirtioGPU: SUCCESS') {
        throw "VirtIO-GPU package test failed (exit $LASTEXITCODE)."
    }
    Write-Host "Manifest integrity: PASS ($($manifest.files.Count) files)"
    Write-Host "VBoxManage version: PASS ($version)"
    Write-Host 'VirtIO-GPU/Venus test: PASS'
}
finally {
    if ($cleanup -and (Test-Path -LiteralPath $cleanup)) {
        Remove-Item -LiteralPath $cleanup -Recurse -Force
    }
}

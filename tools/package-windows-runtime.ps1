[CmdletBinding()]
param(
    [string]$OutputDirectory = (Join-Path (Split-Path $PSScriptRoot -Parent) '.build\windows\package'),
    [string]$ZipPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$binRoot = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\bin'
if (-not [IO.Path]::IsPathRooted($OutputDirectory)) {
    $OutputDirectory = Join-Path $repoRoot $OutputDirectory
}
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if ($ZipPath -and -not [IO.Path]::IsPathRooted($ZipPath)) {
    $ZipPath = Join-Path $repoRoot $ZipPath
}

function Require-File([string]$Path, [string]$Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
    }
}

$required = @(
    'VBoxRT.dll', 'VBoxVMM.dll', 'VBoxSVC.exe', 'VBoxHeadless.exe',
    'VBoxManage.exe', 'VBoxC.dll', 'VBoxProxyStub.dll', 'VBoxDD.dll',
    'VBoxDD2.dll', 'VBoxDDU.dll', 'VMMR0.r0', 'VBoxSup.sys',
    'VBoxEFI-amd64.fd', 'comregister.cmd'
)
foreach ($name in $required) {
    Require-File (Join-Path $binRoot $name) "Windows runtime artifact $name"
}

if (Test-Path -LiteralPath $OutputDirectory) {
    Remove-Item -LiteralPath $OutputDirectory -Recurse -Force
}
$runtimeBin = Join-Path $OutputDirectory 'bin'
$runtimeDocs = Join-Path $OutputDirectory 'docs'
$runtimeValidation = Join-Path $OutputDirectory 'validation'
New-Item -ItemType Directory -Force $runtimeBin, $runtimeDocs, $runtimeValidation | Out-Null

foreach ($name in $required) {
    $sourcePath = Join-Path $binRoot $name
    # kBuild may finish a locked executable in the object directory while the
    # install copy is still held by Windows. Prefer that newer, complete target
    # so the package does not silently contain an older CLI.
    if ($name -eq 'VBoxManage.exe') {
        $candidate = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\obj\VBoxManage\VBoxManage.exe'
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            $installed = Get-Item -LiteralPath $sourcePath
            $built = Get-Item -LiteralPath $candidate
            if ($built.LastWriteTimeUtc -gt $installed.LastWriteTimeUtc -and $built.Length -gt 0) {
                $sourcePath = $candidate
            }
        }
    }
    Copy-Item -LiteralPath $sourcePath -Destination (Join-Path $runtimeBin $name)
}
$testExe = Join-Path $binRoot 'testcase\tstVirtioGPU.exe'
if (Test-Path -LiteralPath $testExe -PathType Leaf) {
    New-Item -ItemType Directory -Force (Join-Path $runtimeBin 'testcase') | Out-Null
    Copy-Item -LiteralPath $testExe -Destination (Join-Path $runtimeBin 'testcase')
}
$docRoot = Join-Path $repoRoot 'doc'
$docFiles = @(Get-ChildItem -LiteralPath $docRoot -Filter '*.md' -File)
if (-not $docFiles.Count) { throw "Missing runtime documentation under $docRoot" }
foreach ($docFile in $docFiles) {
    Copy-Item -LiteralPath $docFile.FullName -Destination $runtimeDocs
}

$validationFiles = @(
    'virtio-gpu-validation.json', 'baseline-validation.json',
    'virtio-gpu-tests.log', 'build.log'
)
foreach ($name in $validationFiles) {
    $source = Join-Path $repoRoot (Join-Path '.build\windows' $name)
    if (Test-Path -LiteralPath $source -PathType Leaf) {
        Copy-Item -LiteralPath $source -Destination $runtimeValidation
    }
}

$files = Get-ChildItem -LiteralPath $OutputDirectory -File -Recurse | ForEach-Object {
    $relative = $_.FullName.Substring($OutputDirectory.Length).TrimStart('\', '/')
    [ordered]@{
        path = $relative.Replace('\', '/')
        bytes = $_.Length
        sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
    }
}
$manifest = [ordered]@{
    format = 1
    product = 'VirtualBox 7.2.6 VirtIO-GPU Venus Windows runtime'
    generatedAt = (Get-Date).ToString('o')
    sourceBin = 'VirtualBox-7.2.6/out/win.amd64/release/bin'
    files = @($files)
    validation = @($validationFiles | ForEach-Object { "validation/$_" })
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'manifest.json') -Encoding utf8

if ($ZipPath) {
    $zipParent = Split-Path $ZipPath -Parent
    if ($zipParent) { New-Item -ItemType Directory -Force $zipParent | Out-Null }
    if (Test-Path -LiteralPath $ZipPath) { Remove-Item -LiteralPath $ZipPath -Force }
    Compress-Archive -Path (Join-Path $OutputDirectory '*') -DestinationPath $ZipPath -CompressionLevel Optimal
    Write-Host "Runtime package: $ZipPath"
}
Write-Host "Runtime directory: $OutputDirectory"
Write-Host "Packaged files: $(@($files).Count)"

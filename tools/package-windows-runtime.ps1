[CmdletBinding()]
param(
    [string]$OutputDirectory,
    [string]$ZipPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $repoRoot '.build\windows\package'
}
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
    'VBoxRT.dll', 'VBoxVMM.dll', 'VBoxSVC.exe', 'VBoxSDS.exe', 'VBoxHeadless.exe',
    'VBoxManage.exe', 'VBoxC.dll', 'VBoxProxyStub.dll', 'VBoxDD.dll',
    'VBoxDD2.dll', 'VBoxDDU.dll', 'VMMR0.r0', 'VBoxSup.sys',
    'VBoxEFI-amd64.fd', 'VBoxSup.inf', 'comregister.cmd'
)
foreach ($name in $required) {
    Require-File (Join-Path $binRoot $name) "Windows runtime artifact $name"
}
$testNames = @('tstVirtioGPU.exe', 'tstLdr.exe', 'tstLdrLoadConfig.exe')
foreach ($name in $testNames) {
    Require-File (Join-Path $binRoot (Join-Path 'testcase' $name)) "runtime validation program $name"
}

if (Test-Path -LiteralPath $OutputDirectory) {
    Remove-Item -LiteralPath $OutputDirectory -Recurse -Force
}
$runtimeBin = Join-Path $OutputDirectory 'bin'
$runtimeDocs = Join-Path $OutputDirectory 'docs'
$runtimeValidation = Join-Path $OutputDirectory 'validation'
New-Item -ItemType Directory -Force $runtimeBin, $runtimeDocs, $runtimeValidation | Out-Null
$runtimeTools = Join-Path $OutputDirectory 'tools'
New-Item -ItemType Directory -Force $runtimeTools | Out-Null
foreach ($name in @('get-linux-test-image.ps1', 'linux-test-image.json', 'test-linux-virtio-gpu.ps1')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination $runtimeTools
}

# VBoxManage unattended install needs the distro templates at runtime.  Keep
# the complete template set beside the binaries so a clean ZIP extraction can
# create Ubuntu/Debian guests without depending on the source checkout.
$templateSource = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Main\UnattendedTemplates'
$templateTarget = Join-Path $runtimeBin 'UnattendedTemplates'
if (-not (Test-Path -LiteralPath $templateSource -PathType Container)) {
    throw "Missing unattended template directory: $templateSource"
}
Copy-Item -LiteralPath $templateSource -Destination $templateTarget -Recurse

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
    if ($name -eq 'VBoxDD.dll') {
        $candidate = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\obj\VBoxDD\VBoxDD.dll'
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
New-Item -ItemType Directory -Force (Join-Path $runtimeBin 'testcase') | Out-Null
foreach ($name in $testNames) {
    $testExe = Join-Path $binRoot (Join-Path 'testcase' $name)
    Copy-Item -LiteralPath $testExe -Destination (Join-Path $runtimeBin 'testcase')
}
# kBuild places a VBoxRT hard link beside dynamically linked test programs.
# A ZIP needs a regular copy so the loader probes also run after extraction.
Copy-Item -LiteralPath (Join-Path $runtimeBin 'VBoxRT.dll') -Destination (Join-Path $runtimeBin 'testcase')
$docRoot = Join-Path $repoRoot 'doc'
$docFiles = @(Get-ChildItem -LiteralPath $docRoot -Filter '*.md' -File)
if (-not $docFiles.Count) { throw "Missing runtime documentation under $docRoot" }
foreach ($docFile in $docFiles) {
    Copy-Item -LiteralPath $docFile.FullName -Destination $runtimeDocs
}

$validationFiles = @(
    'virtio-gpu-validation.json', 'baseline-validation.json',
    'virtio-gpu-tests.log', 'build.log', 'ldr-load-config-tests.log',
    'vmmr0-load-tests.log', 'virtio-gpu-vm-validation.json', 'virtio-gpu-vm-validation.VBox.log',
    'virtio-gpu-vm-display-validation.json', 'virtio-gpu-vm-display-validation.VBox.log',
    'gpu-display-port.log', 'build-display-port.log', 'pci-cap-before.log',
    'build-linux-shm-id.log', 'vm-observation-tests.log'
)
foreach ($name in $validationFiles) {
    $source = Join-Path $repoRoot (Join-Path '.build\windows' $name)
    if (Test-Path -LiteralPath $source -PathType Leaf) {
        Copy-Item -LiteralPath $source -Destination $runtimeValidation
    }
}

# Guest evidence has no seed media or keys: those are removed by the runner.
foreach ($scenario in @('linux-venus-before', 'linux-venus-final', 'linux-software-final', 'linux-venus-repeat',
                        'linux-venus-final-admin46', 'linux-venus-save-restore-admin47',
                        'linux-venus-cpu2-admin48', 'linux-venus-reset-admin51',
                        'linux-venus-object-admin52', 'linux-venus-object-save-admin53',
                        'linux-venus-object-reset-admin55', 'linux-venus-submit2-admin56',
                        'linux-venus-final-dispatcher', 'linux-venus-final-dispatcher-save',
                        'linux-venus-final-dispatcher-repeat-current', 'linux-venus-final-built',
                        'linux-venus-final-object-id', 'linux-venus-final-object-id-save',
                        'linux-venus-final-object-id-repeat', 'linux-venus-cursor-final',
                        'linux-venus-cursor-final-repeat', 'linux-venus-cursor-final-save',
                        'linux-venus-cursor-final-reset', 'linux-venus-cursor-final-cpu2-long',
                        'linux-venus-opaque-objects5', 'linux-venus-recording-defer30',
                        'linux-venus-recording-save5', 'linux-venus-recording-reset5',
                        'linux-venus-host-objects7', 'linux-venus-host-objects30-7', 'linux-venus-host-objects-save7',
                        'linux-venus-host-objects-reset7', 'linux-venus-host-sync3', 'linux-venus-host-sync-repeat3',
                        'linux-venus-host-sync-save3', 'linux-venus-host-sync-reset3',
                        'linux-venus-save-final3', 'linux-venus-reset-fixed',
                        'linux-venus-final-stress30-current', 'linux-venus-final-repeat-current',
                        'linux-venus-final-cpu2-createbuffer5-debug3', 'linux-venus-final-cpu1-save5-fix2',
                        'linux-venus-final-cpu2-ssm-reset30-fix1', 'linux-venus-object-cleanup-cpu1-save5-final',
                        'linux-venus-buffer-view-cpu1-save5-final-pass')) {
    $sourceDir = Join-Path $repoRoot (Join-Path '.build\windows' $scenario)
    if (Test-Path -LiteralPath $sourceDir -PathType Container) {
        $sourceRoot = $sourceDir
        if (Test-Path -LiteralPath (Join-Path $sourceDir 'report') -PathType Container) {
            $sourceRoot = Join-Path $sourceDir 'report'
        }
        $destination = Join-Path $runtimeValidation $scenario
        New-Item -ItemType Directory -Force $destination | Out-Null
        foreach ($name in @('report.json', 'guest.log', 'serial.log', 'VBox.log',
                            'guest-vulkan.log', 'guest-vulkan-restore.log', 'guest-vulkan-reset.log')) {
            $source = Join-Path $sourceRoot $name
            if (Test-Path -LiteralPath $source -PathType Leaf) {
                Copy-Item -LiteralPath $source -Destination $destination
            }
        }
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

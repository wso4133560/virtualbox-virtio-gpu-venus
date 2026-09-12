# User-mode smoke tests only. Does not install/register services or load drivers.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$binRoot = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\bin'
$reportRoot = Join-Path $repoRoot '.build\windows'
$results = @()
$requiredFiles = @('VBoxRT.dll', 'VBoxVMM.dll', 'VBoxDD.dll', 'VBoxDD2.dll', 'VBoxDDU.dll', 'VBoxSVC.exe', 'VBoxManage.exe', 'VBoxHeadless.exe', 'VMMR0.r0', 'VBoxSup.sys')
foreach ($name in $requiredFiles) {
    $path = Join-Path $binRoot $name
    if (-not (Test-Path $path)) { throw "Missing build artifact: $path" }
    $bytes = [IO.File]::ReadAllBytes($path)
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3c)
    if ([BitConverter]::ToUInt32($bytes, $peOffset) -ne 0x4550 -or [BitConverter]::ToUInt16($bytes, $peOffset + 4) -ne 0x8664) {
        throw "Artifact is not a Windows amd64 PE image: $name"
    }
    $results += [ordered]@{ file = $name; bytes = $bytes.Length; sha256 = (Get-FileHash $path -Algorithm SHA256).Hash.ToLowerInvariant() }
}
$checks = @(
    @{ name = 'version'; exe = 'VBoxManage.exe'; arguments = @('--version'); expect = '^7\.2\.6r172322\s*$' },
    @{ name = 'headless-help'; exe = 'VBoxHeadless.exe'; arguments = @('--help'); expect = 'Oracle VirtualBox Headless Interface 7\.2\.6' },
    @{ name = 'test-xml'; exe = 'testcase\tstRTXml.exe'; arguments = @(); expect = 'tstRTXml: SUCCESS' },
    @{ name = 'test-semevent'; exe = 'testcase\tstRTSemEvent.exe'; arguments = @(); expect = 'tstRTSemEvent: SUCCESS' }
)
foreach ($check in $checks) {
    $arguments = $check.arguments
    $output = & (Join-Path $binRoot $check.exe) @arguments 2>&1
    $testExit = $LASTEXITCODE
    $output | Set-Content (Join-Path $reportRoot "$($check.name).log") -Encoding utf8
    if ($testExit -ne 0 -or ($output -join "`n") -notmatch $check.expect) {
        throw "$($check.name) failed (exit $testExit); see .build/windows/$($check.name).log"
    }
    Write-Host "$($check.name): PASS"
}
$report = [ordered]@{
    date = (Get-Date).ToString('o')
    version = '7.2.6r172322'
    buildProfile = 'Windows amd64 command-line development build; hardening/GUI/3D/additions disabled'
    checks = @($checks.name)
    artifacts = $results
    driverSignature = (Get-AuthenticodeSignature (Join-Path $binRoot 'VBoxSup.sys')).Status.ToString()
    guestBootVerified = $false
    venusVerified = $false
}
$report | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $reportRoot 'baseline-validation.json') -Encoding utf8
Write-Host 'Baseline user-mode smoke tests passed. Guest boot and Venus require separate validation.'

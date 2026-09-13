[CmdletBinding()]
param(
    [string]$RuntimeDirectory,
    [switch]$NoProxy,
    [string]$ReportPath,
    [switch]$AutoElevate
)

$ErrorActionPreference = 'Stop'
if (-not $RuntimeDirectory) {
    $RuntimeDirectory = Join-Path (Split-Path $PSScriptRoot -Parent) 'VirtualBox-7.2.6\out\win.amd64\release\bin'
}
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$required = @('VBoxSVC.exe', 'VBoxC.dll', 'VBoxProxyStub.dll', 'comregister.cmd')
foreach ($name in $required) {
    if (-not (Test-Path -LiteralPath (Join-Path $runtime $name) -PathType Leaf)) {
        throw "Missing runtime artifact: $name"
    }
}

$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = [Security.Principal.WindowsPrincipal]::new($identity)
$isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    if ($AutoElevate) {
        $reportFullPath = $null
        if ($ReportPath) {
            $reportFullPath = [IO.Path]::GetFullPath((Join-Path (Get-Location).Path $ReportPath))
        }
        $forward = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ('"' + $PSCommandPath.Replace('"', '\"') + '"'))
        $forward += @('-RuntimeDirectory', ('"' + $runtime.Replace('"', '\"') + '"'))
        if ($NoProxy) { $forward += '-NoProxy' }
        if ($reportFullPath) {
            $forward += @('-ReportPath', ('"' + $reportFullPath.Replace('"', '\"') + '"'))
        }
        try {
            $elevated = Start-Process -FilePath 'powershell.exe' -Verb RunAs -WorkingDirectory (Get-Location).Path `
                        -ArgumentList ($forward -join ' ') -Wait -PassThru
        } catch {
            if ($reportFullPath) {
                $reportParent = Split-Path -Parent $reportFullPath
                if ($reportParent) { New-Item -ItemType Directory -Force $reportParent | Out-Null }
                [ordered]@{
                    runtimeDirectory = $runtime
                    administrator = $false
                    autoElevateRequested = $true
                    elevationStarted = $false
                    commandExitCode = $null
                    progIdRegistered = $false
                    elevationError = $_.Exception.Message
                    checkedAt = (Get-Date).ToString('o')
                } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $reportFullPath -Encoding utf8
            }
            throw
        }
        if ($reportFullPath -and -not (Test-Path -LiteralPath $reportFullPath)) {
            $reportParent = Split-Path -Parent $reportFullPath
            if ($reportParent) { New-Item -ItemType Directory -Force $reportParent | Out-Null }
            [ordered]@{
                runtimeDirectory = $runtime
                administrator = $false
                autoElevateRequested = $true
                elevationStarted = $true
                commandExitCode = $elevated.ExitCode
                progIdRegistered = $false
                elevationError = 'Elevated process returned without writing a verification report.'
                checkedAt = (Get-Date).ToString('o')
            } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $reportFullPath -Encoding utf8
        }
        exit $elevated.ExitCode
    }
    throw 'Administrator token required. Open an elevated PowerShell and rerun this script.'
}

$args = if ($NoProxy) { '--no-proxy' } else { '--proxy' }
$outputPath = Join-Path ([IO.Path]::GetTempPath()) ("vbox-comregister-" + [guid]::NewGuid().ToString('N') + '.log')
$errorPath = Join-Path ([IO.Path]::GetTempPath()) ("vbox-comregister-" + [guid]::NewGuid().ToString('N') + '.err')
try {
    $process = Start-Process -FilePath 'cmd.exe' -ArgumentList @('/d', '/s', '/c', "call `"$runtime\comregister.cmd`" $args") `
        -WorkingDirectory $runtime -Wait -PassThru -RedirectStandardOutput $outputPath -RedirectStandardError $errorPath
    $output = ((Get-Content -LiteralPath $outputPath -Raw -ErrorAction SilentlyContinue) +
               (Get-Content -LiteralPath $errorPath -Raw -ErrorAction SilentlyContinue)).Trim()
} finally {
    if (Test-Path -LiteralPath $outputPath) { Remove-Item -LiteralPath $outputPath -Force }
    if (Test-Path -LiteralPath $errorPath) { Remove-Item -LiteralPath $errorPath -Force }
}

$progId = 'HKCR:\VirtualBox.VirtualBox'
$clsid = 'HKCR:\CLSID\{B1A7A4F2-47B9-4A1E-82B2-07CCD5323C3F}'
$registered = (Test-Path -LiteralPath $progId) -and (Test-Path -LiteralPath $clsid)
$report = [ordered]@{
    runtimeDirectory = $runtime
    administrator = $isAdmin
    commandExitCode = $process.ExitCode
    progIdRegistered = $registered
    output = $output.Trim()
    checkedAt = (Get-Date).ToString('o')
}
if ($ReportPath) {
    $reportParent = Split-Path -Parent $ReportPath
    if ($reportParent) { New-Item -ItemType Directory -Force $reportParent | Out-Null }
    $report | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $ReportPath -Encoding utf8
}
if ($process.ExitCode -ne 0 -or -not $registered) {
    throw "COM registration did not verify (exit $($process.ExitCode), progIdRegistered=$registered)."
}
Write-Host "COM registration: PASS"
Write-Host "Runtime: $runtime"
if ($ReportPath) { Write-Host "Report: $ReportPath" }

# Creates a disposable VM and verifies the real VirtualBox launch path
# with the VirtIO-GPU Venus controller. Guest Vulkan validation remains a
# separate step because it depends on the selected guest image and drivers.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$IsoPath,
    [string]$RuntimeDirectory,
    [string]$VmName,
    [ValidateSet('auto', 'software', 'venus')][string]$GpuBackend = 'venus',
    [ValidateRange(1024, 131072)][int]$MemoryMB = 4096,
    [ValidateRange(1, 300)][int]$TimeoutSeconds = 60,
    [ValidateRange(0, 600)][int]$ObserveSeconds = 15,
    [switch]$KeepVm,
    [string]$ReportPath
)

$ErrorActionPreference = 'Stop'
if (-not $RuntimeDirectory) {
    $RuntimeDirectory = Join-Path (Split-Path $PSScriptRoot -Parent) 'VirtualBox-7.2.6\out\win.amd64\release\bin'
}
if (-not $VmName) {
    $VmName = 'virtio-gpu-smoke-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
}
if (-not $ReportPath) {
    $ReportPath = Join-Path (Split-Path $PSScriptRoot -Parent) '.build\windows\virtio-gpu-vm-validation.json'
}
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$vboxManage = Join-Path $runtime 'VBoxManage.exe'
$iso = (Resolve-Path -LiteralPath $IsoPath).Path
if (-not (Test-Path -LiteralPath $vboxManage -PathType Leaf)) { throw "Missing VBoxManage.exe: $vboxManage" }
if (-not (Test-Path -LiteralPath $iso -PathType Leaf)) { throw "Missing ISO: $iso" }

function Invoke-VBoxManage {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)
    # Windows PowerShell 5.1 turns native stderr into ErrorRecords. Capture all
    # output before testing the exit code so the complete diagnostic survives.
    $savedPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = & $vboxManage @Arguments 2>&1
        $commandExit = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $savedPreference
    }
    if ($commandExit -ne 0) {
        throw "VBoxManage $($Arguments -join ' ') failed (exit ${commandExit}): $($output -join ' ')"
    }
    return @($output)
}

$reportParent = Split-Path -Parent $ReportPath
if ($reportParent) { New-Item -ItemType Directory -Force $reportParent | Out-Null }
$created = $false
$started = $false
$state = 'not-created'
$failure = $null
$phase = 'create'
$vmBase = Join-Path (Split-Path $PSScriptRoot -Parent) '.build\windows\vms'
$savedLog = [IO.Path]::ChangeExtension([IO.Path]::GetFullPath($ReportPath), '.VBox.log')
$logCaptured = $false
try {
    Invoke-VBoxManage @('createvm', '--name', $VmName, '--basefolder', $vmBase, '--register') | Out-Null
    $created = $true
    $state = 'created'
    $phase = 'configure'
    Invoke-VBoxManage @('modifyvm', $VmName, '--memory', $MemoryMB, '--vram', 64,
                        '--graphicscontroller', 'virtio-gpu', '--gpu-backend', $GpuBackend,
                        '--firmware', 'efi', '--audio-enabled', 'off') | Out-Null
    Invoke-VBoxManage @('storagectl', $VmName, '--name', 'SATA', '--add', 'sata', '--controller', 'IntelAhci') | Out-Null
    Invoke-VBoxManage @('storageattach', $VmName, '--storagectl', 'SATA', '--port', 0,
                        '--device', 0, '--type', 'dvddrive', '--medium', $iso) | Out-Null
    $phase = 'start'
    Invoke-VBoxManage @('startvm', $VmName, '--type', 'headless') | Out-Null
    $started = $true
    $phase = 'observe'
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    do {
        $info = Invoke-VBoxManage @('showvminfo', $VmName, '--machinereadable')
        $stateLine = $info | Where-Object { $_ -match '^VMState="([^"]+)"' } | Select-Object -First 1
        $state = if ($stateLine -match '^VMState="([^"]+)"') { $Matches[1] } else { 'unknown' }
        if ($state -eq 'running') { break }
        if ([DateTime]::UtcNow -ge $deadline) { throw "VM did not reach running state within $TimeoutSeconds seconds (state=$state)." }
        Start-Sleep -Milliseconds 500
    } while ($true)
    if ($ObserveSeconds -gt 0) {
        Start-Sleep -Seconds $ObserveSeconds
    }
    $phase = 'complete'
    Write-Host "VirtIO-GPU VM launch: PASS (state=$state, name=$VmName)"
}
catch {
    $failure = $_.Exception.Message
    Write-Error $failure -ErrorAction Continue
}
finally {
    if ($started) {
        try {
            Invoke-VBoxManage @('controlvm', $VmName, 'acpipowerbutton') | Out-Null
            $shutdownDeadline = [DateTime]::UtcNow.AddSeconds(15)
            do {
                Start-Sleep -Milliseconds 500
                $shutdownInfo = Invoke-VBoxManage @('showvminfo', $VmName, '--machinereadable')
                $shutdownLine = $shutdownInfo | Where-Object { $_ -match '^VMState="([^"]+)"' } | Select-Object -First 1
                $shutdownState = if ($shutdownLine -match '^VMState="([^"]+)"') { $Matches[1] } else { 'unknown' }
            } while ($shutdownState -notin @('poweroff', 'aborted') -and [DateTime]::UtcNow -lt $shutdownDeadline)
            if ($shutdownState -notin @('poweroff', 'aborted')) {
                Invoke-VBoxManage @('controlvm', $VmName, 'poweroff') | Out-Null
            }
        } catch { }
    }
    if ($created -and -not $KeepVm) {
        $logPath = Join-Path (Join-Path $vmBase $VmName) 'Logs\VBox.log'
        if (Test-Path -LiteralPath $logPath -PathType Leaf) {
            Copy-Item -LiteralPath $logPath -Destination $savedLog -Force
            $logCaptured = $true
        }
        try { Invoke-VBoxManage @('unregistervm', $VmName, '--delete') | Out-Null } catch { }
    }
    [ordered]@{
        timestamp = (Get-Date).ToString('o')
        vmName = $VmName
        iso = $iso
        runtimeDirectory = $runtime
        graphicsController = 'virtio-gpu'
        gpuBackend = $GpuBackend
        memoryMB = $MemoryMB
        observeSeconds = $ObserveSeconds
        created = $created
        started = $started
        state = $state
        phase = $phase
        logPath = if ($logCaptured) { $savedLog } else { $null }
        keptVm = [bool]$KeepVm
        passed = [bool]($started -and $state -eq 'running' -and -not $failure)
        failure = $failure
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $ReportPath -Encoding utf8
}
if ($failure) { throw $failure }

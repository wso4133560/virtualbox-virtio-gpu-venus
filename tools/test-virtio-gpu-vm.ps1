# Creates a disposable Linux VM and verifies the real VirtualBox launch path
# with the VirtIO-GPU Venus controller. Guest Vulkan validation remains a
# separate step because it depends on the selected guest image and drivers.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$IsoPath,
    [string]$RuntimeDirectory = (Join-Path (Split-Path $PSScriptRoot -Parent) 'VirtualBox-7.2.6\out\win.amd64\release\bin'),
    [string]$VmName = ('virtio-gpu-smoke-' + [guid]::NewGuid().ToString('N').Substring(0, 8)),
    [ValidateRange(1024, 131072)][int]$MemoryMB = 4096,
    [ValidateRange(1, 300)][int]$TimeoutSeconds = 60,
    [switch]$KeepVm,
    [string]$ReportPath = (Join-Path (Split-Path $PSScriptRoot -Parent) '.build\windows\virtio-gpu-vm-validation.json')
)

$ErrorActionPreference = 'Stop'
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$vboxManage = Join-Path $runtime 'VBoxManage.exe'
$iso = (Resolve-Path -LiteralPath $IsoPath).Path
if (-not (Test-Path -LiteralPath $vboxManage -PathType Leaf)) { throw "Missing VBoxManage.exe: $vboxManage" }
if (-not (Test-Path -LiteralPath $iso -PathType Leaf)) { throw "Missing ISO: $iso" }

function Invoke-VBoxManage {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)
    $output = & $vboxManage @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "VBoxManage $($Arguments -join ' ') failed (exit $LASTEXITCODE): $($output -join ' ')"
    }
    return @($output)
}

$reportParent = Split-Path -Parent $ReportPath
if ($reportParent) { New-Item -ItemType Directory -Force $reportParent | Out-Null }
$created = $false
$started = $false
$state = 'not-created'
$failure = $null
try {
    Invoke-VBoxManage @('createvm', '--name', $VmName, '--register') | Out-Null
    $created = $true
    Invoke-VBoxManage @('modifyvm', $VmName, '--memory', $MemoryMB, '--vram', 64,
                        '--graphicscontroller', 'virtio-gpu', '--gpu-backend', 'venus',
                        '--firmware', 'efi', '--audio-enabled', 'off') | Out-Null
    Invoke-VBoxManage @('storagectl', $VmName, '--name', 'SATA', '--add', 'sata', '--controller', 'IntelAhci') | Out-Null
    Invoke-VBoxManage @('storageattach', $VmName, '--storagectl', 'SATA', '--port', 0,
                        '--device', 0, '--type', 'dvddrive', '--medium', $iso) | Out-Null
    Invoke-VBoxManage @('startvm', $VmName, '--type', 'headless') | Out-Null
    $started = $true
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    do {
        $info = Invoke-VBoxManage @('showvminfo', $VmName, '--machinereadable')
        $stateLine = $info | Where-Object { $_ -match '^VMState="([^"]+)"' } | Select-Object -First 1
        $state = if ($stateLine -match '^VMState="([^"]+)"') { $Matches[1] } else { 'unknown' }
        if ($state -eq 'running') { break }
        if ([DateTime]::UtcNow -ge $deadline) { throw "VM did not reach running state within $TimeoutSeconds seconds (state=$state)." }
        Start-Sleep -Milliseconds 500
    } while ($true)
    Write-Host "VirtIO-GPU VM launch: PASS (state=$state, name=$VmName)"
}
catch {
    $failure = $_.Exception.Message
    Write-Error $failure
}
finally {
    if ($started) {
        try { Invoke-VBoxManage @('controlvm', $VmName, 'acpipowerbutton') | Out-Null } catch { }
    }
    if ($created -and -not $KeepVm) {
        try { Invoke-VBoxManage @('unregistervm', $VmName, '--delete') | Out-Null } catch { }
    }
    [ordered]@{
        timestamp = (Get-Date).ToString('o')
        vmName = $VmName
        iso = $iso
        runtimeDirectory = $runtime
        graphicsController = 'virtio-gpu'
        gpuBackend = 'venus'
        memoryMB = $MemoryMB
        created = $created
        started = $started
        state = $state
        keptVm = [bool]$KeepVm
        passed = [bool]($started -and $state -eq 'running' -and -not $failure)
        failure = $failure
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $ReportPath -Encoding utf8
}
if ($failure) { throw $failure }

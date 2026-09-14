# Boots a pinned Ubuntu cloud image with NoCloud seed media. The base image is
# read-only input; each invocation owns its VDI, SSH key and loopback forwarding.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$ImagePath,
    [string]$RuntimeDirectory,
    [string]$QemuImgPath = 'C:\msys64\ucrt64\bin\qemu-img.exe',
    [ValidateSet('software', 'venus')][string]$GpuBackend = 'software',
    [ValidateRange(60, 1800)][int]$TimeoutSeconds = 600,
    [ValidateRange(1024, 65535)][int]$SshPort = 2222,
    [ValidateRange(1, 16)][int]$CpuCount = 1,
    [string]$ReportDirectory
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
if (-not $RuntimeDirectory) {
    $RuntimeDirectory = if (Test-Path (Join-Path $repo 'bin\VBoxManage.exe')) { Join-Path $repo 'bin' }
                        else { Join-Path $repo 'VirtualBox-7.2.6\out\win.amd64\release\bin' }
}
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path
$manage = Join-Path $runtime 'VBoxManage.exe'
$ssh = Join-Path $env:WINDIR 'System32\OpenSSH\ssh.exe'
$keygen = Join-Path $env:WINDIR 'System32\OpenSSH\ssh-keygen.exe'
$image = (Resolve-Path -LiteralPath $ImagePath).Path
foreach ($path in @($manage, $ssh, $keygen, $QemuImgPath)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing tool: $path" }
}
$pin = Get-Content (Join-Path $PSScriptRoot 'linux-test-image.json') -Raw | ConvertFrom-Json
if ((Get-Item $image).Length -ne $pin.bytes -or (Get-FileHash $image -Algorithm SHA256).Hash -ne $pin.sha256) {
    throw 'Image does not match tools/linux-test-image.json. Refusing to boot an unverified image.'
}
$vmName = 'virtio-linux-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
$vmBase = Join-Path $repo '.build\windows\vms'
$vmDir = Join-Path $vmBase $vmName
if (-not $ReportDirectory) { $ReportDirectory = Join-Path $repo ('.build\windows\' + $vmName) }
$reportDir = [IO.Path]::GetFullPath($ReportDirectory)
if (Test-Path -LiteralPath $reportDir) { throw "Report directory already exists: $reportDir" }
New-Item -ItemType Directory -Force $reportDir | Out-Null
$seedDir = Join-Path $reportDir 'seed'
New-Item -ItemType Directory $seedDir | Out-Null
$key = Join-Path $seedDir 'id_ed25519'
$serial = Join-Path $reportDir 'serial.log'
$utf8 = New-Object Text.UTF8Encoding($false)

function Invoke-Native([string]$Exe, [string[]]$Arguments) {
    $saved = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = & $Exe @Arguments 2>&1
        $code = $LASTEXITCODE
    } finally { $ErrorActionPreference = $saved }
    if ($code -ne 0) { throw "$(Split-Path $Exe -Leaf) $($Arguments -join ' ') failed ($code): $($output -join ' ')" }
    return @($output)
}
function VBox([string[]]$Arguments) { Invoke-Native $manage $Arguments }
function Vm-State {
    $info = VBox @('showvminfo', $vmName, '--machinereadable')
    $line = $info | Where-Object { $_ -match '^VMState="([^"]+)"' } | Select-Object -First 1
    if ($line -match '^VMState="([^"]+)"') { return $Matches[1] }
    return 'unknown'
}

$created = $false
$phase = 'prepare'
$failure = $null
$cleanupErrors = @()
$sshReady = $false
$driverBound = $false
$hostVisible = $null
$readySeconds = $null
$hostVulkanDevice = $null
$runtimeHashes = [ordered]@{}
foreach ($name in @('VBoxDD.dll', 'VBoxHeadless.exe', 'VMMR0.r0')) {
    $runtimeHashes[$name] = (Get-FileHash (Join-Path $runtime $name) -Algorithm SHA256).Hash
}
$ready = $false
$state = 'not-created'
try {
    # ProcessStartInfo preserves the empty passphrase on both PowerShell 5/7.
    $si = New-Object Diagnostics.ProcessStartInfo
    $si.FileName = $keygen
    $si.Arguments = '-q -t ed25519 -N "" -C virtio-linux-test -f "' + $key + '"'
    $si.UseShellExecute = $false
    $si.CreateNoWindow = $true
    $kp = [Diagnostics.Process]::Start($si)
    $kp.WaitForExit()
    if ($kp.ExitCode -ne 0) { throw 'Temporary SSH key generation failed.' }
    $pubkey = (Get-Content ($key + '.pub') -Raw).Trim()
    $userData = @"
#cloud-config
hostname: $vmName
manage_etc_hosts: true
disable_root: true
ssh_pwauth: false
users:
  - name: codex
    groups: [sudo, video, render]
    shell: /bin/bash
    lock_passwd: true
    sudo: ['ALL=(ALL) NOPASSWD:ALL']
    ssh_authorized_keys:
      - $pubkey
package_update: false
package_upgrade: false
runcmd:
  - [sh, -c, 'echo VIRTIO_CLOUD_READY > /var/tmp/virtio-cloud-ready; echo VIRTIO_CLOUD_READY > /dev/ttyS0']
"@
    [IO.File]::WriteAllText((Join-Path $seedDir 'user-data'), $userData.Replace("`r`n", "`n") + "`n", $utf8)
    [IO.File]::WriteAllText((Join-Path $seedDir 'meta-data'), "instance-id: $vmName`nlocal-hostname: $vmName`n", $utf8)
    # A VISO is handled by VirtualBox's ISO reader; no external ISO writer or
    # installer modification is needed. Single-quoted paths must remain literal.
    if ($seedDir.Contains("'")) { throw 'Seed path cannot contain a single quote.' }
    $viso = Join-Path $seedDir 'seed.viso'
    $visoText = "--iprt-iso-maker-file-marker-bourne-sh $([guid]::NewGuid()) --volume-id CIDATA --rock-ridge --joliet --file-mode=0444 '/user-data=$(Join-Path $seedDir 'user-data')' '/meta-data=$(Join-Path $seedDir 'meta-data')'"
    [IO.File]::WriteAllText($viso, $visoText, $utf8)
    $phase = 'create'
    VBox @('createvm', '--name', $vmName, '--ostype', 'Ubuntu_64', '--basefolder', $vmBase, '--register') | Out-Null
    $created = $true
    $disk = Join-Path $vmDir 'system.vdi'
    Invoke-Native $QemuImgPath @('convert', '-f', 'qcow2', '-O', 'vdi', $image, $disk) | Out-Null
    VBox @('modifymedium', 'disk', $disk, '--resize', '20480') | Out-Null
    VBox @('modifyvm', $vmName, '--memory', '4096', '--cpus', "$CpuCount", '--firmware', 'bios',
           '--graphicscontroller', 'virtio-gpu', '--gpu-backend', $GpuBackend, '--audio-enabled', 'off',
           '--nic1', 'nat', '--natpf1', "ssh,tcp,127.0.0.1,$SshPort,,22", '--boot1', 'disk', '--boot2', 'none',
           '--uart1', '0x3f8', '4', '--uartmode1', 'file', $serial) | Out-Null
    VBox @('storagectl', $vmName, '--name', 'SATA', '--add', 'sata', '--controller', 'IntelAhci', '--portcount', '2') | Out-Null
    VBox @('storageattach', $vmName, '--storagectl', 'SATA', '--port', '0', '--device', '0', '--type', 'hdd', '--medium', $disk) | Out-Null
    VBox @('storageattach', $vmName, '--storagectl', 'SATA', '--port', '1', '--device', '0', '--type', 'dvddrive', '--medium', $viso) | Out-Null
    $phase = 'boot'
    $bootStart = [DateTime]::UtcNow
    VBox @('startvm', $vmName, '--type', 'headless') | Out-Null
    Write-Host "Linux VM started: $vmName ($GpuBackend); serial log: $serial"
    $sshArgs = @('-T', '-i', $key, '-p', "$SshPort", '-o', 'BatchMode=yes', '-o', 'IdentitiesOnly=yes',
                 '-o', 'ConnectTimeout=5', '-o', 'ConnectionAttempts=1', '-o', 'ServerAliveInterval=10',
                 '-o', 'ServerAliveCountMax=2', '-o', 'StrictHostKeyChecking=accept-new',
                 '-o', "UserKnownHostsFile=$(Join-Path $seedDir 'known_hosts')", 'codex@127.0.0.1')
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    $nextProgress = [DateTime]::UtcNow
    do {
        $state = Vm-State
        if ($state -ne 'running') { throw "VM stopped before guest readiness (state=$state)." }
        $saved = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $probe = & $ssh @sshArgs 'test -f /var/tmp/virtio-cloud-ready && echo VIRTIO_SSH_READY' 2>&1
            $code = $LASTEXITCODE
        } finally { $ErrorActionPreference = $saved }
        if ($code -eq 0 -and ($probe -join "`n") -match 'VIRTIO_SSH_READY') { $sshReady = $true; break }
        if ([DateTime]::UtcNow -ge $nextProgress) {
            Write-Host "Waiting for Linux cloud-init and SSH ($state)..."
            $nextProgress = [DateTime]::UtcNow.AddSeconds(30)
        }
        Start-Sleep -Seconds 3
    } while ([DateTime]::UtcNow -lt $deadline)
    if (-not $sshReady) { throw 'Linux cloud-init/SSH readiness timed out. Inspect serial.log.' }
    $ready = $true
    $readySeconds = [Math]::Round(([DateTime]::UtcNow - $bootStart).TotalSeconds, 2)
    $phase = 'guest-driver'
    # Keep raw evidence even on a missing module or driver timeout. Success
    # requires an actual DRM node bound to virtio_gpu, not just the PCI ID.
    $guestProbe = @'
set -u
echo VIRTIO_GUEST_BEGIN
uname -a
cat /etc/os-release
cloud-init status || true
sudo timeout 30 modprobe virtio_gpu
echo "modprobe_exit=$?"
command -v lspci >/dev/null && lspci -nnk -d 1af4:1050
command -v lspci >/dev/null && sudo lspci -vv -d 1af4:1050
ls -l /dev/dri 2>/dev/null || true
found=0
for card in /sys/class/drm/card[0-9]*; do
    [ -e "$card/device/driver" ] || continue
    pci=$(readlink -f "$card/device")
    driver=$(basename "$(readlink -f "$card/device/driver")")
    echo "drm_node=$card pci_driver=$driver"
    # Linux DRM uses the PCI parent as card.device; the functional GPU driver
    # is bound to its virtio child. Verify both associations and the real node.
    for child in "$pci"/virtio*; do
        [ -e "$child/driver" ] || continue
        function_driver=$(basename "$(readlink -f "$child/driver")")
        echo "virtio_child=$child driver=$function_driver"
        if [ "$function_driver" = virtio_gpu ] && [ -c "/dev/dri/$(basename "$card")" ]; then found=1; fi
    done
done
sudo dmesg | tail -n 220
echo VIRTIO_GPU_KERNEL_LOG
sudo dmesg | grep -Ei 'virtio|\[drm\]|BAR 4' || true
if [ "$found" = 1 ]; then echo VIRTIO_DRM_BOUND; else echo VIRTIO_DRM_MISSING; fi
echo VIRTIO_GUEST_END
'@
    $guestProbePath = Join-Path $reportDir 'guest-probe.sh'
    [IO.File]::WriteAllText($guestProbePath, $guestProbe.Replace("`r`n", "`n") + "`n", $utf8)
    $saved = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        # Windows PowerShell appends CRLF to piped strings. Strip CR at the
        # remote boundary so the extra trailing line cannot become a command.
        $guestOutput = Get-Content $guestProbePath -Raw | & $ssh @sshArgs "tr -d '\r' | bash -s" 2>&1
        $guestExit = $LASTEXITCODE
    } finally { $ErrorActionPreference = $saved }
    $guestText = $guestOutput -join "`n"
    [IO.File]::WriteAllText((Join-Path $reportDir 'guest.log'), $guestText, $utf8)
    $driverBound = $guestExit -eq 0 -and $guestText -match '(?m)^VIRTIO_DRM_BOUND$' -and $guestText -match '(?m)^VIRTIO_GUEST_END$'
    $hostVisible = $guestText -match '\[drm\] features:.*\+host_visible'
    if (-not $driverBound) { throw 'Linux reached SSH, but VirtIO-GPU DRM binding failed. Inspect guest.log.' }
    if ($GpuBackend -eq 'venus' -and -not $hostVisible) { throw 'Linux rejected the Venus host-visible shared-memory region. Inspect guest.log.' }
    $phase = 'complete'
} catch {
    $failure = $_.Exception.Message
    Write-Warning $failure
} finally {
    if ($created) {
        try {
            $state = Vm-State
            if ($state -notin @('poweroff', 'aborted')) {
                VBox @('controlvm', $vmName, 'acpipowerbutton') | Out-Null
                $stopDeadline = [DateTime]::UtcNow.AddSeconds(20)
                do { Start-Sleep -Milliseconds 500; $state = Vm-State }
                while ($state -notin @('poweroff', 'aborted') -and [DateTime]::UtcNow -lt $stopDeadline)
                if ($state -notin @('poweroff', 'aborted')) { VBox @('controlvm', $vmName, 'poweroff') | Out-Null }
            }
        } catch { $cleanupErrors += $_.Exception.Message }
        $log = Join-Path $vmDir 'Logs\VBox.log'
        if (Test-Path -LiteralPath $log) {
            Copy-Item -LiteralPath $log -Destination (Join-Path $reportDir 'VBox.log')
            $match = [regex]::Match((Get-Content $log -Raw), "Vulkan host device '([^']+)'")
            if ($match.Success) { $hostVulkanDevice = $match.Groups[1].Value }
        }
        try { VBox @('unregistervm', $vmName, '--delete') | Out-Null }
        catch { $cleanupErrors += $_.Exception.Message }
    }
    # Remove authentication material after every run, including failed boots.
    # VM disk deletion failures are visible in the report; never kill services.
    Remove-Item -LiteralPath $seedDir -Recurse -Force
    [ordered]@{
        timestamp = (Get-Date).ToString('o'); vmName = $vmName; gpuBackend = $GpuBackend
        sourceImage = $pin; diskMB = 20480; cpuCount = $CpuCount; sshBind = "127.0.0.1:$SshPort"
        created = $created; guestReady = $ready; sshReady = $sshReady; drmDriverBound = $driverBound
        hostVisible = $hostVisible
        readySeconds = $readySeconds; hostVulkanDevice = $hostVulkanDevice; runtimeHashes = $runtimeHashes
        guestVulkanVerified = $false; phase = $phase; failure = $failure
        cleanupErrors = @($cleanupErrors)
        passed = [bool]($driverBound -and -not $failure -and -not $cleanupErrors.Count)
    } | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $reportDir 'report.json') -Encoding utf8
}
if ($failure) { throw $failure }
if ($cleanupErrors.Count) { throw "VM cleanup failed: $($cleanupErrors -join '; ')" }
Write-Host "Linux VirtIO-GPU driver: PASS; report: $reportDir"

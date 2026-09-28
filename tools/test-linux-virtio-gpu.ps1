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
    [ValidateRange(1024, 65536)][int]$MemoryMB = 4096,
    [ValidateSet('ich9', 'piix3')][string]$Chipset = 'ich9',
    [switch]$VerifyGuestVulkan,
    [switch]$RunVulkanWorkload,
    [ValidateRange(0, 3600)][int]$WorkloadSeconds = 0,
    [switch]$SkipGuestVulkanInfo,
    [switch]$VerifyReset,
    [switch]$VerifySaveRestore,
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
$runtimeHome = Join-Path $reportDir 'vbox-home'
New-Item -ItemType Directory -Force $runtimeHome | Out-Null
$previousVBoxUserHome = $env:VBOX_USER_HOME
$previousPath = $env:Path
$runtimeComponents = @('VBoxManage.exe', 'VBoxSVC.exe', 'VBoxC.dll', 'VBoxDD.dll', 'VBoxHeadless.exe', 'VMMR0.r0')
foreach ($name in $runtimeComponents) {
    $component = Join-Path $runtime $name
    if (-not (Test-Path -LiteralPath $component -PathType Leaf)) {
        throw "Runtime directory is incomplete; missing $component"
    }
}
$seedDir = Join-Path $reportDir 'seed'
New-Item -ItemType Directory $seedDir | Out-Null
$authDir = Join-Path ([IO.Path]::GetTempPath()) ('virtio-linux-auth-' + $vmName)
New-Item -ItemType Directory $authDir | Out-Null
$key = Join-Path $authDir 'id_ed25519'
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
function Stop-TestSshProcesses {
    $processes = @(Get-Process -Name ssh -ErrorAction SilentlyContinue |
        Where-Object { $_.Id -notin $sshPidsBefore })
    foreach ($process in $processes) {
        try { Wait-Process -Id $process.Id -Timeout 5 -ErrorAction SilentlyContinue } catch { }
    }
    $processes = @($processes | Where-Object { -not $_.HasExited })
    foreach ($process in $processes) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    }
    foreach ($process in $processes) {
        try { Wait-Process -Id $process.Id -Timeout 5 -ErrorAction SilentlyContinue } catch { }
    }
}

$created = $false
$phase = 'prepare'
$failure = $null
$cleanupErrors = @()
$sshPidsBefore = @(Get-Process -Name ssh -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
$sshReady = $false
$driverBound = $false
$displayModeVerified = $false
$hostVisible = $null
$readySeconds = $null
$hostVulkanDevice = $null
$guestVulkanVerified = $false
$guestVulkanExit = $null
$guestVulkanDevice = $null
$guestVulkanWorkloadVerified = $false
$guestVulkanWorkloadExit = $null
$guestVulkanWorkloadIterations = $null
$saveRestoreVerified = $false
$saveRestoreVulkanExit = $null
$resetVerified = $false
$resetVulkanExit = $null
$vboxSupState = $null
$vboxSupWin32Exit = $null
$runtimeHashes = [ordered]@{}
foreach ($name in $runtimeComponents) {
    $runtimeHashes[$name] = (Get-FileHash (Join-Path $runtime $name) -Algorithm SHA256).Hash
}
$supQuery = (& sc.exe query VBoxSup 2>$null | Out-String)
$supStateMatch = [regex]::Match($supQuery, '(?m)^\s*STATE\s*:\s*\d+\s+(\S+)')
if ($supStateMatch.Success) { $vboxSupState = $supStateMatch.Groups[1].Value }
$supExitMatch = [regex]::Match($supQuery, '(?m)^\s*WIN32_EXIT_CODE\s*:\s*\d+\s+\((0x[0-9A-Fa-f]+)\)')
if ($supExitMatch.Success) { $vboxSupWin32Exit = $supExitMatch.Groups[1].Value }
$env:VBOX_USER_HOME = $runtimeHome
$env:Path = "$runtime;$env:Path"
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
    if ($kp.ExitCode -ne 0 -and -not (Test-Path -LiteralPath $key)) {
        throw 'Temporary SSH key generation failed.'
    }
    $pubkeyPath = $key + '.pub'
    [string]$pubkey = ''
    if (Test-Path -LiteralPath $pubkeyPath) {
        $pubkey = Get-Content $pubkeyPath -Raw
        if ($pubkey) { $pubkey = $pubkey.Trim() }
    }
    if (-not $pubkey) {
        $saved = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $pubkeyOutput = & $keygen -y -f $key 2>$null
            $pubkeyExit = $LASTEXITCODE
        } finally { $ErrorActionPreference = $saved }
        if ($pubkeyExit -ne 0 -or -not $pubkeyOutput) {
            throw 'Temporary SSH public-key derivation failed.'
        }
        $pubkey = ($pubkeyOutput -join "`n").Trim()
    }
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
    VBox @('modifyvm', $vmName, '--memory', "$MemoryMB", '--cpus', "$CpuCount", '--chipset', $Chipset, '--firmware', 'bios',
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
display_mode_found=0
for mode_file in /sys/class/drm/card*-*/modes; do
    [ -f "$mode_file" ] || continue
    echo "drm_modes=$mode_file"
    cat "$mode_file"
    if grep -qx '1024x768' "$mode_file"; then display_mode_found=1; fi
done
if [ "$display_mode_found" = 1 ]; then echo VIRTIO_DISPLAY_MODE_PASS; else echo VIRTIO_DISPLAY_MODE_MISSING; fi
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
    $displayModeVerified = $guestExit -eq 0 -and $guestText -match '(?m)^VIRTIO_DISPLAY_MODE_PASS$'
    $hostVisible = $guestText -match '\[drm\] features:.*\+host_visible'
    if (-not $driverBound) { throw 'Linux reached SSH, but VirtIO-GPU DRM binding failed. Inspect guest.log.' }
    if (-not $displayModeVerified) { throw 'Linux VirtIO-GPU DRM binding succeeded, but no 1024x768 mode was reported. Inspect guest.log.' }
    if ($GpuBackend -eq 'venus' -and -not $hostVisible) { throw 'Linux rejected the Venus host-visible shared-memory region. Inspect guest.log.' }
    if ($VerifyGuestVulkan)
    {
        if ($WorkloadSeconds -gt 0 -and -not $RunVulkanWorkload)
        {
            throw '-WorkloadSeconds requires -RunVulkanWorkload.'
        }
        $phase = 'guest-vulkan'
        $guestVulkanProbe = @'
set -o pipefail
echo VIRTIO_VULKAN_BEGIN
sudo DEBIAN_FRONTEND=noninteractive apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq __VULKAN_PACKAGES__
if ! command -v vulkaninfo >/dev/null; then
    echo VULKANINFO_MISSING
    exit 127
fi
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/virtio_icd.json
if [ "__SKIP_VULKANINFO__" = "1" ]; then
echo VULKANINFO_SKIPPED
echo vulkaninfo_exit=0
rc=0
else
VN_DEBUG=all VK_LOADER_DEBUG=all timeout 20s strace -f -tt -e trace=ioctl -o /var/tmp/virtio-vulkaninfo-strace.log vulkaninfo --summary >/var/tmp/virtio-vulkaninfo-summary.log 2>&1 &
vulkaninfo_pid=$!
sleep 2
sudo python3 -c 'import mmap,struct; f=open("/sys/bus/pci/devices/0000:00:02.0/resource4","rb"); m=mmap.mmap(f.fileno(),4096,access=mmap.ACCESS_READ); print("BAR4_HEAD_TAIL_STATUS", [hex(struct.unpack_from("<I",m,o)[0]) for o in (0,64,128)])' || true
for i in $(seq 1 30); do sudo python3 -c 'import mmap,struct; f=open("/sys/bus/pci/devices/0000:00:02.0/resource4","rb"); m=mmap.mmap(f.fileno(),4096,access=mmap.ACCESS_READ); print([hex(struct.unpack_from("<I",m,o)[0]) for o in (0,64,128)])' || true; sleep 0.1; done
wait "$vulkaninfo_pid"
rc=$?
cat /var/tmp/virtio-vulkaninfo-summary.log
echo VULKANINFO_IOCTL_TRACE
cat /var/tmp/virtio-vulkaninfo-strace.log || true
echo "vulkaninfo_exit=$rc"
if [ "$rc" = 0 ]; then echo VULKANINFO_PASS; else echo VULKANINFO_FAIL; fi
fi
if [ "$rc" = 0 ] && [ "__RUN_VULKAN_WORKLOAD__" = "1" ]; then
cat >/tmp/virtio-vulkan-smoke.c <<'EOF'
#include <vulkan/vulkan.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int fail(const char *where, VkResult rc)
{
    fprintf(stderr, "VULKAN_WORKLOAD_FAIL %s rc=%d\n", where, (int)rc);
    return 1;
}

static int fail_iteration(const char *where, VkResult rc, unsigned long long iteration)
{
    fprintf(stderr, "VULKAN_WORKLOAD_FAIL %s rc=%d iteration=%llu\n",
            where, (int)rc, iteration);
    return 1;
}

int main(void)
{
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer command = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkResult rc;
    const unsigned workload_seconds = __WORKLOAD_SECONDS__;
    unsigned long long iterations = 0;
    uint32_t count = 0, family = UINT32_MAX, type = UINT32_MAX;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkMemoryRequirements requirements;
    VkBool32 coherent = VK_FALSE;
    void *mapped = NULL;

    VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "virtualbox-virtio-gpu-smoke",
        .applicationVersion = 1,
        .pEngineName = "virtualbox-virtio-gpu",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app,
    };
    rc = vkCreateInstance(&instance_info, NULL, &instance);
    if (rc != VK_SUCCESS) return fail("vkCreateInstance", rc);
    rc = vkEnumeratePhysicalDevices(instance, &count, NULL);
    if (rc != VK_SUCCESS || count == 0) return fail("vkEnumeratePhysicalDevices", rc);
    VkPhysicalDevice *devices = calloc(count, sizeof(*devices));
    if (!devices) return fail("calloc", VK_ERROR_OUT_OF_HOST_MEMORY);
    rc = vkEnumeratePhysicalDevices(instance, &count, devices);
    if (rc != VK_SUCCESS) return fail("vkEnumeratePhysicalDevices.data", rc);
    physical = devices[0];
    free(devices);

    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, NULL);
    VkQueueFamilyProperties *families = calloc(count, sizeof(*families));
    if (!families) return fail("calloc.queue", VK_ERROR_OUT_OF_HOST_MEMORY);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, families);
    for (uint32_t i = 0; i < count; ++i)
        if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && families[i].queueCount) {
            family = i;
            break;
        }
    if (family == UINT32_MAX)
        for (uint32_t i = 0; i < count; ++i)
            if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && families[i].queueCount) {
                family = i;
                break;
            }
    free(families);
    if (family == UINT32_MAX) return fail("queue.family", VK_ERROR_INITIALIZATION_FAILED);

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = family,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };
    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
    };
    rc = vkCreateDevice(physical, &device_info, NULL, &device);
    if (rc != VK_SUCCESS) return fail("vkCreateDevice", rc);
    vkGetDeviceQueue(device, family, 0, &queue);
    vkGetPhysicalDeviceMemoryProperties(physical, &memory_properties);

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 4096,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    rc = vkCreateBuffer(device, &buffer_info, NULL, &buffer);
    if (rc != VK_SUCCESS) return fail("vkCreateBuffer", rc);
    vkGetBufferMemoryRequirements(device, buffer, &requirements);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        VkMemoryPropertyFlags flags = memory_properties.memoryTypes[i].propertyFlags;
        if ((requirements.memoryTypeBits & (UINT32_C(1) << i)) &&
            (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            if (type == UINT32_MAX || (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                type = i;
                coherent = (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
            }
        }
    }
    if (type == UINT32_MAX) return fail("memory.type", VK_ERROR_MEMORY_MAP_FAILED);
    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = type,
    };
    rc = vkAllocateMemory(device, &allocate_info, NULL, &memory);
    if (rc != VK_SUCCESS) return fail("vkAllocateMemory", rc);
    rc = vkBindBufferMemory(device, buffer, memory, 0);
    if (rc != VK_SUCCESS) return fail("vkBindBufferMemory", rc);
    rc = vkMapMemory(device, memory, 0, 4096, 0, &mapped);
    if (rc != VK_SUCCESS) return fail("vkMapMemory", rc);
    memset(mapped, 0, 4096);
    if (!coherent) {
        VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, memory, 0, 4096 };
        rc = vkFlushMappedMemoryRanges(device, 1, &range);
        if (rc != VK_SUCCESS) return fail("vkFlushMappedMemoryRanges", rc);
    }

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = family,
    };
    rc = vkCreateCommandPool(device, &pool_info, NULL, &pool);
    if (rc != VK_SUCCESS) return fail("vkCreateCommandPool", rc);
    VkCommandBufferAllocateInfo command_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    rc = vkAllocateCommandBuffers(device, &command_info, &command);
    if (rc != VK_SUCCESS) return fail("vkAllocateCommandBuffers", rc);
    VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    rc = vkBeginCommandBuffer(command, &begin_info);
    if (rc != VK_SUCCESS) return fail("vkBeginCommandBuffer", rc);
    vkCmdFillBuffer(command, buffer, 0, 4096, UINT32_C(0xa5a5a5a5));
    rc = vkEndCommandBuffer(command);
    if (rc != VK_SUCCESS) return fail("vkEndCommandBuffer", rc);
    VkFenceCreateInfo fence_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    rc = vkCreateFence(device, &fence_info, NULL, &fence);
    if (rc != VK_SUCCESS) return fail("vkCreateFence", rc);
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command,
    };
    time_t const start = time(NULL);
    do {
        if (iterations) {
            rc = vkResetFences(device, 1, &fence);
            if (rc != VK_SUCCESS) return fail("vkResetFences", rc);
        }
        rc = vkQueueSubmit(queue, 1, &submit, fence);
        if (rc != VK_SUCCESS) return fail("vkQueueSubmit", rc);
        rc = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_C(10000000000));
        if (rc != VK_SUCCESS) return fail_iteration("vkWaitForFences", rc, iterations);
        if (!coherent) {
            VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, memory, 0, 4096 };
            rc = vkInvalidateMappedMemoryRanges(device, 1, &range);
            if (rc != VK_SUCCESS) return fail("vkInvalidateMappedMemoryRanges", rc);
        }
        uint32_t *words = mapped;
        for (unsigned i = 0; i < 1024; ++i)
            if (words[i] != UINT32_C(0xa5a5a5a5)) {
                fprintf(stderr, "VULKAN_WORKLOAD_FAIL data[%u]=0x%08x iteration=%llu\n",
                        i, words[i], iterations);
                return 1;
            }
        ++iterations;
    } while (workload_seconds && (unsigned long)(time(NULL) - start) < workload_seconds);
    printf("VULKAN_WORKLOAD_PASS family=%u memoryType=%u coherent=%u iterations=%llu seconds=%u\n",
           family, type, coherent, iterations, workload_seconds);
    vkDestroyFence(device, fence, NULL);
    vkFreeCommandBuffers(device, pool, 1, &command);
    vkDestroyCommandPool(device, pool, NULL);
    vkUnmapMemory(device, memory);
    vkFreeMemory(device, memory, NULL);
    vkDestroyBuffer(device, buffer, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    return 0;
}
EOF
cc -std=c11 -O2 -Wall -Wextra /tmp/virtio-vulkan-smoke.c -o /tmp/virtio-vulkan-smoke -lvulkan
timeout __WORKLOAD_TIMEOUT_SECONDS__s /tmp/virtio-vulkan-smoke
workload_rc=$?
echo "vulkan_workload_exit=$workload_rc"
if [ "$workload_rc" = 0 ]; then echo VULKAN_WORKLOAD_PASS; else echo VULKAN_WORKLOAD_FAIL; fi
else
workload_rc=0
fi
echo VIRTIO_VULKAN_END
if [ "$rc" != 0 ]; then exit "$rc"; fi
exit "$workload_rc"
'@
        $guestVulkanProbe = $guestVulkanProbe.Replace('__RUN_VULKAN_WORKLOAD__', $(if ($RunVulkanWorkload) { '1' } else { '0' }))
        $guestVulkanProbe = $guestVulkanProbe.Replace('__SKIP_VULKANINFO__', $(if ($SkipGuestVulkanInfo) { '1' } else { '0' }))
        $guestVulkanProbe = $guestVulkanProbe.Replace('__WORKLOAD_SECONDS__', $WorkloadSeconds.ToString([Globalization.CultureInfo]::InvariantCulture))
        $workloadTimeoutSeconds = [Math]::Max(30, $WorkloadSeconds + 30)
        $guestVulkanProbe = $guestVulkanProbe.Replace('__WORKLOAD_TIMEOUT_SECONDS__', $workloadTimeoutSeconds.ToString([Globalization.CultureInfo]::InvariantCulture))
        $guestVulkanProbe = $guestVulkanProbe.Replace('__VULKAN_PACKAGES__', $(if ($RunVulkanWorkload) { 'mesa-vulkan-drivers vulkan-tools strace build-essential libvulkan-dev' } else { 'mesa-vulkan-drivers vulkan-tools strace' }))
        $guestVulkanPath = Join-Path $reportDir 'guest-vulkan-probe.sh'
        [IO.File]::WriteAllText($guestVulkanPath, $guestVulkanProbe.Replace("`r`n", "`n") + "`n", $utf8)
        $saved = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $guestVulkanCommand = "tr -d '\r' | timeout $TimeoutSeconds bash -s"
            $guestVulkanOutput = Get-Content $guestVulkanPath -Raw | & $ssh @sshArgs $guestVulkanCommand 2>&1
            $guestVulkanExit = $LASTEXITCODE
        } finally { $ErrorActionPreference = $saved }
        $guestVulkanText = $guestVulkanOutput -join "`n"
        [IO.File]::WriteAllText((Join-Path $reportDir 'guest-vulkan.log'), $guestVulkanText, $utf8)
        $vulkanInfoExitMatch = [regex]::Match($guestVulkanText, '(?im)^vulkaninfo_exit=(\d+)\s*$')
        if ($vulkanInfoExitMatch.Success) { $guestVulkanExit = [int]$vulkanInfoExitMatch.Groups[1].Value }
        $guestVulkanVerified = $guestVulkanExit -eq 0 -and ($SkipGuestVulkanInfo -or $guestVulkanText -match '(?m)^VULKANINFO_PASS$')
        $guestVulkanWorkloadExit = 0
        $workloadExitMatch = [regex]::Match($guestVulkanText, '(?im)^vulkan_workload_exit=(\d+)\s*$')
        if ($workloadExitMatch.Success) { $guestVulkanWorkloadExit = [int]$workloadExitMatch.Groups[1].Value }
        $workloadIterationsMatch = [regex]::Match($guestVulkanText, '(?im)^VULKAN_WORKLOAD_PASS.*\biterations=(\d+)\b')
        if ($workloadIterationsMatch.Success) { $guestVulkanWorkloadIterations = [uint64]$workloadIterationsMatch.Groups[1].Value }
        $guestVulkanWorkloadVerified = -not $RunVulkanWorkload -or ($guestVulkanText -match '(?m)^VULKAN_WORKLOAD_PASS$' -and $guestVulkanWorkloadExit -eq 0)
        $deviceMatch = [regex]::Match($guestVulkanText, '(?im)^\s*deviceName\s*=\s*(.+?)\s*$')
        if ($deviceMatch.Success) { $guestVulkanDevice = $deviceMatch.Groups[1].Value.Trim() }
        if (-not $guestVulkanVerified) { throw "Guest vulkaninfo failed (exit $guestVulkanExit). Inspect guest-vulkan.log." }
        if (-not $guestVulkanWorkloadVerified) { throw "Guest Vulkan workload failed (exit $guestVulkanWorkloadExit). Inspect guest-vulkan.log." }
    }
    if ($VerifySaveRestore)
    {
        if (-not $VerifyGuestVulkan -or -not $RunVulkanWorkload)
        {
            throw '-VerifySaveRestore requires -VerifyGuestVulkan and -RunVulkanWorkload.'
        }
        $phase = 'save-restore'
        VBox @('controlvm', $vmName, 'savestate') | Out-Null
        $saveDeadline = [DateTime]::UtcNow.AddSeconds(60)
        do { Start-Sleep -Milliseconds 500; $state = Vm-State }
        while ($state -notin @('saved', 'aborted') -and [DateTime]::UtcNow -lt $saveDeadline)
        if ($state -ne 'saved') { throw "VM save state did not complete (state=$state)." }
        VBox @('startvm', $vmName, '--type', 'headless') | Out-Null
        $restoreDeadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
        $restoreReady = $false
        do {
            $state = Vm-State
            if ($state -ne 'running') { Start-Sleep -Seconds 1; continue }
            $saved = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'Continue'
                $restoreProbe = & $ssh @sshArgs 'test -f /var/tmp/virtio-cloud-ready && echo VIRTIO_SSH_READY' 2>&1
                $restoreProbeExit = $LASTEXITCODE
            } finally { $ErrorActionPreference = $saved }
            if ($restoreProbeExit -eq 0 -and ($restoreProbe -join "`n") -match 'VIRTIO_SSH_READY')
            {
                $restoreReady = $true
                break
            }
            Start-Sleep -Seconds 3
        } while ([DateTime]::UtcNow -lt $restoreDeadline)
        if (-not $restoreReady) { throw 'SSH did not return after VM saved-state restore.' }
        $restorePath = Join-Path $reportDir 'guest-vulkan-restore.log'
        $saved = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $restoreOutput = Get-Content $guestVulkanPath -Raw | & $ssh @sshArgs $guestVulkanCommand 2>&1
            $restoreCommandExit = $LASTEXITCODE
        } finally { $ErrorActionPreference = $saved }
        $restoreText = $restoreOutput -join "`n"
        [IO.File]::WriteAllText($restorePath, $restoreText, $utf8)
        $restoreExitMatch = [regex]::Match($restoreText, '(?im)^vulkan_workload_exit=(\d+)\s*$')
        if ($restoreExitMatch.Success) { $saveRestoreVulkanExit = [int]$restoreExitMatch.Groups[1].Value }
        elseif ($null -ne $restoreCommandExit) { $saveRestoreVulkanExit = $restoreCommandExit }
        $saveRestoreVerified = ($SkipGuestVulkanInfo -or $restoreText -match '(?m)^VULKANINFO_PASS$') -and
            $restoreText -match '(?m)^VULKAN_WORKLOAD_PASS$' -and $saveRestoreVulkanExit -eq 0
        if (-not $saveRestoreVerified) { throw "Saved-state restore Vulkan workload failed (exit $saveRestoreVulkanExit). Inspect guest-vulkan-restore.log." }
    }
    if ($VerifyReset)
    {
        if (-not $VerifyGuestVulkan -or -not $RunVulkanWorkload)
        {
            throw '-VerifyReset requires -VerifyGuestVulkan and -RunVulkanWorkload.'
        }
        $phase = 'reset'
        Stop-TestSshProcesses
        VBox @('controlvm', $vmName, 'reset') | Out-Null
        $resetDeadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
        $resetReady = $false
        do {
            $state = Vm-State
            if ($state -ne 'running') { Start-Sleep -Seconds 1; continue }
            $saved = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'Continue'
                $resetProbe = & $ssh @sshArgs 'test -f /var/tmp/virtio-cloud-ready && echo VIRTIO_SSH_READY' 2>&1
                $resetProbeExit = $LASTEXITCODE
            } finally { $ErrorActionPreference = $saved }
            if ($resetProbeExit -eq 0 -and ($resetProbe -join "`n") -match 'VIRTIO_SSH_READY')
            {
                $resetReady = $true
                break
            }
            Start-Sleep -Seconds 3
        } while ([DateTime]::UtcNow -lt $resetDeadline)
        if (-not $resetReady) { throw 'SSH did not return after VM reset.' }
        $resetPath = Join-Path $reportDir 'guest-vulkan-reset.log'
        $saved = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $resetOutput = Get-Content $guestVulkanPath -Raw | & $ssh @sshArgs $guestVulkanCommand 2>&1
            $resetCommandExit = $LASTEXITCODE
        } finally { $ErrorActionPreference = $saved }
        $resetText = $resetOutput -join "`n"
        [IO.File]::WriteAllText($resetPath, $resetText, $utf8)
        $resetExitMatch = [regex]::Match($resetText, '(?im)^vulkan_workload_exit=(\d+)\s*$')
        if ($resetExitMatch.Success) { $resetVulkanExit = [int]$resetExitMatch.Groups[1].Value }
        elseif ($null -ne $resetCommandExit) { $resetVulkanExit = $resetCommandExit }
        $resetVerified = ($SkipGuestVulkanInfo -or $resetText -match '(?m)^VULKANINFO_PASS$') -and
            $resetText -match '(?m)^VULKAN_WORKLOAD_PASS$' -and $resetVulkanExit -eq 0
        if (-not $resetVerified) { throw "VM reset Vulkan workload failed (exit $resetVulkanExit). Inspect guest-vulkan-reset.log." }
    }
    $phase = 'complete'
} catch {
    $failure = $_.Exception.Message
    Write-Warning $failure
} finally {
    try {
        # A failed SSH probe or remote shell can outlive the pipeline when the
        # guest is powered off.  Reap only SSH processes created by this run so
        # the private key can be removed below without touching user sessions.
        Stop-TestSshProcesses
        if ($created) {
            try {
                $state = 'unknown'
                for ($stateAttempt = 0; $stateAttempt -lt 20; $stateAttempt++) {
                    try { $state = Vm-State; break }
                    catch {
                        # VBoxSVC can invalidate the direct session while
                        # VBoxHeadless is shutting down.  Give that process a
                        # short window to exit before treating the VM as
                        # stopped; a persistent live process remains an error.
                        # A direct-session invalidation is the terminal state
                        # we need for cleanup, even if VBoxHeadless has not
                        # disappeared from the process table yet.
                        if ($_.Exception.Message -match 'VBOX_E_INVALID_OBJECT_STATE|VBOX_E_VM_ERROR') {
                            $state = 'aborted'
                            break
                        }
                        if (-not (Get-Process -Name VBoxHeadless -ErrorAction SilentlyContinue)) {
                            $state = 'aborted'
                            break
                        }
                        if ($stateAttempt -eq 19) { throw }
                        Start-Sleep -Milliseconds 500
                    }
                }
                if ($state -notin @('poweroff', 'aborted')) {
                    VBox @('controlvm', $vmName, 'acpipowerbutton') | Out-Null
                    $stopDeadline = [DateTime]::UtcNow.AddSeconds(20)
                    do {
                        Start-Sleep -Milliseconds 500
                        try { $state = Vm-State }
                        catch {
                            if ($_.Exception.Message -match 'VBOX_E_INVALID_OBJECT_STATE|VBOX_E_VM_ERROR') {
                                $state = 'aborted'
                                break
                            }
                            throw
                        }
                    }
                    while ($state -notin @('poweroff', 'aborted') -and [DateTime]::UtcNow -lt $stopDeadline)
                    if ($state -notin @('poweroff', 'aborted')) { VBox @('controlvm', $vmName, 'poweroff') | Out-Null }
                }
            } catch { $cleanupErrors += $_.Exception.Message }
            $log = Join-Path $vmDir 'Logs\VBox.log'
            if (Test-Path -LiteralPath $log) {
                try {
                    Copy-Item -LiteralPath $log -Destination (Join-Path $reportDir 'VBox.log')
                    $match = [regex]::Match((Get-Content $log -Raw), "Vulkan host device '([^']+)'")
                    if ($match.Success) { $hostVulkanDevice = $match.Groups[1].Value }
                } catch { $cleanupErrors += "VBox.log collection failed: $($_.Exception.Message)" }
            }
            $unregistered = $false
            for ($attempt = 0; $attempt -lt 10 -and -not $unregistered; $attempt++) {
                try {
                    VBox @('unregistervm', $vmName, '--delete') | Out-Null
                    $unregistered = $true
                } catch {
                    if ($attempt -lt 9) { Start-Sleep -Milliseconds 500 }
                    else { $cleanupErrors += $_.Exception.Message }
                }
            }
        }
        # Remove authentication material after every run, including failed boots.
        # VM disk deletion failures are visible in the report; never kill services.
        foreach ($temporaryDir in @($authDir, $seedDir)) {
            $temporaryName = Split-Path -Leaf $temporaryDir
            $temporaryRemoved = $false
            for ($attempt = 0; $attempt -lt 10 -and -not $temporaryRemoved; $attempt++) {
                try {
                    Stop-TestSshProcesses
                    Get-ChildItem -LiteralPath $temporaryDir -Force -Recurse -ErrorAction SilentlyContinue |
                        ForEach-Object { $_.Attributes = [IO.FileAttributes]::Normal }
                    Remove-Item -LiteralPath $temporaryDir -Recurse -Force -ErrorAction Stop
                    $temporaryRemoved = $true
                } catch {
                    try {
                        Get-ChildItem -LiteralPath $temporaryDir -Force -File -Recurse -ErrorAction SilentlyContinue |
                            ForEach-Object { & cmd.exe /d /c ('del /f /q "' + $_.FullName + '"') | Out-Null }
                        Get-ChildItem -LiteralPath $temporaryDir -Force -Directory -Recurse -ErrorAction SilentlyContinue |
                            Sort-Object FullName -Descending |
                            ForEach-Object { & cmd.exe /d /c ('rmdir /q "' + $_.FullName + '"') | Out-Null }
                        & cmd.exe /d /c ('rmdir /q "' + $temporaryDir + '"') | Out-Null
                        $temporaryRemoved = -not (Test-Path -LiteralPath $temporaryDir)
                    } catch { $temporaryRemoved = $false }
                    if (-not $temporaryRemoved -and $attempt -lt 9) { Start-Sleep -Milliseconds 500 }
                    elseif (-not $temporaryRemoved) { $cleanupErrors += "$temporaryName cleanup failed: $($_.Exception.Message)" }
                }
            }
        }
        [ordered]@{
            timestamp = (Get-Date).ToString('o'); vmName = $vmName; gpuBackend = $GpuBackend
            sourceImage = $pin; diskMB = 20480; cpuCount = $CpuCount; memoryMB = $MemoryMB; chipset = $Chipset; sshBind = "127.0.0.1:$SshPort"
            created = $created; guestReady = $ready; sshReady = $sshReady; drmDriverBound = $driverBound
            displayModeVerified = $displayModeVerified; hostVisible = $hostVisible
            readySeconds = $readySeconds; hostVulkanDevice = $hostVulkanDevice
            runtimeDirectory = $runtime; vboxUserHome = $runtimeHome; runtimeHashes = $runtimeHashes
            vboxSupState = $vboxSupState; vboxSupWin32Exit = $vboxSupWin32Exit
            guestVulkanVerified = $guestVulkanVerified; guestVulkanExit = $guestVulkanExit
            guestVulkanDevice = $guestVulkanDevice; guestVulkanWorkloadVerified = $guestVulkanWorkloadVerified
            guestVulkanWorkloadExit = $guestVulkanWorkloadExit; guestVulkanWorkloadIterations = $guestVulkanWorkloadIterations
            workloadSeconds = $WorkloadSeconds; saveRestoreVerified = $saveRestoreVerified
            saveRestoreVulkanExit = $saveRestoreVulkanExit; resetVerified = $resetVerified
            resetVulkanExit = $resetVulkanExit; phase = $phase; failure = $failure
            cleanupErrors = @($cleanupErrors)
            passed = [bool]($driverBound -and $displayModeVerified -and -not $failure -and -not $cleanupErrors.Count -and
                (-not $VerifySaveRestore -or $saveRestoreVerified) -and
                (-not $VerifyReset -or $resetVerified))
        } | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $reportDir 'report.json') -Encoding utf8
    } finally {
        $env:VBOX_USER_HOME = $previousVBoxUserHome
        $env:Path = $previousPath
    }
}
if ($failure) { throw $failure }
if ($cleanupErrors.Count) { throw "VM cleanup failed: $($cleanupErrors -join '; ')" }
Write-Host "Linux VirtIO-GPU driver: PASS; report: $reportDir"

# Runs the production VirtIO transport/GPU callbacks with bounded test RAM.
# Does not register services, install drivers, or launch a VM.
[CmdletBinding()]
param(
    [ValidateRange(1, 300)][int]$TimeoutSeconds = 60,
    [switch]$IncludeRegistration
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$bin = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\bin'
$exe = Join-Path $bin 'testcase\tstVirtioGPU.exe'
$module = Join-Path $bin 'VBoxDD.dll'
$outputDir = Join-Path $repoRoot '.build\windows'
foreach ($path in @($exe, $module)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing build artifact: $path" }
}
New-Item -ItemType Directory -Force $outputDir | Out-Null
$start = [Diagnostics.ProcessStartInfo]::new($exe)
if ($IncludeRegistration) {
    # Windows PowerShell 5.1 does not expose ProcessStartInfo.ArgumentList.
    $start.Arguments = '"' + $module.Replace('"', '\"') + '"'
}
$start.WorkingDirectory = $bin
$start.UseShellExecute = $false
$start.CreateNoWindow = $true
$start.RedirectStandardOutput = $true
$start.RedirectStandardError = $true
$process = [Diagnostics.Process]::new()
$process.StartInfo = $start
$timedOut = $false
try {
    if (-not $process.Start()) { throw 'Could not start VirtIO-GPU tests.' }
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
        $process.Kill($true)
        $process.WaitForExit()
        $timedOut = $true
    }
    $text = $stdout.GetAwaiter().GetResult() + $stderr.GetAwaiter().GetResult()
    $testExit = $process.ExitCode
} finally {
    $process.Dispose()
}
[IO.File]::WriteAllText((Join-Path $outputDir 'virtio-gpu-tests.log'), $text, [Text.UTF8Encoding]::new($false))
Write-Host $text
$passed = -not $timedOut -and $testExit -eq 0 -and $text -match '(?m)^tstVirtioGPU: SUCCESS\s*$'
$groups = @([regex]::Matches($text, '(?m)^tstVirtioGPU: (.+?)\s+: PASSED\s*$') | ForEach-Object { $_.Groups[1].Value.Trim() })
$requiredGroups = @(
    'Vulkan RGBA8 resource format'
    'R8G8B8A8 resource control path'
    'Vulkan persistent buffer transfer throughput'
)
$missingGroups = @($requiredGroups | Where-Object { $_ -notin $groups })
$passed = $passed -and $missingGroups.Count -eq 0
$bufferCopyMatch = [regex]::Match($text, '(?m)^tstVirtioGPU: persistent Vulkan buffer copy: single8=(?<single>\d+) ns, batch8=(?<batch>\d+) ns \((?<perCopy>\d+) ns/copy\)\s*$')
$bufferCopy = $null
if ($bufferCopyMatch.Success) {
    $bufferCopy = [ordered]@{
        single8Ns = [uint64]$bufferCopyMatch.Groups['single'].Value
        batch8Ns = [uint64]$bufferCopyMatch.Groups['batch'].Value
        batchNsPerCopy = [uint64]$bufferCopyMatch.Groups['perCopy'].Value
    }
}
$bufferCopyValid = $null -ne $bufferCopy -and $bufferCopy.single8Ns -gt 0 -and $bufferCopy.batch8Ns -gt 0 -and $bufferCopy.batchNsPerCopy -gt 0
$bufferCopy64Match = [regex]::Match($text, '(?m)^tstVirtioGPU: persistent Vulkan buffer copy64: single64=(?<single>\d+) ns, batch64=(?<batch>\d+) ns \((?<perCopy>\d+) ns/copy\)\s*$')
$bufferCopy64 = $null
if ($bufferCopy64Match.Success) {
    $bufferCopy64 = [ordered]@{
        single64Ns = [uint64]$bufferCopy64Match.Groups['single'].Value
        batch64Ns = [uint64]$bufferCopy64Match.Groups['batch'].Value
        batch64NsPerCopy = [uint64]$bufferCopy64Match.Groups['perCopy'].Value
    }
}
$bufferCopy64Valid = $null -ne $bufferCopy64 -and $bufferCopy64.single64Ns -gt 0 -and $bufferCopy64.batch64Ns -gt 0 -and $bufferCopy64.batch64NsPerCopy -gt 0
$passed = $passed -and $bufferCopyValid -and $bufferCopy64Valid

# Verify the production configuration hand-off that the standalone callback test
# cannot exercise without a registered COM server and a bootable guest image.
$configChecks = [ordered]@{}
$configSources = @{
    modifyVm = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Frontends\VBoxManage\VBoxManageModifyVM.cpp'
    consoleConfig = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Main\src-client\ConsoleImplConfigX86.cpp'
    deviceConstruct = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Devices\VirtIO\DevVirtioGPU.cpp'
    displayPort = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Devices\VirtIO\DevVirtioGPU.cpp'
    deviceRegistration = Join-Path $repoRoot 'VirtualBox-7.2.6\src\VBox\Devices\build\VBoxDD.cpp'
}
foreach ($source in $configSources.GetEnumerator()) {
    if (-not (Test-Path -LiteralPath $source.Value -PathType Leaf)) {
        $configChecks[$source.Key] = $false
        continue
    }
    $sourceText = Get-Content -LiteralPath $source.Value -Raw
    $configChecks[$source.Key] = switch ($source.Key) {
        modifyVm { $sourceText -match 'MODIFYVM_GPU_BACKEND' -and $sourceText -match 'VBoxInternal/Devices/virtio-gpu/0/Config/Backend' }
        consoleConfig { $sourceText -match 'InsertConfigNode\(pDevices, "virtio-gpu"' -and $sourceText -match 'InsertConfigNode\(pVirtioGpuInst, "Config", &pVirtioGpuCfg\)' -and $sourceText -match 'InsertConfigString\(pVirtioGpuCfg, "Backend"' }
        deviceConstruct { $sourceText -match 'pfnCFGMQueryStringDef\(pCfg, "Backend"' -and $sourceText -match 'virtioGpuR3ParseBackend' }
        displayPort { $sourceText -match 'PDMIDISPLAYPORT IPort' -and $sourceText -match 'PDMIDISPLAYPORT, &pThisCC->IPort' -and $sourceText -match 'pfnSetRefreshRate = virtioGpuR3PortSetRefreshRate' }
        deviceRegistration { $sourceText -match 'g_DeviceVirtioGPU' -and $sourceText -match 'pfnRegister' }
    }
}
$configChain = [ordered]@{
    verified = @($configChecks.Values) -notcontains $false
    checks = $configChecks
    note = 'Static source checks cover VBoxManage -> Main CFGM -> virtio-gpu construction; real VM boot still requires COM registration and a guest ISO.'
}
$passed = $passed -and $configChain.verified
$hostVulkan = $null
$hostVulkanDeviceMatch = [regex]::Match($text, '(?m)^(?:tstVirtioGPU: )?host Vulkan device: (?<name>.+?)\s*$')
$hostVulkanApiMatch = [regex]::Match($text, '(?m)^(?:tstVirtioGPU: )?host Vulkan api: (?<major>\d+)\.(?<minor>\d+)\.(?<patch>\d+) memoryTypes=(?<memoryTypes>\d+) deviceLocalMiB=(?<deviceLocalMiB>\d+)\s*$')
$hostVulkanHandlesMatch = [regex]::Match($text, '(?m)^(?:tstVirtioGPU: )?host Vulkan external handles: memory=(?<externalMemory>\w+) semaphore=(?<externalSemaphore>\w+)\s*$')
if ($hostVulkanDeviceMatch.Success -and $hostVulkanApiMatch.Success -and $hostVulkanHandlesMatch.Success) {
    $hostVulkan = [ordered]@{
        deviceName = $hostVulkanDeviceMatch.Groups['name'].Value
        apiVersion = "$($hostVulkanApiMatch.Groups['major'].Value).$($hostVulkanApiMatch.Groups['minor'].Value).$($hostVulkanApiMatch.Groups['patch'].Value)"
        memoryTypes = [int]$hostVulkanApiMatch.Groups['memoryTypes'].Value
        deviceLocalMiB = [uint64]$hostVulkanApiMatch.Groups['deviceLocalMiB'].Value
        externalMemory = $hostVulkanHandlesMatch.Groups['externalMemory'].Value -in @('true', 'TRUE', '1')
        externalSemaphore = $hostVulkanHandlesMatch.Groups['externalSemaphore'].Value -in @('true', 'TRUE', '1')
    }
}
$artifacts = @($exe, $module) | ForEach-Object {
    $fullPath = [IO.Path]::GetFullPath($_)
    $relativePath = $fullPath.Substring($repoRoot.Length).TrimStart([char]92, [char]47)
    [ordered]@{ path = $relativePath; sha256 = (Get-FileHash -LiteralPath $fullPath -Algorithm SHA256).Hash }
}
$report = [ordered]@{
    timestamp = (Get-Date).ToString('o')
    scope = if ($IncludeRegistration) {
        'Windows amd64 userspace transport and device callbacks with test RAM, plus registration from the built VBoxDD.dll'
    } else {
        'Windows amd64 userspace transport and device callbacks with test RAM'
    }
    passed = $passed
    exitCode = $testExit
    timedOut = $timedOut
    passedGroups = $groups
    requiredGroups = $requiredGroups
    missingGroups = $missingGroups
    persistentBufferCopy = $bufferCopy
    persistentBufferCopyValid = $bufferCopyValid
    persistentBufferCopy64 = $bufferCopy64
    persistentBufferCopy64Valid = $bufferCopy64Valid
    hostVulkan = $hostVulkan
    configurationChain = $configChain
    artifacts = @($artifacts)
    guestBootVerified = $false
    displayVerified = $false
    venusVerified = $false
}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $outputDir 'virtio-gpu-validation.json') -Encoding utf8
if (-not $passed) { throw "VirtIO-GPU tests failed (exit $testExit). See .build/windows/virtio-gpu-tests.log." }
Write-Host "VirtIO-GPU validation passed: $($groups.Count) groups; report saved in .build/windows/virtio-gpu-validation.json."

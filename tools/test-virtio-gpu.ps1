# Runs the production VirtIO transport/GPU callbacks with bounded test RAM.
# Does not register services, install drivers, or launch a VM.
[CmdletBinding()]
param([ValidateRange(1, 300)][int]$TimeoutSeconds = 60)
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
$start.ArgumentList.Add($module)
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
$artifacts = @($exe, $module) | ForEach-Object {
    [ordered]@{ path = [IO.Path]::GetRelativePath($repoRoot, $_); sha256 = (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash }
}
$report = [ordered]@{
    timestamp = (Get-Date).ToString('o')
    scope = 'Windows amd64 userspace transport and device callbacks with test RAM, plus registration from the built VBoxDD.dll'
    passed = $passed
    exitCode = $testExit
    timedOut = $timedOut
    passedGroups = $groups
    artifacts = @($artifacts)
    guestBootVerified = $false
    displayVerified = $false
    venusVerified = $false
}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $outputDir 'virtio-gpu-validation.json') -Encoding utf8
if (-not $passed) { throw "VirtIO-GPU tests failed (exit $testExit). See .build/windows/virtio-gpu-tests.log." }
Write-Host "VirtIO-GPU validation passed: $($groups.Count) groups; report saved in .build/windows/virtio-gpu-validation.json."

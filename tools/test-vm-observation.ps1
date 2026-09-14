# Regression checks for the real launch script using a process-level fake CLI.
# No VirtualBox service, driver, network or actual VM is used by these checks.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$root = Join-Path $repo ('.build\windows\vm-observation-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $root | Out-Null
$source = @'
using System;
using System.IO;
class FakeVBoxManage {
    static int Main(string[] args) {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string scenario = File.ReadAllText(Path.Combine(root, "scenario")).Trim();
        string countPath = Path.Combine(root, "count");
        string stopped = Path.Combine(root, "stopped");
        if (args[0] == "createvm") {
            File.WriteAllText(countPath, "0");
            if (File.Exists(stopped)) File.Delete(stopped);
        } else if (args[0] == "showvminfo") {
            int count = int.Parse(File.ReadAllText(countPath)) + 1;
            File.WriteAllText(countPath, count.ToString());
            if (scenario == "missing-state" && !File.Exists(stopped)) return 0;
            string state = "running";
            if (File.Exists(stopped)) state = "poweroff";
            else if (scenario == "aborted" && count >= 2) state = "aborted";
            else if (scenario == "late-poweroff" && count >= 3) state = "poweroff";
            Console.WriteLine("VMState=\"" + state + "\"");
        } else if (args[0] == "controlvm") {
            if (scenario == "cleanup-failed") { Console.Error.WriteLine("test shutdown failure"); return 1; }
            File.WriteAllText(stopped, "yes");
        } else if (args[0] == "unregistervm" && scenario == "cleanup-failed") {
            Console.Error.WriteLine("test unregister failure"); return 1;
        }
        return 0;
    }
}
'@
$sourcePath = Join-Path $root 'FakeVBoxManage.cs'
Set-Content $sourcePath $source -Encoding utf8
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
& $compiler /nologo /target:exe ("/out:" + (Join-Path $root 'VBoxManage.exe')) $sourcePath
if ($LASTEXITCODE -ne 0) { throw 'Could not compile fixture CLI.' }
$iso = Join-Path $root 'fixture.iso'
Set-Content $iso 'test fixture only'
$cases = @(
    @{ name = 'stable'; passed = $true; observe = 1 },
    @{ name = 'aborted'; passed = $false; observe = 1 },
    @{ name = 'late-poweroff'; passed = $false; observe = 1 },
    @{ name = 'missing-state'; passed = $false; observe = 0 },
    @{ name = 'cleanup-failed'; passed = $false; observe = 0 }
)
foreach ($case in $cases) {
    Set-Content (Join-Path $root 'scenario') $case.name
    $reportPath = Join-Path $root ($case.name + '.json')
    $saved = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'test-virtio-gpu-vm.ps1') `
            -RuntimeDirectory $root -IsoPath $iso -VmName ('fixture-' + $case.name) `
            -TimeoutSeconds 1 -ObserveSeconds $case.observe -ReportPath $reportPath 2>&1
        $exitCode = $LASTEXITCODE
    } finally { $ErrorActionPreference = $saved }
    $output | Set-Content (Join-Path $root ($case.name + '.log'))
    $report = Get-Content $reportPath -Raw | ConvertFrom-Json
    if ($report.passed -ne $case.passed -or ($exitCode -eq 0) -ne $case.passed) {
        throw "Wrong result for $($case.name): passed=$($report.passed), exit=$exitCode"
    }
    if ($case.name -eq 'late-poweroff' -and $report.state -ne 'poweroff') { throw 'Final observation missed VM shutdown.' }
    if ($case.name -eq 'cleanup-failed' -and $report.cleanupErrors.Count -lt 1) { throw 'Cleanup failure was lost.' }
    Write-Host "VM observation $($case.name): PASS"
}
Write-Host "VM observation: PASS ($($cases.Count) scenarios); evidence: $root"
exit 0

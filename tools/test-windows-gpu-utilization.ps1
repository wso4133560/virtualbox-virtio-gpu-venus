# Runs repeated host Vulkan tests while sampling Windows GPU Engine counters.
# This measures observable host GPU activity for the test process; it does not
# replace guest Vulkan validation.
[CmdletBinding()]
param(
    [ValidateRange(5, 300)][int]$DurationSeconds = 30,
    [ValidateRange(100, 2000)][int]$SampleIntervalMs = 250,
    [ValidateRange(5, 120)][int]$PerRunTimeoutSeconds = 20,
    [ValidateRange(1, 300)][int]$StressSeconds = 5,
    [string]$ReportPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$bin = Join-Path $repoRoot 'VirtualBox-7.2.6\out\win.amd64\release\bin'
$exe = Join-Path $bin 'testcase\tstVirtioGPU.exe'
$outputDir = Join-Path $repoRoot '.build\windows'
if (-not $ReportPath) { $ReportPath = Join-Path $outputDir 'gpu-utilization-validation.json' }
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Missing build artifact: $exe" }
New-Item -ItemType Directory -Force (Split-Path $ReportPath -Parent) | Out-Null

$counterPath = '\GPU Engine(*)\Utilization Percentage'
$samples = [System.Collections.Generic.List[object]]::new()
$runs = 0
$timeouts = 0
$counterErrors = 0
$deadline = (Get-Date).AddSeconds($DurationSeconds)
$previousStress = [Environment]::GetEnvironmentVariable('VBOX_VIRTIO_GPU_STRESS_SECONDS', 'Process')
[Environment]::SetEnvironmentVariable('VBOX_VIRTIO_GPU_STRESS_SECONDS', $StressSeconds.ToString(), 'Process')

try {
    while ((Get-Date) -lt $deadline) {
        $runs++
        $process = Start-Process -FilePath $exe -WorkingDirectory $bin -WindowStyle Hidden -PassThru
        $runDeadline = (Get-Date).AddSeconds($PerRunTimeoutSeconds)
        try {
            while (-not $process.HasExited -and (Get-Date) -lt $runDeadline -and (Get-Date) -lt $deadline) {
            $timestamp = (Get-Date).ToUniversalTime().ToString('o')
            try {
                $counter = Get-Counter -Counter $counterPath -ErrorAction Stop
                $target = @($counter.CounterSamples | Where-Object { $_.InstanceName -match "^pid_$($process.Id)(?:_|$)" })
                $engines = @($target | ForEach-Object {
                    [ordered]@{
                        instance = $_.InstanceName
                        utilizationPercent = [double]$_.CookedValue
                    }
                })
                $values = @($engines | ForEach-Object { $_.utilizationPercent })
                $sum = if ($values.Count) { [double](($values | Measure-Object -Sum).Sum) } else { 0.0 }
                $max = if ($values.Count) { [double](($values | Measure-Object -Maximum).Maximum) } else { 0.0 }
                $samples.Add([ordered]@{
                    timestamp = $timestamp
                    pid = $process.Id
                    engineCount = $engines.Count
                    maxUtilizationPercent = $max
                    sumUtilizationPercent = $sum
                    engines = $engines
                })
            } catch {
                $counterErrors++
            }
                Start-Sleep -Milliseconds $SampleIntervalMs
            }
            if (-not $process.HasExited) {
                $timeouts++
                $process.Kill($true)
                $process.WaitForExit()
            }
        } finally {
            $process.Dispose()
        }
    }
} finally {
    [Environment]::SetEnvironmentVariable('VBOX_VIRTIO_GPU_STRESS_SECONDS', $previousStress, 'Process')
}

$maxValues = @($samples | ForEach-Object { $_.maxUtilizationPercent })
$sumValues = @($samples | ForEach-Object { $_.sumUtilizationPercent })
$report = [ordered]@{
    timestamp = (Get-Date).ToString('o')
    durationSeconds = $DurationSeconds
    sampleIntervalMs = $SampleIntervalMs
    stressSecondsPerRun = $StressSeconds
    counterPath = $counterPath
    processRuns = $runs
    processTimeouts = $timeouts
    counterErrors = $counterErrors
    targetSamples = $samples.Count
    counterAvailable = ($samples.Count -gt 0 -or $counterErrors -gt 0)
    maxEngineUtilizationPercent = if ($maxValues.Count) { [double](($maxValues | Measure-Object -Maximum).Maximum) } else { $null }
    meanEngineUtilizationPercent = if ($maxValues.Count) { [double](($maxValues | Measure-Object -Average).Average) } else { $null }
    maxSumUtilizationPercent = if ($sumValues.Count) { [double](($sumValues | Measure-Object -Maximum).Maximum) } else { $null }
    samples = @($samples)
}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReportPath -Encoding utf8
Write-Host "GPU utilization sampling complete: runs=$runs samples=$($samples.Count) maxEngine=$($report.maxEngineUtilizationPercent)% meanEngine=$($report.meanEngineUtilizationPercent)%"
Write-Host "Report: $ReportPath"

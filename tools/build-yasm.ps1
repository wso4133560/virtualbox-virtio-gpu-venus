[CmdletBinding()]
param([Parameter(Mandatory)][string]$VisualStudioRoot)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$source = Join-Path $repoRoot '.build\deps\yasm-source\yasm-1.3.0'
$output = Join-Path $repoRoot '.build\deps\yasm-vbox'
$vcvars = Join-Path $VisualStudioRoot 'VC\Auxiliary\Build\vcvars64.bat'
$cmake = Join-Path $VisualStudioRoot 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
if (-not (Test-Path $cmake)) { $cmake = (Get-Command cmake.exe -ErrorAction Stop).Source }
$python = (Get-Command python.exe -ErrorAction Stop).Source
& $python (Join-Path $PSScriptRoot 'prepare-yasm.py') $source
if ($LASTEXITCODE -ne 0) { throw 'YASM source preparation failed.' }
New-Item -ItemType Directory -Force $output | Out-Null
$q = [char]34
$configure = "$q$cmake$q -S $q$source$q -B $q$output$q -G $q`NMake Makefiles$q -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DYASM_BUILD_TESTS=OFF -DPYTHON_EXECUTABLE=$q$python$q"
& cmd.exe /d /s /c "call $q$vcvars$q && $configure" > (Join-Path $output 'configure.log') 2>&1
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $output 'configure.log') -Tail 25
    throw 'YASM CMake configuration failed.'
}
& cmd.exe /d /s /c "call $q$vcvars$q && $q$cmake$q --build $q$output$q --target yasm" > (Join-Path $output 'build.log') 2>&1
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $output 'build.log') -Tail 25
    throw 'YASM build failed.'
}
Write-Host "YASM built: $output\yasm.exe"
& $python (Join-Path $PSScriptRoot 'test-yasm.py') (Join-Path $output 'yasm.exe')
if ($LASTEXITCODE -ne 0) { throw 'YASM regression failed.' }

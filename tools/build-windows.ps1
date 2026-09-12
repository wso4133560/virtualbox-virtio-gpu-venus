[CmdletBinding()]
param(
    [ValidateSet('amd64')]
    [string]$TargetArch = 'amd64',
    [int]$Jobs = 8,
    [switch]$ConfigureOnly,
    [switch]$WithoutHardening
)

$ErrorActionPreference = 'Stop'
$sourceRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\VirtualBox-7.2.6')).Path
$vsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vsWhere)) {
    $vsWhere = 'C:\gurobi1001\win64\bin\vswhere.exe'
}
if (-not (Test-Path $vsWhere)) {
    throw '未找到 vswhere.exe，请安装 Visual Studio 2022。'
}

$vsRoot = (& $vsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
if (-not $vsRoot) {
    throw '未找到带 C++ amd64 工具链的 Visual Studio 安装。'
}
$vcVars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvars64.bat'
$kmk = Join-Path $sourceRoot 'kBuild\bin\win.amd64\kmk.exe'
$nasm = (Get-Command nasm.exe -ErrorAction SilentlyContinue).Source
if (-not $nasm) {
    $nasm = Join-Path $env:LOCALAPPDATA 'bin\NASM\nasm.exe'
}
if (-not (Test-Path $nasm)) {
    throw '未找到 NASM，请安装 NASM 并加入 PATH。'
}

$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
$sdkVersions = Get-ChildItem (Join-Path $sdkRoot 'Include') -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^10\.' } |
    Sort-Object Name -Descending
if (-not $sdkVersions) {
    throw '未找到 Windows 10 SDK。'
}
$wdkHeader = $sdkVersions | ForEach-Object { Join-Path $_.FullName 'km\ntifs.h' } | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $wdkHeader) {
    throw '未找到 ntifs.h。请安装与 Windows SDK 匹配的 Windows Driver Kit (WDK)。'
}

if (-not (Get-Command xsltproc.exe -ErrorAction SilentlyContinue)) {
    throw '未找到 xsltproc.exe。请安装 VirtualBox 构建工具包中的 libxslt。'
}

$q = [char]34
$configure = "cscript //nologo configure.vbs --target-arch=$TargetArch --continue-on-error --disable-SDL --disable-COM --with-vc=$q$vsRoot$q --with-sdk10=$q$sdkRoot$q --with-nasm=$q$nasm$q"
$build = "call env.bat && $q$kmk$q VBOX_SVN_REV=172246 TOOL_YASM_AS=$q$nasm$q -j$Jobs quick"
if ($WithoutHardening) {
    $build = "call env.bat && $q$kmk$q VBOX_WITHOUT_HARDENING=1 VBOX_SVN_REV=172246 TOOL_YASM_AS=$q$nasm$q -j$Jobs quick"
}

$command = "call $q$vcVars$q && cd /d $q$sourceRoot$q && $configure"
& cmd.exe /d /s /c $command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
if ($ConfigureOnly) {
    exit 0
}

& cmd.exe /d /s /c "call $q$vcVars$q && cd /d $q$sourceRoot$q && $build"
exit $LASTEXITCODE

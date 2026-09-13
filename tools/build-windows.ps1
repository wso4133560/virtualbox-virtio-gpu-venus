# Development build: no installation, driver loading, or system configuration changes.
[CmdletBinding()]
param(
    [ValidateRange(1, 64)][int]$Jobs = 8,
    [string[]]$Targets = @('VBoxRT', 'VBoxManage', 'VBoxHeadless', 'VBoxSVC', 'VBoxSDS', 'VBoxVMM', 'VBoxDD', 'VBoxDD2', 'VBoxDDU', 'VMMR0', 'VBoxSup', 'VBoxSup-inf', 'tstLdr', 'tstLdrLoadConfig', 'tstRTXml', 'tstRTSemEvent'),
    [string]$VisualStudioRoot,
    [string]$SdkRoot = (Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'),
    [string]$SdkVersion = '10.0.26100.0',
    [string]$WdkRoot,
    [string]$YasmPath,
    [string]$NasmPath,
    [string]$XsltprocPath,
    [switch]$CheckOnly,
    [switch]$ConfigureOnly,
    [switch]$ReuseConfig,
    [switch]$WithoutHardening,
    [switch]$EnableVirtioGpu,
    [switch]$EnableVirtioGpuVenus
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$sourceRoot = Join-Path $repoRoot 'VirtualBox-7.2.6'
$buildRoot = Join-Path $repoRoot '.build\windows'
New-Item -ItemType Directory -Force $buildRoot | Out-Null

function Require-File([string]$Path, [string]$Description) {
    if (-not $Path -or -not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing ${Description}: $Path"
    }
}
function Kmk-Path([string]$Path) { $Path.Replace('\', '/') }
if (-not $VisualStudioRoot) {
    $vswhere = Get-Command vswhere.exe -ErrorAction SilentlyContinue
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if ($vswhere) { $vswherePath = $vswhere.Source }
    if (Test-Path $vswherePath) {
        $VisualStudioRoot = (& $vswherePath -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
    }
    if (-not $VisualStudioRoot) {
        $VisualStudioRoot = Get-ChildItem (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022') -Directory |
            Where-Object { Test-Path (Join-Path $_.FullName 'VC\Auxiliary\Build\vcvars64.bat') } |
            Select-Object -First 1 -ExpandProperty FullName
    }
}
if (-not $VisualStudioRoot) { throw 'Visual Studio 2022 C++ tools were not found.' }
$vcvars = Join-Path $VisualStudioRoot 'VC\Auxiliary\Build\vcvars64.bat'
$kmk = Join-Path $sourceRoot 'kBuild\bin\win.amd64\kmk.exe'
if (-not $WdkRoot) { $WdkRoot = Join-Path $repoRoot '.build\deps\wdk\c' }
if (-not $YasmPath) {
    if (-not $CheckOnly) { & (Join-Path $PSScriptRoot 'build-yasm.ps1') -VisualStudioRoot $VisualStudioRoot }
    $YasmPath = Join-Path $repoRoot '.build\deps\yasm-vbox\yasm.exe'
}
if (-not $NasmPath) { $NasmPath = Join-Path $repoRoot '.build\deps\nasm\nasm-2.16.03\nasm.exe' }
if (-not $XsltprocPath) { $XsltprocPath = Join-Path $repoRoot '.build\deps\msys2\mingw64\bin\xsltproc.exe' }
$midl = Join-Path $SdkRoot "bin\$SdkVersion\x64\midl.exe"
$kmInclude = Join-Path $WdkRoot "Include\$SdkVersion\km"
$kmLib = Join-Path $WdkRoot "Lib\$SdkVersion\km\x64"
Require-File $vcvars 'VS2022 environment'
Require-File $kmk 'kBuild'
Require-File $midl 'SDK MIDL'
Require-File (Join-Path $SdkRoot "Include\$SdkVersion\um\Windows.h") 'SDK headers'
Require-File (Join-Path $kmInclude 'ntifs.h') 'WDK headers'
Require-File (Join-Path $kmLib 'ntoskrnl.lib') 'WDK amd64 libraries'
Require-File $YasmPath 'YASM'
Require-File $NasmPath 'NASM'
Require-File $XsltprocPath 'xsltproc'
foreach ($tool in @($YasmPath, $NasmPath, $XsltprocPath)) {
    & $tool --version
    if ($LASTEXITCODE -ne 0) { throw "Tool cannot run: $tool (exit $LASTEXITCODE)" }
}
Write-Host "SDK/WDK: $SdkVersion; VS: $VisualStudioRoot"
if ($CheckOnly) { return }

# configure.vbs normally overwrites tracked Linux configuration. Preserve bytes,
# then move the Windows output to .build and pass AUTOCFG/LOCALCFG to kBuild.
$q = [char]34
$autoConfig = Join-Path $buildRoot 'AutoConfig.kmk'
$localConfig = Join-Path $buildRoot 'LocalConfig.kmk'
$envFile = Join-Path $buildRoot 'env.bat'
if ($ReuseConfig) {
    Require-File $autoConfig 'reusable Windows AutoConfig.kmk'
    Require-File $envFile 'reusable Windows env.bat'
    Write-Host "Reusing Windows configuration: $buildRoot"
} else {
    $snapshots = @{}
    foreach ($name in @('AutoConfig.kmk', 'configure.log', 'env.bat')) {
        $path = Join-Path $sourceRoot $name
        $snapshots[$path] = if (Test-Path $path) { [IO.File]::ReadAllBytes($path) } else { $null }
    }
    try {
        $vcRoot = Join-Path $VisualStudioRoot 'VC'
        $configure = "cscript //nologo configure.vbs --target-arch=amd64 --disable-sdl --disable-additions --disable-pylint --with-vc=$q$vcRoot$q --with-sdk10=$q$SdkRoot$q --with-midl=$q$midl$q --with-yasm=$q$YasmPath$q --with-nasm=$q$NasmPath$q"
        & cmd.exe /d /s /c "call $q$vcvars$q && cd /d $q$sourceRoot$q && $configure" > (Join-Path $buildRoot 'configure-output.log') 2>&1
        $configureExit = $LASTEXITCODE
        $configureOutput = Get-Content (Join-Path $buildRoot 'configure-output.log') -Raw
        if ($configureExit -ne 0 -or $configureOutput -notmatch 'Execute env.bat once before you start to build VBox:') {
            Get-Content (Join-Path $buildRoot 'configure-output.log') -Tail 35
            throw "Configure failed (exit $configureExit). See .build/windows/configure-output.log."
        }
        $generatedConfig = Get-Content (Join-Path $sourceRoot 'AutoConfig.kmk') -Raw
        foreach ($required in @('VBOX_VCC_TOOL_STEM\s*:= VCC143', 'PATH_TOOL_VCC143\s*:= .+', 'VBOX_MAIN_IDL\s*:= .+', 'PATH_TOOL_YASM\s*:= .+')) {
            if ($generatedConfig -notmatch $required) { throw "Incomplete Windows configuration: missing $required" }
        }
        if ($generatedConfig -notmatch "SDK_WINSDK10_VERSION\s*:= $([regex]::Escape($SdkVersion))\s") {
            throw "Configure selected an SDK other than the requested $SdkVersion."
        }
        Copy-Item (Join-Path $sourceRoot 'AutoConfig.kmk') $autoConfig -Force
        Copy-Item (Join-Path $sourceRoot 'env.bat') $envFile -Force
    } finally {
        foreach ($path in $snapshots.Keys) {
            if ($null -ne $snapshots[$path]) { [IO.File]::WriteAllBytes($path, $snapshots[$path]) }
            elseif (Test-Path $path) { Remove-Item -LiteralPath $path -Force }
        }
    }
}
$local = @(
    "SDK_WINSDK10_VERSION := $SdkVersion",
    "SDK_WINSDK10_MAX_VERSION := $SdkVersion",
    "PATH_SDK_WINSDK10_KM_INC := $(Kmk-Path $kmInclude)",
    "PATH_SDK_WINSDK10_KM_LIB.amd64 := $(Kmk-Path $kmLib)",
    "VBOX_XSLTPROC := $(Kmk-Path $XsltprocPath)",
    'VBOX_YASM_Wno-segreg-in-64bit := -Wno-segreg-in-64bit',
    'VBOX_WITHOUT_ADDITIONS := 1',
    'VBOX_WITH_32_ON_64_MAIN_API :=',
    'VBOX_WITHOUT_DOCS := 1',
    'VBOX_WITH_QTGUI :=',
    'VBOX_WITH_QT_PAYLOAD :=',
    'VBOX_WITH_MESA3D :=',
    'VBOX_WITH_VMSVGA3D :=',
    'VBOX_WITH_VMSVGA3D_DX :=',
    'VBOX_WITH_PYTHON :=',
    'VBOX_WITH_WEBSERVICES :=',
    'VBOX_WITH_JAVA :=',
    # Use the revision recorded in this archive's Version.kmk, not stale output.
    'VBOX_SVN_REV = $(VBOX_SVN_REV_VERSION_FALLBACK)'
)
if ($WithoutHardening) { $local += 'VBOX_WITHOUT_HARDENING := 1' }
if ($EnableVirtioGpu) { $local += 'VBOX_WITH_VIRTIO_GPU := 1' }
if ($EnableVirtioGpuVenus) { $local += 'VBOX_WITH_VIRTIO_GPU_VENUS := 1' }
[IO.File]::WriteAllLines($localConfig, $local, [Text.UTF8Encoding]::new($false))
if ($ConfigureOnly) { Write-Host "Windows configuration: $buildRoot"; return }
# Match the bundled libxml2 Makefile.kmk source list. Upstream configure.js
# defaults enable modules (XPath, HTML, network, schemas) that VBox does not link.
$xmlRoot = Join-Path $sourceRoot 'src\libs\libxml2-2.13.8'
$xmlHeader = Join-Path $xmlRoot 'include\libxml\xmlversion.h'
$xmlStamp = if (Test-Path $xmlHeader) { Get-Item $xmlHeader } else { $null }
$xmlBefore = if ($xmlStamp) { [IO.File]::ReadAllText($xmlHeader) } else { $null }
$xmlMsvcConfig = Join-Path $xmlRoot 'win32\config.msvc'
$xmlMsvcBefore = [IO.File]::ReadAllBytes($xmlMsvcConfig)
Push-Location (Join-Path $xmlRoot 'win32')
try {
    & cscript.exe //nologo //E:JScript configure.js threads=native ftp=no http=no html=no c14n=no catalog=no xpath=no xptr=no xinclude=no iconv=no icu=no zlib=yes lzma=no xml_debug=no regexps=no modules=no schemas=no schematron=no > (Join-Path $buildRoot 'libxml-configure.log') 2>&1
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $xmlHeader)) { throw 'libxml2 configuration failed.' }
    if ($xmlBefore -eq [IO.File]::ReadAllText($xmlHeader)) { (Get-Item $xmlHeader).LastWriteTimeUtc = $xmlStamp.LastWriteTimeUtc }
} finally {
    [IO.File]::WriteAllBytes($xmlMsvcConfig, $xmlMsvcBefore)
    Pop-Location
}
foreach ($target in $Targets) {
    if ($target -notmatch '^[A-Za-z0-9_-]+$') { throw "Invalid build target: $target" }
}
$targetArgs = $Targets -join ' '
$featureArgs = @()
if ($EnableVirtioGpu) { $featureArgs += 'VBOX_WITH_VIRTIO_GPU=1' }
if ($EnableVirtioGpuVenus) { $featureArgs += 'VBOX_WITH_VIRTIO_GPU_VENUS=1' }
$featureArgsText = $featureArgs -join ' '
# The bundled libvpx source keeps vpx_version.h generated by its configure
# step.  The Windows-only target set can reach libvpx without running that
# step, so materialize the deterministic header when it is absent.
$vpxRoot = Join-Path $sourceRoot 'src\libs\libvpx-1.14.1'
$vpxVersionHeader = Join-Path $vpxRoot 'vpx_version.h'
$vpxAsmConfig = Join-Path $vpxRoot 'vpx_config.asm'
if (-not (Test-Path -LiteralPath $vpxVersionHeader -PathType Leaf)) {
    @'
/* Generated by tools/build-windows.ps1 for the bundled libvpx source. */
#define VERSION_MAJOR 1
#define VERSION_MINOR 14
#define VERSION_PATCH 1
#define VERSION_EXTRA ""
#define VERSION_PACKED ((VERSION_MAJOR<<16)|(VERSION_MINOR<<8)|(VERSION_PATCH))
#define VERSION_STRING_NOSP "1.14.1"
#define VERSION_STRING " 1.14.1"
'@ | Set-Content -LiteralPath $vpxVersionHeader -Encoding ASCII
}
if (-not (Test-Path -LiteralPath $vpxAsmConfig -PathType Leaf)) {
    @'
; Generated by tools/build-windows.ps1 for the bundled libvpx source.
%define CONFIG_PIC 1
%define CONFIG_POSTPROC 1
%define CONFIG_VP9_POSTPROC 0
%define VPX_ARCH_X86_64 1
%define VPX_ARCH_X86 0
'@ | Set-Content -LiteralPath $vpxAsmConfig -Encoding ASCII
}
$build = "call $q$envFile$q && $q$kmk$q AUTOCFG=$q$(Kmk-Path $autoConfig)$q LOCALCFG=$q$(Kmk-Path $localConfig)$q SDK_WINSDK10_MAX_VERSION=$SdkVersion $featureArgsText -j$Jobs $targetArgs"
$buildLog = Join-Path $buildRoot 'build.log'
if (Test-Path $buildLog) {
    Copy-Item $buildLog (Join-Path $buildRoot ("build-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss-fff')))
}
& cmd.exe /d /s /c "call $q$vcvars$q && cd /d $q$sourceRoot$q && $build" > $buildLog 2>&1
$buildExit = $LASTEXITCODE
Get-Content $buildLog -Tail 30
if ($buildExit -ne 0) { throw "Build failed (exit $buildExit). See $buildLog" }
Write-Host "Build passed: $targetArgs"

# Whetstone Editor - Windows Build Script
# Prerequisites: Visual Studio 2022, CMake 3.20+, vcpkg
# Usage: .\build.ps1 [-Config Release|Debug] [-VcpkgRoot E:\vcpkg]

param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",

    [string]$VcpkgRoot = $env:VCPKG_ROOT
)

$ErrorActionPreference = "Stop"
$EditorDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$EditorDir = Join-Path $EditorDir "editor"
$BuildDir = Join-Path $EditorDir "build"
$StagingDir = Join-Path $PSScriptRoot "staging"
$Triplet = "x64-windows"

function Write-Step($msg) {
    Write-Host "`n==> $msg" -ForegroundColor Cyan
}

function Test-Command($cmd) {
    return [bool](Get-Command $cmd -ErrorAction SilentlyContinue)
}

# --- Check prerequisites ---

Write-Step "Checking prerequisites"

# CMake
if (-not (Test-Command "cmake")) {
    Write-Error "CMake not found. Install CMake 3.20+ and add it to PATH."
}
$cmakeVersion = (cmake --version | Select-Object -First 1)
Write-Host "  CMake: $cmakeVersion"

# Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    Write-Host "  Visual Studio: $vsPath"
} else {
    Write-Warning "vswhere not found - Visual Studio may not be installed."
}

# vcpkg
if (-not $VcpkgRoot) {
    # Try common locations
    $candidates = @("E:\vcpkg", "C:\vcpkg", "$env:USERPROFILE\vcpkg")
    foreach ($c in $candidates) {
        if (Test-Path (Join-Path $c "vcpkg.exe")) {
            $VcpkgRoot = $c
            break
        }
    }
}

if (-not $VcpkgRoot -or -not (Test-Path (Join-Path $VcpkgRoot "vcpkg.exe"))) {
    Write-Error "vcpkg not found. Set VCPKG_ROOT or pass -VcpkgRoot <path>."
}
Write-Host "  vcpkg: $VcpkgRoot"

$vcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
$toolchainFile = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"

# --- Install vcpkg dependencies ---

Write-Step "Installing vcpkg dependencies"

$packages = @(
    "nlohmann-json:$Triplet",
    "sdl2:$Triplet",
    "imgui[docking-experimental,opengl3-binding]:$Triplet",
    "glad:$Triplet"
)

foreach ($pkg in $packages) {
    Write-Host "  Installing $pkg ..."
    & $vcpkgExe install $pkg --recurse 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "vcpkg install $pkg returned exit code $LASTEXITCODE (may already be installed)"
    }
}

Write-Host "  All vcpkg packages installed." -ForegroundColor Green

# --- Configure CMake ---

Write-Step "Configuring CMake ($Config)"

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

cmake -S $EditorDir -B $BuildDir `
    -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_TOOLCHAIN_FILE="$toolchainFile" `
    -DVCPKG_TARGET_TRIPLET="$Triplet"

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configure failed."
}

# --- Build ---

Write-Step "Building ($Config)"

$targets = @("whetstone_editor", "orchestrator")
foreach ($t in $targets) {
    cmake --build $BuildDir --config $Config --target $t --parallel
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build of $t failed."
    }
}

Write-Host "`n  Build succeeded." -ForegroundColor Green

# --- Stage output for installer ---

Write-Step "Staging build output to $StagingDir"

if (Test-Path $StagingDir) {
    Remove-Item -Recurse -Force $StagingDir
}
New-Item -ItemType Directory -Path $StagingDir | Out-Null

# Copy executables
$binDir = Join-Path $BuildDir $Config
$exes = @("whetstone_editor.exe", "orchestrator.exe")
foreach ($exe in $exes) {
    $src = Join-Path $binDir $exe
    if (Test-Path $src) {
        Copy-Item $src $StagingDir
        Write-Host "  Copied $exe"
    } else {
        Write-Warning "  $exe not found at $src"
    }
}

# Copy DLLs from vcpkg installed dir
$vcpkgBinDir = Join-Path $VcpkgRoot "installed\$Triplet\bin"
if (Test-Path $vcpkgBinDir) {
    $dlls = @("SDL2.dll")
    foreach ($dll in $dlls) {
        $src = Join-Path $vcpkgBinDir $dll
        if (Test-Path $src) {
            Copy-Item $src $StagingDir
            Write-Host "  Copied $dll"
        }
    }
}

# Also copy any DLLs that ended up in the build output directory
Get-ChildItem -Path $binDir -Filter "*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
    if (-not (Test-Path (Join-Path $StagingDir $_.Name))) {
        Copy-Item $_.FullName $StagingDir
        Write-Host "  Copied $($_.Name) (from build output)"
    }
}

Write-Step "Done!"
Write-Host "  Executables staged in: $StagingDir"
Write-Host "  To create the installer, compile setup.iss with Inno Setup."
Write-Host ""

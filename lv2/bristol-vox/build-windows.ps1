# Build Bristol Vox LV2 on Windows (MSYS2 MinGW)
#
# Prerequisites: install MSYS2 from https://www.msys2.org/
# Then in the "UCRT64" shell:
#   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make \
#       mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-lv2 \
#       mingw-w64-ucrt-x86_64-winpthreads
#
# Usage (PowerShell, from this directory):
#   .\build-windows.ps1
#   .\build-windows.ps1 -Install
#   .\build-windows.ps1 -Test

param(
    [switch]$Install,
    [switch]$Test,
    [string]$MsysRoot = "C:\msys64",
    [string]$MsysEnv = "ucrt64"
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path

$make = Join-Path $MsysRoot "$MsysEnv\bin\mingw32-make.exe"
$bash = Join-Path $MsysRoot "usr\bin\bash.exe"

if (-not (Test-Path $make)) {
    Write-Error @"
MinGW make not found at: $make

Install MSYS2 ($MsysRoot) and the UCRT64 toolchain:
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make \
      mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-lv2 \
      mingw-w64-ucrt-x86_64-winpthreads
"@
}

$target = if ($Test -and $Install) { "test install" } elseif ($Test) { "test" } elseif ($Install) { "install" } else { "all" }
$drive = $here.Substring(0, 1).ToLower()
$rest = $here.Substring(2).Replace("\", "/")
$unixPath = "/$drive$rest"
$makeUnix = "/$MsysEnv/bin/mingw32-make"

# Install to the Windows user profile, not MSYS2 /home/.../.lv2
$installDir = $env:USERPROFILE.Replace("\", "/")
if ($installDir -match "^([A-Za-z]):(.*)$") {
    $installUnix = "/" + $Matches[1].ToLower() + $Matches[2] + "/.lv2"
} else {
    $installUnix = $installDir + "/.lv2"
}

$makeArgs = if ($Test -and $Install) {
    "test install INSTALL_DIR='$installUnix'"
} elseif ($Test) {
    "test"
} elseif ($Install) {
    "install INSTALL_DIR='$installUnix'"
} else {
    "all"
}

# Use MSYS bash with UCRT64 on PATH so make finds gcc/pkg-config.
$bashCmd = "export PATH=/$MsysEnv/bin:`$PATH; cd '$unixPath' && '$makeUnix' -f Makefile.win $makeArgs"
& $bash -lc $bashCmd
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Build OK. Bundle: $here\bristol-vox.lv2\"
if ($Install) {
    $dest = Join-Path $env:USERPROFILE ".lv2\bristol-vox.lv2"
    Write-Host "Installed to: $dest"
}
Write-Host ""
Write-Host "Test with Carla (64-bit): add LV2 path $env:USERPROFILE\.lv2"

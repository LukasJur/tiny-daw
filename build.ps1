<#
.SYNOPSIS
    Builds the TinyDAW target. Pass -Run to launch it afterwards if the build succeeds.
#>
param(
    [switch]$Run
)

function Find-CMake {
    # Prefer cmake on PATH if it's there.
    $onPath = Get-Command cmake -ErrorAction SilentlyContinue
    if ($onPath) { return $onPath.Source }

    # Otherwise ask vswhere (Microsoft's own tool for locating Visual Studio
    # installs) where VS lives, rather than hardcoding a version/edition-specific
    # path that would break on a different machine.
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -property installationPath
        if ($vsPath) {
            $bundled = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            if (Test-Path $bundled) { return $bundled }
        }
    }

    return $null
}

$cmake = Find-CMake
if (-not $cmake) {
    Write-Error "Could not find cmake. Install CMake (or Visual Studio's 'Desktop development with C++' workload, which bundles it) and try again."
    exit 1
}

& $cmake --build "$PSScriptRoot\build" --config Debug --target TinyDAW

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if ($Run) {
    & "$PSScriptRoot\build\TinyDAW_artefacts\Debug\Tiny DAW.exe"
}

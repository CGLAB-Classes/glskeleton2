#
# setup.ps1 -- Install build dependencies and initialize git submodules (Windows).
#
# Usage (run in PowerShell as Administrator):
#   .\scripts\setup.ps1
#

$ErrorActionPreference = "Stop"

# Detect standalone (piped) execution vs. running inside the repo
if ($PSCommandPath) {
    $ProjectDir = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
} else {
    $ProjectDir = ""
}

Write-Host "=== glskeleton setup ===" -ForegroundColor Cyan

# --- Check for required tools ---

$missing = @()

# Check Visual Studio 2022+ (version 17.0+)
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -version "[17.0," -latest -prerelease -property installationPath 2>$null
    if (-not $vsPath) {
        # Check if an older version (e.g. VS2019) is installed
        $oldVsPath = & $vsWhere -latest -prerelease -property installationPath 2>$null
        if ($oldVsPath) {
            $oldVsYear = & $vsWhere -latest -prerelease -property catalog_productLineVersion 2>$null
            Write-Host ""
            Write-Host ">> Visual Studio $oldVsYear detected: $oldVsPath" -ForegroundColor Red
            Write-Host ">> This project requires Visual Studio 2022 or later." -ForegroundColor Red
            Write-Host ">> Download: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
            Write-Host "   Ensure 'Desktop development with C++' workload is selected."
            Write-Host ""
            throw "Unsupported Visual Studio version. Please install Visual Studio 2022 or later."
        }
        $missing += "Visual Studio"
    }
    else {
        $vsYear = & $vsWhere -version "[17.0," -latest -prerelease -property catalog_productLineVersion 2>$null
        Write-Host ">> Visual Studio $vsYear found: $vsPath"
    }
} else {
    $missing += "Visual Studio"
}

# Check CMake
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVersion = (cmake --version | Select-Object -First 1)
    Write-Host ">> CMake found: $cmakeVersion"
} else {
    $missing += "CMake"
}

# Check Git
if (Get-Command git -ErrorAction SilentlyContinue) {
    $gitVersion = (git --version)
    Write-Host ">> Git found: $gitVersion"
} else {
    $missing += "Git"
}

# Install missing tools via winget
if ($missing.Count -gt 0) {
    Write-Host "`n>> Missing tools: $($missing -join ', ')" -ForegroundColor Yellow

    if (Get-Command winget -ErrorAction SilentlyContinue) {
        foreach ($tool in $missing) {
            switch ($tool) {
                "Visual Studio" {
                    Write-Host ">> Please install Visual Studio 2022 or later from https://visualstudio.microsoft.com/downloads/"
                    Write-Host "   Ensure 'Desktop development with C++' workload is selected."
                }
                "CMake" {
                    Write-Host ">> Installing CMake via winget..."
                    winget install Kitware.CMake --accept-package-agreements --accept-source-agreements
                }
                "Git" {
                    Write-Host ">> Installing Git via winget..."
                    winget install Git.Git --accept-package-agreements --accept-source-agreements
                }
            }
        }
    } else {
        Write-Host ">> winget not available. Please install missing tools manually." -ForegroundColor Red
    }
}

# --- Initialize git submodules ---

$isGitRepo = $false
if ($ProjectDir -and (Test-Path $ProjectDir)) {
    $isGitRepo = (git -C $ProjectDir rev-parse --is-inside-work-tree 2>$null) -eq "true"
}

if ($isGitRepo) {
    Write-Host "`n>> Initializing git submodules..."
    Set-Location $ProjectDir
    git submodule update --init --recursive

    Write-Host "`n=== Setup complete ===" -ForegroundColor Green
} else {
    Write-Host "`n=== Dependency installation complete ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:"
    Write-Host "  git clone --recursive <your_pa_url>"
}

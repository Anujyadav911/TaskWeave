# TaskWeave CMake Build Script for Windows

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "TaskWeave - CMake Build Script" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Create build directory
$BUILD_DIR = "build"
if (Test-Path $BUILD_DIR) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BUILD_DIR
}

New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
Set-Location $BUILD_DIR

# Detect compiler and set appropriate generator
Write-Host "Detecting build environment..." -ForegroundColor Cyan

$GENERATOR = $null
$USE_GENERATOR = $false

# Check for MinGW/g++
if (Get-Command g++ -ErrorAction SilentlyContinue) {
    $gccVersion = (g++ --version 2>&1 | Select-String "g\+\+").ToString()
    if ($gccVersion) {
        $GENERATOR = "MinGW Makefiles"
        $USE_GENERATOR = $true
        Write-Host "Detected: MinGW/g++ compiler" -ForegroundColor Green
        Write-Host "Using generator: $GENERATOR" -ForegroundColor Green
    }
}

# Check for Visual Studio (if MinGW not found)
if (-not $USE_GENERATOR) {
    if (Test-Path "C:\Program Files\Microsoft Visual Studio") {
        $GENERATOR = "Visual Studio 17 2022"
        $USE_GENERATOR = $true
        Write-Host "Detected: Visual Studio" -ForegroundColor Green
        Write-Host "Using generator: $GENERATOR" -ForegroundColor Green
    }
}

# Check for Ninja (if neither found)
if (-not $USE_GENERATOR) {
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        $GENERATOR = "Ninja"
        $USE_GENERATOR = $true
        Write-Host "Detected: Ninja build system" -ForegroundColor Green
        Write-Host "Using generator: $GENERATOR" -ForegroundColor Green
    }
}

# Configure with CMake
Write-Host ""
Write-Host "Configuring with CMake..." -ForegroundColor Green
if ($USE_GENERATOR) {
    if ($GENERATOR -eq "MinGW Makefiles") {
        # Check if make or mingw32-make is available
        $hasMake = (Get-Command make -ErrorAction SilentlyContinue) -or (Get-Command mingw32-make -ErrorAction SilentlyContinue)
        if (-not $hasMake) {
            Write-Host "Warning: MinGW Makefiles generator requires 'make' or 'mingw32-make'" -ForegroundColor Yellow
            Write-Host "Please install make or use a different generator:" -ForegroundColor Yellow
            Write-Host "  cmake .. -G `"Visual Studio 17 2022`" -A x64" -ForegroundColor White
            Write-Host "  OR install make from MinGW" -ForegroundColor White
        }
    }
    cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
} else {
    Write-Host "Warning: Using default CMake generator. May require manual specification." -ForegroundColor Yellow
    Write-Host "If this fails, specify generator manually:" -ForegroundColor Yellow
    Write-Host "  For MinGW: cmake .. -G `"MinGW Makefiles`" -DCMAKE_BUILD_TYPE=Release" -ForegroundColor White
    Write-Host "  For Visual Studio: cmake .. -G `"Visual Studio 17 2022`" -A x64" -ForegroundColor White
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n❌ CMake configuration failed." -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Build
Write-Host ""
Write-Host "Building TaskWeave..." -ForegroundColor Green
cmake --build . --config Release

# Check if build succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ Build successful!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Executable location: $(Get-Location)\taskweave.exe" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "To run:" -ForegroundColor Yellow
    Write-Host "  cd build && .\taskweave.exe --mode=api" -ForegroundColor White
    Write-Host "  OR" -ForegroundColor White
    Write-Host "  .\build\taskweave.exe --mode=api" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "❌ Build failed. Check errors above." -ForegroundColor Red
    Set-Location ..
    exit 1
}

Set-Location ..

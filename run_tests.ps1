# TaskWeave Test Runner Script for Windows

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "TaskWeave - Running Tests" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (-not (Test-Path "build")) {
    Write-Host "Build directory not found. Building project first..." -ForegroundColor Yellow
    .\build_cmake.ps1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n❌ Build failed. Cannot run tests." -ForegroundColor Red
        exit 1
    }
} else {
    # Check if _deps directory exists (GoogleTest download cache)
    # If it exists but tests failed, might need to clean it
    if (Test-Path "build\_deps") {
        Write-Host "Found existing FetchContent cache..." -ForegroundColor Cyan
    }
}

Set-Location build

# Check if CMake needs to be reconfigured for tests
$needsReconfigure = $false
if (-not (Test-Path "CMakeCache.txt")) {
    $needsReconfigure = $true
} else {
    # Check if BUILD_TESTING is enabled in cache
    $cacheContent = Get-Content "CMakeCache.txt" -Raw
    if ($cacheContent -notmatch "BUILD_TESTING:BOOL=ON") {
        $needsReconfigure = $true
    }
}

# Clean FetchContent cache if reconfiguring (to force fresh download)
if ($needsReconfigure -and (Test-Path "_deps")) {
    Write-Host "Cleaning previous FetchContent cache..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "_deps" -ErrorAction SilentlyContinue
}

# Also check for googletest-populate in build directory (old cache location)
if ($needsReconfigure -and (Test-Path "googletest-populate-prefix")) {
    Write-Host "Cleaning old googletest cache..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "googletest-populate-prefix" -ErrorAction SilentlyContinue
}

if ($needsReconfigure) {
    Write-Host "Reconfiguring CMake with tests enabled..." -ForegroundColor Yellow
    
    # Clean CMake cache to ensure fresh configuration
    if (Test-Path "CMakeCache.txt") {
        Remove-Item "CMakeCache.txt" -ErrorAction SilentlyContinue
    }
    if (Test-Path "CMakeFiles") {
        Remove-Item -Recurse -Force "CMakeFiles" -ErrorAction SilentlyContinue
    }
    
    # Detect generator (same logic as build script)
    $GENERATOR = $null
    $USE_GENERATOR = $false
    
    if (Get-Command g++ -ErrorAction SilentlyContinue) {
        $gccVersion = (g++ --version 2>&1 | Select-String "g\+\+").ToString()
        if ($gccVersion) {
            $GENERATOR = "MinGW Makefiles"
            $USE_GENERATOR = $true
        }
    }
    
    if (-not $USE_GENERATOR) {
        if (Test-Path "C:\Program Files\Microsoft Visual Studio") {
            $GENERATOR = "Visual Studio 17 2022"
            $USE_GENERATOR = $true
        }
    }
    
    if ($USE_GENERATOR) {
        cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    } else {
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n❌ CMake reconfiguration failed." -ForegroundColor Red
        Write-Host "Try cleaning build directory: Remove-Item -Recurse -Force build" -ForegroundColor Yellow
        Set-Location ..
        exit 1
    }
}

# Check if test executable exists
if (-not (Test-Path "taskweave_tests.exe")) {
    Write-Host "Building tests..." -ForegroundColor Yellow
    cmake --build . --target taskweave_tests
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "`n❌ Test build failed." -ForegroundColor Red
        Set-Location ..
        exit 1
    }
}

Write-Host "Running tests..." -ForegroundColor Green
Write-Host ""

# Run tests using CTest
ctest --output-on-failure --verbose

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ All tests passed!" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "❌ Some tests failed. See output above." -ForegroundColor Red
}

Set-Location ..

# TaskWeave Build Script for Windows
# Make sure SQLite is installed first

Write-Host "Building TaskWeave..." -ForegroundColor Green

g++ -std=c++17 `
  -Isrc `
  -Icore `
  -Iapi `
  -Iutils `
  src/main.cpp `
  src/core/Task.cpp `
  core/TaskLoader.cpp `
  core/TaskRegistry.cpp `
  src/executor/ThreadPool.cpp `
  src/scheduler/PriorityScheduler.cpp `
  src/scheduler/RoundRobinScheduler.cpp `
  api/ApiServer.cpp `
  utils/Config.cpp `
  utils/Metrics.cpp `
  utils/Database.cpp `
  -o taskweave.exe `
  -pthread `
  -lws2_32 `
  -lsqlite3

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✅ Build successful! Run with: .\taskweave.exe --mode=api" -ForegroundColor Green
} else {
    Write-Host "`n❌ Build failed. Check errors above." -ForegroundColor Red
}

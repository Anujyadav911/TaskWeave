# TaskWeave API Test Script for PowerShell
# Use this instead of curl in PowerShell

Write-Host "Testing TaskWeave API..." -ForegroundColor Green

# Health check
Write-Host "`n1. Health Check:" -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/health" -Method GET
    Write-Host $response.Content -ForegroundColor Cyan
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
}

# Get all tasks
Write-Host "`n2. Getting all tasks:" -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/tasks" -Method GET
    Write-Host $response.Content -ForegroundColor Cyan
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
}

# Submit a task
Write-Host "`n3. Submitting task..." -ForegroundColor Yellow
$taskId = [int](Get-Date -UFormat %s)
$taskJson = @{
    tasks = @(
        @{
            id = $taskId
            name = "PowerShell Test Task"
            priority = "HIGH"
            max_retries = 2
            type = "print"
            params = @{
                message = "Hello from PowerShell"
            }
        }
    )
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/tasks" `
        -Method POST `
        -ContentType "application/json" `
        -Body $taskJson
    Write-Host $response.Content -ForegroundColor Cyan
    Write-Host "✅ Task submitted successfully!" -ForegroundColor Green
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Write-Host "Response: $($_.Exception.Response)" -ForegroundColor Red
}

# Get metrics
Write-Host "`n4. Metrics:" -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/metrics" -Method GET
    Write-Host $response.Content -ForegroundColor Cyan
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
}

Write-Host "`n✅ All tests completed!" -ForegroundColor Green
Write-Host "Open http://localhost:8080/dashboard in browser" -ForegroundColor Cyan

# Run all unit tests
# This script finds and executes all test executables

param(
    [string]$BuildConfig = "Debug",
    [string]$BuildPath = ".\build"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Running Unit Tests ===" -ForegroundColor Cyan
Write-Host ""

$testExecutables = @(
    "chunk_scheduler_test.exe",
    "chunk_dispatch_test.exe"
)

$testPath = Join-Path $BuildPath $BuildConfig
$allPassed = $true

foreach ($testExe in $testExecutables) {
    $exePath = Join-Path $testPath $testExe
    
    if (-not (Test-Path $exePath)) {
        Write-Host "SKIP: $testExe (not built)" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "Running: $testExe" -ForegroundColor Yellow
    Write-Host "----------------------------------------"
    
    $proc = Start-Process -FilePath $exePath `
                         -WorkingDirectory $testPath `
                         -NoNewWindow `
                         -Wait `
                         -PassThru
    
    $exitCode = $proc.ExitCode
    
    if ($exitCode -eq 0) {
        Write-Host "PASS: $testExe" -ForegroundColor Green
    } else {
        Write-Host "FAIL: $testExe (exit code: $exitCode)" -ForegroundColor Red
        $allPassed = $false
    }
    
    Write-Host ""
}

if ($allPassed) {
    Write-Host "=== All Tests Passed ===" -ForegroundColor Green
    exit 0
} else {
    Write-Host "=== Some Tests Failed ===" -ForegroundColor Red
    exit 1
}

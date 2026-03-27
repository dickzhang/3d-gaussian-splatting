# Smoke test script for validation
# Tests basic functionality with various configurations

param(
    [string]$BuildPath = ".\build\Release",
    [string]$Executable = "gaussian_splatting_gl.exe",
    [int]$Frames = 30
)

$ErrorActionPreference = "Stop"

Write-Host "=== Gaussian Splatting Smoke Tests ===" -ForegroundColor Cyan
Write-Host ""

# Check if executable exists
$exePath = Join-Path $BuildPath $Executable
if (-not (Test-Path $exePath)) {
    Write-Host "ERROR: Executable not found at: $exePath" -ForegroundColor Red
    exit 1
}

# Test configurations
$testCases = @(
    @{
        Name = "Baseline"
        Env = @{
            "GS_CHUNK_THRESHOLD" = "256"
            "GS_DEBUG_CHUNKS" = "0"
            "GS_RUNTIME_LOG_FILE" = "smoke_baseline.log"
        }
    },
    @{
        Name = "Debug Logging Enabled"
        Env = @{
            "GS_CHUNK_THRESHOLD" = "256"
            "GS_DEBUG_CHUNKS" = "1"
            "GS_RUNTIME_LOG_FILE" = "smoke_debug.log"
        }
    },
    @{
        Name = "Low Threshold"
        Env = @{
            "GS_CHUNK_THRESHOLD" = "64"
            "GS_DEBUG_CHUNKS" = "1"
            "GS_RUNTIME_LOG_FILE" = "smoke_low_thresh.log"
        }
    },
    @{
        Name = "High Threshold"
        Env = @{
            "GS_CHUNK_THRESHOLD" = "1024"
            "GS_DEBUG_CHUNKS" = "1"
            "GS_RUNTIME_LOG_FILE" = "smoke_high_thresh.log"
        }
    }
)

$results = @()

foreach ($test in $testCases) {
    Write-Host "Running test: $($test.Name)" -ForegroundColor Yellow
    
    # Set environment variables
    foreach ($key in $test.Env.Keys) {
        Set-Item -Path "env:$key" -Value $test.Env[$key]
    }
    
    # Run executable
    $startTime = Get-Date
    $proc = Start-Process -FilePath $exePath `
                         -WorkingDirectory $BuildPath `
                         -NoNewWindow `
                         -Wait `
                         -PassThru
    $endTime = Get-Date
    $duration = ($endTime - $startTime).TotalSeconds
    
    $exitCode = $proc.ExitCode
    
    # Check results
    $status = "PASS"
    $statusColor = "Green"
    
    if ($exitCode -ne 0) {
        $status = "FAIL"
        $statusColor = "Red"
    }
    
    # Check log file if it was written
    $logFile = $test.Env["GS_RUNTIME_LOG_FILE"]
    $logErrors = @()
    if (Test-Path $logFile) {
        $logErrors = Select-String -Path $logFile -Pattern "ERROR|FAIL|assert|crash" -CaseSensitive:$false
    }
    
    if ($logErrors.Count -gt 0) {
        $status = "WARN"
        $statusColor = "Yellow"
        Write-Host "  Found $($logErrors.Count) potential errors in log" -ForegroundColor Yellow
    }
    
    $results += [PSCustomObject]@{
        Test = $test.Name
        Status = $status
        ExitCode = $exitCode
        Duration_Sec = [math]::Round($duration, 2)
        LogErrors = $logErrors.Count
    }
    
    Write-Host "  Status: $status (Exit: $exitCode, Duration: $([math]::Round($duration, 2))s)" -ForegroundColor $statusColor
    Write-Host ""
    
    # Clean up environment
    foreach ($key in $test.Env.Keys) {
        Remove-Item -Path "env:$key" -ErrorAction SilentlyContinue
    }
}

# Summary
Write-Host "=== Test Summary ===" -ForegroundColor Cyan
$results | Format-Table -AutoSize

$failedCount = ($results | Where-Object { $_.Status -eq "FAIL" }).Count
$warnCount = ($results | Where-Object { $_.Status -eq "WARN" }).Count

if ($failedCount -gt 0) {
    Write-Host "FAILED: $failedCount test(s) failed" -ForegroundColor Red
    exit 1
} elseif ($warnCount -gt 0) {
    Write-Host "PASSED with warnings: $warnCount test(s) had warnings" -ForegroundColor Yellow
    exit 0
} else {
    Write-Host "SUCCESS: All tests passed" -ForegroundColor Green
    exit 0
}

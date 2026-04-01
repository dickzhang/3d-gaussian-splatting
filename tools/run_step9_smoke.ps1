param(
    [string]$Configuration = "Release",
    [string]$SourceConfigPath = "build/source_ply.txt",
    [string]$OutputCachePath = "",
    [string]$RuntimeConfigPath = "assets/configs/model_path.txt",
    [int]$ObserveSeconds = 10,
    [switch]$RequireAcceptanceMarker,
    [switch]$RequireGpuCompactionValidation,
    [switch]$RequireDrawSubmissionMarker,
    [string]$ExpectedDrawPath = "",
    [string]$ExpectedDrawDomain = "",
    [string]$ExpectedScheduleLookup = "",
    [ValidateSet("", "auto", "cpu", "gpu", "full")]
    [string]$ChunkSchedulerMode = "",
    [switch]$ForceSeededPath,
    [switch]$RunMissingPayloadCheck,
    [switch]$SkipVisualCheckPrompt,
    [switch]$KeepViewerOpen
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent $PSScriptRoot
Set-Location $workspaceRoot

function Resolve-StepPath {
    param([string]$PathValue)

    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return $PathValue
    }

    return [System.IO.Path]::GetFullPath((Join-Path $workspaceRoot $PathValue))
}

function Get-RelativePathForConfig {
    param(
        [string]$BaseDirectory,
        [string]$TargetPath
    )

    $baseUri = New-Object System.Uri(([System.IO.Path]::GetFullPath($BaseDirectory).TrimEnd('\') + '\'))
    $targetUri = New-Object System.Uri([System.IO.Path]::GetFullPath($TargetPath))
    return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($targetUri).ToString()).Replace('/', '/')
}

function Get-DefaultCachePathFromSourceConfig {
    param([string]$ConfigPath)

    $configDir = Split-Path -Parent $ConfigPath
    $configuredLines = @(Get-Content $ConfigPath | Where-Object { $_.Trim() -ne "" -and -not $_.TrimStart().StartsWith("#") })
    if ($configuredLines.Length -eq 0) {
        throw "No PLY path found in source config: $ConfigPath"
    }

    $plyPath = $configuredLines[0].Trim()
    if (-not [System.IO.Path]::IsPathRooted($plyPath)) {
        $plyPath = [System.IO.Path]::GetFullPath((Join-Path $configDir $plyPath))
    }

    return Join-Path (Split-Path -Parent $plyPath) (([System.IO.Path]::GetFileNameWithoutExtension($plyPath)) + ".gsplatcache")
}

function Resolve-CacheOutputPathFromSourceConfig {
    param(
        [string]$ConfigPath,
        [string]$RequestedOutputPath
    )

    $defaultPath = Get-DefaultCachePathFromSourceConfig -ConfigPath $ConfigPath
    if ([string]::IsNullOrWhiteSpace($RequestedOutputPath)) {
        return $defaultPath
    }

    $defaultDirectory = Split-Path -Parent $defaultPath
    $resolvedRequestedPath = Resolve-StepPath $RequestedOutputPath
    $requestedLeaf = [System.IO.Path]::GetFileName($resolvedRequestedPath)
    if ([string]::IsNullOrWhiteSpace($requestedLeaf)) {
        Write-Host "Ignoring requested output directory and writing cache files next to source PLY: $defaultDirectory"
        return $defaultPath
    }

    $requestedExtension = [System.IO.Path]::GetExtension($requestedLeaf)
    if ([string]::IsNullOrWhiteSpace($requestedExtension)) {
        $requestedLeaf = "$requestedLeaf.gsplatcache"
    }
    elseif ($requestedExtension -ne ".gsplatcache") {
        $requestedLeaf = [System.IO.Path]::ChangeExtension($requestedLeaf, ".gsplatcache")
        Write-Host "Output filename normalized to .gsplatcache: $requestedLeaf"
    }

    if ((Split-Path -Parent $resolvedRequestedPath) -and ((Split-Path -Parent $resolvedRequestedPath) -ne $defaultDirectory)) {
        Write-Host "Ignoring requested output directory and writing cache files next to source PLY: $defaultDirectory"
    }

    return Join-Path $defaultDirectory $requestedLeaf
}

function Invoke-StepCommand {
    param(
        [string]$Title,
        [scriptblock]$Action
    )

    Write-Host "== $Title =="
    & $Action
    Write-Host ""
}

function Invoke-RuntimeValidation {
    param(
        [string]$RuntimeExePath,
        [string]$RuntimeConfigFile,
        [string]$CacheManifestPath,
        [int]$ObserveDurationSeconds,
        [bool]$LeaveViewerRunning,
        [bool]$AllowFailure,
        [hashtable]$ExtraEnvironment = @{}
    )

    $stdoutPath = Join-Path $workspaceRoot "build\step9-runtime.stdout.log"
    $stderrPath = Join-Path $workspaceRoot "build\step9-runtime.stderr.log"
    $validationLogPath = Join-Path $workspaceRoot "build\step9-runtime.validation.log"
    Remove-Item $stdoutPath -ErrorAction SilentlyContinue
    Remove-Item $stderrPath -ErrorAction SilentlyContinue
    Remove-Item $validationLogPath -ErrorAction SilentlyContinue

    $originalRuntimeConfig = Get-Content $RuntimeConfigFile -Raw
    $originalRuntimeLogPath = $env:GS_RUNTIME_LOG_FILE
    $relativeCachePath = Get-RelativePathForConfig -BaseDirectory (Split-Path -Parent $RuntimeConfigFile) -TargetPath $CacheManifestPath
    Set-Content -Path $RuntimeConfigFile -Value $relativeCachePath
    $env:GS_RUNTIME_LOG_FILE = $validationLogPath

    $process = $null
    try {
        $envAssignments = @(
            'set "GS_CHUNK_SCHEDULER_MODE="',
            'set "GS_CHUNK_FORCE_SEEDED_PATH="',
            'set "GS_VALIDATE_GPU_COMPACTION="',
            ('set "GS_RUNTIME_LOG_FILE={0}"' -f $validationLogPath)
        )
        foreach ($entry in $ExtraEnvironment.GetEnumerator()) {
            $envAssignments += ('set "{0}={1}"' -f $entry.Key, $entry.Value)
        }

        $quotedRuntimeExePath = '"' + $RuntimeExePath + '"'
        $quotedStdoutPath = '"' + $stdoutPath + '"'
        $quotedStderrPath = '"' + $stderrPath + '"'
        $commandChain = ($envAssignments + @("$quotedRuntimeExePath 1> $quotedStdoutPath 2> $quotedStderrPath")) -join ' && '
        $process = Start-Process -FilePath "cmd.exe" -ArgumentList @("/d", "/c", $commandChain) -WorkingDirectory $workspaceRoot -PassThru
        Start-Sleep -Seconds ([Math]::Max(1, $ObserveDurationSeconds))

        $stdout = if (Test-Path $stdoutPath) { @(Get-Content $stdoutPath) } else { @() }
        $stderr = if (Test-Path $stderrPath) { @(Get-Content $stderrPath) } else { @() }
        $validationLog = if (Test-Path $validationLogPath) { @(Get-Content $validationLogPath) } else { @() }

        if (@($stdout).Length -gt 0) {
            Write-Host "-- stdout --"
            $stdout | ForEach-Object { Write-Host $_ }
        }
        if (@($stderr).Length -gt 0) {
            Write-Host "-- stderr --"
            $stderr | ForEach-Object { Write-Host $_ }
        }
        if (@($validationLog).Length -gt 0) {
            Write-Host "-- validation log --"
            $validationLog | ForEach-Object { Write-Host $_ }
        }

        $process.Refresh()
        $hasExited = $process.HasExited
        if ($hasExited) {
            $process.WaitForExit()
        }
        $exitCode = if ($hasExited) { $process.ExitCode } else { $null }
        if ($hasExited -and -not $AllowFailure -and $null -ne $exitCode -and $exitCode -ne 0) {
            throw "Runtime exited with code $exitCode"
        }

        return [pscustomobject]@{
            Process = $process
            Stdout = $stdout
            Stderr = $stderr
            ValidationLog = $validationLog
            HasExited = $hasExited
            ExitCode = $exitCode
        }
    }
    finally {
        Set-Content -Path $RuntimeConfigFile -Value $originalRuntimeConfig
        if ($null -eq $originalRuntimeLogPath) {
            Remove-Item Env:GS_RUNTIME_LOG_FILE -ErrorAction SilentlyContinue
        }
        else {
            $env:GS_RUNTIME_LOG_FILE = $originalRuntimeLogPath
        }
        if ($process -and -not $LeaveViewerRunning -and -not $process.HasExited) {
            & taskkill /PID $process.Id /T /F *> $null
            Start-Sleep -Milliseconds 200
        }
    }
}

function Test-LogContains {
    param(
        [string[]]$Lines,
        [string]$Pattern
    )

    return @($Lines | Where-Object { $_ -like "*$Pattern*" }).Length -gt 0
}

function Test-DrawSubmissionMarker {
    param(
        [string[]]$Lines,
        [string]$ExpectedPath,
        [string]$ExpectedDomain,
        [string]$ExpectedLookup
    )

    $matches = @($Lines | Where-Object { $_ -like "*DRAW_SUBMISSION:*" })
    if ($matches.Length -eq 0) {
        return $false
    }

    foreach ($line in $matches) {
        $pathOk = [string]::IsNullOrWhiteSpace($ExpectedPath) -or $line.Contains("path=$ExpectedPath")
        $domainOk = [string]::IsNullOrWhiteSpace($ExpectedDomain) -or $line.Contains("draw_domain=$ExpectedDomain")
        $lookupOk = [string]::IsNullOrWhiteSpace($ExpectedLookup) -or $line.Contains("schedule_lookup=$ExpectedLookup")
        if ($pathOk -and $domainOk -and $lookupOk) {
            return $true
        }
    }

    return $false
}

function Get-FirstExistingPayloadPath {
    param([string]$ManifestPath)

    $directory = Split-Path -Parent $ManifestPath
    $stem = [System.IO.Path]::GetFileNameWithoutExtension($ManifestPath)
    $suffixes = @('.pos.byte', '.other.byte', '.color.byte', '.sh.byte', '.chunk.byte')
    foreach ($suffix in $suffixes) {
        $candidate = Join-Path $directory ($stem + $suffix)
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "No payload file found next to manifest: $ManifestPath"
}

function Invoke-MissingPayloadCheck {
    param(
        [string]$RuntimeExePath,
        [string]$RuntimeConfigFile,
        [string]$CacheManifestPath,
        [int]$ObserveDurationSeconds
    )

    $payloadPath = Get-FirstExistingPayloadPath -ManifestPath $CacheManifestPath
    $backupPath = "$payloadPath.bak"
    Move-Item -LiteralPath $payloadPath -Destination $backupPath -Force
    try {
        $result = Invoke-RuntimeValidation -RuntimeExePath $RuntimeExePath -RuntimeConfigFile $RuntimeConfigFile -CacheManifestPath $CacheManifestPath -ObserveDurationSeconds $ObserveDurationSeconds -LeaveViewerRunning $false -AllowFailure $true
        $combinedLogs = @($result.ValidationLog + $result.Stdout + $result.Stderr)
        $sawFailureBanner = Test-LogContains -Lines $combinedLogs -Pattern "Failed to load cache asset"
        $sawMissingPayloadError = @($combinedLogs | Where-Object { $_ -match 'Missing .* payload file:' }).Length -gt 0
        if (-not $result.HasExited -or $result.ExitCode -eq 0) {
            throw "Missing-payload validation expected runtime startup failure, but the process did not fail"
        }
        if (-not $sawFailureBanner -or -not $sawMissingPayloadError) {
            throw "Missing-payload validation did not emit the expected cache load diagnostics"
        }

        Write-Host "Missing-payload validation passed."
    }
    finally {
        if (Test-Path $backupPath) {
            Move-Item -LiteralPath $backupPath -Destination $payloadPath -Force
        }
    }
}

$builderExePath = Resolve-StepPath (Join-Path "build" (Join-Path $Configuration "gs_cache_builder.exe"))
$runtimeExePath = Resolve-StepPath (Join-Path "build" (Join-Path $Configuration "gaussian_splatting_gl.exe"))
$sourceConfigPath = Resolve-StepPath $SourceConfigPath
$runtimeConfigPath = Resolve-StepPath $RuntimeConfigPath

if (-not (Test-Path $builderExePath)) {
    throw "Builder executable not found: $builderExePath"
}

if (-not (Test-Path $runtimeExePath)) {
    throw "Runtime executable not found: $runtimeExePath"
}

if (-not (Test-Path $sourceConfigPath)) {
    throw "Source config not found: $sourceConfigPath"
}

if (-not (Test-Path $runtimeConfigPath)) {
    throw "Runtime config not found: $runtimeConfigPath"
}

$outputCachePath = if ([string]::IsNullOrWhiteSpace($OutputCachePath)) {
    Get-DefaultCachePathFromSourceConfig -ConfigPath $sourceConfigPath
} else {
    Resolve-CacheOutputPathFromSourceConfig -ConfigPath $sourceConfigPath -RequestedOutputPath $OutputCachePath
}

Invoke-StepCommand "Build" {
    cmake --build build --config $Configuration
}

Invoke-StepCommand "Generate Cache" {
    if ([string]::IsNullOrWhiteSpace($OutputCachePath)) {
        & $builderExePath $sourceConfigPath
    }
    else {
        & $builderExePath $sourceConfigPath ([System.IO.Path]::GetFileName($outputCachePath))
    }
}

Invoke-StepCommand "Launch Runtime" {
    $extraEnvironment = @{}
    if ($RequireGpuCompactionValidation) {
        $extraEnvironment["GS_VALIDATE_GPU_COMPACTION"] = "1"
    }
    if (-not [string]::IsNullOrWhiteSpace($ChunkSchedulerMode)) {
        $extraEnvironment["GS_CHUNK_SCHEDULER_MODE"] = $ChunkSchedulerMode
    }
    if ($ForceSeededPath) {
        $extraEnvironment["GS_CHUNK_FORCE_SEEDED_PATH"] = "1"
    }

    $runtimeResult = Invoke-RuntimeValidation -RuntimeExePath $runtimeExePath -RuntimeConfigFile $runtimeConfigPath -CacheManifestPath $outputCachePath -ObserveDurationSeconds $ObserveSeconds -LeaveViewerRunning $KeepViewerOpen.IsPresent -AllowFailure $false -ExtraEnvironment $extraEnvironment
    $combinedLogs = @($runtimeResult.ValidationLog + $runtimeResult.Stdout + $runtimeResult.Stderr)

    if ($RequireAcceptanceMarker) {
        if (-not (Test-LogContains -Lines $combinedLogs -Pattern "ACCEPT: pipeline frame completed")) {
            throw "Acceptance marker not found in runtime logs"
        }
        if (Test-LogContains -Lines $combinedLogs -Pattern "ACCEPT_WARN: composite reference path unavailable") {
            throw "Composite reference path fallback was triggered during acceptance run"
        }
        if (Test-LogContains -Lines $combinedLogs -Pattern "View-data compute pass failed") {
            throw "View-data pipeline reported a failure during acceptance run"
        }
        Write-Host "Acceptance marker validation passed."
    }

    if ($RequireDrawSubmissionMarker -or $RequireAcceptanceMarker) {
        if (-not (Test-DrawSubmissionMarker -Lines $combinedLogs -ExpectedPath $ExpectedDrawPath -ExpectedDomain $ExpectedDrawDomain -ExpectedLookup $ExpectedScheduleLookup)) {
            throw "Draw submission marker not found in runtime logs, or it did not match the expected path/domain/lookup"
        }
        Write-Host "Draw submission marker validation passed."
    }

    if ($RequireGpuCompactionValidation) {
        if (Test-LogContains -Lines $combinedLogs -Pattern "GPU_COMPACTION_VALIDATE_MISMATCH") {
            throw "GPU compaction validation reported a CPU/GPU mismatch"
        }
        if (-not (Test-LogContains -Lines $combinedLogs -Pattern "GPU_COMPACTION_VALIDATE_OK")) {
            throw "GPU compaction validation marker not found in runtime logs"
        }
        Write-Host "GPU compaction validation passed."
    }

    if ($KeepViewerOpen) {
        Write-Host "Viewer left running. PID: $($runtimeResult.Process.Id)"
    }
}

if ($RunMissingPayloadCheck) {
    Invoke-StepCommand "Missing Payload Validation" {
        Invoke-MissingPayloadCheck -RuntimeExePath $runtimeExePath -RuntimeConfigFile $runtimeConfigPath -CacheManifestPath $outputCachePath -ObserveDurationSeconds 2
    }
}

Write-Host "== Visual Checklist =="
Write-Host "1. Confirm the model appears and camera controls respond."
Write-Host "2. Confirm depth sorting is stable during camera movement."
Write-Host "3. Confirm view-data shading and final composite look correct."

if ($RequireAcceptanceMarker) {
    Write-Host "4. Confirm logs contain: ACCEPT: pipeline frame completed"
}

if ($RequireDrawSubmissionMarker -or $RequireAcceptanceMarker) {
    Write-Host "5. Confirm logs contain: DRAW_SUBMISSION with the expected path/domain/lookup fields."
}

if ($RequireGpuCompactionValidation) {
    Write-Host "6. Confirm logs contain: GPU_COMPACTION_VALIDATE_OK and no mismatch marker."
}

if ($RunMissingPayloadCheck) {
    Write-Host "7. Confirm missing-payload validation fails with explicit cache-read diagnostics."
}

if (-not $SkipVisualCheckPrompt) {
    $answer = Read-Host "Visual check passed? [y/N]"
    if ($answer -notmatch '^(y|Y)$') {
        throw "Visual verification not confirmed"
    }
}

Write-Host "Step 9 smoke test sequence completed."
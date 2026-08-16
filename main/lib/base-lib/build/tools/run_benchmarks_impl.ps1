[CmdletBinding()]
param (
    [string]$TargetConfig = "",
    [ValidateSet("regular", "deep")]
    [string]$Mode = ""
)

if ($Mode) {
    $global:BENCHMARK_MODE = $Mode
}

if (-not $global:WORKSPACE_DIR) {
    $global:WORKSPACE_DIR = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
}

# Execute build phase before running benchmarks
& "$PSScriptRoot\build_all_impl.ps1" -TargetConfig $TargetConfig
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build phase failed with exit code $LASTEXITCODE - aborting benchmarks." -ForegroundColor Red
    $global:LASTEXITCODE = $LASTEXITCODE
    exit $LASTEXITCODE
}

$passedCount = 0
$failedCount = 0
$skippedCount = 0
$failedSuites = [System.Collections.Generic.List[string]]::new()

$currentMode = if ($global:BENCHMARK_MODE) { $global:BENCHMARK_MODE } else { "regular" }

foreach ($suite in $global:RUN_SUITES) {
    $parts = $suite.Split(';')
    $configName = $parts[0]
    $runnerType = $parts[1]
    $binaryPath = $parts[2]

    if ($TargetConfig -and ($TargetConfig -ne $configName)) {
        continue
    }

    $fullBinaryPath = Join-Path $global:WORKSPACE_DIR $binaryPath

    # On Windows, try finding binary with .exe extension or in Release/ folder if direct path does not exist
    if (-not (Test-Path $fullBinaryPath -PathType Leaf)) {
        if (Test-Path "$fullBinaryPath.exe" -PathType Leaf) {
            $fullBinaryPath = "$fullBinaryPath.exe"
        } else {
            $dir = Split-Path $fullBinaryPath -Parent
            $name = Split-Path $fullBinaryPath -Leaf
            $relExe = Join-Path $dir "Release\$name.exe"
            if (Test-Path $relExe -PathType Leaf) {
                $fullBinaryPath = $relExe
            }
        }
    }

    if (-not (Test-Path $fullBinaryPath -PathType Leaf)) {
        Write-Host "Warning: Binary not found: $fullBinaryPath - skipping $configName ($binaryPath)" -ForegroundColor Yellow
        $skippedCount++
        continue
    }

    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Running Benchmarks: $configName (Mode: $currentMode)" -ForegroundColor Cyan
    Write-Host "Runner:             $runnerType"
    Write-Host "Binary:             $fullBinaryPath"
    Write-Host "========================================================================" -ForegroundColor Cyan

    $exitCode = 0

    switch ($runnerType) {
        "native" {
            & "$fullBinaryPath" --benchmark
            $exitCode = $LASTEXITCODE
        }
        "node" {
            if ($global:NODE_BIN) {
                & $global:NODE_BIN "$fullBinaryPath" --benchmark
                $exitCode = $LASTEXITCODE
            } else {
                Write-Host "Warning: Node.js not found, cannot run $binaryPath - skipping." -ForegroundColor Yellow
                $skippedCount++
                continue
            }
        }
        "node_wasm_dir" {
            if ($global:NODE_BIN) {
                $binaryDir = Split-Path $fullBinaryPath -Parent
                $binaryName = Split-Path $fullBinaryPath -Leaf
                Push-Location $binaryDir
                & $global:NODE_BIN "$binaryName" --benchmark
                $exitCode = $LASTEXITCODE
                Pop-Location
            } else {
                Write-Host "Warning: Node.js not found, cannot run $binaryPath - skipping." -ForegroundColor Yellow
                $skippedCount++
                continue
            }
        }
        "wasm" {
            Write-Host "Warning: WASM standalone benchmark runner not supported on Windows native PowerShell - skipping." -ForegroundColor Yellow
            $skippedCount++
            continue
        }
        "none" {
            Write-Host "Skipping $binaryPath (runner type 'none' - no standalone runner available)."
            $skippedCount++
            continue
        }
        default {
            Write-Host "Error: Unknown runner type '$runnerType' for $configName" -ForegroundColor Red
            $failedSuites.Add("$configName ($binaryPath)")
            $failedCount++
            continue
        }
    }

    if ($exitCode -eq 0) {
        $passedCount++
    } else {
        Write-Host "Error: Benchmarks FAILED for $configName ($binaryPath) - exit code $exitCode" -ForegroundColor Red
        $failedSuites.Add("$configName ($binaryPath)")
        $failedCount++
    }
}

Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host "Benchmark Run Summary:"
Write-Host "  Passed:  $passedCount suites"
Write-Host "  Failed:  $failedCount suites"
Write-Host "  Skipped: $skippedCount suites"

if ($failedSuites.Count -gt 0) {
    Write-Host "  Failed suites:" -ForegroundColor Red
    foreach ($s in $failedSuites) {
        Write-Host "    - $s" -ForegroundColor Red
    }
    $global:LASTEXITCODE = 1
    exit 1
} else {
    Write-Host "All benchmark suites completed successfully!" -ForegroundColor Green
    $global:LASTEXITCODE = 0
    exit 0
}

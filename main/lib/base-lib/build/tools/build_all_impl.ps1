[CmdletBinding()]
param (
    [string]$TargetConfig = ""
)

if (-not $global:WORKSPACE_DIR) {
    $global:WORKSPACE_DIR = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
}

$cmakeBin = if ($global:CMAKE_BIN) { $global:CMAKE_BIN } else { "cmake" }

# Verify configuration exists if specified
if ($TargetConfig) {
    $configFound = $false
    foreach ($config in $global:BUILD_CONFIGS) {
        $parts = $config.Split(';')
        if ($parts[0] -eq $TargetConfig) {
            $configFound = $true
            break
        }
    }
    if (-not $configFound) {
        Write-Error "Error: Unknown configuration '$TargetConfig'"
        Write-Host "Available configurations: "
        foreach ($config in $global:BUILD_CONFIGS) {
            $parts = $config.Split(';')
            Write-Host "  - $($parts[0])"
        }
        $global:LASTEXITCODE = 1
        exit 1
    }
}

$builtCount = 0
$failedConfigs = [System.Collections.Generic.List[string]]::new()
$skippedConfigs = [System.Collections.Generic.List[string]]::new()

foreach ($config in $global:BUILD_CONFIGS) {
    $parts = $config.Split(';')
    $configName  = $parts[0]
    $cCompiler   = $parts[1]
    $cxxCompiler = $parts[2]
    $extraFlags  = if ($parts.Count -gt 3) { $parts[3] } else { "" }

    if ($TargetConfig -and ($TargetConfig -ne $configName)) {
        continue
    }

    # Check if compiler exists
    $compilerExists = $false
    if ((Test-Path $cCompiler -PathType Leaf) -or (Get-Command $cCompiler -ErrorAction SilentlyContinue)) {
        $compilerExists = $true
    }

    if (-not $compilerExists) {
        Write-Host "Warning: Compiler '$cCompiler' not found. Skipping $configName." -ForegroundColor Yellow
        $skippedConfigs.Add($configName)
        continue
    }

    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Building Configuration: $configName" -ForegroundColor Cyan
    Write-Host "C Compiler:             $cCompiler"
    Write-Host "C++ Compiler:           $cxxCompiler"
    Write-Host "Extra CMake Flags:      $extraFlags"
    Write-Host "========================================================================" -ForegroundColor Cyan

    $buildDir = Join-Path $global:WORKSPACE_DIR "build\$configName"
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    }

    $generatorFlags = @()
    if (Get-Command "ninja" -ErrorAction SilentlyContinue) {
        $generatorFlags += "-G", "Ninja"
    }

    $cmakeArgs = @(
        "-DCMAKE_C_COMPILER=$cCompiler",
        "-DCMAKE_CXX_COMPILER=$cxxCompiler",
        "-DCMAKE_BUILD_TYPE=Release"
    ) + $generatorFlags

    if ($extraFlags) {
        $flagArray = $extraFlags.Split(' ') | Where-Object { $_ -ne "" }
        $cmakeArgs += $flagArray
    }

    $cmakeArgs += "-S", $global:WORKSPACE_DIR
    $cmakeArgs += "-B", $buildDir

    & $cmakeBin @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: CMake configuration failed for $configName" -ForegroundColor Red
        $failedConfigs.Add($configName)
        continue
    }

    & $cmakeBin --build $buildDir --config Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: Build failed for $configName" -ForegroundColor Red
        $failedConfigs.Add($configName)
        continue
    }

    $builtCount++
}

Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host "Build Summary:"
Write-Host "  Successfully built: $builtCount configurations"
if ($skippedConfigs.Count -gt 0) {
    Write-Host "  Skipped (compiler missing): $($skippedConfigs -join ', ')" -ForegroundColor Yellow
}
if ($failedConfigs.Count -gt 0) {
    Write-Host "  Failed: $($failedConfigs -join ', ')" -ForegroundColor Red
    $global:LASTEXITCODE = 1
    exit 1
} else {
    Write-Host "All built configurations completed successfully!" -ForegroundColor Green
    $global:LASTEXITCODE = 0
    exit 0
}

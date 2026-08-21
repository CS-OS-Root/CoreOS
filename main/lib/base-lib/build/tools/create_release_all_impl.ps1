[CmdletBinding()]
param (
    [string]$TargetConfig = "",
    [switch]$Publish = $false,
    [string]$Version = ""
)

if (-not $global:WORKSPACE_DIR) {
    $global:WORKSPACE_DIR = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
}

$productName = if ($global:PRODUCT_NAME) { $global:PRODUCT_NAME } else { (Split-Path $global:WORKSPACE_DIR -Leaf) }
$appTarget   = if ($global:APP_TARGET) { $global:APP_TARGET } else { "$productName-app" }
$cmakeBin    = if ($global:CMAKE_BIN) { $global:CMAKE_BIN } else { "cmake" }
$releasesServerUrl = "https://releases.home.schlegel.ovh"

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

function Get-PlatformName {
    param (
        [string]$ConfigName,
        [string]$CCompiler,
        [string]$CustomPlatform
    )

    if ($CustomPlatform) {
        return $CustomPlatform
    }

    if ($ConfigName.Contains("wasm") -or $CCompiler.Contains("emcc")) {
        return "wasm"
    }

    if ($IsWindows -or ($env:OS -and $env:OS.Contains("Windows"))) {
        return "windows64"
    }
    
    if ($IsLinux) {
        return "linux_x86_64"
    }

    return "windows64"
}

# Determine version
$targetVersion = $Version
if (-not $targetVersion) {
    Write-Host "Fetching existing releases for '$productName' from $releasesServerUrl/$productName/ ..."
    try {
        $html = Invoke-RestMethod -Uri "$releasesServerUrl/$productName/?raw=true" -Method Get -ErrorAction SilentlyContinue
        $matches = [regex]::Matches($html, '(\d+\.\d+\.\d+)')
        $versions = @($matches | ForEach-Object { $_.Value } | Select-Object -Unique | Sort-Object { [version]$_ })
        
        if ($versions.Count -gt 0) {
            $latest = $versions[-1]
            $vParts = $latest.Split('.')
            $major = [int]$vParts[0]
            $minor = [int]$vParts[1]
            $patch = [int]$vParts[2] + 1
            $targetVersion = "$major.$minor.$patch"
            Write-Host "Auto-incremented next version: $targetVersion (from $latest)" -ForegroundColor Green
        } else {
            $targetVersion = "0.0.1"
            Write-Host "No existing version found on server. Using initial version: $targetVersion" -ForegroundColor Green
        }
    } catch {
        $targetVersion = "0.0.1"
        Write-Host "Could not query server. Defaulting to initial version: $targetVersion" -ForegroundColor Yellow
    }
} else {
    Write-Host "Using specified version: $targetVersion" -ForegroundColor Green
}

$builtCount = 0
$failedConfigs = [System.Collections.Generic.List[string]]::new()
$skippedConfigs = [System.Collections.Generic.List[string]]::new()
$processedReleaseDirs = [System.Collections.Generic.List[PSObject]]::new()

foreach ($config in $global:BUILD_CONFIGS) {
    $parts = $config.Split(';')
    $configName     = $parts[0]
    $cCompiler      = $parts[1]
    $cxxCompiler    = $parts[2]
    $extraFlags     = if ($parts.Count -gt 3) { $parts[3] } else { "" }
    $customPlatform = if ($parts.Count -gt 4) { $parts[4] } else { "" }

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

    $platform = Get-PlatformName -ConfigName $configName -CCompiler $cCompiler -CustomPlatform $customPlatform

    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Building Release Configuration: $configName (v$targetVersion)" -ForegroundColor Cyan
    Write-Host "Platform:                      $platform"
    Write-Host "C Compiler:                    $cCompiler"
    Write-Host "C++ Compiler:                  $cxxCompiler"
    Write-Host "Target Executable:             $appTarget"
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
        "-DCMAKE_BUILD_TYPE=Release",
        "-DMLA_APP_VERSION=$targetVersion"
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

    & $cmakeBin --build $buildDir --config Release --target $appTarget
    if ($LASTEXITCODE -ne 0) {
        & $cmakeBin --build $buildDir --config Release
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Error: Build failed for $configName" -ForegroundColor Red
            $failedConfigs.Add($configName)
            continue
        }
    }

    # Staging release directory: release/{platform}/{compiler}/
    $releaseDir = Join-Path $global:WORKSPACE_DIR "release\$platform\$configName"
    if (-not (Test-Path $releaseDir)) {
        New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
    }

    # Copy app binary only, ignoring test binaries
    $copiedAny = $false
    $candidateFiles = Get-ChildItem -Path $buildDir -Recurse -File | Where-Object {
        $_.Name -notmatch "test|Test" -and
        $_.Extension -notmatch "\.a|\.o|\.ninja|\.cpp|\.c" -and
        ($_.Name.StartsWith($appTarget) -or $_.Extension -eq ".exe" -or $_.Extension -eq ".wasm")
    }

    foreach ($file in $candidateFiles) {
        Copy-Item -Path $file.FullName -Destination $releaseDir -Force
        Write-Host "Copied app binary: $($file.Name) -> release/$platform/$configName/" -ForegroundColor Green
        $copiedAny = $true
    }

    if (-not $copiedAny) {
        Write-Host "Warning: No app binary found to stage for configuration $configName" -ForegroundColor Yellow
    }

    $processedReleaseDirs.Add([PSCustomObject]@{
        Platform   = $platform
        Compiler   = $configName
        ReleaseDir = $releaseDir
    })

    $builtCount++
}

Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host "Release Build Summary:"
Write-Host "  Successfully built: $builtCount configurations"
if ($skippedConfigs.Count -gt 0) {
    Write-Host "  Skipped (compiler missing): $($skippedConfigs -join ', ')" -ForegroundColor Yellow
}
if ($failedConfigs.Count -gt 0) {
    Write-Host "  Failed: $($failedConfigs -join ', ')" -ForegroundColor Red
    $global:LASTEXITCODE = 1
    exit 1
}

if (-not $Publish) {
    Write-Host "Release files staged in: $global:WORKSPACE_DIR\release\" -ForegroundColor Green
    $global:LASTEXITCODE = 0
    exit 0
}

Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host "Publishing Release v$targetVersion to Miniserve ($releasesServerUrl)..." -ForegroundColor Cyan
Write-Host "========================================================================" -ForegroundColor Cyan

function Ensure-RemoteDir {
    param (
        [string]$TargetPath
    )
    $parts = $TargetPath.Split('/') | Where-Object { $_ -ne "" }
    $current = ""
    foreach ($part in $parts) {
        if (Get-Command "curl.exe" -ErrorAction SilentlyContinue) {
            & curl.exe -s -o NUL -F "mkdir=$part" "$releasesServerUrl/upload?path=$current/" | Out-Null
        }
        $current = "$current/$part"
    }
}

# Upload files to Miniserve
$uploadSuccess = $true
foreach ($item in $processedReleaseDirs) {
    $rdir = $item.ReleaseDir
    if (-not (Test-Path $rdir)) { continue }

    $targetPath = "/$productName/$targetVersion/$($item.Platform)/$($item.Compiler)"
    $uploadUrl = "$releasesServerUrl/upload?path=$targetPath"

    $files = Get-ChildItem -Path $rdir -File
    if ($files.Count -eq 0) {
        Write-Host "Skipping publish for $($item.Platform)/$($item.Compiler): release directory $rdir is empty." -ForegroundColor Yellow
        continue
    }

    Ensure-RemoteDir -TargetPath $targetPath
    foreach ($file in $files) {
        Write-Host "Uploading $($file.Name) to $targetPath ..."
        try {
            if (Get-Command "curl.exe" -ErrorAction SilentlyContinue) {
                & curl.exe -s -w "%{http_code}" -o NUL -F "path=@$($file.FullName)" $uploadUrl | Out-Null
                Write-Host "  Uploaded $($file.Name) successfully." -ForegroundColor Green
            } else {
                $form = @{
                    path = $file
                }
                Invoke-RestMethod -Uri $uploadUrl -Method Post -Form $form | Out-Null
                Write-Host "  Uploaded $($file.Name) successfully." -ForegroundColor Green
            }
        } catch {
            Write-Host "  Failed to upload $($file.Name): $_" -ForegroundColor Red
            $uploadSuccess = $false
        }
    }
}

if ($uploadSuccess) {
    Write-Host "Successfully published release $targetVersion to $releasesServerUrl/$productName/$targetVersion/" -ForegroundColor Green
    $global:LASTEXITCODE = 0
    exit 0
} else {
    Write-Host "Error: Release publish failed due to upload errors." -ForegroundColor Red
    $global:LASTEXITCODE = 1
    exit 1
}

# Release packaging script for RinEngine
# Copies compiled binary, Qt runtime, and game assets to Release/ folder
#
# Usage: .\scripts\package_release.ps1
#        .\scripts\package_release.ps1 -QtPath "C:\Qt\6.5.3\mingw_64"

param(
    [string]$QtPath = "C:\Qt\6.5.3\mingw_64",
    [string]$BuildConfig = "Desktop_Qt_6_5_3_MinGW_64_bit-Debug",
    [string]$ReleaseDir = "D:\RinEngine\Release",
    [switch]$SkipDeployQt = $false
)

$ErrorActionPreference = "Stop"

$ProjectRoot = "D:\RinEngine\RinEngine"
$BuildDir = Join-Path $ProjectRoot "build\$BuildConfig"
$ExePath = Join-Path $BuildDir "appRinEngine.exe"
$AssetsSource = Join-Path $ProjectRoot "assets"

Write-Host "=== RinEngine Release Packaging ===" -ForegroundColor Cyan
Write-Host ""

# Validation
if (-not (Test-Path $ExePath)) {
    Write-Error "Build output not found: $ExePath"
    Write-Host "Run build first: cmake --build `"$BuildDir`" --target appRinEngine"
    exit 1
}

if (-not $SkipDeployQt) {
    $windeployqt = Join-Path $QtPath "bin\windeployqt.exe"
    if (-not (Test-Path $windeployqt)) {
        Write-Error "windeployqt not found: $windeployqt"
        Write-Host "Specify Qt path: .\scripts\package_release.ps1 -QtPath `"C:\path\to\Qt\6.5.3\mingw_64`""
        exit 1
    }
}

# Step 1: Prepare Release directory (preserve license files)
Write-Host "[1/5] Preparing Release directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $ReleaseDir -Force | Out-Null
New-Item -ItemType Directory -Path "$ReleaseDir\Runtime" -Force | Out-Null
New-Item -ItemType Directory -Path "$ReleaseDir\saves" -Force | Out-Null

# Remove old files except license files and Runtime/saves directories
Get-ChildItem -LiteralPath $ReleaseDir -File | Where-Object { $_.Name -notmatch '^NOTICE|^LICENSE' } | Remove-Item -Force
Get-ChildItem -LiteralPath "$ReleaseDir\Runtime" -Recurse | Remove-Item -Force -Recurse -ErrorAction SilentlyContinue

# Step 2: Copy executable
Write-Host "[2/5] Copying executable..." -ForegroundColor Yellow
Copy-Item $ExePath -Destination $ReleaseDir -Force
Write-Host "  appRinEngine.exe -> Release\" -ForegroundColor Green

# Step 3: Deploy Qt runtime
if (-not $SkipDeployQt) {
    Write-Host "[3/5] Deploying Qt runtime (windeployqt)..." -ForegroundColor Yellow
    $tempDir = Join-Path $env:TEMP "rinengine_deploy"
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null
    Copy-Item $ExePath -Destination $tempDir -Force

    & $windeployqt "$tempDir\appRinEngine.exe" --no-translations --no-compiler-runtime 2>&1 | Out-Null

    # Move Qt DLLs to Runtime/
    $dllCount = 0
    Get-ChildItem -LiteralPath $tempDir -File "*.dll" | ForEach-Object {
        Copy-Item $_.FullName -Destination "$ReleaseDir\Runtime\" -Force
        $dllCount++
    }
    Write-Host "  $dllCount Qt DLLs -> Release\Runtime\" -ForegroundColor Green

    # Move Qt plugin directories to Runtime/
    $pluginDirs = @("platforms", "styles", "imageformats", "multimedia", "networkinformation", "tls", "generic", "iconengines", "playlistformats", "qmltooling")
    $pluginCount = 0
    foreach ($dir in $pluginDirs) {
        $srcDir = Join-Path $tempDir $dir
        if (Test-Path $srcDir) {
            Copy-Item $srcDir -Destination "$ReleaseDir\Runtime\$dir" -Recurse -Force
            $pluginCount++
        }
    }
    Write-Host "  $pluginCount Qt plugin directories -> Release\Runtime\" -ForegroundColor Green

    # Copy QML modules if any
    $qmlDir = Join-Path $tempDir "qml"
    if (Test-Path $qmlDir) {
        Copy-Item $qmlDir -Destination "$ReleaseDir\Runtime\qml" -Recurse -Force
        Write-Host "  QML modules -> Release\Runtime\qml\" -ForegroundColor Green
    }

    # Create qt.conf for runtime path redirection
    $qtConf = @"
[Paths]
Prefix = .
Libraries = Runtime
Plugins = Runtime
Qml2Imports = Runtime/qml
"@
    Set-Content -Path "$ReleaseDir\qt.conf" -Value $qtConf -Encoding ASCII
    Write-Host "  qt.conf created (Runtime/ redirection)" -ForegroundColor Green

    # Cleanup temp
    Remove-Item $tempDir -Recurse -Force -ErrorAction SilentlyContinue
} else {
    Write-Host "[3/5] Qt deployment skipped (--SkipDeployQt)" -ForegroundColor Yellow
}

# Step 4: Copy assets
Write-Host "[4/5] Copying game assets..." -ForegroundColor Yellow
if (Test-Path $AssetsSource) {
    $destAssets = "$ReleaseDir\assets"
    New-Item -ItemType Directory -Path $destAssets -Force | Out-Null
    Copy-Item "$AssetsSource\*" -Destination $destAssets -Recurse -Force

    $fileCount = (Get-ChildItem -LiteralPath $destAssets -Recurse -File | Measure-Object).Count
    $dirCount = (Get-ChildItem -LiteralPath $destAssets -Recurse -Directory | Measure-Object).Count
    Write-Host "  assets/ -> Release\assets\ ($fileCount files, $dirCount dirs)" -ForegroundColor Green
} else {
    Write-Warning "  Assets not found: $AssetsSource"
}

# Step 5: Verify
Write-Host "[5/5] Verifying..." -ForegroundColor Yellow
$releaseFiles = Get-ChildItem -LiteralPath $ReleaseDir -Recurse | Measure-Object
Write-Host "  Total items in Release: $($releaseFiles.Count)" -ForegroundColor Green

Write-Host ""
Write-Host "=== Done! ===" -ForegroundColor Cyan
Write-Host "Release folder: $ReleaseDir"
Write-Host ""

# Show structure
Write-Host "Structure:" -ForegroundColor Cyan
Get-ChildItem -LiteralPath $ReleaseDir | ForEach-Object {
    if ($_.PSIsContainer) {
        $count = (Get-ChildItem -LiteralPath $_.FullName -Recurse -File | Measure-Object).Count
        Write-Host "  $($_.Name)/ ($count files)"
    } else {
        Write-Host "  $($_.Name)"
    }
}

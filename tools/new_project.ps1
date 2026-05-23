param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Name,

    [string]$OutputRoot,

    [switch]$Force,

    [switch]$InitGit
)

$ErrorActionPreference = "Stop"

function Convert-ToTargetName {
    param([string]$Value)

    $target = $Value.Trim().ToLowerInvariant()
    $target = [regex]::Replace($target, "[^a-z0-9]+", "_")
    $target = $target.Trim("_")

    if ([string]::IsNullOrWhiteSpace($target)) {
        throw "Project name must contain at least one letter or number."
    }

    if ($target[0] -match "[0-9]") {
        $target = "game_$target"
    }

    return $target
}

function Convert-ToDefineName {
    param([string]$Value)

    return ([regex]::Replace($Value.ToUpperInvariant(), "[^A-Z0-9]+", "_")).Trim("_")
}

function Convert-ToFolderName {
    param([string]$Value)

    $invalid = [regex]::Escape(([System.IO.Path]::GetInvalidFileNameChars() -join ""))
    $folder = [regex]::Replace($Value.Trim(), "[$invalid]", "")
    $folder = [regex]::Replace($folder, "\s+", "")

    if ([string]::IsNullOrWhiteSpace($folder)) {
        throw "Project name does not produce a valid folder name."
    }

    return $folder
}

function Convert-ToCMakePath {
    param([string]$Value)

    return $Value.Replace("\", "/")
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$frameworkRoot = Resolve-Path (Join-Path $scriptDir "..")
$templateRoot = Join-Path $scriptDir "project_template"

if (!(Test-Path $templateRoot)) {
    throw "Project template not found: $templateRoot"
}

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Split-Path -Parent $frameworkRoot
}

$outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
$folderName = Convert-ToFolderName $Name
$projectDir = Join-Path $outputRootPath $folderName
$targetName = Convert-ToTargetName $Name
$defineName = Convert-ToDefineName $targetName
$frameworkPath = Convert-ToCMakePath ([System.IO.Path]::GetFullPath($frameworkRoot))
$projectPath = Convert-ToCMakePath ([System.IO.Path]::GetFullPath($projectDir))

if ((Test-Path $projectDir) -and !$Force) {
    $existingFiles = @(Get-ChildItem -LiteralPath $projectDir -Force -ErrorAction SilentlyContinue)
    if ($existingFiles.Count -gt 0) {
        throw "Project directory already exists and is not empty: $projectDir. Use -Force to overwrite template files."
    }
}

New-Item -ItemType Directory -Force -Path $projectDir | Out-Null
Copy-Item -Path (Join-Path $templateRoot "*") -Destination $projectDir -Recurse -Force

$runtimeAssets = @(
    "assets/shaders/crt.fs",
    "assets/textures/noise.png"
)

foreach ($asset in $runtimeAssets) {
    $sourceAsset = Join-Path $frameworkRoot $asset
    $targetAsset = Join-Path $projectDir $asset

    if (Test-Path $sourceAsset) {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetAsset) | Out-Null
        Copy-Item -LiteralPath $sourceAsset -Destination $targetAsset -Force
    }
}

$replacements = @{
    "@PROJECT_NAME@" = $Name
    "@PROJECT_TARGET@" = $targetName
    "@PROJECT_DEFINE@" = $defineName
    "@R2D_FRAMEWORK_PATH@" = $frameworkPath
    "@PROJECT_DIR@" = $projectPath
}

$textExtensions = @(".bat", ".c", ".cmake", ".h", ".ini", ".md", ".ps1", ".txt", ".gitignore")
$files = Get-ChildItem -LiteralPath $projectDir -Recurse -File -Force

foreach ($file in $files) {
    if ($textExtensions -notcontains $file.Extension -and $file.Name -ne ".gitignore") {
        continue
    }

    $content = Get-Content -LiteralPath $file.FullName -Raw
    foreach ($key in $replacements.Keys) {
        $content = $content.Replace($key, $replacements[$key])
    }
    Set-Content -LiteralPath $file.FullName -Value $content -NoNewline
}

if ($InitGit) {
    git -C $projectDir init | Out-Null
}

Write-Host "Created Retro2D project:"
Write-Host "  Name:   $Name"
Write-Host "  Target: $targetName"
Write-Host "  Path:   $projectDir"
Write-Host ""
Write-Host "Next:"
Write-Host "  cd `"$projectDir`""
Write-Host "  .\build.bat debug"

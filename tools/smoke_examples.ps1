param(
    [switch]$Full,
    [switch]$SkipDist
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")
$BuildScript = Join-Path $RootDir "build.bat"

if (-not (Test-Path $BuildScript)) {
    throw "Missing build script: $BuildScript"
}

$targets = @(
    "r2d_template_game",
    "r2d_platformer_example",
    "r2d_topdown_example",
    "r2d_collect",
    "r2d_relic_run",
    "r2d_palette_example"
)

if ($Full) {
    $targets = @(
        "r2d_hello_index",
        "r2d_input_example",
        "r2d_ui_example",
        "r2d_audio_example",
        "r2d_state_example",
        "r2d_collision_example",
        "r2d_particle_example",
        "r2d_palette_example",
        "r2d_time_example",
        "r2d_save_example",
        "r2d_template_game",
        "r2d_platformer_example",
        "r2d_topdown_example",
        "r2d_collect",
        "r2d_relic_run"
    )
}

$distTargets = @(
    "r2d_template_game",
    "r2d_collect",
    "r2d_relic_run",
    "r2d_palette_example"
)

function Invoke-SmokeBuild {
    param(
        [string]$Mode,
        [string]$Target
    )

    Write-Host ("> build.bat {0} {1}" -f $Mode, $Target)
    & $BuildScript $Mode $Target
    if ($LASTEXITCODE -ne 0) {
        throw "build.bat $Mode $Target failed with exit code $LASTEXITCODE"
    }
}

Push-Location $RootDir
try {
    foreach ($target in $targets) {
        Invoke-SmokeBuild -Mode "debug" -Target $target
        Invoke-SmokeBuild -Mode "release" -Target $target
    }

    if (-not $SkipDist) {
        foreach ($target in $distTargets) {
            Invoke-SmokeBuild -Mode "dist" -Target $target
        }
    }

    Write-Host "Smoke examples OK"
} finally {
    Pop-Location
}

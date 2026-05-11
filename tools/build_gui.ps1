param(
    [switch]$SelfTest,
    [string]$SmokeBuild,
    [string]$AutoBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")
$BuildScript = Join-Path $RootDir "build.bat"
$TargetConfigPath = Join-Path $PSScriptRoot "build_targets.json"
$LogPath = Join-Path $RootDir "build_gui.log"

trap {
    $message = $_.Exception.ToString()
    $line = "[{0}] {1}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"), $message
    Add-Content -Path $LogPath -Value $line
    Write-Error $message
    exit 1
}

function Write-DiagnosticLog {
    param([string]$Message)

    $line = "[{0}] {1}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"), $Message
    Add-Content -Path $LogPath -Value $line
}

function Show-LauncherError {
    param(
        [string]$Title,
        [System.Exception]$Exception
    )

    $message = $Exception.ToString()
    Write-DiagnosticLog $message

    if ([System.Windows.Forms.Application] -as [type]) {
        [void][System.Windows.Forms.MessageBox]::Show(
            $message,
            $Title,
            [System.Windows.Forms.MessageBoxButtons]::OK,
            [System.Windows.Forms.MessageBoxIcon]::Error
        )
    } else {
        Write-Error $message
    }
}

function Get-BuildTargets {
    if (-not (Test-Path $TargetConfigPath)) {
        throw "Missing target config: $TargetConfigPath"
    }

    $config = Get-Content -Raw $TargetConfigPath | ConvertFrom-Json
    if (-not $config.targets -or $config.targets.Count -eq 0) {
        throw "No build targets were found in $TargetConfigPath"
    }

    foreach ($target in $config.targets) {
        if (-not $target.name -or -not $target.label) {
            throw "Each target needs a name and label in $TargetConfigPath"
        }
    }

    return $config.targets
}

function Get-ExePath {
    param(
        [string]$Config,
        [string]$Target
    )

    return Join-Path $RootDir ("build\{0}\{1}.exe" -f $Config, $Target)
}

function New-ProcessStartInfo {
    param(
        [string]$Arguments
    )

    $info = [System.Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $env:ComSpec
    $info.Arguments = '/d /c ""{0}" {1}"' -f $BuildScript, $Arguments
    $info.WorkingDirectory = $RootDir
    $info.UseShellExecute = $false
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.CreateNoWindow = $true
    return $info
}

$Targets = Get-BuildTargets

if (-not (Test-Path $BuildScript)) {
    throw "Missing build script: $BuildScript"
}

if ($SelfTest) {
    Write-Host "Build GUI config OK"
    Write-Host ("Targets: " + (($Targets | ForEach-Object { $_.name }) -join ", "))
    exit 0
}

if ($SmokeBuild) {
    $info = New-ProcessStartInfo -Arguments $SmokeBuild
    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $info
    [void]$process.Start()
    $stdout = $process.StandardOutput.ReadToEnd()
    $stderr = $process.StandardError.ReadToEnd()
    $process.WaitForExit()
    if ($stdout) {
        Write-Host $stdout
    }
    if ($stderr) {
        Write-Error $stderr
    }
    exit $process.ExitCode
}

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

[System.Windows.Forms.Application]::EnableVisualStyles()
[System.Windows.Forms.Application]::SetUnhandledExceptionMode([System.Windows.Forms.UnhandledExceptionMode]::CatchException)

[System.Windows.Forms.Application]::add_ThreadException({
    param($sender, $eventArgs)
    Show-LauncherError -Title "Build launcher error" -Exception $eventArgs.Exception
})

[System.AppDomain]::CurrentDomain.add_UnhandledException({
    param($sender, $eventArgs)
    if ($eventArgs.ExceptionObject -is [System.Exception]) {
        Show-LauncherError -Title "Build launcher error" -Exception $eventArgs.ExceptionObject
    } else {
        Write-DiagnosticLog ([string]$eventArgs.ExceptionObject)
    }
})

$form = [System.Windows.Forms.Form]::new()
$form.Text = "Retro2DFramework Build"
$form.StartPosition = "CenterScreen"
$form.MinimumSize = [System.Drawing.Size]::new(760, 520)
$form.Size = [System.Drawing.Size]::new(860, 600)

$font = [System.Drawing.Font]::new("Segoe UI", 9)
$monoFont = [System.Drawing.Font]::new("Consolas", 9)
$form.Font = $font

$main = [System.Windows.Forms.TableLayoutPanel]::new()
$main.Dock = "Fill"
$main.ColumnCount = 1
$main.RowCount = 4
$main.Padding = [System.Windows.Forms.Padding]::new(12)
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::Percent, 100))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
$form.Controls.Add($main)

$title = [System.Windows.Forms.Label]::new()
$title.Text = "Retro2DFramework Build"
$title.AutoSize = $true
$title.Font = [System.Drawing.Font]::new("Segoe UI", 13, [System.Drawing.FontStyle]::Bold)
$title.Margin = [System.Windows.Forms.Padding]::new(0, 0, 0, 10)
$main.Controls.Add($title, 0, 0)

$controls = [System.Windows.Forms.TableLayoutPanel]::new()
$controls.Dock = "Top"
$controls.ColumnCount = 6
$controls.RowCount = 2
$controls.AutoSize = $true
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::Absolute, 140))
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::Absolute, 220))
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::Percent, 100))
[void]$controls.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
$main.Controls.Add($controls, 0, 1)

$configLabel = [System.Windows.Forms.Label]::new()
$configLabel.Text = "Config"
$configLabel.AutoSize = $true
$configLabel.Anchor = "Left"
$controls.Controls.Add($configLabel, 0, 0)

$configCombo = [System.Windows.Forms.ComboBox]::new()
$configCombo.DropDownStyle = "DropDownList"
$configCombo.Items.AddRange(@("Debug", "Release"))
$configCombo.SelectedIndex = 0
$configCombo.Dock = "Fill"
$controls.Controls.Add($configCombo, 1, 0)

$targetLabel = [System.Windows.Forms.Label]::new()
$targetLabel.Text = "Target"
$targetLabel.AutoSize = $true
$targetLabel.Anchor = "Left"
$targetLabel.Margin = [System.Windows.Forms.Padding]::new(14, 0, 3, 0)
$controls.Controls.Add($targetLabel, 2, 0)

$targetCombo = [System.Windows.Forms.ComboBox]::new()
$targetCombo.DropDownStyle = "DropDownList"
$targetCombo.DisplayMember = "label"
$targetCombo.ValueMember = "name"
$targetCombo.Dock = "Fill"
foreach ($target in $Targets) {
    [void]$targetCombo.Items.Add($target)
}
$targetCombo.SelectedIndex = 0
$controls.Controls.Add($targetCombo, 3, 0)

$buttonPanel = [System.Windows.Forms.FlowLayoutPanel]::new()
$buttonPanel.FlowDirection = "LeftToRight"
$buttonPanel.WrapContents = $false
$buttonPanel.AutoSize = $true
$buttonPanel.Anchor = "Right"
$controls.Controls.Add($buttonPanel, 5, 0)

$configureButton = [System.Windows.Forms.Button]::new()
$configureButton.Text = "Configure"
$configureButton.Width = 92
$buttonPanel.Controls.Add($configureButton)

$buildButton = [System.Windows.Forms.Button]::new()
$buildButton.Text = "Build"
$buildButton.Width = 92
$buttonPanel.Controls.Add($buildButton)

$buildRunButton = [System.Windows.Forms.Button]::new()
$buildRunButton.Text = "Build && Run"
$buildRunButton.Width = 100
$buttonPanel.Controls.Add($buildRunButton)

$statusLabel = [System.Windows.Forms.Label]::new()
$statusLabel.Text = "Ready"
$statusLabel.AutoSize = $true
$statusLabel.ForeColor = [System.Drawing.Color]::FromArgb(70, 70, 70)
$statusLabel.Margin = [System.Windows.Forms.Padding]::new(0, 8, 0, 0)
$controls.SetColumnSpan($statusLabel, 6)
$controls.Controls.Add($statusLabel, 0, 1)

$logBox = [System.Windows.Forms.TextBox]::new()
$logBox.Multiline = $true
$logBox.ReadOnly = $true
$logBox.ScrollBars = "Both"
$logBox.WordWrap = $false
$logBox.Dock = "Fill"
$logBox.Font = $monoFont
$logBox.BackColor = [System.Drawing.Color]::FromArgb(30, 30, 30)
$logBox.ForeColor = [System.Drawing.Color]::FromArgb(235, 235, 235)
$logBox.Margin = [System.Windows.Forms.Padding]::new(0, 12, 0, 8)
$main.Controls.Add($logBox, 0, 2)

$footer = [System.Windows.Forms.Label]::new()
$footer.Text = "Targets are loaded from tools\build_targets.json. Build logic stays in build.bat."
$footer.AutoSize = $true
$footer.ForeColor = [System.Drawing.Color]::FromArgb(90, 90, 90)
$main.Controls.Add($footer, 0, 3)

$runningProcess = $null
$activeBuildLog = $null
$activeLogOffset = 0
$activeOnSuccess = $null

function Add-Log {
    param([string]$Text)

    if ($form.IsDisposed) {
        return
    }

    $append = {
        param([string]$Message)
        $logBox.AppendText($Message + [Environment]::NewLine)
        $logBox.SelectionStart = $logBox.TextLength
        $logBox.ScrollToCaret()
    }

    if ($logBox.InvokeRequired) {
        [void]$logBox.BeginInvoke($append, @($Text))
    } else {
        & $append $Text
    }
}

function Set-BuildUiEnabled {
    param([bool]$Enabled)

    $apply = {
        param([bool]$IsEnabled)
        $configCombo.Enabled = $IsEnabled
        $targetCombo.Enabled = $IsEnabled
        $configureButton.Enabled = $IsEnabled
        $buildButton.Enabled = $IsEnabled
        $buildRunButton.Enabled = $IsEnabled
    }

    if ($form.InvokeRequired) {
        [void]$form.BeginInvoke($apply, @($Enabled))
    } else {
        & $apply $Enabled
    }
}

function Set-Status {
    param(
        [string]$Text,
        [System.Drawing.Color]$Color = [System.Drawing.Color]::FromArgb(70, 70, 70)
    )

    $apply = {
        param([string]$Message, [System.Drawing.Color]$MessageColor)
        $statusLabel.Text = $Message
        $statusLabel.ForeColor = $MessageColor
    }

    if ($form.InvokeRequired) {
        [void]$form.BeginInvoke($apply, @($Text, $Color))
    } else {
        & $apply $Text $Color
    }
}

function Start-BuildCommand {
    param(
        [string]$Arguments,
        [scriptblock]$OnSuccess = $null
    )

    if ($runningProcess -ne $null -and -not $runningProcess.HasExited) {
        Add-Log "A build is already running."
        return
    }

    Set-BuildUiEnabled $false
    Set-Status "Running..." ([System.Drawing.Color]::FromArgb(30, 90, 160))
    Add-Log ""
    Add-Log ("> build.bat " + $Arguments)

    $script:activeBuildLog = Join-Path $RootDir "build_gui_last_build.log"
    $script:activeLogOffset = 0
    $script:activeOnSuccess = $OnSuccess
    Set-Content -Path $script:activeBuildLog -Value "" -Encoding UTF8

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $process.StartInfo.FileName = $env:ComSpec
    $process.StartInfo.Arguments = '/d /c ""{0}" {1} > "{2}" 2>&1"' -f $BuildScript, $Arguments, $script:activeBuildLog
    $process.StartInfo.WorkingDirectory = $RootDir
    $process.StartInfo.UseShellExecute = $false
    $process.StartInfo.CreateNoWindow = $true

    $script:runningProcess = $process

    try {
        [void]$process.Start()
        $buildTimer.Start()
    } catch {
        $script:runningProcess = $null
        Set-BuildUiEnabled $true
        Set-Status "Could not start build" ([System.Drawing.Color]::FromArgb(170, 50, 45))
        Add-Log $_.Exception.Message
    }
}

function Get-SelectedTargetName {
    return [string]$targetCombo.SelectedItem.name
}

function Get-SelectedTargetCanRun {
    return [bool]$targetCombo.SelectedItem.run
}

$buildTimer = [System.Windows.Forms.Timer]::new()
$buildTimer.Interval = 250
$buildTimer.Add_Tick({
    try {
        if ($activeBuildLog -and (Test-Path $activeBuildLog)) {
            $text = Get-Content -Raw $activeBuildLog
            if ($text.Length -gt $activeLogOffset) {
                $chunk = $text.Substring($activeLogOffset)
                $script:activeLogOffset = $text.Length
                foreach ($line in ($chunk -split "`r?`n")) {
                    if ($line.Length -gt 0) {
                        Add-Log $line
                    }
                }
            }
        }

        if ($runningProcess -ne $null -and $runningProcess.HasExited) {
            $buildTimer.Stop()
            $code = $runningProcess.ExitCode
            $runningProcess.Dispose()
            $script:runningProcess = $null
            Set-BuildUiEnabled $true

            if ($code -eq 0) {
                Set-Status "Build finished successfully" ([System.Drawing.Color]::FromArgb(30, 130, 70))
                Add-Log "Build finished successfully."
                if ($activeOnSuccess) {
                    & $activeOnSuccess
                }
            } else {
                Set-Status ("Build failed with exit code {0}" -f $code) ([System.Drawing.Color]::FromArgb(170, 50, 45))
                Add-Log ("Build failed with exit code {0}." -f $code)
            }

            $script:activeOnSuccess = $null
        }
    } catch {
        $buildTimer.Stop()
        Write-DiagnosticLog $_.Exception.ToString()
        Set-BuildUiEnabled $true
        Set-Status "Build launcher error" ([System.Drawing.Color]::FromArgb(170, 50, 45))
        Add-Log $_.Exception.Message
    }
})

$configureButton.Add_Click({
    try {
        Start-BuildCommand -Arguments "configure"
    } catch {
        Show-LauncherError -Title "Configure failed to start" -Exception $_.Exception
    }
})

$buildButton.Add_Click({
    try {
        $config = [string]$configCombo.SelectedItem
        $target = Get-SelectedTargetName
        Start-BuildCommand -Arguments ('"{0}" "{1}"' -f $config.ToLowerInvariant(), $target)
    } catch {
        Show-LauncherError -Title "Build failed to start" -Exception $_.Exception
    }
})

$buildRunButton.Add_Click({
    try {
        $config = [string]$configCombo.SelectedItem
        $target = Get-SelectedTargetName

        if (-not (Get-SelectedTargetCanRun)) {
            Add-Log "Select a runnable target before using Build & Run."
            return
        }

        $runAfterBuild = {
            $exePath = Get-ExePath -Config $config -Target $target
            if (-not (Test-Path $exePath)) {
                Add-Log ("Executable not found: {0}" -f $exePath)
                return
            }

            Add-Log ("> " + $exePath)
            Start-Process -FilePath $exePath -WorkingDirectory (Split-Path $exePath)
        }.GetNewClosure()

        Start-BuildCommand -Arguments ('"{0}" "{1}"' -f $config.ToLowerInvariant(), $target) -OnSuccess $runAfterBuild
    } catch {
        Show-LauncherError -Title "Build and run failed to start" -Exception $_.Exception
    }
})

$form.Add_FormClosing({
    param($sender, $eventArgs)

    if ($runningProcess -ne $null -and -not $runningProcess.HasExited) {
        $answer = [System.Windows.Forms.MessageBox]::Show(
            "A build is still running. Close the launcher anyway?",
            "Build running",
            [System.Windows.Forms.MessageBoxButtons]::YesNo,
            [System.Windows.Forms.MessageBoxIcon]::Warning
        )

        if ($answer -ne [System.Windows.Forms.DialogResult]::Yes) {
            $eventArgs.Cancel = $true
        }
    }
})

$autoBuildTimer = $null
if ($AutoBuild) {
    $autoBuildTimer = [System.Windows.Forms.Timer]::new()
    $autoBuildTimer.Interval = 500
    $autoBuildTimer.Add_Tick({
        $autoBuildTimer.Stop()
        Start-BuildCommand -Arguments $AutoBuild -OnSuccess {
            $form.Close()
        }
    })
    $form.Add_Shown({
        $autoBuildTimer.Start()
    })
}

$result = $form.ShowDialog()
exit 0

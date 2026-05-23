param(
    [switch]$SelfTest
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RootDir = Resolve-Path (Join-Path $PSScriptRoot "..")
$NewProjectScript = Join-Path $PSScriptRoot "new_project.ps1"
$LogPath = Join-Path $RootDir "new_project_gui.log"

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

function Convert-ToTargetName {
    param([string]$Value)

    $target = $Value.Trim().ToLowerInvariant()
    $target = [regex]::Replace($target, "[^a-z0-9]+", "_")
    $target = $target.Trim("_")

    if ([string]::IsNullOrWhiteSpace($target)) {
        return ""
    }

    if ($target[0] -match "[0-9]") {
        $target = "game_$target"
    }

    return $target
}

function Convert-ToFolderName {
    param([string]$Value)

    $invalid = [regex]::Escape(([System.IO.Path]::GetInvalidFileNameChars() -join ""))
    $folder = [regex]::Replace($Value.Trim(), "[$invalid]", "")
    $folder = [regex]::Replace($folder, "\s+", "")
    return $folder
}

function Get-DefaultOutputRoot {
    return Split-Path -Parent $RootDir
}

function Quote-ProcessArgument {
    param([string]$Value)

    return '"' + $Value.Replace('"', '\"') + '"'
}

if (-not (Test-Path $NewProjectScript)) {
    throw "Missing project generator: $NewProjectScript"
}

if ($SelfTest) {
    Write-Host "New project GUI config OK"
    Write-Host ("Generator: " + $NewProjectScript)
    exit 0
}

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

[System.Windows.Forms.Application]::EnableVisualStyles()
[System.Windows.Forms.Application]::SetUnhandledExceptionMode([System.Windows.Forms.UnhandledExceptionMode]::CatchException)

[System.Windows.Forms.Application]::add_ThreadException({
    param($sender, $eventArgs)
    Show-LauncherError -Title "New project launcher error" -Exception $eventArgs.Exception
})

[System.AppDomain]::CurrentDomain.add_UnhandledException({
    param($sender, $eventArgs)
    if ($eventArgs.ExceptionObject -is [System.Exception]) {
        Show-LauncherError -Title "New project launcher error" -Exception $eventArgs.ExceptionObject
    } else {
        Write-DiagnosticLog ([string]$eventArgs.ExceptionObject)
    }
})

$form = [System.Windows.Forms.Form]::new()
$form.Text = "Retro2DFramework New Project"
$form.StartPosition = "CenterScreen"
$form.MinimumSize = [System.Drawing.Size]::new(760, 470)
$form.Size = [System.Drawing.Size]::new(760, 470)

$font = [System.Drawing.Font]::new("Segoe UI", 9)
$monoFont = [System.Drawing.Font]::new("Consolas", 9)
$form.Font = $font

$main = [System.Windows.Forms.TableLayoutPanel]::new()
$main.Dock = "Fill"
$main.ColumnCount = 1
$main.RowCount = 5
$main.Padding = [System.Windows.Forms.Padding]::new(12)
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::Percent, 100))
[void]$main.RowStyles.Add([System.Windows.Forms.RowStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
$form.Controls.Add($main)

$title = [System.Windows.Forms.Label]::new()
$title.Text = "Create Retro2D Project"
$title.AutoSize = $true
$title.Font = [System.Drawing.Font]::new("Segoe UI", 13, [System.Drawing.FontStyle]::Bold)
$title.Margin = [System.Windows.Forms.Padding]::new(0, 0, 0, 10)
$main.Controls.Add($title, 0, 0)

$fields = [System.Windows.Forms.TableLayoutPanel]::new()
$fields.Dock = "Top"
$fields.ColumnCount = 3
$fields.RowCount = 4
$fields.AutoSize = $true
[void]$fields.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
[void]$fields.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::Percent, 100))
[void]$fields.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
$main.Controls.Add($fields, 0, 1)

$nameLabel = [System.Windows.Forms.Label]::new()
$nameLabel.Text = "Project name"
$nameLabel.AutoSize = $true
$nameLabel.Anchor = "Left"
$fields.Controls.Add($nameLabel, 0, 0)

$nameBox = [System.Windows.Forms.TextBox]::new()
$nameBox.Dock = "Fill"
$nameBox.Margin = [System.Windows.Forms.Padding]::new(8, 0, 0, 8)
$fields.Controls.Add($nameBox, 1, 0)
$fields.SetColumnSpan($nameBox, 2)

$outputLabel = [System.Windows.Forms.Label]::new()
$outputLabel.Text = "Location"
$outputLabel.AutoSize = $true
$outputLabel.Anchor = "Left"
$fields.Controls.Add($outputLabel, 0, 1)

$outputBox = [System.Windows.Forms.TextBox]::new()
$outputBox.Dock = "Fill"
$outputBox.Text = Get-DefaultOutputRoot
$outputBox.Margin = [System.Windows.Forms.Padding]::new(8, 0, 8, 8)
$fields.Controls.Add($outputBox, 1, 1)

$browseButton = [System.Windows.Forms.Button]::new()
$browseButton.Text = "Browse..."
$browseButton.Width = 90
$browseButton.Margin = [System.Windows.Forms.Padding]::new(0, 0, 0, 8)
$fields.Controls.Add($browseButton, 2, 1)

$targetLabel = [System.Windows.Forms.Label]::new()
$targetLabel.Text = "Target"
$targetLabel.AutoSize = $true
$targetLabel.Anchor = "Left"
$fields.Controls.Add($targetLabel, 0, 2)

$targetValue = [System.Windows.Forms.Label]::new()
$targetValue.Text = ""
$targetValue.AutoSize = $true
$targetValue.Anchor = "Left"
$targetValue.ForeColor = [System.Drawing.Color]::FromArgb(70, 70, 70)
$targetValue.Margin = [System.Windows.Forms.Padding]::new(8, 0, 0, 8)
$fields.Controls.Add($targetValue, 1, 2)
$fields.SetColumnSpan($targetValue, 2)

$pathLabel = [System.Windows.Forms.Label]::new()
$pathLabel.Text = "Folder"
$pathLabel.AutoSize = $true
$pathLabel.Anchor = "Left"
$fields.Controls.Add($pathLabel, 0, 3)

$pathValue = [System.Windows.Forms.Label]::new()
$pathValue.Text = ""
$pathValue.AutoSize = $true
$pathValue.Anchor = "Left"
$pathValue.ForeColor = [System.Drawing.Color]::FromArgb(70, 70, 70)
$pathValue.Margin = [System.Windows.Forms.Padding]::new(8, 0, 0, 0)
$fields.Controls.Add($pathValue, 1, 3)
$fields.SetColumnSpan($pathValue, 2)

$options = [System.Windows.Forms.FlowLayoutPanel]::new()
$options.FlowDirection = "LeftToRight"
$options.WrapContents = $false
$options.AutoSize = $true
$options.Margin = [System.Windows.Forms.Padding]::new(0, 12, 0, 0)
$main.Controls.Add($options, 0, 2)

$initGitCheck = [System.Windows.Forms.CheckBox]::new()
$initGitCheck.Text = "Initialize Git repository"
$initGitCheck.AutoSize = $true
$options.Controls.Add($initGitCheck)

$forceCheck = [System.Windows.Forms.CheckBox]::new()
$forceCheck.Text = "Overwrite template files if folder exists"
$forceCheck.AutoSize = $true
$forceCheck.Margin = [System.Windows.Forms.Padding]::new(18, 0, 0, 0)
$options.Controls.Add($forceCheck)

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
$main.Controls.Add($logBox, 0, 3)

$footerPanel = [System.Windows.Forms.TableLayoutPanel]::new()
$footerPanel.Dock = "Bottom"
$footerPanel.ColumnCount = 2
$footerPanel.RowCount = 1
[void]$footerPanel.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::Percent, 100))
[void]$footerPanel.ColumnStyles.Add([System.Windows.Forms.ColumnStyle]::new([System.Windows.Forms.SizeType]::AutoSize))
$main.Controls.Add($footerPanel, 0, 4)

$statusLabel = [System.Windows.Forms.Label]::new()
$statusLabel.Text = "Ready"
$statusLabel.AutoSize = $true
$statusLabel.Anchor = "Left"
$statusLabel.ForeColor = [System.Drawing.Color]::FromArgb(70, 70, 70)
$footerPanel.Controls.Add($statusLabel, 0, 0)

$buttonPanel = [System.Windows.Forms.FlowLayoutPanel]::new()
$buttonPanel.FlowDirection = "LeftToRight"
$buttonPanel.WrapContents = $false
$buttonPanel.AutoSize = $true
$buttonPanel.Anchor = "Right"
$footerPanel.Controls.Add($buttonPanel, 1, 0)

$openButton = [System.Windows.Forms.Button]::new()
$openButton.Text = "Open Folder"
$openButton.Width = 100
$openButton.Enabled = $false
$buttonPanel.Controls.Add($openButton)

$createButton = [System.Windows.Forms.Button]::new()
$createButton.Text = "Create"
$createButton.Width = 100
$buttonPanel.Controls.Add($createButton)

$createdPath = $null

function Add-Log {
    param([string]$Text)

    $logBox.AppendText($Text + [Environment]::NewLine)
    $logBox.SelectionStart = $logBox.TextLength
    $logBox.ScrollToCaret()
}

function Set-Status {
    param(
        [string]$Text,
        [System.Drawing.Color]$Color = [System.Drawing.Color]::FromArgb(70, 70, 70)
    )

    $statusLabel.Text = $Text
    $statusLabel.ForeColor = $Color
}

function Update-Preview {
    $name = $nameBox.Text
    $target = Convert-ToTargetName $name
    $folder = Convert-ToFolderName $name
    $root = $outputBox.Text.Trim()

    $targetValue.Text = if ($target) { $target } else { "(waiting for name)" }

    if ($folder -and $root) {
        $pathValue.Text = Join-Path $root $folder
    } else {
        $pathValue.Text = "(waiting for location)"
    }
}

function Set-CreationUiEnabled {
    param([bool]$Enabled)

    $nameBox.Enabled = $Enabled
    $outputBox.Enabled = $Enabled
    $browseButton.Enabled = $Enabled
    $initGitCheck.Enabled = $Enabled
    $forceCheck.Enabled = $Enabled
    $createButton.Enabled = $Enabled
}

function Start-ProjectCreate {
    $name = $nameBox.Text.Trim()
    $outputRoot = $outputBox.Text.Trim()

    if ([string]::IsNullOrWhiteSpace($name)) {
        [void][System.Windows.Forms.MessageBox]::Show(
            "Write a project name first.",
            "Missing project name",
            [System.Windows.Forms.MessageBoxButtons]::OK,
            [System.Windows.Forms.MessageBoxIcon]::Warning
        )
        return
    }

    if ([string]::IsNullOrWhiteSpace($outputRoot)) {
        [void][System.Windows.Forms.MessageBox]::Show(
            "Choose a project location first.",
            "Missing location",
            [System.Windows.Forms.MessageBoxButtons]::OK,
            [System.Windows.Forms.MessageBoxIcon]::Warning
        )
        return
    }

    Set-CreationUiEnabled $false
    Set-Status "Creating project..." ([System.Drawing.Color]::FromArgb(30, 90, 160))
    Add-Log ""
    Add-Log ("> new_project.ps1 `"{0}`" -OutputRoot `"{1}`"" -f $name, $outputRoot)

    $arguments = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", (Quote-ProcessArgument $NewProjectScript),
        (Quote-ProcessArgument $name),
        "-OutputRoot", (Quote-ProcessArgument $outputRoot)
    )

    if ($forceCheck.Checked) {
        $arguments += "-Force"
    }

    if ($initGitCheck.Checked) {
        $arguments += "-InitGit"
    }

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $process.StartInfo.FileName = "powershell"
    $process.StartInfo.Arguments = $arguments -join " "
    $process.StartInfo.WorkingDirectory = $RootDir
    $process.StartInfo.UseShellExecute = $false
    $process.StartInfo.RedirectStandardOutput = $true
    $process.StartInfo.RedirectStandardError = $true
    $process.StartInfo.CreateNoWindow = $true

    try {
        [void]$process.Start()
        $stdout = $process.StandardOutput.ReadToEnd()
        $stderr = $process.StandardError.ReadToEnd()
        $process.WaitForExit()

        if ($stdout) {
            foreach ($line in ($stdout -split "`r?`n")) {
                if ($line.Length -gt 0) {
                    Add-Log $line
                }
            }
        }

        if ($stderr) {
            foreach ($line in ($stderr -split "`r?`n")) {
                if ($line.Length -gt 0) {
                    Add-Log $line
                }
            }
        }

        if ($process.ExitCode -eq 0) {
            $script:createdPath = $pathValue.Text
            $openButton.Enabled = $true
            Set-Status "Project created" ([System.Drawing.Color]::FromArgb(30, 130, 70))
        } else {
            Set-Status ("Create failed with exit code {0}" -f $process.ExitCode) ([System.Drawing.Color]::FromArgb(170, 50, 45))
        }
    } catch {
        Show-LauncherError -Title "Create project failed" -Exception $_.Exception
        Set-Status "Create failed" ([System.Drawing.Color]::FromArgb(170, 50, 45))
    } finally {
        $process.Dispose()
        Set-CreationUiEnabled $true
    }
}

$nameBox.Add_TextChanged({ Update-Preview })
$outputBox.Add_TextChanged({ Update-Preview })

$browseButton.Add_Click({
    try {
        $dialog = [System.Windows.Forms.FolderBrowserDialog]::new()
        $dialog.Description = "Choose where the project folder will be created."
        $dialog.SelectedPath = $outputBox.Text
        $dialog.ShowNewFolderButton = $true

        if ($dialog.ShowDialog($form) -eq [System.Windows.Forms.DialogResult]::OK) {
            $outputBox.Text = $dialog.SelectedPath
        }
    } catch {
        Show-LauncherError -Title "Browse failed" -Exception $_.Exception
    }
})

$createButton.Add_Click({
    try {
        Start-ProjectCreate
    } catch {
        Show-LauncherError -Title "Create project failed" -Exception $_.Exception
    }
})

$openButton.Add_Click({
    try {
        if ($createdPath -and (Test-Path $createdPath)) {
            Start-Process -FilePath explorer.exe -ArgumentList @($createdPath)
        }
    } catch {
        Show-LauncherError -Title "Open folder failed" -Exception $_.Exception
    }
})

Update-Preview
$result = $form.ShowDialog()
exit 0

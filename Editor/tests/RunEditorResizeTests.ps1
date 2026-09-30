param([ValidateSet('SDL3', 'D3D11')][string]$Backend = 'SDL3')
$ErrorActionPreference = 'Stop'
$workspace = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$binaryDir = Join-Path $workspace 'ZEngine/bin/win_x64'
$reports = Join-Path $workspace 'out/test-results'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
$stdout = Join-Path $reports "editor-resize-$Backend.log"
$stderr = Join-Path $reports "editor-resize-$Backend-error.log"
# The probe temporarily shows/maximizes only its own window, then exits without saving assets.
$probe = Start-Process -FilePath (Join-Path $binaryDir 'ZENGINE_editor.exe') -ArgumentList "--backend=$Backend", '--smoke-material=Materials/M2Demo/Seed.mat', '--smoke-resize' -WorkingDirectory $binaryDir -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
if (-not $probe.WaitForExit(30000)) {
    Stop-Process -Id $probe.Id
    throw 'Editor resize test exceeded 30 seconds.'
}
Get-Content $stdout
Get-Content $stderr
Write-Output "Editor resize test exit: $($probe.ExitCode)"
exit $probe.ExitCode

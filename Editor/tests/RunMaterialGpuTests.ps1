param([ValidateSet('SDL3', 'D3D11')][string]$Backend = 'SDL3')
$ErrorActionPreference = 'Stop'
$workspace = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$binaryDir = Join-Path $workspace 'ZEngine/bin/win_x64'
$reports = Join-Path $workspace 'out/test-results'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
$stdout = Join-Path $reports "material-m2-$Backend.log"
$stderr = Join-Path $reports "material-m2-$Backend-error.log"
$probe = Start-Process -FilePath (Join-Path $binaryDir 'ZENGINE_material_gpu_tests.exe') -ArgumentList $Backend -WorkingDirectory $binaryDir -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
if (-not $probe.WaitForExit(30000)) {
    Stop-Process -Id $probe.Id
    throw 'Material GPU test exceeded 30 seconds.'
}
Get-Content $stdout
Get-Content $stderr
Write-Output "GPU test exit: $($probe.ExitCode)"
exit $probe.ExitCode

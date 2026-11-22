#Requires -Version 5.1
<#
.SYNOPSIS
  Bake built-in C++ effects for D3D11 and D3D12 on Windows.

.DESCRIPTION
  With no effect selection, bakes every registered effect. Each backend writes
  the EffectDesc cache beside the effect's first HLSL source. Unsupported
  techniques are skipped; an effect must have at least one valid technique.

.EXAMPLE
  .\game\Tool\bake_kfx.ps1 -AllEffects -Force
  .\game\Tool\bake_kfx.ps1 -Effects SimpleAlbedoNormal,SkyBox -Force
#>
param(
    [string[]]$Effects = @(),
    [switch]$AllEffects,
    [string[]]$Factories = @("D3D11", "D3D12"),
    [string]$Config = "",
    [string]$BinDir = "",
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not $BinDir) {
    $BinDir = Join-Path $RepoRoot "ZEngine\bin\win_x64"
}
$BinDir = (Resolve-Path -LiteralPath $BinDir).Path
$BakeExe = Join-Path $BinDir "ZENGINE_shader_bake.exe"
if (-not (Test-Path -LiteralPath $BakeExe)) {
    $candidates = Get-ChildItem -LiteralPath $BinDir -Filter "ZENGINE_shader_bake*.exe" | Sort-Object Name
    if ($candidates) { $BakeExe = $candidates[0].FullName }
}
if (-not (Test-Path -LiteralPath $BakeExe)) {
    throw "ZENGINE_shader_bake not found under $BinDir. Build target ZENGINE_shader_bake first."
}
if (-not $Config) {
    $Config = Join-Path $RepoRoot "ZEngine\assets\KlayGE.cfg"
}
$Config = (Resolve-Path -LiteralPath $Config).Path
$AssetsShaders = Join-Path $RepoRoot "ZEngine\assets\shaders"
$commonArgs = @("--config", $Config, "--assets-dir", $AssetsShaders)
if ($Force) { $commonArgs += "--force" }
if ($AllEffects -or $Effects.Count -eq 0) { $commonArgs += "--all-effects" }
foreach ($effect in $Effects) { $commonArgs += @("--effect", $effect) }

Push-Location -LiteralPath $BinDir
try {
    foreach ($factory in $Factories) {
        Write-Host "Baking effects with $factory" -ForegroundColor Cyan
        $bakeArgs = $commonArgs + @("--factory", $factory)
        & $BakeExe @bakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "Bake failed for factory=$factory (exit $LASTEXITCODE)"
        }
    }
}
finally {
    Pop-Location
}
Write-Host "Done. EffectDesc caches were written beside the HLSL sources." -ForegroundColor Green

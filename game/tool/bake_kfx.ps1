#Requires -Version 5.1
<#
.SYNOPSIS
  One-command multi-target shader bake on Windows: D3D11 + D3D12 (d3d_12) .kfx files.

.DESCRIPTION
  Runs ZENGINE_shader_bake twice (--factory D3D11, then D3D12) so each .shader
  gets Foo.d3d_11_0.kfx and Foo.d3d_12.kfx next to the source.
  Mac metal_spirv bake is not included here (needs D3D12 CompileShader SPIR-V path).

.EXAMPLE
  .\game\tool\bake_kfx.ps1
  .\game\tool\bake_kfx.ps1 -Shaders SimpleAlbedoNormal.shader,SkyBox.shader -Force
#>
param(
	[string[]]$Shaders = @(
		"SimpleAlbedoNormal.shader",
		"SkyBox.shader",
		"RmlUi.shader",
		"LightSourceProxy.shader"
	),
	[string[]]$Factories = @("D3D11", "D3D12"),
	[string]$Config = "",
	[string]$BinDir = "",
	[switch]$Force
)

$ErrorActionPreference = "Stop"
# game/tool -> Game -> repo root
$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not $BinDir) {
	$BinDir = Join-Path $RepoRoot "zengine\bin\win_x64"
}
$BakeExe = Join-Path $BinDir "ZENGINE_shader_bake.exe"
if (-not (Test-Path $BakeExe)) {
	# Multi-config / suffix variants
	$candidates = Get-ChildItem -Path $BinDir -Filter "ZENGINE_shader_bake*.exe" -ErrorAction SilentlyContinue
	if ($candidates) {
		$BakeExe = $candidates[0].FullName
	}
}
if (-not (Test-Path $BakeExe)) {
	Write-Error "ZENGINE_shader_bake not found under $BinDir. Build target ZENGINE_shader_bake first."
}

if (-not $Config) {
	$Config = Join-Path $RepoRoot "zengine\assets\KlayGE.cfg"
}

$AssetsShaders = Join-Path $RepoRoot "zengine\assets\shaders"
$commonArgs = @(
	"--config", $Config,
	"--assets-dir", $AssetsShaders
)
if ($Force) {
	$commonArgs += "--force"
}

foreach ($factory in $Factories) {
	Write-Host "=== Bake factory=$factory ===" -ForegroundColor Cyan
	$args = $commonArgs + @("--factory", $factory) + $Shaders
	& $BakeExe @args
	if ($LASTEXITCODE -ne 0) {
		Write-Error "Bake failed for factory=$factory (exit $LASTEXITCODE)"
	}
}

Write-Host "Done. Expected outputs next to each .shader:" -ForegroundColor Green
Write-Host "  *.d3d_11_0.kfx  (D3D11)"
Write-Host "  *.d3d_12.kfx    (Win D3D12)"

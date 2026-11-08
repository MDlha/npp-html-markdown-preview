$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation = & $locator -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $installation) { throw 'MSVC x64 nie je dostupné.' }
$cmake = Join-Path $installation 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
& $cmake -S $projectRoot -B (Join-Path $projectRoot 'build') -G 'Visual Studio 18 2026' -A x64
if ($LASTEXITCODE -ne 0) { throw 'Konfigurácia zlyhala.' }
& $cmake --build (Join-Path $projectRoot 'build') --config Release
if ($LASTEXITCODE -ne 0) { throw 'Zostavenie zlyhalo.' }
if (Test-Path -LiteralPath (Join-Path $projectRoot 'tests/PluginAbiCheck.cpp')) {
    & $ctest --test-dir (Join-Path $projectRoot 'build') -C Release --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Skúšky zlyhali.' }
}

$ErrorActionPreference = "Stop"
$mingwBin = Join-Path $env:LOCALAPPDATA "w64devkit\w64devkit\bin"
$env:Path = "$mingwBin;" + $env:Path

$exe = Join-Path $PSScriptRoot "build\jogo_novo.exe"
if (-not (Test-Path $exe)) {
    & (Join-Path $PSScriptRoot "build.ps1")
}

& $exe

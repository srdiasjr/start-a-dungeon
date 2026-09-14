$ErrorActionPreference = "Stop"

$mingwBin = Join-Path $env:LOCALAPPDATA "w64devkit\w64devkit\bin"

function Refresh-Path {
    $machine = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
    $user = [System.Environment]::GetEnvironmentVariable("Path", "User")
    $env:Path = "$mingwBin;$machine;$user"
}

Refresh-Path

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake nao encontrado. Feche e abra o terminal, ou reinstale com: winget install Kitware.CMake --scope user"
}

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    throw "g++ nao encontrado. Feche e abra o terminal, ou reinstale com: winget install BrechtSanders.WinLibs.POSIX.UCRT --scope user"
}

if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    throw "Ninja nao encontrado. Reinstale com: winget install Ninja-build.Ninja"
}

Set-Location $PSScriptRoot

Write-Host "Configurando o projeto..."
cmake --preset debug

Write-Host "Compilando..."
cmake --build --preset debug

Write-Host "Pronto. Executavel em: $PSScriptRoot\build\jogo_novo.exe"

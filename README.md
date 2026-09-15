# Jogo Novo

Projeto C++ com [Raylib](https://www.raylib.com/) para criar jogos 2D e 3D.

## Requisitos (ja instalados nesta maquina)

- CMake 4
- Ninja
- MinGW-w64 / GCC 16 (`w64devkit`)
- Git
- Raylib 5.5 (baixada na primeira compilacao)

Se o terminal nao encontrar `g++` ou `cmake`, feche e abra um terminal novo para recarregar o PATH.

## Como compilar e rodar

Na pasta do projeto:

```powershell
.\build.ps1
.\run.ps1
```

Ou manualmente:

```powershell
cmake --preset debug
cmake --build --preset debug
.\build\jogo_novo.exe
```

No Cursor, use **Terminal > Run Build Task** para compilar.

## Controles do exemplo

- **WASD** ou **setas**: mover o personagem
- Colete as moedas douradas para somar pontos

## Onde programar

Edite `src/main.cpp`. Coloque imagens, sons e fontes em `assets/`.

Cheatsheet da Raylib: https://www.raylib.com/cheatsheet/cheatsheet.html
"# start-a-dungeon" 

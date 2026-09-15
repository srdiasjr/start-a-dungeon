#pragma once

#include "raylib.h"

constexpr int kMaxPecasMapa = 512;
constexpr int kMaxCaminhoMapa = 260;
constexpr int kMaxNomeMapa = 64;
constexpr int kMaxMapasLista = 32;

enum class TipoPecaMapa {
    Chao = 0,
    Bloco,
    Muro,
    Rocha,
    Arvore,
    Pilar,
    Fonte,
    Madeira,
    Caixa,
    Arbusto,
    Contagem
};

enum class EditorFase {
    Menu = 0,
    CriarNovo,
    ModificarLista,
    Construindo
};

struct PecaMapa {
    TipoPecaMapa tipo;
    float x;
    float y; // base (chao do bloco)
    float z;
    float halfX;
    float halfZ;
    float altura;
    float yaw;
    float escala;
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct ObstaculoColisao {
    float x;
    float z;
    float halfX;
    float halfZ;
    float yBase;
    float yTopo;
};

struct MapaMundo {
    char nome[kMaxNomeMapa];
    float tamanho;
    bool noite;
    Color corChao;
    Color corCeu;
    PecaMapa pecas[kMaxPecasMapa];
    int quantidade;
};

struct PresetEditor {
    const char* nome;
    TipoPecaMapa tipo;
    float halfX;
    float halfZ;
    float altura;
    float escala;
};

struct MapaListaItem {
    char nome[kMaxNomeMapa];
    char caminho[kMaxCaminhoMapa];
    bool builtin;
};

struct ListaMapas {
    MapaListaItem itens[kMaxMapasLista];
    int quantidade;
};

struct EditorMapa {
    bool ativo;
    EditorFase fase;
    int preset;
    float grid;
    float yaw;
    float halfX;
    float halfZ;
    float altura;
    Color corPeca;
    float cursorX;
    float cursorY;
    float cursorZ;
    bool cursorValido;
    bool snap;
    bool encaixeSolido; // true = nao atravessa blocos (empilha em cima)
    bool sujo;
    char status[128];
    float statusTempo;
    char caminhoArquivo[kMaxCaminhoMapa];

    float novoTamanho;
    bool novoNoite;
    Color novoCorChao;
    char novoNome[kMaxNomeMapa];

    ListaMapas lista;
    int listaSelecionada;
};

const PresetEditor* EditorPresets(int* quantidade);
const char* NomeTipoPeca(TipoPecaMapa tipo);
TipoPecaMapa TipoPecaDeNome(const char* nome);

void MapaLimpar(MapaMundo* mapa);
void MapaCarregarPadrao(MapaMundo* mapa);
void MapaNovoEmBranco(MapaMundo* mapa, const char* nome, float tamanho, bool noite, Color corChao);
bool MapaCarregarDeArquivo(MapaMundo* mapa, const char* caminho);
bool MapaSalvarEmArquivo(const MapaMundo& mapa, const char* caminho);
bool MapaTentarCarregar(MapaMundo* mapa, char* caminhoUsado, int caminhoCapacidade);
bool MapaEhCasaBase(const char* caminhoOuNome);
bool MapaCarregarCasaBase(MapaMundo* mapa, char* caminhoUsado, int caminhoCapacidade);
bool MapaCarregarDungeon(MapaMundo* mapa, int dungeonId, char* caminhoUsado, int caminhoCapacidade);
const char* NomeDungeon(int dungeonId);
int QuantidadeDungeonsMundo1();
bool MapaResolverCaminhoSalvar(char* out, int capacidade);
void MapaResolverPastaMaps(char* out, int capacidade);
void MapaListarDisponiveis(ListaMapas* lista);

ObstaculoColisao PecaParaColisao(const PecaMapa& peca);
bool PecaTemColisao(const PecaMapa& peca);

Color MapaCorCeu(const MapaMundo& mapa);
void MapaDesenharBase(const MapaMundo& mapa, float tempo);
void MapaDesenharPecas(const MapaMundo& mapa, float tempo);
void MapaDesenharPeca(const PecaMapa& peca, float tempo, float alpha);

void EditorIniciar(EditorMapa* editor, const char* caminhoAtual);
void EditorDefinirStatus(EditorMapa* editor, const char* msg);
void EditorAtualizar(
    EditorMapa* editor,
    MapaMundo* mapa,
    const Camera3D& camera,
    bool mouseSobreUi);
void EditorDesenharUI(EditorMapa* editor, MapaMundo* mapa, bool* pediuSair);
void EditorDesenharGhost(const EditorMapa& editor, float tempo);
bool EditorEstaConstruindo(const EditorMapa& editor);

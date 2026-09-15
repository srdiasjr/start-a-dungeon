#include "raylib.h"
#include "rlgl.h"
#include "mapa.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <utility>

namespace {
constexpr int kLarguraTela = 1280;
constexpr int kAlturaTela = 720;
constexpr float kVelocidadeJogador = 7.0f;
constexpr float kVelocidadeCorrida = 12.5f;
constexpr float kVelocidadeAgachado = 3.2f;
constexpr float kVelocidadeAr = 3.5f;
constexpr float kAceleracaoAr = 8.0f;
constexpr float kForcaPulo = 7.5f;
constexpr float kForcaPuloAgachado = 11.0f;
constexpr float kGravidade = 22.0f;
constexpr float kRaioJogador = 0.42f;
constexpr float kTamanhoArena = 40.0f;
constexpr float kRaioInimigo = 0.45f;
constexpr float kDistanciaCamera = 3.4f;
constexpr float kDistanciaCameraMin = 1.5f;
constexpr float kDistanciaCameraMax = 7.0f;
constexpr float kRaioCamera = 0.28f;
constexpr float kCameraLado = 0.55f;
constexpr float kAlturaPivot = 1.48f;
constexpr float kAlturaPivotAgachado = 0.92f;
constexpr float kSensibilidadeMouse = 0.12f;
constexpr float kPitchMin = -55.0f;
constexpr float kPitchMax = 35.0f;
constexpr float kSuavidadeGiro = 10.0f;
constexpr float kDuracaoAtaque = 0.40f;
constexpr float kCooldownAtaque = 0.10f;
constexpr float kAlcanceAtaque = 2.35f;
constexpr float kDanoEspadaMadeira = 4.0f;
constexpr float kVidaInimigoMax = 20.0f;
constexpr float kVelocidadeInimigo = 3.2f;
constexpr float kVidaJogadorMax = 100.0f;
constexpr float kDanoInimigoBasico = 8.0f;
constexpr float kAlcanceAtaqueInimigo = 1.85f;
constexpr float kDuracaoAtaqueInimigo = 0.58f;
constexpr float kCooldownAtaqueInimigo = 1.15f;
constexpr float kCooldownArremesso = 1.7f;
constexpr float kAlcanceArremessoMin = 2.2f;
constexpr float kAlcanceArremessoMax = 13.0f;
constexpr float kAlturaInalcancavel = 0.65f;
constexpr int kMaxPedras = 48;
constexpr float kXpPorKill = 40.0f;
constexpr float kXpChefe = 150.0f;
constexpr float kXpBaseNivel = 100.0f;
constexpr int kNivelMax = 50;
constexpr int kPontosPorNivel = 2;
// Stats bem fracos por ponto (vao ser upados muitas vezes ate o 50)
constexpr float kStatManaCustoPorPonto = 0.007f;   // -0.7% custo (piso 45% do custo)
constexpr float kStatManaPoderPorPonto = 0.010f;   // +1.0% poder de feitiço
constexpr float kStatForcaPorPonto = 0.012f;       // +1.2% dano fisico
constexpr float kStatResistPorPonto = 0.009f;      // ~0.9% mitigaçao efetiva por ponto
// Altura maxima do pulo mais forte (agachar + pular)
constexpr float kAlturaPuloMax = (kForcaPuloAgachado * kForcaPuloAgachado) / (2.0f * kGravidade);
constexpr float kVidaChefeMax = 80.0f;
constexpr float kDanoChefeBasico = 10.0f;
constexpr float kDanoChefePorradao = 50.0f;
constexpr float kRaioChefe = 1.05f;
constexpr float kAlcanceChefeBasico = 2.4f;
constexpr float kAlcancePorradao = 3.4f;
constexpr float kImpulsoPorradao = 13.5f;
constexpr float kDuracaoPorradao = 0.95f;
constexpr float kCooldownPorradao = 3.2f;
constexpr float kVelocidadeChefe = 2.4f;

const Color kCorPele{232, 188, 150, 255};
const Color kCorCabelo{46, 38, 58, 255};
const Color kCorCamisa{72, 168, 232, 255};
const Color kCorCamisaEscura{42, 112, 168, 255};
const Color kCorCalca{38, 52, 86, 255};
const Color kCorSapato{28, 24, 32, 255};
const Color kCorOlho{245, 248, 255, 255};
const Color kCorPupila{24, 28, 40, 255};
const Color kCorLamina{210, 220, 230, 255};
const Color kCorGuarda{180, 150, 60, 255};
const Color kCorCabo{90, 50, 35, 255};
const Color kCorInimigo{160, 55, 55, 255};
const Color kCorInimigoEscuro{100, 30, 30, 255};
const Color kCorChefe{95, 28, 48, 255};
const Color kCorChefeEscuro{55, 14, 28, 255};
const Color kCorMadeiraCabo{72, 42, 22, 255};
const Color kCorMadeiraGuarda{110, 72, 38, 255};
const Color kCorMadeiraLamina{148, 102, 55, 255};
const Color kCorMadeiraPonta{175, 128, 72, 255};
constexpr float kTempoSpawnInimigo = 4.0f;
constexpr float kDelayEntreWaves = 3.5f;
constexpr int kTotalWaves = 10;
constexpr int kInimigosWave1 = 2;
constexpr float kAumentoPorWave = 1.25f;
constexpr int kMaxInimigos = 16;
constexpr float kXpPorWaveBase = 35.0f;
constexpr float kXpPorDungeonBase = 180.0f;
constexpr float kVelocidadeVoador = 1.55f;
constexpr int kConsoleCapacidade = 80;
constexpr float kDistanciaBau = 2.4f;
constexpr float kBauX = -4.0f;
constexpr float kBauZ = -0.8f;
constexpr float kBauPedestalAltura = 0.42f;
constexpr float kMesaX = 4.0f;
constexpr float kMesaZ = -0.8f;
constexpr float kArmarioX = 0.0f;
constexpr float kArmarioZ = -4.6f;
constexpr float kPortalX = 0.0f;
constexpr float kPortalZ = 16.0f;
constexpr float kBonecoX = 3.2f;
constexpr float kBonecoZ = 2.2f;
constexpr float kDistanciaInteracao = 3.4f;
constexpr float kManaMax = 100.0f;
constexpr float kManaRegen = 7.5f;
constexpr float kCustoBolaFogo = 28.0f;
constexpr float kDanoBolaFogoImpacto = 3.0f;
constexpr float kDanoQueimaPorTick = 1.25f;
constexpr float kIntervaloQueima = 0.45f;
constexpr float kDuracaoQueima = 2.5f;
constexpr float kVelocidadeBolaFogo = 16.0f;
constexpr float kCooldownBolaFogo = 1.45f;
constexpr float kVidaBonecoMax = 10000.0f;
constexpr int kMaxBolasFogo = 12;

struct Personagem {
    Vector3 posicao;
    float yaw;
    float cicloPasso;
    float intensidadeAndar;
    float intensidadeCorrida;
    float agachar;
    float velocidadeY;
    float velocidadeX;
    float velocidadeZ;
    float fasePulo;
    float ataque;       // 0 idle, 0..1 durante golpe
    float bloqueio;     // 0..1 quanto esta bloqueando
    float cooldownAtaque;
    float vida;
    float xp;
    int nivel;
    float hitFlash;
    float knockbackX;
    float knockbackZ;
    float moveFrente;   // -1 atras .. +1 frente (local)
    float moveLado;     // -1 esquerda .. +1 direita (local)
    bool miraTravada;
    bool temEspada;
    bool espadaMadeira;
    bool temMagiaFogo;
    bool usandoMagia; // true = LMB lanca magia; false = espada
    bool jaAcertouGolpe;
    bool noChao;
    float mana;
    float cooldownMagia;
    int pontosDisponiveis;
    int statMana;
    int statForca;
    int statResistencia;
};

float MultCustoMana(const Personagem& j);
float MultPoderFeitico(const Personagem& j);
float MultForcaFisica(const Personagem& j);
float MultResistencia(const Personagem& j);
float CustoBolaFogoAtual(const Personagem& j);

enum class EstacaoAberta {
    Nenhuma = 0,
    Bau,
    MesaMagica,
    Armario,
    Portal
};

enum class AbaBau {
    CorpoACorpo = 0,
    Distancia,
    Defesa
};

enum class AbaMesa {
    Encantamentos = 0,
    Pocoes,
    Magias
};

enum class TipoVisualInimigo {
    Normal = 0,
    Demonio,
    Voador,
    Caverna,
    Mutante,
    Elite
};

struct Bau {
    Vector3 posicao;
    float tampaAngulo;
    bool aberto;
};

struct Inimigo {
    Vector3 posicao;
    float yaw;
    float vida;
    float vidaMax;
    float hitFlash;
    float cicloPasso;
    float ataque;
    float porradao;
    float cooldownAtaque;
    float cooldownArremesso;
    float cooldownPorradao;
    float knockbackX;
    float knockbackZ;
    float recuo;
    float queimaTempo;
    float queimaTick;
    float queimaDano;
    float danoAtaque;
    float velocidadeMov;
    float alturaVoo;
    TipoVisualInimigo visual;
    bool jaAcertouGolpe;
    bool jaAcertouPorradao;
    bool chefe;
    bool boneco;
    bool voador;
    bool vivo;
};

struct Pedra {
    Vector3 posicao;
    Vector3 velocidade;
    float tempoVida;
    bool ativa;
};

struct BolaFogo {
    Vector3 posicao;
    Vector3 velocidade;
    float tempoVida;
    float danoImpacto;
    float danoQueima;
    bool ativa;
};

struct ConsoleComandos {
    bool aberto;
    char texto[kConsoleCapacidade];
    int comprimento;
    char mensagem[96];
    float mensagemTempo;
};

struct CameraOrbit {
    float yaw;
    float pitch;
    float distancia;
};

struct CameraEditorLivre {
    Vector3 posicao;
    float yaw;
    float pitch;
    float fovy;
    float velocidade;
};

struct SnapshotAntesEditor {
    bool valido;
    Vector3 posicao;
    float yaw;
    float velocidadeX;
    float velocidadeY;
    float velocidadeZ;
    float knockbackX;
    float knockbackZ;
    float agachar;
    float ataque;
    float bloqueio;
    float intensidadeAndar;
    float intensidadeCorrida;
    float moveFrente;
    float moveLado;
    float fasePulo;
    bool noChao;
    bool miraTravada;
    float orbitaYaw;
    float orbitaPitch;
    bool cursorLivre;
};

struct Obstaculo {
    float x;
    float z;
    float halfX;
    float halfZ;
    float yBase;
    float yTopo;
};

MapaMundo gMapa{};
EditorMapa gEditor{};
CameraEditorLivre gCamEditor{};
SnapshotAntesEditor gSnapshotEditor{};
char gCaminhoMapa[kMaxCaminhoMapa]{};
int gDungeonAtual = -1; // -1 = casa

Obstaculo ObstaculoBauPedestal() {
    return Obstaculo{kBauX, kBauZ, 0.85f, 0.70f, 0.0f, 0.95f};
}

Obstaculo ObstaculoMesa() {
    return Obstaculo{kMesaX, kMesaZ, 1.05f, 0.70f, 0.0f, 1.05f};
}

Obstaculo ObstaculoArmario() {
    return Obstaculo{kArmarioX, kArmarioZ, 0.85f, 0.55f, 0.0f, 2.0f};
}

bool EstamosNaCasa() {
    return MapaEhCasaBase(gCaminhoMapa) || MapaEhCasaBase(gMapa.nome);
}

template <typename Fn>
void ParaCadaObstaculoColisao(Fn&& fn) {
    for (int i = 0; i < gMapa.quantidade; ++i) {
        if (!PecaTemColisao(gMapa.pecas[i])) continue;
        const ObstaculoColisao c = PecaParaColisao(gMapa.pecas[i]);
        fn(Obstaculo{c.x, c.z, c.halfX, c.halfZ, c.yBase, c.yTopo});
    }
    if (EstamosNaCasa()) {
        fn(ObstaculoBauPedestal());
        fn(ObstaculoMesa());
        fn(ObstaculoArmario());
    }
}

float ComprimentoXZ(Vector3 v);
Vector3 FrenteYaw(float yawGraus);
void DesenharEspada(bool madeira);

float RaioDoInimigo(const Inimigo& inimigo) {
    return inimigo.chefe ? kRaioChefe : kRaioInimigo;
}

float VidaMaxDoInimigo(const Inimigo& inimigo) {
    if (inimigo.vidaMax > 0.0f) return inimigo.vidaMax;
    if (inimigo.boneco) return kVidaBonecoMax;
    return inimigo.chefe ? kVidaChefeMax : kVidaInimigoMax;
}

float MultiplicadorDungeon(int dungeonId) {
    switch (std::clamp(dungeonId, 0, 4)) {
        case 0: return 1.00f;
        case 1: return 1.35f;
        case 2: return 1.70f;
        case 3: return 2.15f;
        default: return 2.80f;
    }
}

bool PontoSobreObstaculo(float x, float z, const Obstaculo& o, float margem) {
    return x >= o.x - o.halfX - margem && x <= o.x + o.halfX + margem &&
           z >= o.z - o.halfZ - margem && z <= o.z + o.halfZ + margem;
}

float AlturaSoloEm(float x, float z, float raio, float yAtual) {
    float altura = 0.0f;
    ParaCadaObstaculoColisao([&](const Obstaculo& o) {
        if (!PontoSobreObstaculo(x, z, o, raio * 0.40f)) return;
        // So pousa no topo se o pe esta perto/acima dele (nao puxa pro telhado)
        if (yAtual + 0.55f < o.yTopo) return;
        if (yAtual + 0.15f < o.yBase) return;
        altura = std::max(altura, o.yTopo);
    });
    return altura;
}

void ResolverColisaoMapa(Vector3* pos, float raio) {
    const float pe = pos->y;
    const float cabeca = pos->y + 1.70f;
    ParaCadaObstaculoColisao([&](const Obstaculo& o) {
        // Passa por baixo de pecas elevadas (telhado, lintel, lanternas)
        if (cabeca <= o.yBase + 0.02f) return;
        // Anda em cima da peca
        if (pe + 0.10f >= o.yTopo) return;

        const float minX = o.x - o.halfX;
        const float maxX = o.x + o.halfX;
        const float minZ = o.z - o.halfZ;
        const float maxZ = o.z + o.halfZ;

        const float cx = std::clamp(pos->x, minX, maxX);
        const float cz = std::clamp(pos->z, minZ, maxZ);
        float dx = pos->x - cx;
        float dz = pos->z - cz;
        float d2 = dx * dx + dz * dz;

        if (d2 < 0.00001f) {
            const float esquerda = pos->x - minX;
            const float direita = maxX - pos->x;
            const float baixo = pos->z - minZ;
            const float cima = maxZ - pos->z;
            const float menor = std::min(std::min(esquerda, direita), std::min(baixo, cima));
            if (menor == esquerda) pos->x = minX - raio;
            else if (menor == direita) pos->x = maxX + raio;
            else if (menor == baixo) pos->z = minZ - raio;
            else pos->z = maxZ + raio;
            return;
        }

        if (d2 < raio * raio) {
            const float d = std::sqrt(d2);
            const float push = (raio - d) / d;
            pos->x += dx * push;
            pos->z += dz * push;
        }
    });
}

void SepararCirculosXZ(Vector3* a, float raioA, Vector3* b, float raioB) {
    float dx = a->x - b->x;
    float dz = a->z - b->z;
    float d2 = dx * dx + dz * dz;
    const float minDist = raioA + raioB;
    if (d2 < 0.00001f) {
        a->x += minDist * 0.5f;
        b->x -= minDist * 0.5f;
        return;
    }
    const float d = std::sqrt(d2);
    if (d >= minDist) return;
    const float push = (minDist - d) * 0.5f;
    dx /= d;
    dz /= d;
    a->x += dx * push;
    a->z += dz * push;
    b->x -= dx * push;
    b->z -= dz * push;
}

void ResolverColisaoEntidades(Personagem* jogador, Inimigo* inimigos, int quantidade) {
    // Inimigo x inimigo (algumas passadas pra estabilizar)
    for (int passa = 0; passa < 3; ++passa) {
        for (int i = 0; i < quantidade; ++i) {
            if (!inimigos[i].vivo) continue;
            for (int j = i + 1; j < quantidade; ++j) {
                if (!inimigos[j].vivo) continue;
                SepararCirculosXZ(&inimigos[i].posicao, RaioDoInimigo(inimigos[i]),
                                  &inimigos[j].posicao, RaioDoInimigo(inimigos[j]));
            }
        }
    }

    // Jogador x inimigos (nao empurra se o jogador esta elevado)
    for (int i = 0; i < quantidade; ++i) {
        if (!inimigos[i].vivo) continue;
        if (jogador->posicao.y > inimigos[i].posicao.y + kAlturaInalcancavel) continue;
        SepararCirculosXZ(&jogador->posicao, kRaioJogador,
                          &inimigos[i].posicao, RaioDoInimigo(inimigos[i]));
    }

    ResolverColisaoMapa(&jogador->posicao, kRaioJogador);
    for (int i = 0; i < quantidade; ++i) {
        if (!inimigos[i].vivo) continue;
        ResolverColisaoMapa(&inimigos[i].posicao, RaioDoInimigo(inimigos[i]));
        const float limite = gMapa.tamanho / 2.0f - 1.0f;
        inimigos[i].posicao.x = std::clamp(inimigos[i].posicao.x, -limite, limite);
        inimigos[i].posicao.z = std::clamp(inimigos[i].posicao.z, -limite, limite);
    }
}

void DesenharMapa(float tempo) {
    MapaDesenharBase(gMapa, tempo);
    MapaDesenharPecas(gMapa, tempo);
}

void DesenharMinimapa(const Personagem& jogador, const Inimigo* inimigos, int quantidade) {
    const float mx = 18.0f;
    const float my = 70.0f;
    const float size = 168.0f;
    const float arena = std::max(16.0f, gMapa.tamanho);
    const float half = arena * 0.5f;

    DrawRectangleRounded(Rectangle{mx - 6, my - 6, size + 12, size + 12}, 0.08f, 6, Color{10, 14, 18, 210});
    DrawRectangle(static_cast<int>(mx), static_cast<int>(my), static_cast<int>(size), static_cast<int>(size), Color{48, 78, 42, 255});
    // Praca
    const float praca = std::min(18.0f, arena * 0.45f);
    const float px = mx + ((-praca * 0.5f) / arena + 0.5f) * size;
    const float pz = my + ((-praca * 0.5f) / arena + 0.5f) * size;
    const float pw = (praca / arena) * size;
    DrawRectangle(static_cast<int>(px), static_cast<int>(pz), static_cast<int>(pw), static_cast<int>(pw), Color{150, 140, 118, 255});

    auto paraMapa = [&](float wx, float wz) -> Vector2 {
        return Vector2{
            mx + (wx / arena + 0.5f) * size,
            my + (wz / arena + 0.5f) * size,
        };
    };

    for (int i = 0; i < gMapa.quantidade; ++i) {
        if (!PecaTemColisao(gMapa.pecas[i])) continue;
        const ObstaculoColisao o = PecaParaColisao(gMapa.pecas[i]);
        const Vector2 c = paraMapa(o.x, o.z);
        const float w = (o.halfX * 2.0f / arena) * size;
        const float h = (o.halfZ * 2.0f / arena) * size;
        DrawRectangle(static_cast<int>(c.x - w * 0.5f), static_cast<int>(c.y - h * 0.5f),
                      std::max(2, static_cast<int>(w)), std::max(2, static_cast<int>(h)),
                      Color{70, 66, 58, 230});
    }

    for (int i = 0; i < quantidade; ++i) {
        if (!inimigos[i].vivo) continue;
        const Vector2 ie = paraMapa(inimigos[i].posicao.x, inimigos[i].posicao.z);
        DrawCircle(static_cast<int>(ie.x), static_cast<int>(ie.y), 4.0f, Color{220, 70, 70, 255});
    }

    const Vector2 jp = paraMapa(jogador.posicao.x, jogador.posicao.z);
    DrawCircle(static_cast<int>(jp.x), static_cast<int>(jp.y), 5.0f, Color{80, 190, 255, 255});
    const Vector3 frente = FrenteYaw(jogador.yaw);
    DrawLine(static_cast<int>(jp.x), static_cast<int>(jp.y),
             static_cast<int>(jp.x + frente.x * 10.0f),
             static_cast<int>(jp.y + frente.z * 10.0f),
             Color{200, 240, 255, 255});

    DrawRectangleLines(static_cast<int>(mx), static_cast<int>(my), static_cast<int>(size), static_cast<int>(size), Color{200, 210, 220, 120});
    DrawText("MAPA", static_cast<int>(mx), static_cast<int>(my - 18), 16, Color{210, 220, 230, 255});
    (void)half;
}

void DesenharEspada(bool madeira);
float ComprimentoXZ(Vector3 v);
Vector3 FrenteYaw(float yawGraus);

float ComprimentoXZ(Vector3 v) {
    return std::sqrt(v.x * v.x + v.z * v.z);
}

Vector3 NormalizarXZ(Vector3 v) {
    const float comprimento = ComprimentoXZ(v);
    if (comprimento <= 0.0001f) return Vector3{0.0f, 0.0f, 0.0f};
    return Vector3{v.x / comprimento, 0.0f, v.z / comprimento};
}

float LerpAngulo(float atual, float alvo, float t) {
    float delta = alvo - atual;
    while (delta > 180.0f) delta -= 360.0f;
    while (delta < -180.0f) delta += 360.0f;
    return atual + delta * std::clamp(t, 0.0f, 1.0f);
}

float LerpF(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

float SmoothStep(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

Vector3 FrenteYaw(float yawGraus) {
    const float yaw = yawGraus * DEG2RAD;
    return Vector3{-std::sin(yaw), 0.0f, -std::cos(yaw)};
}

Vector3 FrenteDaCamera(float yawGraus) {
    return FrenteYaw(yawGraus);
}

Vector3 DireitaDaCamera(float yawGraus) {
    const Vector3 frente = FrenteDaCamera(yawGraus);
    return Vector3{-frente.z, 0.0f, frente.x};
}

float DistanciaCameraLivreDeSolidos(Vector3 pivot, Vector3 dirAfastar, float distDesejada) {
    const float distMax = std::max(kDistanciaCameraMin, distDesejada);
    constexpr int kPassos = 28;
    float livre = distMax;
    for (int i = 1; i <= kPassos; ++i) {
        const float t = distMax * (static_cast<float>(i) / static_cast<float>(kPassos));
        const Vector3 p{
            pivot.x + dirAfastar.x * t,
            pivot.y + dirAfastar.y * t,
            pivot.z + dirAfastar.z * t,
        };
        bool bateu = false;
        ParaCadaObstaculoColisao([&](const Obstaculo& o) {
            if (bateu) return;
            // Ignora pisos baixinhos (nao devem empurrar a camera)
            if (o.yTopo - o.yBase <= 0.35f && o.yBase < 0.4f) return;
            if (p.y < o.yBase - kRaioCamera || p.y > o.yTopo + kRaioCamera) return;
            if (p.x >= o.x - o.halfX - kRaioCamera && p.x <= o.x + o.halfX + kRaioCamera &&
                p.z >= o.z - o.halfZ - kRaioCamera && p.z <= o.z + o.halfZ + kRaioCamera) {
                bateu = true;
            }
        });
        if (bateu) {
            livre = distMax * (static_cast<float>(i - 1) / static_cast<float>(kPassos));
            break;
        }
    }
    return std::clamp(livre - 0.12f, kDistanciaCameraMin * 0.75f, distMax);
}

void AplicarCamera(Camera3D* camera, Vector3 posicaoJogador, const CameraOrbit& orbita, float agachar) {
    const float yaw = orbita.yaw * DEG2RAD;
    const float pitch = orbita.pitch * DEG2RAD;
    const float cosPitch = std::cos(pitch);
    const float alturaPivot = kAlturaPivot + (kAlturaPivotAgachado - kAlturaPivot) * agachar;

    const Vector3 frente{
        -std::sin(yaw) * cosPitch,
        std::sin(pitch),
        -std::cos(yaw) * cosPitch,
    };
    const Vector3 direita = DireitaDaCamera(orbita.yaw);
    const Vector3 pivot{
        posicaoJogador.x + direita.x * kCameraLado,
        posicaoJogador.y + alturaPivot,
        posicaoJogador.z + direita.z * kCameraLado,
    };

    const Vector3 dirCam{-frente.x, -frente.y, -frente.z};
    const float dist = DistanciaCameraLivreDeSolidos(pivot, dirCam, orbita.distancia);

    camera->position = Vector3{
        pivot.x + dirCam.x * dist,
        pivot.y + dirCam.y * dist,
        pivot.z + dirCam.z * dist,
    };
    camera->target = Vector3{
        pivot.x + frente.x * 2.5f,
        pivot.y + frente.y * 2.5f,
        pivot.z + frente.z * 2.5f,
    };
}

void AplicarCameraEditor(Camera3D* camera, const CameraEditorLivre& cam) {
    const float yaw = cam.yaw * DEG2RAD;
    const float pitch = cam.pitch * DEG2RAD;
    const float cosPitch = std::cos(pitch);
    const Vector3 frente{
        -std::sin(yaw) * cosPitch,
        std::sin(pitch),
        -std::cos(yaw) * cosPitch,
    };
    camera->position = cam.posicao;
    camera->target = Vector3{
        cam.posicao.x + frente.x,
        cam.posicao.y + frente.y,
        cam.posicao.z + frente.z,
    };
    camera->fovy = cam.fovy;
    camera->up = Vector3{0.0f, 1.0f, 0.0f};
}

void AtualizarCameraEditorLivre(CameraEditorLivre* cam, float dt, bool bloquearLook) {
    constexpr float kSensEditor = 0.18f;
    constexpr float kVelBase = 14.0f;
    constexpr float kVelRapida = 28.0f;

    if (!bloquearLook && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        const Vector2 delta = GetMouseDelta();
        cam->yaw -= delta.x * kSensEditor;
        cam->pitch -= delta.y * kSensEditor;
        cam->pitch = std::clamp(cam->pitch, -89.0f, 89.0f);
    }

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        // Zoom / aproximar: move na direcao do olhar
        const float yaw = cam->yaw * DEG2RAD;
        const float pitch = cam->pitch * DEG2RAD;
        const float cosPitch = std::cos(pitch);
        const Vector3 frente{
            -std::sin(yaw) * cosPitch,
            std::sin(pitch),
            -std::cos(yaw) * cosPitch,
        };
        const float passo = wheel * 2.2f;
        cam->posicao.x += frente.x * passo;
        cam->posicao.y += frente.y * passo;
        cam->posicao.z += frente.z * passo;
        cam->fovy = std::clamp(cam->fovy - wheel * 2.0f, 25.0f, 90.0f);
    }

    Vector3 entrada{0.0f, 0.0f, 0.0f};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) entrada.z += 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) entrada.z -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) entrada.x += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) entrada.x -= 1.0f;
    if (IsKeyDown(KEY_SPACE)) entrada.y += 1.0f;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) entrada.y -= 1.0f;

    const float len = std::sqrt(entrada.x * entrada.x + entrada.y * entrada.y + entrada.z * entrada.z);
    if (len > 0.0001f) {
        entrada.x /= len;
        entrada.y /= len;
        entrada.z /= len;

        const float yaw = cam->yaw * DEG2RAD;
        const float pitch = cam->pitch * DEG2RAD;
        const float cosPitch = std::cos(pitch);
        const Vector3 frente{
            -std::sin(yaw) * cosPitch,
            std::sin(pitch),
            -std::cos(yaw) * cosPitch,
        };
        const Vector3 direita{
            std::cos(yaw),
            0.0f,
            -std::sin(yaw),
        };

        const bool rapido = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        const float vel = (rapido ? kVelRapida : kVelBase) * cam->velocidade;
        cam->posicao.x += (frente.x * entrada.z + direita.x * entrada.x) * vel * dt;
        cam->posicao.y += (frente.y * entrada.z + entrada.y) * vel * dt;
        cam->posicao.z += (frente.z * entrada.z + direita.z * entrada.x) * vel * dt;
    }
}

void SalvarSnapshotAntesEditor(const Personagem& jogador, const CameraOrbit& orbita, bool cursorLivre) {
    gSnapshotEditor.valido = true;
    gSnapshotEditor.posicao = jogador.posicao;
    gSnapshotEditor.yaw = jogador.yaw;
    gSnapshotEditor.velocidadeX = jogador.velocidadeX;
    gSnapshotEditor.velocidadeY = jogador.velocidadeY;
    gSnapshotEditor.velocidadeZ = jogador.velocidadeZ;
    gSnapshotEditor.knockbackX = jogador.knockbackX;
    gSnapshotEditor.knockbackZ = jogador.knockbackZ;
    gSnapshotEditor.agachar = jogador.agachar;
    gSnapshotEditor.ataque = jogador.ataque;
    gSnapshotEditor.bloqueio = jogador.bloqueio;
    gSnapshotEditor.intensidadeAndar = jogador.intensidadeAndar;
    gSnapshotEditor.intensidadeCorrida = jogador.intensidadeCorrida;
    gSnapshotEditor.moveFrente = jogador.moveFrente;
    gSnapshotEditor.moveLado = jogador.moveLado;
    gSnapshotEditor.fasePulo = jogador.fasePulo;
    gSnapshotEditor.noChao = jogador.noChao;
    gSnapshotEditor.miraTravada = jogador.miraTravada;
    gSnapshotEditor.orbitaYaw = orbita.yaw;
    gSnapshotEditor.orbitaPitch = orbita.pitch;
    gSnapshotEditor.cursorLivre = cursorLivre;
}

void RestaurarSnapshotAntesEditor(Personagem* jogador, CameraOrbit* orbita, bool* cursorLivre) {
    if (!gSnapshotEditor.valido || !jogador || !orbita) return;
    jogador->posicao = gSnapshotEditor.posicao;
    jogador->yaw = gSnapshotEditor.yaw;
    jogador->velocidadeX = gSnapshotEditor.velocidadeX;
    jogador->velocidadeY = gSnapshotEditor.velocidadeY;
    jogador->velocidadeZ = gSnapshotEditor.velocidadeZ;
    jogador->knockbackX = gSnapshotEditor.knockbackX;
    jogador->knockbackZ = gSnapshotEditor.knockbackZ;
    jogador->agachar = gSnapshotEditor.agachar;
    jogador->ataque = gSnapshotEditor.ataque;
    jogador->bloqueio = gSnapshotEditor.bloqueio;
    jogador->intensidadeAndar = gSnapshotEditor.intensidadeAndar;
    jogador->intensidadeCorrida = gSnapshotEditor.intensidadeCorrida;
    jogador->moveFrente = gSnapshotEditor.moveFrente;
    jogador->moveLado = gSnapshotEditor.moveLado;
    jogador->fasePulo = gSnapshotEditor.fasePulo;
    jogador->noChao = gSnapshotEditor.noChao;
    jogador->miraTravada = gSnapshotEditor.miraTravada;
    orbita->yaw = gSnapshotEditor.orbitaYaw;
    orbita->pitch = gSnapshotEditor.orbitaPitch;
    if (cursorLivre) *cursorLivre = gSnapshotEditor.cursorLivre;
    gSnapshotEditor.valido = false;
}

void IniciarCameraEditorDeOrbit(const Personagem& jogador, const CameraOrbit& orbita) {
    Camera3D tmp{};
    tmp.up = Vector3{0.0f, 1.0f, 0.0f};
    tmp.fovy = 60.0f;
    tmp.projection = CAMERA_PERSPECTIVE;
    AplicarCamera(&tmp, jogador.posicao, orbita, jogador.agachar);
    gCamEditor.posicao = tmp.position;
    gCamEditor.yaw = orbita.yaw;
    gCamEditor.pitch = orbita.pitch;
    gCamEditor.fovy = 60.0f;
    gCamEditor.velocidade = 1.0f;
}

Vector3 LerMovimento(float yawCamera) {
    Vector3 entrada{0.0f, 0.0f, 0.0f};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) entrada.z += 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) entrada.z -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) entrada.x += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) entrada.x -= 1.0f;

    if (ComprimentoXZ(entrada) <= 0.0001f) return Vector3{0.0f, 0.0f, 0.0f};

    const Vector3 frente = FrenteDaCamera(yawCamera);
    const Vector3 direita = DireitaDaCamera(yawCamera);
    Vector3 mundo{
        frente.x * entrada.z + direita.x * entrada.x,
        0.0f,
        frente.z * entrada.z + direita.z * entrada.x,
    };
    return NormalizarXZ(mundo);
}

void DesenharEspada(bool madeira) {
    // Empunhadura na origem local da mao; lamina sobe no +Y local do braco
    if (madeira) {
        DrawCube(Vector3{0.0f, 0.02f, 0.0f}, 0.06f, 0.14f, 0.06f, kCorMadeiraCabo);
        DrawCube(Vector3{0.0f, 0.10f, 0.0f}, 0.22f, 0.04f, 0.08f, kCorMadeiraGuarda);
        DrawCube(Vector3{0.0f, 0.55f, 0.0f}, 0.05f, 0.85f, 0.09f, kCorMadeiraLamina);
        DrawCube(Vector3{0.0f, 0.98f, 0.0f}, 0.03f, 0.08f, 0.05f, kCorMadeiraPonta);
    } else {
        DrawCube(Vector3{0.0f, 0.02f, 0.0f}, 0.06f, 0.14f, 0.06f, kCorCabo);
        DrawCube(Vector3{0.0f, 0.10f, 0.0f}, 0.22f, 0.04f, 0.08f, kCorGuarda);
        DrawCube(Vector3{0.0f, 0.55f, 0.0f}, 0.05f, 0.85f, 0.09f, kCorLamina);
        DrawCube(Vector3{0.0f, 0.98f, 0.0f}, 0.03f, 0.08f, 0.05f, Color{240, 245, 255, 255});
    }
}

void DesenharPerna(float ladoX, float passo, float amp, float a, float noAr, float faseQueda, float corrida,
                   float moveFrente, float moveLado) {
    const float fase = passo + (ladoX > 0.0f ? 3.14159265f : 0.0f);
    const float onda = std::sin(fase);
    const float t = SmoothStep(faseQueda);
    const float assimetria = (ladoX < 0.0f ? 1.0f : 0.75f);
    const float pesoFrente = std::clamp(std::fabs(moveFrente), 0.0f, 1.0f);
    const float pesoLado = std::clamp(std::fabs(moveLado), 0.0f, 1.0f);

    // Frente/tras: swing no eixo X (moveFrente negativo = andar de costas)
    const float swingCoxa = onda * (38.0f + 26.0f * corrida - 14.0f * a) * amp * moveFrente;
    const float coxaChao = swingCoxa * (1.0f - a * 0.7f) + a * 72.0f;
    const float coxaAr = LerpF(28.0f * assimetria, 10.0f, t);
    const float angCoxa = LerpF(coxaChao, coxaAr, noAr);

    // Strafe: perna abre/fecha no eixo Z
    const float swingLateral = onda * (34.0f + 16.0f * corrida) * amp * moveLado * (1.0f - a * 0.5f);

    const float flexaoChao =
        std::max(0.0f, -std::cos(fase)) * (42.0f + 28.0f * corrida) * amp *
        (0.35f + 0.65f * pesoFrente + 0.55f * pesoLado) * (1.0f - a);
    const float flexaoAr = LerpF(58.0f * assimetria, 34.0f, t);
    const float angJoelho = -(LerpF(flexaoChao, flexaoAr, noAr) + a * 118.0f);

    const float alturaQuadril = 0.70f - a * 0.30f - noAr * 0.04f;
    const float avancoQuadril = -0.04f * a - 0.05f * corrida * pesoFrente
        + noAr * LerpF(0.06f, 0.0f, t)
        + moveLado * 0.03f * amp;

    rlPushMatrix();
    rlTranslatef(ladoX + moveLado * 0.04f * amp, alturaQuadril, avancoQuadril);
    rlRotatef(swingLateral, 0.0f, 0.0f, 1.0f);
    rlRotatef(angCoxa, 1.0f, 0.0f, 0.0f);
    rlRotatef(moveLado * 8.0f * amp, 0.0f, 1.0f, 0.0f);

    DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.15f, 0.30f, 0.16f, kCorCalca);
    DrawSphere(Vector3{0.0f, -0.30f, 0.0f}, 0.08f, kCorCalca);

    rlPushMatrix();
    rlTranslatef(0.0f, -0.30f, 0.0f);
    rlRotatef(angJoelho, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.13f, 0.28f, 0.14f, kCorCalca);
    DrawCube(Vector3{0.0f, -0.36f, -0.02f}, 0.16f, 0.10f, 0.26f, kCorSapato);
    rlPopMatrix();

    rlPopMatrix();
}

void DesenharBraco(float ladoX, float swing, float a, float queda, float noAr, float faseQueda,
                   float corrida, float ataque, float bloqueio, bool comEspada, bool madeira) {
    const float lado = (ladoX < 0.0f) ? -1.0f : 1.0f;
    const float t = SmoothStep(faseQueda);

    // Curva limpa do golpe: carrega -> corta -> volta
    const float carrega = SmoothStep(std::clamp(ataque / 0.28f, 0.0f, 1.0f));
    const float corta = SmoothStep(std::clamp((ataque - 0.22f) / 0.34f, 0.0f, 1.0f));
    const float volta = SmoothStep(std::clamp((ataque - 0.62f) / 0.38f, 0.0f, 1.0f));
    const float noGolpe = (ataque > 0.001f) ? 1.0f : 0.0f;

    float pitchAr = LerpF(-48.0f, -16.0f, t);
    float rollAr = lado * LerpF(14.0f, 40.0f, t);
    float flexaoChao = corrida * 40.0f + std::max(0.0f, swing) * 0.15f;
    float flexaoAr = LerpF(28.0f, 12.0f, t);

    float pitchExtra = 0.0f;
    float yawExtra = 0.0f;
    float rollExtra = 0.0f;
    float flexaoExtra = 0.0f;
    float rotEspada = 12.0f;
    float pitchEspada = -90.0f;

    if (comEspada) {
        // Guardia baixa / pronta
        pitchExtra = -28.0f;
        yawExtra = lado * 12.0f;
        rollExtra = lado * 6.0f;
        flexaoExtra = 28.0f;

        // Bloqueio: lamina vertical a frente do peito
        if (bloqueio > 0.001f) {
            pitchExtra = LerpF(pitchExtra, -52.0f, bloqueio);
            yawExtra = LerpF(yawExtra, lado * -18.0f, bloqueio);
            rollExtra = LerpF(rollExtra, lado * 8.0f, bloqueio);
            flexaoExtra = LerpF(flexaoExtra, 78.0f, bloqueio);
            rotEspada = LerpF(rotEspada, 0.0f, bloqueio);
            pitchEspada = LerpF(pitchEspada, -78.0f, bloqueio);
        }

        // Corte diagonal controlado (sem exagero)
        if (noGolpe > 0.0f) {
            const float pitchKey = LerpF(
                LerpF(-10.0f, 38.0f, carrega),
                LerpF(-78.0f, -42.0f, volta),
                corta);
            const float yawKey = LerpF(
                LerpF(lado * 18.0f, lado * 42.0f, carrega),
                LerpF(lado * -48.0f, lado * -8.0f, volta),
                corta);
            const float rollKey = LerpF(
                LerpF(lado * -8.0f, lado * -22.0f, carrega),
                LerpF(lado * 28.0f, lado * 10.0f, volta),
                corta);
            const float flexKey = LerpF(
                LerpF(22.0f, 16.0f, carrega),
                LerpF(52.0f, 30.0f, volta),
                corta);
            const float rotKey = LerpF(
                LerpF(8.0f, -12.0f, carrega),
                LerpF(28.0f, 12.0f, volta),
                corta);

            const float peso = 1.0f - volta * 0.15f;
            pitchExtra = LerpF(pitchExtra, pitchKey, peso);
            yawExtra = LerpF(yawExtra, yawKey, peso);
            rollExtra = LerpF(rollExtra, rollKey, peso);
            flexaoExtra = flexKey;
            rotEspada = rotKey;
            pitchEspada = -90.0f;
        }
    } else {
        // Braco livre: apoio no bloqueio, leve contrapeso no golpe
        if (bloqueio > 0.001f) {
            pitchExtra = LerpF(0.0f, -58.0f, bloqueio);
            yawExtra = LerpF(0.0f, lado * -28.0f, bloqueio);
            rollExtra = LerpF(0.0f, lado * 18.0f, bloqueio);
            flexaoExtra = LerpF(0.0f, 70.0f, bloqueio);
        }
        if (noGolpe > 0.0f) {
            const float pitchKey = LerpF(
                LerpF(-8.0f, -36.0f, carrega),
                LerpF(12.0f, -10.0f, volta),
                corta);
            const float yawKey = LerpF(
                LerpF(lado * 10.0f, lado * 32.0f, carrega),
                LerpF(lado * -8.0f, 0.0f, volta),
                corta);
            pitchExtra = LerpF(pitchExtra, pitchKey, 0.85f);
            yawExtra = LerpF(yawExtra, yawKey, 0.85f);
            rollExtra = LerpF(rollExtra, lado * 12.0f * carrega * (1.0f - volta), 0.85f);
            flexaoExtra = LerpF(flexaoExtra, LerpF(18.0f, 40.0f, corta) * (1.0f - volta * 0.5f), 0.85f);
        }
    }

    const float flexaoCotovelo = LerpF(flexaoChao, flexaoAr, noAr) + flexaoExtra;
    const float maskAndar = (1.0f - noAr) * (1.0f - bloqueio) * (1.0f - noGolpe);

    rlPushMatrix();
    rlTranslatef(ladoX, 1.42f - queda + noAr * LerpF(0.05f, 0.0f, t), -0.04f * a - 0.04f * corrida);
    rlRotatef(yawExtra, 0.0f, 1.0f, 0.0f);
    rlRotatef(-swing * maskAndar + 8.0f + a * 12.0f + pitchAr * noAr + pitchExtra, 1.0f, 0.0f, 0.0f);
    rlRotatef(rollAr * noAr + rollExtra, 0.0f, 0.0f, 1.0f);
    DrawCube(Vector3{0.0f, -0.14f, 0.0f}, 0.13f, 0.28f, 0.13f, kCorCamisa);
    DrawSphere(Vector3{0.0f, -0.28f, 0.0f}, 0.07f, kCorPele);

    rlPushMatrix();
    rlTranslatef(0.0f, -0.28f, 0.0f);
    rlRotatef(flexaoCotovelo, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.12f, 0.0f}, 0.11f, 0.22f, 0.11f, kCorPele);
    DrawSphere(Vector3{0.0f, -0.24f, 0.0f}, 0.07f, kCorPele);

    if (comEspada) {
        rlPushMatrix();
        rlTranslatef(0.04f, -0.22f, 0.02f);
        rlRotatef(pitchEspada, 1.0f, 0.0f, 0.0f);
        rlRotatef(rotEspada, 0.0f, 0.0f, 1.0f);
        DesenharEspada(madeira);
        rlPopMatrix();
    }
    rlPopMatrix();

    rlPopMatrix();
}

void DesenharPersonagem(const Personagem& personagem, float tempo) {
    // Pose de morte: caido de lado no chao
    if (personagem.vida <= 0.0f) {
        rlPushMatrix();
        rlTranslatef(personagem.posicao.x, personagem.posicao.y + 0.28f, personagem.posicao.z);
        rlRotatef(personagem.yaw, 0.0f, 1.0f, 0.0f);
        rlRotatef(92.0f, 0.0f, 0.0f, 1.0f);
        rlRotatef(-12.0f, 1.0f, 0.0f, 0.0f);

        DrawCylinder(Vector3{0.0f, -0.02f, 0.0f}, 0.55f, 0.48f, 0.006f, 20, Color{0, 0, 0, 80});

        // Pernas frouxas
        rlPushMatrix();
        rlTranslatef(-0.13f, 0.55f, 0.05f);
        rlRotatef(28.0f, 1.0f, 0.0f, 0.0f);
        DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.15f, 0.30f, 0.16f, kCorCalca);
        rlTranslatef(0.0f, -0.30f, 0.0f);
        rlRotatef(-35.0f, 1.0f, 0.0f, 0.0f);
        DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.13f, 0.28f, 0.14f, kCorCalca);
        DrawCube(Vector3{0.0f, -0.36f, -0.02f}, 0.16f, 0.10f, 0.26f, kCorSapato);
        rlPopMatrix();

        rlPushMatrix();
        rlTranslatef(0.13f, 0.55f, -0.02f);
        rlRotatef(8.0f, 1.0f, 0.0f, 0.0f);
        DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.15f, 0.30f, 0.16f, kCorCalca);
        rlTranslatef(0.0f, -0.30f, 0.0f);
        rlRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
        DrawCube(Vector3{0.0f, -0.15f, 0.0f}, 0.13f, 0.28f, 0.14f, kCorCalca);
        DrawCube(Vector3{0.0f, -0.36f, -0.02f}, 0.16f, 0.10f, 0.26f, kCorSapato);
        rlPopMatrix();

        DrawCube(Vector3{0.0f, 0.78f, 0.0f}, 0.42f, 0.16f, 0.26f, kCorCalca);
        DrawCube(Vector3{0.0f, 1.18f, 0.0f}, 0.46f, 0.52f, 0.28f, kCorCamisa);
        DrawCube(Vector3{0.0f, 0.96f, 0.0f}, 0.44f, 0.14f, 0.26f, kCorCamisaEscura);

        // Bracos caidos
        rlPushMatrix();
        rlTranslatef(-0.32f, 1.20f, 0.08f);
        rlRotatef(-55.0f, 1.0f, 0.0f, 0.0f);
        rlRotatef(25.0f, 0.0f, 0.0f, 1.0f);
        DrawCube(Vector3{0.0f, -0.20f, 0.0f}, 0.12f, 0.40f, 0.12f, kCorCamisa);
        rlPopMatrix();
        rlPushMatrix();
        rlTranslatef(0.32f, 1.18f, -0.06f);
        rlRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
        rlRotatef(-40.0f, 0.0f, 0.0f, 1.0f);
        DrawCube(Vector3{0.0f, -0.20f, 0.0f}, 0.12f, 0.40f, 0.12f, kCorCamisa);
        if (personagem.temEspada) {
            rlTranslatef(0.0f, -0.42f, 0.0f);
            rlRotatef(70.0f, 1.0f, 0.0f, 0.0f);
            DesenharEspada(personagem.espadaMadeira);
        }
        rlPopMatrix();

        rlPushMatrix();
        rlTranslatef(0.02f, 1.52f, 0.04f);
        rlRotatef(18.0f, 1.0f, 0.0f, 0.0f);
        DrawSphere(Vector3{0.0f, 0.16f, 0.0f}, 0.20f, kCorPele);
        DrawSphere(Vector3{0.0f, 0.24f, -0.02f}, 0.18f, kCorCabelo);
        DrawSphere(Vector3{-0.07f, 0.18f, -0.15f}, 0.045f, kCorOlho);
        DrawSphere(Vector3{0.07f, 0.18f, -0.15f}, 0.045f, kCorOlho);
        DrawSphere(Vector3{-0.07f, 0.18f, -0.185f}, 0.022f, kCorPupila);
        DrawSphere(Vector3{0.07f, 0.18f, -0.185f}, 0.022f, kCorPupila);
        rlPopMatrix();

        rlPopMatrix();
        (void)tempo;
        return;
    }

    const float idle = std::sin(tempo * 2.1f);
    const float respiracao = 1.0f + idle * 0.015f;
    const float a = personagem.agachar;
    const float noAr = personagem.noChao ? 0.0f : 1.0f;
    const float faseQueda = personagem.fasePulo;
    const float t = SmoothStep(faseQueda);
    const float amp = personagem.noChao ? personagem.intensidadeAndar : 0.0f;
    const float corrida = personagem.noChao ? personagem.intensidadeCorrida : 0.0f;
    const float moveF = personagem.moveFrente;
    const float moveL = personagem.moveLado;
    const float mira = personagem.miraTravada ? 1.0f : 0.0f;
    const float balancoIdle = idle * 2.0f * (1.0f - amp) * (1.0f - noAr);
    const float passo = personagem.cicloPasso;
    const float bob = personagem.noChao
        ? (std::fabs(std::sin(passo)) * (0.06f + 0.08f * corrida - 0.03f * a) * amp + idle * 0.015f * (1.0f - amp))
        : 0.0f;

    // Bracos: menos balanco de caminhada na mira; leve sway no strafe
    const float swingAndar = std::sin(passo) * (34.0f + 40.0f * corrida - 12.0f * a) * amp * (1.0f - mira * 0.75f);
    const float swingStrafe = std::sin(passo) * 18.0f * amp * moveL * mira;
    const float swingBraco = swingAndar + swingStrafe;

    const float carregaBody = SmoothStep(std::clamp(personagem.ataque / 0.28f, 0.0f, 1.0f));
    const float cortaBody = SmoothStep(std::clamp((personagem.ataque - 0.22f) / 0.34f, 0.0f, 1.0f));
    const float voltaBody = SmoothStep(std::clamp((personagem.ataque - 0.62f) / 0.38f, 0.0f, 1.0f));
    const float noGolpe = (personagem.ataque > 0.001f) ? 1.0f : 0.0f;
    const float b = personagem.bloqueio;

    const float inclinacaoAndar = -moveF * (5.0f + 12.0f * corrida) * amp;
    const float inclinacao = inclinacaoAndar - a * 8.0f
        + noAr * LerpF(-10.0f, 6.0f, t)
        + b * 6.0f
        + mira * 3.0f
        + (carregaBody * 4.0f - cortaBody * 10.0f + voltaBody * 3.0f) * noGolpe;
    const float torsao = (LerpF(10.0f * carregaBody, -18.0f * cortaBody, cortaBody) * (1.0f - voltaBody * 0.7f)) * noGolpe
        + b * -6.0f
        + moveL * 6.0f * amp * mira;
    const float queda = a * 0.36f + b * 0.04f;
    const float giroCabeca = std::sin(tempo * 1.3f) * 3.0f * (1.0f - amp * 0.5f) * (1.0f - noAr) * (1.0f - corrida) * (1.0f - mira)
        - cortaBody * 6.0f * noGolpe
        + b * 4.0f;
    const float inclinaLado = -moveL * (10.0f + 6.0f * corrida) * amp;

    rlPushMatrix();
    rlTranslatef(personagem.posicao.x, personagem.posicao.y + bob, personagem.posicao.z);
    rlRotatef(personagem.yaw, 0.0f, 1.0f, 0.0f);
    rlRotatef(torsao, 0.0f, 1.0f, 0.0f);
    rlRotatef(inclinacao, 1.0f, 0.0f, 0.0f);
    rlRotatef(balancoIdle - cortaBody * 4.0f * noGolpe + inclinaLado, 0.0f, 0.0f, 1.0f);
    rlRotatef(std::sin(passo) * 4.0f * corrida * amp * (1.0f - b) * (1.0f - noGolpe) * (1.0f - mira), 0.0f, 0.0f, 1.0f);

    if (personagem.noChao) {
        DrawCylinder(Vector3{0.0f, 0.002f, 0.0f}, 0.48f, 0.42f, 0.006f, 20, Color{0, 0, 0, 70});
    }

    DesenharPerna(-0.13f, passo, amp, a, noAr, faseQueda, corrida, moveF, moveL);
    DesenharPerna(0.13f, passo, amp, a, noAr, faseQueda, corrida, moveF, moveL);

    rlPushMatrix();
    rlTranslatef(0.0f, -queda, -0.03f * a - 0.05f * corrida);
    DrawCube(Vector3{0.0f, 0.84f, 0.0f}, 0.42f, 0.16f, 0.26f, kCorCalca);
    rlPopMatrix();

    rlPushMatrix();
    rlTranslatef(0.0f, -queda, -0.03f * a - 0.05f * corrida);
    rlScalef(1.0f, respiracao, 1.0f);
    DrawCube(Vector3{0.0f, 1.24f, 0.0f}, 0.46f, 0.52f, 0.28f, kCorCamisa);
    DrawCube(Vector3{0.0f, 1.02f, 0.0f}, 0.44f, 0.14f, 0.26f, kCorCamisaEscura);
    rlPopMatrix();

    // Esquerdo apoia no bloqueio / contrapeso leve; direito com espada se tiver
    DesenharBraco(-0.30f, swingBraco, a, queda, noAr, faseQueda, corrida,
                  personagem.ataque, personagem.bloqueio, false, false);
    DesenharBraco(0.30f, -swingBraco, a, queda, noAr, faseQueda, corrida,
                  personagem.ataque, personagem.bloqueio, personagem.temEspada, personagem.espadaMadeira);

    rlPushMatrix();
    rlTranslatef(0.0f, 1.58f - queda, -0.04f * a - 0.06f * corrida);
    rlRotatef(giroCabeca, 0.0f, 1.0f, 0.0f);
    rlRotatef(noAr * LerpF(-8.0f, 10.0f, t) - mira * 4.0f, 1.0f, 0.0f, 0.0f);
    DrawSphere(Vector3{0.0f, 0.16f, 0.0f}, 0.20f, kCorPele);
    DrawSphere(Vector3{0.0f, 0.24f, -0.02f}, 0.18f, kCorCabelo);
    DrawSphere(Vector3{-0.07f, 0.18f, -0.15f}, 0.045f, kCorOlho);
    DrawSphere(Vector3{0.07f, 0.18f, -0.15f}, 0.045f, kCorOlho);
    DrawSphere(Vector3{-0.07f, 0.18f, -0.185f}, 0.022f, kCorPupila);
    DrawSphere(Vector3{0.07f, 0.18f, -0.185f}, 0.022f, kCorPupila);
    DrawSphere(Vector3{0.0f, 0.10f, -0.18f}, 0.035f, Color{214, 150, 132, 255});
    rlPopMatrix();

    rlPopMatrix();
}

void DesenharInimigo(const Inimigo& inimigo, float tempo) {
    if (!inimigo.vivo && inimigo.hitFlash <= 0.0f) return;

    const float flash = inimigo.hitFlash;
    const bool chefe = inimigo.chefe;
    const bool boneco = inimigo.boneco;
    Color corpo = flash > 0.0f ? Color{255, 220, 220, 255} : (chefe ? kCorChefe : kCorInimigo);
    Color detalhe = flash > 0.0f ? RAYWHITE : (chefe ? kCorChefeEscuro : kCorInimigoEscuro);
    if (flash <= 0.0f) {
        switch (inimigo.visual) {
            case TipoVisualInimigo::Demonio:
                corpo = Color{170, 35, 35, 255}; detalhe = Color{90, 15, 15, 255}; break;
            case TipoVisualInimigo::Voador:
                corpo = Color{190, 55, 40, 255}; detalhe = Color{110, 25, 20, 255}; break;
            case TipoVisualInimigo::Caverna:
                corpo = Color{95, 85, 75, 255}; detalhe = Color{55, 48, 42, 255}; break;
            case TipoVisualInimigo::Mutante:
                corpo = Color{70, 130, 55, 255}; detalhe = Color{40, 80, 30, 255}; break;
            case TipoVisualInimigo::Elite:
                corpo = Color{230, 230, 240, 255}; detalhe = Color{170, 175, 200, 255}; break;
            default:
                if (boneco) { corpo = Color{190, 150, 95, 255}; detalhe = Color{140, 100, 60, 255}; }
                break;
        }
    }

    const float carrega = SmoothStep(std::clamp(inimigo.ataque / 0.30f, 0.0f, 1.0f));
    const float golpe = SmoothStep(std::clamp((inimigo.ataque - 0.28f) / 0.30f, 0.0f, 1.0f));
    const float volta = SmoothStep(std::clamp((inimigo.ataque - 0.65f) / 0.35f, 0.0f, 1.0f));
    const float noGolpe = (inimigo.ataque > 0.001f) ? 1.0f : 0.0f;
    const float slamCarrega = SmoothStep(std::clamp(inimigo.porradao / 0.40f, 0.0f, 1.0f));
    const float slamImpacto = SmoothStep(std::clamp((inimigo.porradao - 0.40f) / 0.18f, 0.0f, 1.0f));
    const float slamVolta = SmoothStep(std::clamp((inimigo.porradao - 0.70f) / 0.30f, 0.0f, 1.0f));
    const float noSlam = (inimigo.porradao > 0.001f) ? 1.0f : 0.0f;
    const float r = inimigo.recuo;
    const float escala = chefe ? 1.85f : 1.0f;

    const float bob = std::fabs(std::sin(inimigo.cicloPasso)) * 0.05f * (1.0f - r)
        * (1.0f - noGolpe * 0.5f) * (1.0f - noSlam);
    const float swing = boneco ? 0.0f
        : (std::sin(inimigo.cicloPasso) * 28.0f * (1.0f - noGolpe) * (1.0f - noSlam) * (1.0f - r));
    const float inclinacao = -golpe * 12.0f * noGolpe + r * 22.0f
        + slamCarrega * -28.0f * noSlam + slamImpacto * 55.0f * noSlam - slamVolta * 25.0f * noSlam;
    const float torsao = LerpF(8.0f * carrega, -14.0f * golpe, golpe) * noGolpe - r * 10.0f;

    // Onda de choque do porradao
    if (chefe && inimigo.porradao > 0.42f && inimigo.porradao < 0.85f) {
        const float t = (inimigo.porradao - 0.42f) / 0.43f;
        const float raioOnda = LerpF(0.6f, kAlcancePorradao + 0.4f, t) * escala * 0.55f;
        const unsigned char alpha = static_cast<unsigned char>(180 * (1.0f - t));
        DrawCylinder(
            Vector3{inimigo.posicao.x, inimigo.posicao.y + 0.04f, inimigo.posicao.z},
            raioOnda, raioOnda * 0.92f, 0.08f, 24,
            Color{200, 80, 60, alpha});
    }

    if (inimigo.queimaTempo > 0.0f && inimigo.vivo) {
        const float f = 0.6f + 0.4f * std::sin(tempo * 14.0f);
        DrawSphere(Vector3{inimigo.posicao.x, inimigo.posicao.y + 1.2f * escala, inimigo.posicao.z},
                   0.55f * f, Color{255, 90, 20, 70});
    }

    rlPushMatrix();
    rlTranslatef(inimigo.posicao.x, inimigo.posicao.y + bob * escala, inimigo.posicao.z);
    rlRotatef(inimigo.yaw, 0.0f, 1.0f, 0.0f);
    rlRotatef(torsao, 0.0f, 1.0f, 0.0f);
    rlRotatef(inclinacao, 1.0f, 0.0f, 0.0f);
    rlScalef(escala, escala, escala);

    DrawCylinder(Vector3{0.0f, 0.002f, 0.0f}, 0.45f, 0.40f, 0.006f, 16, Color{0, 0, 0, 70});

    // Pernas
    rlPushMatrix();
    rlTranslatef(-0.12f, 0.70f, 0.0f);
    rlRotatef(swing + r * 18.0f, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.28f, 0.0f}, 0.14f, 0.50f, 0.16f, detalhe);
    rlPopMatrix();
    rlPushMatrix();
    rlTranslatef(0.12f, 0.70f, 0.0f);
    rlRotatef(-swing + r * 10.0f, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.28f, 0.0f}, 0.14f, 0.50f, 0.16f, detalhe);
    rlPopMatrix();

    DrawCube(Vector3{0.0f, 1.10f, 0.0f}, 0.48f, 0.55f, 0.30f, corpo);
    DrawCube(Vector3{0.0f, 0.82f, 0.0f}, 0.50f, 0.12f, 0.32f, detalhe);
    if (chefe) {
        DrawCube(Vector3{0.0f, 1.28f, 0.0f}, 0.56f, 0.18f, 0.36f, detalhe);
    }
    if (boneco) {
        DrawCube(Vector3{0.0f, 1.20f, -0.18f}, 0.35f, 0.08f, 0.08f, Color{120, 80, 45, 255});
        DrawCube(Vector3{0.0f, 0.55f, 0.0f}, 0.12f, 1.0f, 0.12f, Color{120, 85, 50, 255});
    }
    if (inimigo.voador) {
        const float bater = std::sin(inimigo.cicloPasso * 2.0f) * 18.0f;
        rlPushMatrix();
        rlTranslatef(-0.38f, 1.25f, 0.0f);
        rlRotatef(bater, 0.0f, 0.0f, 1.0f);
        DrawCube(Vector3{-0.28f, 0.0f, 0.0f}, 0.55f, 0.06f, 0.28f, detalhe);
        rlPopMatrix();
        rlPushMatrix();
        rlTranslatef(0.38f, 1.25f, 0.0f);
        rlRotatef(-bater, 0.0f, 0.0f, 1.0f);
        DrawCube(Vector3{0.28f, 0.0f, 0.0f}, 0.55f, 0.06f, 0.28f, detalhe);
        rlPopMatrix();
    }

    // Braco esquerdo
    const float pitchEsq = -swing * 0.8f + carrega * -35.0f * noGolpe + r * -25.0f
        + slamCarrega * -80.0f * noSlam + slamImpacto * 40.0f * noSlam;
    rlPushMatrix();
    rlTranslatef(-0.30f, 1.30f, 0.0f);
    rlRotatef(pitchEsq, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.22f, 0.0f}, 0.12f, 0.42f, 0.12f, corpo);
    rlPopMatrix();

    // Braco direito (ataque / slam)
    const float pitchDir = LerpF(
        LerpF(swing * 0.8f, 45.0f, carrega),
        LerpF(-95.0f, -20.0f, volta),
        golpe) * noGolpe
        + swing * 0.8f * (1.0f - noGolpe) * (1.0f - noSlam)
        + r * 30.0f
        + slamCarrega * -110.0f * noSlam
        + slamImpacto * 70.0f * noSlam;
    rlPushMatrix();
    rlTranslatef(0.30f, 1.30f, 0.0f);
    rlRotatef(pitchDir, 1.0f, 0.0f, 0.0f);
    rlRotatef(golpe * -25.0f * noGolpe, 0.0f, 0.0f, 1.0f);
    DrawCube(Vector3{0.0f, -0.22f, 0.0f}, 0.12f, 0.42f, 0.12f, corpo);
    rlPopMatrix();

    DrawSphere(Vector3{0.0f, 1.55f, 0.0f}, 0.20f, corpo);
    DrawSphere(Vector3{-0.07f, 1.58f, -0.14f}, 0.04f, RAYWHITE);
    DrawSphere(Vector3{0.07f, 1.58f, -0.14f}, 0.04f, RAYWHITE);
    DrawSphere(Vector3{-0.07f, 1.58f, -0.17f}, 0.02f, BLACK);
    DrawSphere(Vector3{0.07f, 1.58f, -0.17f}, 0.02f, BLACK);

    rlPopMatrix();
    (void)tempo;
}

void DesenharBarra(float x, float y, float w, float h, float pct, Color preenchimento, Color fundo) {
    pct = std::clamp(pct, 0.0f, 1.0f);
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), fundo);
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w * pct), static_cast<int>(h), preenchimento);
    DrawRectangleLines(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), Color{255, 255, 255, 90});
}

void DesenharBarraVidaInimigo2D(const Inimigo& inimigo, const Camera3D& camera) {
    if (!inimigo.vivo) return;

    const float alturaBarra = inimigo.chefe ? 3.55f : 2.05f;
    const Vector3 topo{inimigo.posicao.x, inimigo.posicao.y + alturaBarra, inimigo.posicao.z};
    const Vector2 tela = GetWorldToScreen(topo, camera);
    if (tela.y < -40.0f || tela.y > GetScreenHeight() + 40.0f) return;
    if (tela.x < -80.0f || tela.x > GetScreenWidth() + 80.0f) return;

    const float w = inimigo.chefe ? 120.0f : (inimigo.boneco ? 110.0f : 78.0f);
    const float h = inimigo.chefe ? 14.0f : 10.0f;
    const float x = tela.x - w * 0.5f;
    const float y = tela.y - 8.0f;
    const float pct = inimigo.vida / VidaMaxDoInimigo(inimigo);
    const Color fill = inimigo.chefe ? Color{255, 90, 40, 255}
        : (inimigo.boneco ? Color{220, 170, 70, 255} : Color{220, 70, 70, 255});
    DesenharBarra(x, y, w, h, pct, fill, Color{20, 20, 28, 220});
    if (inimigo.chefe) {
        DrawText("CHEFE", static_cast<int>(x), static_cast<int>(y - 16), 14, Color{255, 180, 90, 255});
    } else if (inimigo.boneco) {
        DrawText("BONECO", static_cast<int>(x), static_cast<int>(y - 16), 14, Color{230, 200, 120, 255});
        DrawText(TextFormat("%.0f", inimigo.vida), static_cast<int>(x + w + 4), static_cast<int>(y - 2), 12,
                 Color{230, 210, 160, 230});
    }
    if (inimigo.queimaTempo > 0.0f) {
        DrawText("QUEIMANDO", static_cast<int>(x), static_cast<int>(y - 30), 12, Color{255, 140, 40, 255});
    }
}

void DesenharHudJogador(const Personagem& jogador) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    const float painelW = 268.0f;
    const float painelH = 124.0f;
    const float x = static_cast<float>(sw) - painelW - 18.0f;
    const float y = static_cast<float>(sh) - painelH - 18.0f;

    DrawRectangleRounded(Rectangle{x, y, painelW, painelH}, 0.12f, 8, Color{12, 14, 22, 220});
    DrawRectangleRoundedLines(Rectangle{x, y, painelW, painelH}, 0.12f, 8, Color{70, 90, 130, 180});

    DrawText(TextFormat("Nv. %d/%d", jogador.nivel, kNivelMax), static_cast<int>(x + 14), static_cast<int>(y + 10), 18, Color{200, 210, 230, 255});
    if (jogador.pontosDisponiveis > 0) {
        DrawText(TextFormat("Pts %d (C)", jogador.pontosDisponiveis), static_cast<int>(x + 130), static_cast<int>(y + 10), 16,
                 Color{255, 210, 100, 255});
    }

    const float vidaPct = jogador.vida / kVidaJogadorMax;
    DrawText("HP", static_cast<int>(x + 14), static_cast<int>(y + 36), 16, Color{255, 140, 140, 255});
    DesenharBarra(x + 46, y + 38, 170, 14, vidaPct, Color{210, 55, 65, 255}, Color{35, 28, 36, 255});
    DrawText(TextFormat("%.0f/%.0f", jogador.vida, kVidaJogadorMax),
             static_cast<int>(x + 222), static_cast<int>(y + 36), 14, Color{230, 200, 200, 255});

    const float manaPct = jogador.mana / kManaMax;
    DrawText("MP", static_cast<int>(x + 14), static_cast<int>(y + 60), 16, Color{140, 180, 255, 255});
    DesenharBarra(x + 46, y + 62, 170, 14, manaPct, Color{70, 120, 230, 255}, Color{24, 30, 48, 255});
    DrawText(TextFormat("%.0f/%.0f", jogador.mana, kManaMax),
             static_cast<int>(x + 222), static_cast<int>(y + 60), 14, Color{190, 210, 250, 255});

    const float xpMax = kXpBaseNivel * static_cast<float>(jogador.nivel);
    const float xpPct = (xpMax > 0.0f) ? (jogador.xp / xpMax) : 0.0f;
    DrawText("XP", static_cast<int>(x + 14), static_cast<int>(y + 84), 16, Color{140, 210, 255, 255});
    DesenharBarra(x + 46, y + 86, 170, 14, xpPct, Color{70, 160, 230, 255}, Color{28, 34, 48, 255});
    DrawText(TextFormat("%.0f/%.0f", jogador.xp, xpMax),
             static_cast<int>(x + 222), static_cast<int>(y + 84), 14, Color{190, 220, 245, 255});
}

void DesenharSlotEquipamento(float x, float y, float size, const char* titulo, bool destaque) {
    const Color fundo = destaque ? Color{28, 34, 52, 230} : Color{16, 18, 26, 220};
    const Color borda = destaque ? Color{210, 180, 90, 220} : Color{90, 100, 120, 160};
    DrawRectangleRounded(Rectangle{x, y, size, size}, 0.14f, 6, fundo);
    DrawRectangleRoundedLines(Rectangle{x, y, size, size}, 0.14f, 6, borda);
    const int tw = MeasureText(titulo, 14);
    DrawText(titulo, static_cast<int>(x + (size - tw) * 0.5f), static_cast<int>(y + size - 20), 14,
             destaque ? Color{240, 220, 160, 255} : Color{160, 170, 185, 200});
}

void DesenharIconeEspadaHud(float cx, float cy, bool madeira) {
    const Color cabo = madeira ? kCorMadeiraCabo : Color{90, 50, 35, 255};
    const Color guarda = madeira ? kCorMadeiraGuarda : Color{180, 150, 60, 255};
    const Color lamina = madeira ? kCorMadeiraLamina : Color{210, 220, 230, 255};
    const Color ponta = madeira ? kCorMadeiraPonta : Color{230, 235, 245, 255};
    const Color fio = madeira ? Color{190, 145, 85, 220} : Color{240, 245, 255, 220};

    DrawRectangle(static_cast<int>(cx - 4), static_cast<int>(cy + 8), 8, 18, cabo);
    DrawRectangle(static_cast<int>(cx - 14), static_cast<int>(cy + 4), 28, 6, guarda);
    DrawRectangle(static_cast<int>(cx - 5), static_cast<int>(cy - 28), 10, 34, lamina);
    DrawRectangle(static_cast<int>(cx - 2), static_cast<int>(cy - 28), 4, 34, fio);
    DrawTriangle(
        Vector2{cx, cy - 36},
        Vector2{cx - 5, cy - 28},
        Vector2{cx + 5, cy - 28},
        ponta);
}

void DesenharPainelEquipamento(const Personagem& jogador) {
    const float size = 72.0f;
    const float gap = 10.0f;
    const float totalW = size * 3.0f + gap * 2.0f;
    const float x0 = 18.0f;
    const float y0 = static_cast<float>(GetScreenHeight()) - size - 22.0f;

    DrawRectangleRounded(Rectangle{x0 - 8, y0 - 28, totalW + 16, size + 40}, 0.1f, 6, Color{10, 12, 18, 200});

    DesenharSlotEquipamento(x0, y0, size, "Utilitario", false);
    DrawText("-", static_cast<int>(x0 + size * 0.5f - 4), static_cast<int>(y0 + size * 0.35f), 22, Color{90, 100, 115, 180});

    const float xMeio = x0 + size + gap;
    const bool armaAtiva = jogador.temEspada && !jogador.usandoMagia;
    if (jogador.temEspada) {
        DesenharSlotEquipamento(xMeio, y0, size, "Madeira", armaAtiva);
        DesenharIconeEspadaHud(xMeio + size * 0.5f, y0 + size * 0.42f, jogador.espadaMadeira);
    } else {
        DesenharSlotEquipamento(xMeio, y0, size, "Arma", false);
        DrawText("-", static_cast<int>(xMeio + size * 0.5f - 4), static_cast<int>(y0 + size * 0.35f), 22, Color{90, 100, 115, 180});
    }

    const float xDir = x0 + (size + gap) * 2.0f;
    const bool magiaAtiva = jogador.temMagiaFogo && jogador.usandoMagia;
    if (jogador.temMagiaFogo) {
        DesenharSlotEquipamento(xDir, y0, size, "Fogo", magiaAtiva);
        DrawCircle(static_cast<int>(xDir + size * 0.5f), static_cast<int>(y0 + size * 0.40f), 16, Color{255, 120, 40, 255});
        DrawCircle(static_cast<int>(xDir + size * 0.5f), static_cast<int>(y0 + size * 0.40f), 8, Color{255, 220, 80, 255});
    } else {
        DesenharSlotEquipamento(xDir, y0, size, "Magia", false);
        DrawText("-", static_cast<int>(xDir + size * 0.5f - 4), static_cast<int>(y0 + size * 0.35f), 22, Color{90, 100, 115, 180});
    }
}

void DesenharBau(const Bau& bau) {
    rlPushMatrix();
    rlTranslatef(bau.posicao.x, 0.0f, bau.posicao.z);

    DrawCube(Vector3{0.0f, 0.12f, 0.0f}, 1.55f, 0.24f, 1.25f, Color{110, 104, 92, 255});
    DrawCube(Vector3{0.0f, 0.30f, 0.0f}, 1.35f, 0.28f, 1.05f, Color{130, 122, 108, 255});
    DrawCube(Vector3{0.0f, 0.42f, 0.0f}, 1.45f, 0.08f, 1.15f, Color{145, 136, 118, 255});

    rlTranslatef(0.0f, kBauPedestalAltura, 0.0f);
    DrawCube(Vector3{0.0f, 0.28f, 0.0f}, 1.1f, 0.55f, 0.75f, Color{118, 78, 38, 255});
    DrawCube(Vector3{0.0f, 0.28f, 0.0f}, 1.18f, 0.08f, 0.82f, Color{92, 58, 28, 255});
    DrawCube(Vector3{0.0f, 0.12f, 0.38f}, 0.18f, 0.18f, 0.08f, Color{180, 150, 60, 255});

    rlPushMatrix();
    rlTranslatef(0.0f, 0.55f, -0.35f);
    rlRotatef(-bau.tampaAngulo, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.08f, 0.35f}, 1.12f, 0.14f, 0.78f, Color{138, 92, 48, 255});
    DrawCube(Vector3{0.0f, 0.08f, 0.35f}, 1.0f, 0.06f, 0.68f, Color{160, 110, 60, 255});
    rlPopMatrix();

    rlPopMatrix();
}

void DesenharMesaMagica() {
    rlPushMatrix();
    rlTranslatef(kMesaX, 0.0f, kMesaZ);
    DrawCube(Vector3{0.0f, 0.42f, 0.0f}, 1.7f, 0.12f, 1.05f, Color{72, 48, 110, 255});
    DrawCube(Vector3{0.0f, 0.78f, 0.0f}, 1.55f, 0.08f, 0.92f, Color{96, 70, 150, 255});
    DrawCube(Vector3{-0.65f, 0.20f, -0.35f}, 0.14f, 0.40f, 0.14f, Color{60, 40, 90, 255});
    DrawCube(Vector3{0.65f, 0.20f, -0.35f}, 0.14f, 0.40f, 0.14f, Color{60, 40, 90, 255});
    DrawCube(Vector3{-0.65f, 0.20f, 0.35f}, 0.14f, 0.40f, 0.14f, Color{60, 40, 90, 255});
    DrawCube(Vector3{0.65f, 0.20f, 0.35f}, 0.14f, 0.40f, 0.14f, Color{60, 40, 90, 255});
    DrawSphere(Vector3{0.0f, 1.05f, 0.0f}, 0.22f, Color{255, 140, 60, 220});
    DrawSphere(Vector3{0.0f, 1.05f, 0.0f}, 0.12f, Color{255, 230, 120, 255});
    rlPopMatrix();
}

void DesenharArmario() {
    rlPushMatrix();
    rlTranslatef(kArmarioX, 0.0f, kArmarioZ);
    DrawCube(Vector3{0.0f, 0.95f, 0.0f}, 1.35f, 1.90f, 0.55f, Color{92, 66, 42, 255});
    DrawCube(Vector3{-0.32f, 0.95f, -0.28f}, 0.58f, 1.70f, 0.08f, Color{110, 80, 52, 255});
    DrawCube(Vector3{0.32f, 0.95f, -0.28f}, 0.58f, 1.70f, 0.08f, Color{110, 80, 52, 255});
    DrawCube(Vector3{-0.10f, 0.95f, -0.32f}, 0.08f, 0.12f, 0.08f, Color{180, 150, 70, 255});
    DrawCube(Vector3{0.10f, 0.95f, -0.32f}, 0.08f, 0.12f, 0.08f, Color{180, 150, 70, 255});
    rlPopMatrix();
}

void DesenharPortalDungeon(float tempo) {
    const float pulso = 0.55f + 0.45f * std::sin(tempo * 2.2f);
    rlPushMatrix();
    rlTranslatef(kPortalX, 0.0f, kPortalZ);
    DrawCube(Vector3{0.0f, 1.8f, 0.08f}, 3.4f, 3.4f, 0.14f, Color{
        static_cast<unsigned char>(50 + 50 * pulso),
        static_cast<unsigned char>(20 + 30 * pulso),
        static_cast<unsigned char>(90 + 70 * pulso), 190});
    DrawSphere(Vector3{0.0f, 1.8f, 0.2f}, 0.55f + 0.12f * pulso, Color{160, 90, 255, 90});
    DrawCube(Vector3{0.0f, 0.08f, 0.9f}, 2.6f, 0.12f, 1.6f, Color{45, 38, 70, 255});
    rlPopMatrix();
}

void DesenharLuzesCasa(float tempo) {
    const float pisca = 0.75f + 0.25f * std::sin(tempo * 5.5f);
    const Vector3 lanternas[] = {
        {-2.2f, 2.05f, 9.0f},
        {2.2f, 2.05f, 9.0f},
        {-2.2f, 2.05f, 12.5f},
        {2.2f, 2.05f, 12.5f},
    };
    for (const Vector3& p : lanternas) {
        DrawSphere(p, 0.18f, Color{255, 190, 90, static_cast<unsigned char>(200 * pisca)});
        DrawSphere(p, 0.42f, Color{255, 140, 40, static_cast<unsigned char>(55 * pisca)});
    }
    // Janelas iluminadas
    DrawCube(Vector3{-7.05f, 1.65f, -2.5f}, 0.08f, 1.0f, 1.6f, Color{255, 200, 110, 90});
    DrawCube(Vector3{7.05f, 1.65f, -2.5f}, 0.08f, 1.0f, 1.6f, Color{255, 200, 110, 90});
    DrawCube(Vector3{-7.05f, 1.65f, 1.5f}, 0.08f, 1.0f, 1.6f, Color{255, 200, 110, 90});
    DrawCube(Vector3{7.05f, 1.65f, 1.5f}, 0.08f, 1.0f, 1.6f, Color{255, 200, 110, 90});
    // Brilho da lareira
    DrawSphere(Vector3{-5.6f, 0.7f, -5.5f}, 0.35f + 0.08f * pisca, Color{255, 120, 40, 100});
}

Rectangle AbaRet(float x, float y, float w, float h) {
    return Rectangle{x, y, w, h};
}

bool DesenharAba(Rectangle r, const char* texto, bool ativa) {
    const Color fundo = ativa ? Color{55, 70, 110, 255} : Color{28, 32, 44, 255};
    const Color borda = ativa ? Color{210, 190, 120, 255} : Color{90, 100, 120, 180};
    DrawRectangleRounded(r, 0.12f, 6, fundo);
    DrawRectangleRoundedLines(r, 0.12f, 6, borda);
    const int tw = MeasureText(texto, 16);
    DrawText(texto, static_cast<int>(r.x + (r.width - tw) * 0.5f), static_cast<int>(r.y + 10), 16,
             ativa ? Color{245, 230, 180, 255} : Color{170, 180, 200, 220});
    return CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DesenharSlotItemPainel(Rectangle r, const char* nome, bool disponivel, bool equipado, bool* clicou) {
    const bool hover = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.1f, 6, hover ? Color{40, 36, 28, 255} : Color{24, 26, 34, 255});
    DrawRectangleRoundedLines(r, 0.1f, 6,
                              equipado ? Color{240, 200, 90, 255}
                                       : (disponivel ? Color{150, 130, 80, 220} : Color{70, 75, 90, 160}));
    if (clicou) *clicou = hover && disponivel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    DrawText(nome, static_cast<int>(r.x + 12), static_cast<int>(r.y + 18), 18,
             disponivel ? Color{230, 220, 200, 255} : Color{110, 115, 130, 200});
    if (equipado) {
        DrawText("EQUIPADO", static_cast<int>(r.x + 12), static_cast<int>(r.y + r.height - 28), 14,
                 Color{255, 210, 100, 255});
    } else if (!disponivel) {
        DrawText("vazio", static_cast<int>(r.x + 12), static_cast<int>(r.y + r.height - 28), 14,
                 Color{100, 105, 120, 180});
    }
}

// Retorna true se UI consumiu input (mouse livre)
bool AtualizarUIEstacoes(
    EstacaoAberta* estacao,
    AbaBau* abaBau,
    AbaMesa* abaMesa,
    Personagem* jogador) {
    if (*estacao == EstacaoAberta::Nenhuma) return false;

    if (IsKeyPressed(KEY_ESCAPE)) {
        *estacao = EstacaoAberta::Nenhuma;
        return true;
    }

    const float pw = 520.0f;
    const float ph = 360.0f;
    const float px = (GetScreenWidth() - pw) * 0.5f;
    const float py = (GetScreenHeight() - ph) * 0.5f;
    const Rectangle painel{px, py, pw, ph};

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 120});
    DrawRectangleRounded(painel, 0.06f, 8, Color{16, 18, 28, 245});
    DrawRectangleRoundedLines(painel, 0.06f, 8, Color{180, 150, 80, 220});

    if (*estacao == EstacaoAberta::Bau) {
        DrawText("BAU DE ARMAMENTOS", static_cast<int>(px + 140), static_cast<int>(py + 16), 24, Color{240, 220, 160, 255});
        if (DesenharAba(AbaRet(px + 20, py + 56, 150, 36), "Corpo a corpo", *abaBau == AbaBau::CorpoACorpo)) {
            *abaBau = AbaBau::CorpoACorpo;
        }
        if (DesenharAba(AbaRet(px + 185, py + 56, 140, 36), "Distancia", *abaBau == AbaBau::Distancia)) {
            *abaBau = AbaBau::Distancia;
        }
        if (DesenharAba(AbaRet(px + 340, py + 56, 140, 36), "Defesa", *abaBau == AbaBau::Defesa)) {
            *abaBau = AbaBau::Defesa;
        }

        if (*abaBau == AbaBau::CorpoACorpo) {
            bool clicou = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 200, 90}, "Espada de madeira", true,
                                  jogador->temEspada && jogador->espadaMadeira, &clicou);
            if (clicou) {
                jogador->temEspada = true;
                jogador->espadaMadeira = true;
                jogador->usandoMagia = false;
            }
            DesenharIconeEspadaHud(px + 300, py + 165, true);
            DrawText("Clique para equipar", static_cast<int>(px + 40), static_cast<int>(py + 230), 16,
                     Color{190, 195, 210, 220});
        } else if (*abaBau == AbaBau::Distancia) {
            bool dummy = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 280, 90}, "Nenhuma arma a distancia", false, false, &dummy);
        } else {
            bool dummy = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 280, 90}, "Nenhuma defesa", false, false, &dummy);
        }
    } else if (*estacao == EstacaoAberta::MesaMagica) {
        DrawText("MESA MAGICA", static_cast<int>(px + 175), static_cast<int>(py + 16), 24, Color{200, 170, 255, 255});
        if (DesenharAba(AbaRet(px + 20, py + 56, 150, 36), "Encantamentos", *abaMesa == AbaMesa::Encantamentos)) {
            *abaMesa = AbaMesa::Encantamentos;
        }
        if (DesenharAba(AbaRet(px + 185, py + 56, 140, 36), "Pocoes", *abaMesa == AbaMesa::Pocoes)) {
            *abaMesa = AbaMesa::Pocoes;
        }
        if (DesenharAba(AbaRet(px + 340, py + 56, 140, 36), "Magias", *abaMesa == AbaMesa::Magias)) {
            *abaMesa = AbaMesa::Magias;
        }

        if (*abaMesa == AbaMesa::Magias) {
            bool clicou = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 240, 100}, "Magia de fogo", true,
                                  jogador->temMagiaFogo && jogador->usandoMagia, &clicou);
            if (clicou) {
                jogador->temMagiaFogo = true;
                jogador->usandoMagia = true;
            }
            DrawCircle(static_cast<int>(px + 360), static_cast<int>(py + 170), 28, Color{255, 110, 40, 255});
            DrawCircle(static_cast<int>(px + 360), static_cast<int>(py + 170), 14, Color{255, 220, 90, 255});
            DrawText("Mira no alvo e clique esquerdo para lancar", static_cast<int>(px + 40),
                     static_cast<int>(py + 250), 16, Color{200, 190, 230, 230});
            DrawText(TextFormat("Custo: %.0f mana | CD: %.1fs | queima curta", CustoBolaFogoAtual(*jogador), kCooldownBolaFogo),
                     static_cast<int>(px + 40), static_cast<int>(py + 275), 16, Color{150, 180, 255, 230});
        } else if (*abaMesa == AbaMesa::Pocoes) {
            bool dummy = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 280, 90}, "Nenhuma pocao", false, false, &dummy);
        } else {
            bool dummy = false;
            DesenharSlotItemPainel(Rectangle{px + 40, py + 120, 280, 90}, "Nenhum encantamento", false, false, &dummy);
        }
    } else if (*estacao == EstacaoAberta::Armario) {
        DrawText("ARMARIO", static_cast<int>(px + 200), static_cast<int>(py + 16), 24, Color{220, 200, 170, 255});
        DrawText("Seu inventario de viagem (vazio por enquanto)", static_cast<int>(px + 70),
                 static_cast<int>(py + 120), 18, Color{190, 195, 210, 230});
        DrawText("Equipamentos da dungeon aparecerao aqui depois.", static_cast<int>(px + 55),
                 static_cast<int>(py + 160), 16, Color{150, 155, 170, 220});
        bool dummy = false;
        DesenharSlotItemPainel(Rectangle{px + 40, py + 210, 200, 80}, "Slot vazio", false, false, &dummy);
        DesenharSlotItemPainel(Rectangle{px + 260, py + 210, 200, 80}, "Slot vazio", false, false, &dummy);
    }

    DrawText("E / Esc fecha", static_cast<int>(px + pw - 130), static_cast<int>(py + ph - 32), 16,
             Color{160, 170, 190, 220});
    return true;
}

bool AtualizarUIPortal(
    EstacaoAberta* estacao,
    int* mundoSelecionado,
    int* dungeonEscolhida) {
    if (*estacao != EstacaoAberta::Portal) return false;

    if (IsKeyPressed(KEY_ESCAPE)) {
        *estacao = EstacaoAberta::Nenhuma;
        return true;
    }

    const float pw = 640.0f;
    const float ph = 460.0f;
    const float px = (GetScreenWidth() - pw) * 0.5f;
    const float py = (GetScreenHeight() - ph) * 0.5f;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 140});
    DrawRectangleRounded(Rectangle{px, py, pw, ph}, 0.05f, 8, Color{14, 12, 28, 250});
    DrawRectangleRoundedLines(Rectangle{px, py, pw, ph}, 0.05f, 8, Color{150, 120, 220, 230});
    DrawText("PORTAL DAS DUNGEONS", static_cast<int>(px + 160), static_cast<int>(py + 18), 26,
             Color{220, 200, 255, 255});

    const char* mundos[] = {"Mundo 1", "Mundo 2", "Mundo 3"};
    for (int i = 0; i < 3; ++i) {
        Rectangle r{px + 30.0f + i * 195.0f, py + 60.0f, 180.0f, 42.0f};
        const bool liberado = (i == 0);
        const bool sel = (*mundoSelecionado == i);
        Color fundo = !liberado ? Color{30, 28, 36, 255}
            : (sel ? Color{70, 50, 120, 255} : Color{40, 36, 58, 255});
        DrawRectangleRounded(r, 0.12f, 6, fundo);
        DrawRectangleRoundedLines(r, 0.12f, 6, liberado ? Color{180, 150, 255, 220} : Color{70, 70, 80, 180});
        DrawText(mundos[i], static_cast<int>(r.x + 48), static_cast<int>(r.y + 12), 18,
                 liberado ? RAYWHITE : Color{110, 110, 120, 200});
        if (!liberado) {
            DrawText("bloqueado", static_cast<int>(r.x + 48), static_cast<int>(r.y + 26), 12,
                     Color{140, 110, 120, 200});
        }
        if (liberado && CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *mundoSelecionado = i;
        }
    }

    DrawText("Dungeons do Mundo 1", static_cast<int>(px + 30), static_cast<int>(py + 120), 20,
             Color{200, 190, 230, 255});

    *dungeonEscolhida = -1;
    if (*mundoSelecionado == 0) {
        for (int i = 0; i < QuantidadeDungeonsMundo1(); ++i) {
            Rectangle r{px + 30.0f, py + 155.0f + i * 52.0f, pw - 60.0f, 46.0f};
            const bool hover = CheckCollisionPointRec(GetMousePosition(), r);
            DrawRectangleRounded(r, 0.1f, 6, hover ? Color{55, 42, 90, 255} : Color{28, 26, 44, 255});
            DrawRectangleRoundedLines(r, 0.1f, 6, hover ? Color{230, 200, 120, 255} : Color{110, 100, 150, 180});
            DrawText(TextFormat("%d. %s", i + 1, NomeDungeon(i)),
                     static_cast<int>(r.x + 16), static_cast<int>(r.y + 12), 20, RAYWHITE);
            const char* hint = (i == 0) ? "10 waves"
                : (i == 1) ? "infernal + voadores"
                : (i == 2) ? "monstros da caverna"
                : (i == 3) ? "animais mutantes"
                : "inimigos elite";
            DrawText(hint, static_cast<int>(r.x + 360), static_cast<int>(r.y + 14), 16,
                     Color{170, 160, 200, 220});
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                *dungeonEscolhida = i;
                *estacao = EstacaoAberta::Nenhuma;
            }
        }
    } else {
        DrawText("Este mundo ainda nao foi revelado.", static_cast<int>(px + 140),
                 static_cast<int>(py + 240), 20, Color{160, 150, 180, 220});
    }

    DrawText("Esc fecha", static_cast<int>(px + pw - 100), static_cast<int>(py + ph - 30), 16,
             Color{150, 150, 170, 200});
    return true;
}

bool AtualizarUIStats(Personagem* jogador, bool* aberto) {
    if (!*aberto) return false;

    // C ja e tratado no loop principal (toggle). Aqui so Esc fecha,
    // senao IsKeyPressed(KEY_C) ainda vale neste frame e fecha na hora de abrir.
    if (IsKeyPressed(KEY_ESCAPE)) {
        *aberto = false;
        return true;
    }

    const float pw = 460.0f;
    const float ph = 360.0f;
    const float px = (GetScreenWidth() - pw) * 0.5f;
    const float py = (GetScreenHeight() - ph) * 0.5f;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 120});
    DrawRectangleRounded(Rectangle{px, py, pw, ph}, 0.06f, 8, Color{16, 18, 28, 250});
    DrawRectangleRoundedLines(Rectangle{px, py, pw, ph}, 0.06f, 8, Color{180, 160, 90, 220});
    DrawText("ATRIBUTOS", static_cast<int>(px + 150), static_cast<int>(py + 16), 26, Color{240, 220, 160, 255});
    DrawText(TextFormat("Nivel %d/%d  |  Pontos: %d", jogador->nivel, kNivelMax, jogador->pontosDisponiveis),
             static_cast<int>(px + 90), static_cast<int>(py + 52), 18, Color{200, 210, 230, 255});

    struct LinhaStat {
        const char* nome;
        int* valor;
        const char* desc;
    };
    LinhaStat linhas[] = {
        {"Mana", &jogador->statMana, "menos custo / feitiços um pouco mais fortes"},
        {"Forca", &jogador->statForca, "mais dano fisico (espada)"},
        {"Resistencia", &jogador->statResistencia, "menos dano recebido"},
    };

    for (int i = 0; i < 3; ++i) {
        const float y = py + 95.0f + i * 70.0f;
        DrawRectangleRounded(Rectangle{px + 24, y, pw - 48, 58}, 0.1f, 6, Color{28, 30, 42, 255});
        DrawText(linhas[i].nome, static_cast<int>(px + 40), static_cast<int>(y + 10), 20, RAYWHITE);
        DrawText(linhas[i].desc, static_cast<int>(px + 40), static_cast<int>(y + 34), 14, Color{160, 170, 190, 220});
        DrawText(TextFormat("%d", *linhas[i].valor), static_cast<int>(px + 300), static_cast<int>(y + 16), 22,
                 Color{255, 220, 120, 255});

        Rectangle rMais{px + pw - 90, y + 12, 44, 34};
        const bool pode = jogador->pontosDisponiveis > 0;
        const bool hover = CheckCollisionPointRec(GetMousePosition(), rMais);
        DrawRectangleRounded(rMais, 0.15f, 6, pode ? (hover ? Color{70, 120, 70, 255} : Color{45, 90, 50, 255})
                                                   : Color{40, 40, 48, 255});
        DrawText("+", static_cast<int>(rMais.x + 14), static_cast<int>(rMais.y + 4), 24,
                 pode ? RAYWHITE : Color{100, 100, 110, 200});
        if (pode && hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            (*linhas[i].valor) += 1;
            jogador->pontosDisponiveis -= 1;
        }
    }

    DrawText(TextFormat("Custo magia: %.0f%%  |  Feitico: +%.0f%%  |  Fisico: +%.0f%%  |  Res: %.0f%% dmg",
                        MultCustoMana(*jogador) * 100.0f,
                        (MultPoderFeitico(*jogador) - 1.0f) * 100.0f,
                        (MultForcaFisica(*jogador) - 1.0f) * 100.0f,
                        (1.0f - MultResistencia(*jogador)) * 100.0f),
             static_cast<int>(px + 28), static_cast<int>(py + ph - 36), 14, Color{170, 190, 220, 230});
    return true;
}

void AplicarQueimaInimigo(Inimigo* alvo, float danoTick) {
    alvo->queimaTempo = kDuracaoQueima;
    alvo->queimaTick = 0.0f;
    alvo->queimaDano = danoTick;
    alvo->hitFlash = std::max(alvo->hitFlash, 0.25f);
}

void AtualizarQueimaInimigos(Inimigo* inimigos, int quantidade, float dt) {
    for (int i = 0; i < quantidade; ++i) {
        Inimigo& inimigo = inimigos[i];
        if (!inimigo.vivo || inimigo.queimaTempo <= 0.0f) continue;
        inimigo.queimaTempo -= dt;
        inimigo.queimaTick -= dt;
        if (inimigo.queimaTick <= 0.0f) {
            inimigo.queimaTick = kIntervaloQueima;
            const float tick = inimigo.queimaDano > 0.0f ? inimigo.queimaDano : kDanoQueimaPorTick;
            inimigo.vida -= tick;
            inimigo.hitFlash = 0.20f;
            if (inimigo.vida <= 0.0f) {
                inimigo.vida = 0.0f;
                inimigo.vivo = false;
                inimigo.queimaTempo = 0.0f;
            }
        }
    }
}

void LimparBolasFogo(BolaFogo* bolas, int maxBolas) {
    for (int i = 0; i < maxBolas; ++i) bolas[i].ativa = false;
}

bool LancarBolaFogo(Personagem* jogador, BolaFogo* bolas, int maxBolas, const Camera3D& camera) {
    if (!jogador->temMagiaFogo || !jogador->usandoMagia) return false;
    if (jogador->cooldownMagia > 0.0f) return false;
    const float custo = CustoBolaFogoAtual(*jogador);
    if (jogador->mana < custo) return false;

    int slot = -1;
    for (int i = 0; i < maxBolas; ++i) {
        if (!bolas[i].ativa) { slot = i; break; }
    }
    if (slot < 0) return false;

    Vector3 dir{
        camera.target.x - camera.position.x,
        camera.target.y - camera.position.y,
        camera.target.z - camera.position.z,
    };
    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (len < 0.001f) return false;
    dir.x /= len; dir.y /= len; dir.z /= len;

    const float poder = MultPoderFeitico(*jogador);
    bolas[slot].ativa = true;
    bolas[slot].tempoVida = 2.2f;
    bolas[slot].danoImpacto = kDanoBolaFogoImpacto * poder;
    bolas[slot].danoQueima = kDanoQueimaPorTick * poder;
    bolas[slot].posicao = Vector3{
        jogador->posicao.x + dir.x * 0.7f,
        jogador->posicao.y + 1.35f + dir.y * 0.2f,
        jogador->posicao.z + dir.z * 0.7f,
    };
    bolas[slot].velocidade = Vector3{
        dir.x * kVelocidadeBolaFogo,
        dir.y * kVelocidadeBolaFogo,
        dir.z * kVelocidadeBolaFogo,
    };
    jogador->mana -= custo;
    jogador->cooldownMagia = kCooldownBolaFogo;
    return true;
}

void AtualizarBolasFogo(BolaFogo* bolas, int maxBolas, Inimigo* inimigos, int quantidade, float dt) {
    for (int i = 0; i < maxBolas; ++i) {
        BolaFogo& b = bolas[i];
        if (!b.ativa) continue;
        b.posicao.x += b.velocidade.x * dt;
        b.posicao.y += b.velocidade.y * dt;
        b.posicao.z += b.velocidade.z * dt;
        b.tempoVida -= dt;
        if (b.tempoVida <= 0.0f || b.posicao.y < -1.0f) {
            b.ativa = false;
            continue;
        }

        bool acertou = false;
        for (int j = 0; j < quantidade; ++j) {
            Inimigo& alvo = inimigos[j];
            if (!alvo.vivo) continue;
            const float raio = RaioDoInimigo(alvo) + 0.35f;
            const float cy = alvo.posicao.y + (alvo.chefe ? 1.5f : 1.0f);
            const float dx = b.posicao.x - alvo.posicao.x;
            const float dy = b.posicao.y - cy;
            const float dz = b.posicao.z - alvo.posicao.z;
            if (dx * dx + dy * dy + dz * dz <= raio * raio) {
                alvo.vida -= b.danoImpacto;
                alvo.hitFlash = 0.35f;
                AplicarQueimaInimigo(&alvo, b.danoQueima);
                if (alvo.vida <= 0.0f) {
                    alvo.vida = 0.0f;
                    alvo.vivo = false;
                }
                acertou = true;
                break;
            }
        }
        if (acertou) b.ativa = false;
    }
}

void DesenharBolasFogo(const BolaFogo* bolas, int maxBolas) {
    for (int i = 0; i < maxBolas; ++i) {
        if (!bolas[i].ativa) continue;
        DrawSphere(bolas[i].posicao, 0.28f, Color{255, 120, 40, 255});
        DrawSphere(bolas[i].posicao, 0.14f, Color{255, 230, 120, 255});
        DrawSphere(Vector3{bolas[i].posicao.x, bolas[i].posicao.y - 0.05f, bolas[i].posicao.z},
                   0.40f, Color{255, 80, 20, 70});
    }
}

void SpawnerBonecoTreino(Inimigo* inimigo) {
    inimigo->posicao = Vector3{kBonecoX, 0.0f, kBonecoZ};
    ResolverColisaoMapa(&inimigo->posicao, kRaioInimigo);
    inimigo->vidaMax = kVidaBonecoMax;
    inimigo->vida = kVidaBonecoMax;
    inimigo->vivo = true;
    inimigo->chefe = false;
    inimigo->boneco = true;
    inimigo->voador = false;
    inimigo->visual = TipoVisualInimigo::Normal;
    inimigo->danoAtaque = 0.0f;
    inimigo->velocidadeMov = 0.0f;
    inimigo->alturaVoo = 0.0f;
    inimigo->hitFlash = 0.0f;
    inimigo->ataque = 0.0f;
    inimigo->porradao = 0.0f;
    inimigo->cooldownAtaque = 9999.0f;
    inimigo->cooldownArremesso = 9999.0f;
    inimigo->cooldownPorradao = 9999.0f;
    inimigo->knockbackX = 0.0f;
    inimigo->knockbackZ = 0.0f;
    inimigo->recuo = 0.0f;
    inimigo->cicloPasso = 0.0f;
    inimigo->jaAcertouGolpe = false;
    inimigo->jaAcertouPorradao = false;
    inimigo->yaw = 180.0f;
    inimigo->queimaTempo = 0.0f;
    inimigo->queimaTick = 0.0f;
    inimigo->queimaDano = 0.0f;
}

void ConfigurarInimigoDungeon(Inimigo* inimigo, int dungeonId, int wave, bool chefe) {
    const float mult = MultiplicadorDungeon(dungeonId) * (1.0f + (wave - 1) * 0.08f);
    inimigo->boneco = false;
    inimigo->chefe = chefe;
    inimigo->voador = false;
    inimigo->alturaVoo = 0.0f;
    inimigo->visual = TipoVisualInimigo::Normal;

    float vidaBase = chefe ? kVidaChefeMax : kVidaInimigoMax;
    float danoBase = chefe ? kDanoChefeBasico : kDanoInimigoBasico;
    float velBase = chefe ? kVelocidadeChefe : kVelocidadeInimigo;

    switch (dungeonId) {
        case 1: // Nether
            inimigo->visual = (GetRandomValue(0, 99) < 40) ? TipoVisualInimigo::Voador : TipoVisualInimigo::Demonio;
            inimigo->voador = inimigo->visual == TipoVisualInimigo::Voador;
            vidaBase *= inimigo->voador ? 0.85f : 1.1f;
            danoBase *= 1.15f;
            velBase = inimigo->voador ? kVelocidadeVoador : velBase * 1.05f;
            break;
        case 2: // Caverna
            inimigo->visual = TipoVisualInimigo::Caverna;
            vidaBase *= 1.25f;
            danoBase *= 1.1f;
            velBase *= 0.95f;
            break;
        case 3: // Floresta
            inimigo->visual = TipoVisualInimigo::Mutante;
            vidaBase *= 1.15f;
            danoBase *= 1.2f;
            velBase *= 1.15f;
            break;
        case 4: // Arena final
            inimigo->visual = chefe ? TipoVisualInimigo::Elite : TipoVisualInimigo::Elite;
            vidaBase *= 1.35f;
            danoBase *= 1.35f;
            velBase *= 1.1f;
            break;
        default:
            break;
    }

    if (inimigo->voador) {
        inimigo->alturaVoo = 1.55f + static_cast<float>(GetRandomValue(0, 40)) * 0.01f;
        inimigo->posicao.y = inimigo->alturaVoo;
    }

    inimigo->vidaMax = vidaBase * mult;
    inimigo->vida = inimigo->vidaMax;
    inimigo->danoAtaque = danoBase * mult;
    inimigo->velocidadeMov = velBase;
    inimigo->vivo = true;
    inimigo->hitFlash = 0.0f;
    inimigo->ataque = 0.0f;
    inimigo->porradao = 0.0f;
    inimigo->cooldownAtaque = chefe ? 0.8f : (0.35f + static_cast<float>(GetRandomValue(0, 40)) * 0.01f);
    inimigo->cooldownArremesso = 0.0f;
    inimigo->cooldownPorradao = chefe ? 1.5f : 0.0f;
    inimigo->knockbackX = 0.0f;
    inimigo->knockbackZ = 0.0f;
    inimigo->recuo = 0.0f;
    inimigo->cicloPasso = static_cast<float>(GetRandomValue(0, 100)) * 0.1f;
    inimigo->jaAcertouGolpe = false;
    inimigo->jaAcertouPorradao = false;
    inimigo->queimaTempo = 0.0f;
    inimigo->queimaTick = 0.0f;
    inimigo->queimaDano = 0.0f;
}

void SpawnerInimigoDungeon(Inimigo* inimigo, float offsetAngulo, int dungeonId, int wave) {
    const float ang = offsetAngulo * DEG2RAD;
    const float raio = 9.0f + static_cast<float>(GetRandomValue(0, 60)) * 0.05f;
    inimigo->posicao = Vector3{std::sin(ang) * raio, 0.0f, std::cos(ang) * raio};
    if (std::fabs(inimigo->posicao.x) < 2.5f && std::fabs(inimigo->posicao.z) < 2.5f) {
        inimigo->posicao.x = 7.0f;
        inimigo->posicao.z = -6.0f;
    }
    ResolverColisaoMapa(&inimigo->posicao, kRaioInimigo);
    ConfigurarInimigoDungeon(inimigo, dungeonId, wave, false);
    inimigo->yaw = 180.0f;
}

void SpawnerChefeDungeon(Inimigo* inimigo, int dungeonId, int wave) {
    inimigo->posicao = Vector3{0.0f, 0.0f, -8.0f};
    ResolverColisaoMapa(&inimigo->posicao, kRaioChefe);
    ConfigurarInimigoDungeon(inimigo, dungeonId, wave, true);
    inimigo->yaw = 0.0f;
}

int ContarInimigosDaWave(int wave, int dungeonId) {
    if (wave >= kTotalWaves) return 1;
    float quantidade = static_cast<float>(kInimigosWave1) + dungeonId * 0.35f;
    for (int i = 1; i < wave; ++i) {
        quantidade *= kAumentoPorWave;
    }
    return std::clamp(static_cast<int>(std::lround(quantidade)), 1, kMaxInimigos);
}

int ContarInimigosVivos(const Inimigo* inimigos, int quantidade) {
    int vivos = 0;
    for (int i = 0; i < quantidade; ++i) {
        if (inimigos[i].vivo) ++vivos;
    }
    return vivos;
}

int IniciarWaveDungeon(Inimigo* inimigos, int wave, int dungeonId) {
    for (int i = 0; i < kMaxInimigos; ++i) {
        inimigos[i] = Inimigo{};
        inimigos[i].vivo = false;
    }

    if (wave >= kTotalWaves) {
        SpawnerChefeDungeon(&inimigos[0], dungeonId, wave);
        return 1;
    }

    const int quantidade = ContarInimigosDaWave(wave, dungeonId);
    for (int i = 0; i < quantidade; ++i) {
        const float angulo = (360.0f / static_cast<float>(quantidade)) * static_cast<float>(i)
            + static_cast<float>(GetRandomValue(-12, 12));
        SpawnerInimigoDungeon(&inimigos[i], angulo, dungeonId, wave);
    }
    for (int passa = 0; passa < 8; ++passa) {
        for (int i = 0; i < quantidade; ++i) {
            for (int j = i + 1; j < quantidade; ++j) {
                SepararCirculosXZ(&inimigos[i].posicao, RaioDoInimigo(inimigos[i]),
                                  &inimigos[j].posicao, RaioDoInimigo(inimigos[j]));
            }
            ResolverColisaoMapa(&inimigos[i].posicao, RaioDoInimigo(inimigos[i]));
        }
    }
    return quantidade;
}

float XpNecessario(int nivel) {
    return kXpBaseNivel * static_cast<float>(std::max(1, nivel));
}

float MultCustoMana(const Personagem& j) {
    const float m = 1.0f - kStatManaCustoPorPonto * static_cast<float>(j.statMana);
    return std::clamp(m, 0.45f, 1.0f);
}

float MultPoderFeitico(const Personagem& j) {
    return 1.0f + kStatManaPoderPorPonto * static_cast<float>(j.statMana);
}

float MultForcaFisica(const Personagem& j) {
    return 1.0f + kStatForcaPorPonto * static_cast<float>(j.statForca);
}

float MultResistencia(const Personagem& j) {
    return 1.0f / (1.0f + kStatResistPorPonto * static_cast<float>(j.statResistencia));
}

float CustoBolaFogoAtual(const Personagem& j) {
    return kCustoBolaFogo * MultCustoMana(j);
}

void GanharXp(Personagem* jogador, float quantidade) {
    if (jogador->nivel >= kNivelMax) {
        jogador->xp = 0.0f;
        return;
    }
    jogador->xp += quantidade;
    while (jogador->nivel < kNivelMax && jogador->xp >= XpNecessario(jogador->nivel)) {
        jogador->xp -= XpNecessario(jogador->nivel);
        jogador->nivel += 1;
        jogador->pontosDisponiveis += kPontosPorNivel;
        jogador->vida = std::min(kVidaJogadorMax, jogador->vida + 25.0f);
    }
    if (jogador->nivel >= kNivelMax) {
        jogador->nivel = kNivelMax;
        jogador->xp = 0.0f;
    }
}

float XpPorWave(int dungeonId, int wave) {
    return kXpPorWaveBase + wave * 12.0f + dungeonId * 18.0f;
}

float XpPorDungeon(int dungeonId) {
    return kXpPorDungeonBase + dungeonId * 120.0f;
}

void AplicarDanoJogador(Personagem* jogador, float dano, Vector3 origemKnock) {
    float danoFinal = dano * MultResistencia(*jogador);
    if (jogador->bloqueio > 0.55f) {
        danoFinal *= 0.18f;
    }
    jogador->vida = std::max(0.0f, jogador->vida - danoFinal);
    jogador->hitFlash = 0.28f;

    Vector3 dir{
        jogador->posicao.x - origemKnock.x,
        0.0f,
        jogador->posicao.z - origemKnock.z,
    };
    dir = NormalizarXZ(dir);
    const float forca = (jogador->bloqueio > 0.55f) ? 2.2f : 5.5f;
    jogador->knockbackX += dir.x * forca;
    jogador->knockbackZ += dir.z * forca;
}

bool DispararPedra(Pedra* pedras, int maxPedras, Vector3 origem, Vector3 alvo) {
    for (int i = 0; i < maxPedras; ++i) {
        if (pedras[i].ativa) continue;
        Vector3 dir{
            alvo.x - origem.x,
            0.0f,
            alvo.z - origem.z,
        };
        const float dist = ComprimentoXZ(dir);
        if (dist < 0.001f) return false;
        dir.x /= dist;
        dir.z /= dist;

        const float tempoVoo = std::clamp(dist / 11.0f, 0.45f, 1.35f);
        const float dy = (alvo.y + 0.9f) - origem.y;
        pedras[i].ativa = true;
        pedras[i].posicao = origem;
        pedras[i].velocidade = Vector3{
            dir.x * (dist / tempoVoo),
            (dy / tempoVoo) + 0.5f * kGravidade * tempoVoo,
            dir.z * (dist / tempoVoo),
        };
        pedras[i].tempoVida = 3.0f;
        return true;
    }
    return false;
}

void AtualizarPedras(Pedra* pedras, int maxPedras, Personagem* jogador, float dt) {
    for (int i = 0; i < maxPedras; ++i) {
        Pedra& p = pedras[i];
        if (!p.ativa) continue;

        p.velocidade.y -= kGravidade * dt;
        p.posicao.x += p.velocidade.x * dt;
        p.posicao.y += p.velocidade.y * dt;
        p.posicao.z += p.velocidade.z * dt;
        p.tempoVida -= dt;

        if (p.tempoVida <= 0.0f || p.posicao.y < -0.5f) {
            p.ativa = false;
            continue;
        }

        if (jogador->vida <= 0.0f) continue;

        const float dx = p.posicao.x - jogador->posicao.x;
        const float dy = p.posicao.y - (jogador->posicao.y + 0.9f);
        const float dz = p.posicao.z - jogador->posicao.z;
        if (dx * dx + dy * dy + dz * dz <= 0.55f * 0.55f) {
            AplicarDanoJogador(jogador, kDanoInimigoBasico, p.posicao);
            p.ativa = false;
        }
    }
}

void DesenharPedras(const Pedra* pedras, int maxPedras) {
    for (int i = 0; i < maxPedras; ++i) {
        if (!pedras[i].ativa) continue;
        DrawSphere(pedras[i].posicao, 0.16f, Color{110, 100, 90, 255});
        DrawSphere(Vector3{
            pedras[i].posicao.x + 0.04f,
            pedras[i].posicao.y + 0.03f,
            pedras[i].posicao.z - 0.02f,
        }, 0.10f, Color{130, 120, 108, 255});
    }
}

void AtualizarInimigo(Inimigo* inimigo, Personagem* jogador, float dt, Pedra* pedras, int maxPedras) {
    inimigo->hitFlash = std::max(0.0f, inimigo->hitFlash - dt);
    inimigo->recuo = std::max(0.0f, inimigo->recuo - dt * 2.4f);
    inimigo->cooldownAtaque = std::max(0.0f, inimigo->cooldownAtaque - dt);
    inimigo->cooldownArremesso = std::max(0.0f, inimigo->cooldownArremesso - dt);
    inimigo->cooldownPorradao = std::max(0.0f, inimigo->cooldownPorradao - dt);

    // Knockback suave (chefe resiste mais)
    const float resistenciaKb = inimigo->chefe ? 0.45f : 1.0f;
    inimigo->posicao.x += inimigo->knockbackX * dt * resistenciaKb;
    inimigo->posicao.z += inimigo->knockbackZ * dt * resistenciaKb;
    const float amortecimento = 1.0f - std::exp(-7.0f * dt);
    inimigo->knockbackX *= (1.0f - amortecimento);
    inimigo->knockbackZ *= (1.0f - amortecimento);
    if (std::fabs(inimigo->knockbackX) < 0.05f) inimigo->knockbackX = 0.0f;
    if (std::fabs(inimigo->knockbackZ) < 0.05f) inimigo->knockbackZ = 0.0f;

    if (!inimigo->vivo) return;

    if (inimigo->boneco) {
        inimigo->cicloPasso += dt * 1.2f;
        ResolverColisaoMapa(&inimigo->posicao, RaioDoInimigo(*inimigo));
        return;
    }

    Vector3 paraJogador{
        jogador->posicao.x - inimigo->posicao.x,
        0.0f,
        jogador->posicao.z - inimigo->posicao.z,
    };
    const float dist = ComprimentoXZ(paraJogador);
    if (dist > 0.001f) {
        paraJogador = NormalizarXZ(paraJogador);
        inimigo->yaw = LerpAngulo(inimigo->yaw, atan2f(-paraJogador.x, -paraJogador.z) * RAD2DEG,
                                  1.0f - std::exp(-6.0f * dt));
    }

    const bool emRecuo = inimigo->recuo > 0.15f ||
        ComprimentoXZ(Vector3{inimigo->knockbackX, 0, inimigo->knockbackZ}) > (inimigo->chefe ? 1.4f : 0.8f);
    const bool jogadorAlto = !inimigo->voador &&
        jogador->posicao.y > inimigo->posicao.y + kAlturaInalcancavel;
    const float raioIni = RaioDoInimigo(*inimigo);
    const float distParada = kRaioJogador + raioIni + 0.20f;
    const float vel = (inimigo->velocidadeMov > 0.05f)
        ? inimigo->velocidadeMov
        : (inimigo->chefe ? kVelocidadeChefe : kVelocidadeInimigo);
    const float alcanceBasico = inimigo->chefe ? kAlcanceChefeBasico : kAlcanceAtaqueInimigo;
    const float danoBasico = (inimigo->danoAtaque > 0.05f)
        ? inimigo->danoAtaque
        : (inimigo->chefe ? kDanoChefeBasico : kDanoInimigoBasico);

    // Voadores lentos (nether): flutuam e avancam devagar
    if (inimigo->voador) {
        const float alvoY = inimigo->alturaVoo > 0.2f ? inimigo->alturaVoo : 1.7f;
        inimigo->posicao.y += (alvoY - inimigo->posicao.y) * (1.0f - std::exp(-4.0f * dt));

        if (inimigo->ataque > 0.0f) {
            inimigo->ataque += dt / kDuracaoAtaqueInimigo;
            if (!inimigo->jaAcertouGolpe &&
                inimigo->ataque >= 0.35f && inimigo->ataque <= 0.62f &&
                dist <= alcanceBasico + 0.4f && jogador->vida > 0.0f) {
                AplicarDanoJogador(jogador, danoBasico, inimigo->posicao);
                inimigo->jaAcertouGolpe = true;
            }
            if (inimigo->ataque >= 1.0f) {
                inimigo->ataque = 0.0f;
                inimigo->cooldownAtaque = kCooldownAtaqueInimigo * 1.1f;
            }
        } else if (!emRecuo) {
            if (dist > distParada + 0.35f) {
                inimigo->posicao.x += paraJogador.x * vel * dt;
                inimigo->posicao.z += paraJogador.z * vel * dt;
                inimigo->cicloPasso += dt * 6.0f;
            } else if (inimigo->cooldownAtaque <= 0.0f && jogador->vida > 0.0f) {
                inimigo->ataque = 0.001f;
                inimigo->jaAcertouGolpe = false;
            } else {
                inimigo->cicloPasso += dt * 2.0f;
            }
        }

        const float limite = gMapa.tamanho / 2.0f - 1.0f;
        inimigo->posicao.x = std::clamp(inimigo->posicao.x, -limite, limite);
        inimigo->posicao.z = std::clamp(inimigo->posicao.z, -limite, limite);
        const float yGuard = inimigo->posicao.y;
        ResolverColisaoMapa(&inimigo->posicao, raioIni);
        inimigo->posicao.y = yGuard;
        return;
    }

    // --- CHEFE: porradao no chao ---
    if (inimigo->chefe && inimigo->porradao > 0.0f) {
        inimigo->porradao += dt / kDuracaoPorradao;
        inimigo->cicloPasso += dt * 1.5f;

        if (!inimigo->jaAcertouPorradao &&
            inimigo->porradao >= 0.48f && inimigo->porradao <= 0.62f &&
            dist <= kAlcancePorradao && jogador->vida > 0.0f) {
            AplicarDanoJogador(jogador, kDanoChefePorradao, inimigo->posicao);
            jogador->velocidadeY = kImpulsoPorradao;
            jogador->noChao = false;
            jogador->fasePulo = 0.0f;
            inimigo->jaAcertouPorradao = true;
        }

        if (inimigo->porradao >= 1.0f) {
            inimigo->porradao = 0.0f;
            inimigo->cooldownPorradao = kCooldownPorradao;
            inimigo->cooldownAtaque = 0.6f;
        }
    } else if (inimigo->chefe && inimigo->ataque > 0.0f) {
        inimigo->ataque += dt / kDuracaoAtaqueInimigo;
        inimigo->cicloPasso += dt * 2.0f;

        if (!inimigo->jaAcertouGolpe &&
            inimigo->ataque >= 0.35f && inimigo->ataque <= 0.62f &&
            dist <= alcanceBasico && jogador->vida > 0.0f) {
            AplicarDanoJogador(jogador, danoBasico, inimigo->posicao);
            inimigo->jaAcertouGolpe = true;
        }

        if (inimigo->ataque >= 1.0f) {
            inimigo->ataque = 0.0f;
            inimigo->cooldownAtaque = kCooldownAtaqueInimigo * 0.85f;
        }
    } else if (inimigo->chefe && !emRecuo) {
        // Prefere porradao quando perto; ataque basico no range curto
        if (dist > distParada + 0.35f) {
            inimigo->posicao.x += paraJogador.x * vel * dt;
            inimigo->posicao.z += paraJogador.z * vel * dt;
            inimigo->cicloPasso += dt * 7.5f;
        } else {
            inimigo->cicloPasso += dt * 2.0f;
        }

        if (jogador->vida > 0.0f) {
            if (inimigo->cooldownPorradao <= 0.0f && dist <= kAlcancePorradao + 0.6f) {
                inimigo->porradao = 0.001f;
                inimigo->jaAcertouPorradao = false;
                inimigo->ataque = 0.0f;
            } else if (inimigo->cooldownAtaque <= 0.0f && dist <= alcanceBasico + 0.35f) {
                inimigo->ataque = 0.001f;
                inimigo->jaAcertouGolpe = false;
            }
        }
    } else if (!inimigo->chefe && jogadorAlto && jogador->vida > 0.0f && !emRecuo && inimigo->ataque <= 0.0f) {
        // Inimigo normal: se o jogador esta em cima de algo, arremessa pedra
        if (dist >= kAlcanceArremessoMin && dist <= kAlcanceArremessoMax &&
            inimigo->cooldownArremesso <= 0.0f) {
            const Vector3 origem{
                inimigo->posicao.x,
                inimigo->posicao.y + 1.25f,
                inimigo->posicao.z,
            };
            const Vector3 alvo{
                jogador->posicao.x,
                jogador->posicao.y,
                jogador->posicao.z,
            };
            if (DispararPedra(pedras, maxPedras, origem, alvo)) {
                inimigo->cooldownArremesso = kCooldownArremesso;
                inimigo->cicloPasso += 0.4f;
            }
        }
        if (dist > distParada + 0.8f) {
            inimigo->posicao.x += paraJogador.x * vel * 0.75f * dt;
            inimigo->posicao.z += paraJogador.z * vel * 0.75f * dt;
            inimigo->cicloPasso += dt * 8.0f;
        } else {
            inimigo->cicloPasso += dt * 2.0f;
        }
    } else if (!inimigo->chefe && inimigo->ataque > 0.0f) {
        inimigo->ataque += dt / kDuracaoAtaqueInimigo;
        inimigo->cicloPasso += dt * 2.0f;

        if (!inimigo->jaAcertouGolpe &&
            inimigo->ataque >= 0.35f && inimigo->ataque <= 0.62f &&
            dist <= alcanceBasico && jogador->vida > 0.0f && !jogadorAlto) {
            AplicarDanoJogador(jogador, danoBasico, inimigo->posicao);
            inimigo->jaAcertouGolpe = true;
        }

        if (inimigo->ataque >= 1.0f) {
            inimigo->ataque = 0.0f;
            inimigo->cooldownAtaque = kCooldownAtaqueInimigo;
        }
    } else if (!inimigo->chefe && !emRecuo) {
        if (dist > distParada) {
            inimigo->posicao.x += paraJogador.x * vel * dt;
            inimigo->posicao.z += paraJogador.z * vel * dt;
            inimigo->cicloPasso += dt * 9.0f;
        } else {
            inimigo->cicloPasso += dt * 2.0f;
            if (inimigo->cooldownAtaque <= 0.0f && dist <= alcanceBasico + 0.25f && !jogadorAlto) {
                inimigo->ataque = 0.001f;
                inimigo->jaAcertouGolpe = false;
            }
        }
    } else {
        inimigo->cicloPasso += dt * 1.5f;
    }

    const float limite = gMapa.tamanho / 2.0f - 1.0f;
    inimigo->posicao.x = std::clamp(inimigo->posicao.x, -limite, limite);
    inimigo->posicao.z = std::clamp(inimigo->posicao.z, -limite, limite);
    ResolverColisaoMapa(&inimigo->posicao, raioIni);
}

bool GolpeAcertaInimigo(const Personagem& jogador, const Inimigo& inimigo) {
    if (!inimigo.vivo) return false;
    // Janela ativa do golpe
    if (jogador.ataque < 0.28f || jogador.ataque > 0.62f) return false;

    const Vector3 frente = FrenteYaw(jogador.yaw);
    const Vector3 origem{
        jogador.posicao.x,
        jogador.posicao.y + 1.0f,
        jogador.posicao.z,
    };
    const Vector3 pontoAtaque{
        origem.x + frente.x * 1.35f,
        origem.y,
        origem.z + frente.z * 1.35f,
    };
    const float alturaCentro = inimigo.chefe ? 1.7f : (inimigo.voador ? 0.35f : 1.0f);
    const Vector3 centroInimigo{
        inimigo.posicao.x,
        inimigo.posicao.y + alturaCentro,
        inimigo.posicao.z,
    };

    const float dx = pontoAtaque.x - centroInimigo.x;
    const float dy = pontoAtaque.y - centroInimigo.y;
    const float dz = pontoAtaque.z - centroInimigo.z;
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    const float alcance = kAlcanceAtaque + (inimigo.chefe ? 0.85f : 0.0f);
    if (dist > alcance) return false;

    // Precisa estar na frente do jogador
    const float dot = (centroInimigo.x - origem.x) * frente.x + (centroInimigo.z - origem.z) * frente.z;
    return dot > 0.20f;
}

float DanoDaEspada(const Personagem& jogador) {
    float base = kDanoEspadaMadeira;
    return base * MultForcaFisica(jogador);
}

void AplicarDanoNoInimigo(Inimigo* alvo, const Personagem& jogador, float dano) {
    alvo->vida -= dano;
    alvo->hitFlash = 0.35f;
    alvo->recuo = alvo->chefe ? 0.55f : 1.0f;
    const Vector3 frente = FrenteYaw(jogador.yaw);
    const float forcaKb = alvo->chefe ? 3.5f : 7.0f;
    alvo->knockbackX += frente.x * forcaKb;
    alvo->knockbackZ += frente.z * forcaKb;
    if (alvo->ataque > 0.0f && alvo->ataque < 0.35f) {
        alvo->ataque = 0.0f;
        alvo->cooldownAtaque = 0.35f;
    }
    if (alvo->porradao > 0.0f && alvo->porradao < 0.40f) {
        alvo->porradao = 0.0f;
        alvo->cooldownPorradao = 0.8f;
    }
    if (alvo->vida <= 0.0f) {
        alvo->vida = 0.0f;
        alvo->vivo = false;
        alvo->hitFlash = 0.7f;
        alvo->ataque = 0.0f;
        alvo->porradao = 0.0f;
        alvo->knockbackX += frente.x * 3.0f;
        alvo->knockbackZ += frente.z * 3.0f;
    }
}

// Sempre acerta o alvo principal; 50% de chance de acertar os demais no range/frente
int AplicarGolpeEspada(Personagem* jogador, Inimigo* inimigos, int quantidade) {
    Inimigo* alvos[kMaxInimigos]{};
    float dists[kMaxInimigos]{};
    int total = 0;
    for (int i = 0; i < quantidade; ++i) {
        if (!GolpeAcertaInimigo(*jogador, inimigos[i])) continue;
        const float dx = inimigos[i].posicao.x - jogador->posicao.x;
        const float dz = inimigos[i].posicao.z - jogador->posicao.z;
        alvos[total] = &inimigos[i];
        dists[total] = dx * dx + dz * dz;
        ++total;
    }
    if (total <= 0) return 0;

    // Ordena por distancia (mais perto primeiro)
    for (int i = 0; i < total - 1; ++i) {
        for (int j = i + 1; j < total; ++j) {
            if (dists[j] < dists[i]) {
                std::swap(dists[i], dists[j]);
                std::swap(alvos[i], alvos[j]);
            }
        }
    }

    const float dano = DanoDaEspada(*jogador);
    const bool multiHit = (total > 1) && (GetRandomValue(1, 100) <= 50);
    int acertos = 0;

    AplicarDanoNoInimigo(alvos[0], *jogador, dano);
    if (!alvos[0]->vivo && !alvos[0]->boneco) GanharXp(jogador, alvos[0]->chefe ? kXpChefe : kXpPorKill);
    ++acertos;

    if (multiHit) {
        for (int i = 1; i < total; ++i) {
            AplicarDanoNoInimigo(alvos[i], *jogador, dano);
            if (!alvos[i]->vivo && !alvos[i]->boneco) GanharXp(jogador, alvos[i]->chefe ? kXpChefe : kXpPorKill);
            ++acertos;
        }
    }
    return acertos;
}

void ConsoleDefinirMensagem(ConsoleComandos* console, const char* mensagem) {
    std::strncpy(console->mensagem, mensagem, sizeof(console->mensagem) - 1);
    console->mensagem[sizeof(console->mensagem) - 1] = '\0';
    console->mensagemTempo = 3.5f;
}

void ConsoleLimparTexto(ConsoleComandos* console) {
    console->comprimento = 0;
    console->texto[0] = '\0';
}

bool ComandoIgual(const char* entrada, const char* comando) {
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    for (; *comando != '\0'; ++entrada, ++comando) {
        if (std::tolower(static_cast<unsigned char>(*entrada)) !=
            std::tolower(static_cast<unsigned char>(*comando))) {
            return false;
        }
    }
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    return *entrada == '\0';
}

bool ComandoComPrefixo(const char* entrada, const char* comando, const char** resto) {
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    for (; *comando != '\0'; ++entrada, ++comando) {
        if (std::tolower(static_cast<unsigned char>(*entrada)) !=
            std::tolower(static_cast<unsigned char>(*comando))) {
            return false;
        }
    }
    if (*entrada != '\0' && *entrada != ' ' && *entrada != '\t') return false;
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    if (resto) *resto = entrada;
    return true;
}

bool ParseInteiroPositivo(const char* texto, int* valor) {
    while (*texto == ' ' || *texto == '\t') ++texto;
    if (*texto == '\0') {
        *valor = 1;
        return true;
    }
    if (*texto < '0' || *texto > '9') return false;
    int n = 0;
    while (*texto >= '0' && *texto <= '9') {
        n = n * 10 + (*texto - '0');
        if (n > 99) return false;
        ++texto;
    }
    while (*texto == ' ' || *texto == '\t') ++texto;
    if (*texto != '\0') return false;
    if (n <= 0) return false;
    *valor = n;
    return true;
}

// Aceita "/xp 10000", "/xp10000", "xp 10000" — qualquer quantidade > 0.
bool ParseComandoXp(const char* entrada, float* quantidade) {
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    if (*entrada == '/') ++entrada;

    if (std::tolower(static_cast<unsigned char>(entrada[0])) != 'x' ||
        std::tolower(static_cast<unsigned char>(entrada[1])) != 'p') {
        return false;
    }
    entrada += 2;

    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    if (*entrada == '+') ++entrada;

    double n = 0.0;
    bool viuDigito = false;
    while (*entrada != '\0') {
        if (*entrada >= '0' && *entrada <= '9') {
            if (n < 1.0e12) n = n * 10.0 + static_cast<double>(*entrada - '0');
            viuDigito = true;
            ++entrada;
            continue;
        }
        // Separadores de milhar (10.000 / 10,000)
        if ((*entrada == '.' || *entrada == ',') && viuDigito) {
            ++entrada;
            continue;
        }
        if (*entrada == ' ' || *entrada == '\t') break;
        return false;
    }

    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    if (*entrada != '\0' || !viuDigito || n <= 0.0) return false;

    *quantidade = static_cast<float>(n);
    return true;
}

bool PareceComandoXp(const char* entrada) {
    while (*entrada == ' ' || *entrada == '\t') ++entrada;
    if (*entrada == '/') ++entrada;
    return std::tolower(static_cast<unsigned char>(entrada[0])) == 'x' &&
           std::tolower(static_cast<unsigned char>(entrada[1])) == 'p';
}

void MatarTodosNpcs(Inimigo* inimigos, int quantidade) {
    for (int i = 0; i < quantidade; ++i) {
        if (!inimigos[i].vivo) continue;
        inimigos[i].vivo = false;
        inimigos[i].vida = 0.0f;
        inimigos[i].ataque = 0.0f;
        inimigos[i].porradao = 0.0f;
        inimigos[i].hitFlash = 0.45f;
    }
}

void ExecutarComandoConsole(
    ConsoleComandos* console,
    Inimigo* inimigos,
    int* quantidadeInimigos,
    int* waveAtual,
    bool* aguardandoProximaWave,
    float* timerWave,
    bool* wavesCompletas,
    float* retornoCasaTimer,
    Personagem* jogador,
    bool* pediuEditor) {
    if (ComandoIgual(console->texto, "/killallnpcs")) {
        MatarTodosNpcs(inimigos, *quantidadeInimigos);
        ConsoleDefinirMensagem(console, "todos os npcs mortos");
        return;
    }

    if (ComandoIgual(console->texto, "/build")) {
        if (EstamosNaCasa()) {
            ConsoleDefinirMensagem(console, "a casa base nao pode ser modificada");
            return;
        }
        if (pediuEditor) *pediuEditor = true;
        ConsoleDefinirMensagem(console, "abrindo editor de mapa");
        return;
    }

    const char* argsSkip = nullptr;
    if (ComandoComPrefixo(console->texto, "/skipwave", &argsSkip)) {
        if (EstamosNaCasa()) {
            ConsoleDefinirMensagem(console, "sem waves na casa base");
            return;
        }
        int pular = 1;
        if (!ParseInteiroPositivo(argsSkip, &pular)) {
            ConsoleDefinirMensagem(console, "uso: /skipwave [quantidade]");
            return;
        }

        if (*wavesCompletas) {
            ConsoleDefinirMensagem(console, "waves ja completas");
            return;
        }

        MatarTodosNpcs(inimigos, *quantidadeInimigos);
        *quantidadeInimigos = 0;

        *waveAtual += pular;
        if (*waveAtual >= kTotalWaves) {
            *waveAtual = kTotalWaves;
            *wavesCompletas = true;
            *aguardandoProximaWave = false;
            *timerWave = 0.0f;
            if (jogador) GanharXp(jogador, XpPorDungeon(std::max(0, gDungeonAtual)));
            if (retornoCasaTimer) *retornoCasaTimer = 1.8f;
            ConsoleDefinirMensagem(console, "wave 10 - dungeon concluida, voltando pra casa");
            return;
        }

        *quantidadeInimigos = IniciarWaveDungeon(inimigos, *waveAtual, std::max(0, gDungeonAtual));
        *aguardandoProximaWave = false;
        *timerWave = 0.0f;
        ConsoleDefinirMensagem(console, TextFormat("pulou %d wave(s) - wave %d", pular, *waveAtual));
        return;
    }

    if (PareceComandoXp(console->texto)) {
        if (!jogador) {
            ConsoleDefinirMensagem(console, "jogador indisponivel");
            return;
        }
        float quantidade = 0.0f;
        if (!ParseComandoXp(console->texto, &quantidade)) {
            ConsoleDefinirMensagem(console, "uso: /xp <quantidade>   ex: /xp 10000");
            return;
        }
        const int nivelAntes = jogador->nivel;
        const float xpAntes = jogador->xp;
        GanharXp(jogador, quantidade);
        ConsoleDefinirMensagem(console,
            TextFormat("+%.0f xp | nv %d->%d | xp %.0f->%.0f | pts %d",
                       quantidade, nivelAntes, jogador->nivel, xpAntes, jogador->xp,
                       jogador->pontosDisponiveis));
        return;
    }

    ConsoleDefinirMensagem(console, "comando desconhecido");
}

void AtualizarConsoleComandos(ConsoleComandos* console, float dt,
                              Inimigo* inimigos, int* quantidadeInimigos,
                              int* waveAtual, bool* aguardandoProximaWave,
                              float* timerWave, bool* wavesCompletas,
                              float* retornoCasaTimer, Personagem* jogador,
                              bool* pediuEditor) {
    console->mensagemTempo = std::max(0.0f, console->mensagemTempo - dt);
    if (pediuEditor) *pediuEditor = false;

    if (IsKeyPressed(KEY_T) && !console->aberto) {
        console->aberto = true;
        ConsoleLimparTexto(console);
        while (GetCharPressed() != 0) {
        }
        return;
    }

    if (!console->aberto) return;

    if (IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_T) && console->comprimento == 0)) {
        console->aberto = false;
        ConsoleLimparTexto(console);
        while (GetCharPressed() != 0) {
        }
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        if (console->comprimento > 0) {
            ExecutarComandoConsole(console, inimigos, quantidadeInimigos, waveAtual,
                                   aguardandoProximaWave, timerWave, wavesCompletas,
                                   retornoCasaTimer, jogador, pediuEditor);
        }
        ConsoleLimparTexto(console);
        return;
    }

    if (IsKeyPressed(KEY_BACKSPACE) && console->comprimento > 0) {
        console->texto[--console->comprimento] = '\0';
    }

    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= 32 && ch < 127 && console->comprimento < kConsoleCapacidade - 1) {
            console->texto[console->comprimento++] = static_cast<char>(ch);
            console->texto[console->comprimento] = '\0';
        }
        ch = GetCharPressed();
    }
}

void DesenharConsoleComandos(const ConsoleComandos& console) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();

    if (console.mensagemTempo > 0.0f && console.mensagem[0] != '\0' && !console.aberto) {
        const float alpha = std::clamp(console.mensagemTempo / 0.4f, 0.0f, 1.0f);
        DrawText(console.mensagem, 20, sh - 36,
                 18, Color{180, 255, 170, static_cast<unsigned char>(220.0f * alpha)});
    }

    if (!console.aberto) return;

    DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 90});
    const int painelH = 150;
    const int y0 = sh - painelH;
    DrawRectangle(0, y0, sw, painelH, Color{12, 14, 20, 235});
    DrawRectangle(0, y0, sw, 2, Color{90, 160, 255, 220});

    DrawText("CONSOLE  |  /killallnpcs   /skipwave [n]   /xp <n>   /build   |  Enter executa  Esc/T fecha",
             16, y0 + 12, 16, Color{160, 190, 230, 255});

    DrawRectangle(12, y0 + 48, sw - 24, 36, Color{24, 28, 38, 255});
    DrawRectangleLines(12, y0 + 48, sw - 24, 36, Color{80, 110, 160, 200});

    const char* prompt = TextFormat("> %s", console.texto);
    DrawText(prompt, 22, y0 + 56, 22, RAYWHITE);

    // Cursor piscando
    if (static_cast<int>(GetTime() * 2.0) % 2 == 0) {
        const int textoW = MeasureText(prompt, 22);
        DrawRectangle(22 + textoW + 2, y0 + 58, 2, 20, Color{200, 220, 255, 255});
    }

    if (console.mensagemTempo > 0.0f && console.mensagem[0] != '\0') {
        DrawText(console.mensagem, 16, y0 + 100, 18, Color{160, 255, 150, 255});
    } else {
        DrawText("digite um comando e aperte Enter", 16, y0 + 100, 16, Color{120, 130, 150, 255});
    }
}

enum class AcaoTelaMorte {
    Nenhuma,
    Renascer,
    Sair,
};

Rectangle BotaoTelaMorte(int indice) {
    const float bw = 280.0f;
    const float bh = 52.0f;
    const float x = (static_cast<float>(GetScreenWidth()) - bw) * 0.5f;
    const float y = static_cast<float>(GetScreenHeight()) * 0.48f + static_cast<float>(indice) * 68.0f;
    return Rectangle{x, y, bw, bh};
}

void DesenharBotao(Rectangle r, const char* texto, bool hover) {
    const Color fundo = hover ? Color{70, 28, 32, 245} : Color{28, 18, 24, 235};
    const Color borda = hover ? Color{255, 120, 110, 255} : Color{140, 70, 70, 200};
    DrawRectangleRounded(r, 0.18f, 8, fundo);
    DrawRectangleRoundedLines(r, 0.18f, 8, borda);
    const int tw = MeasureText(texto, 24);
    DrawText(texto, static_cast<int>(r.x + (r.width - tw) * 0.5f),
             static_cast<int>(r.y + 14), 24, hover ? RAYWHITE : Color{230, 210, 210, 255});
}

void DesenharTelaMorte() {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{10, 4, 8, 170});

    const char* titulo = "VOCE MORREU";
    const int tw = MeasureText(titulo, 48);
    DrawText(titulo, (sw - tw) / 2, sh / 2 - 120, 48, Color{255, 90, 90, 255});
    const char* sub = "O personagem caiu em combate";
    const int sw2 = MeasureText(sub, 20);
    DrawText(sub, (sw - sw2) / 2, sh / 2 - 68, 20, Color{210, 180, 180, 220});

    const Vector2 mouse = GetMousePosition();
    const Rectangle rRenascer = BotaoTelaMorte(0);
    const Rectangle rSair = BotaoTelaMorte(1);
    DesenharBotao(rRenascer, "Renascer", CheckCollisionPointRec(mouse, rRenascer));
    DesenharBotao(rSair, "Sair do jogo", CheckCollisionPointRec(mouse, rSair));
}

AcaoTelaMorte AtualizarTelaMorte() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, BotaoTelaMorte(0))) return AcaoTelaMorte::Renascer;
        if (CheckCollisionPointRec(mouse, BotaoTelaMorte(1))) return AcaoTelaMorte::Sair;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) return AcaoTelaMorte::Renascer;
    return AcaoTelaMorte::Nenhuma;
}

void LimparPedras(Pedra* pedras, int maxPedras) {
    for (int i = 0; i < maxPedras; ++i) {
        pedras[i].ativa = false;
    }
}

void InicializarMapaDoArquivo() {
    if (!MapaTentarCarregar(&gMapa, gCaminhoMapa, static_cast<int>(sizeof(gCaminhoMapa)))) {
        MapaResolverCaminhoSalvar(gCaminhoMapa, static_cast<int>(sizeof(gCaminhoMapa)));
        MapaSalvarEmArquivo(gMapa, gCaminhoMapa);
    }
}

void AbrirEditorMapa(
    Personagem* jogador,
    CameraOrbit* orbita,
    bool* cursorLivre,
    ConsoleComandos* console) {
    SalvarSnapshotAntesEditor(*jogador, *orbita, *cursorLivre);
    IniciarCameraEditorDeOrbit(*jogador, *orbita);
    EditorIniciar(&gEditor, gCaminhoMapa);
    console->aberto = false;
    ConsoleLimparTexto(console);
    EnableCursor();
}

void FecharEditorMapa(Personagem* jogador, CameraOrbit* orbita, bool* cursorLivre, bool* cursorEstavaLivre) {
    RestaurarSnapshotAntesEditor(jogador, orbita, cursorLivre);
    gEditor.ativo = false;
    // Forca reaplicar o estado correto do cursor no proximo frame
    if (cursorEstavaLivre) *cursorEstavaLivre = !(*cursorLivre);
    if (*cursorLivre) EnableCursor();
    else DisableCursor();
}

bool EditorMapaAtivo() {
    return gEditor.ativo;
}

void AtualizarModoEditor(float dt, Camera3D* camera) {
    if (EditorEstaConstruindo(gEditor)) {
        AtualizarCameraEditorLivre(&gCamEditor, dt, false);
        AplicarCameraEditor(camera, gCamEditor);
        EditorAtualizar(&gEditor, &gMapa, *camera, false);
    }
}

void DesenharGhostEditor(float tempo) {
    EditorDesenharGhost(gEditor, tempo);
}

void DesenharUiEditor(bool* pediuSair) {
    EditorDesenharUI(&gEditor, &gMapa, pediuSair);
}

void EntrarDungeon(
    int dungeonId,
    Personagem* jogador,
    Inimigo* inimigos,
    int* quantidadeInimigos,
    Pedra* pedras,
    BolaFogo* bolas,
    int* waveAtual,
    bool* aguardandoProximaWave,
    float* timerWave,
    bool* wavesCompletas,
    float* retornoCasaTimer,
    CameraOrbit* orbita) {
    gDungeonAtual = dungeonId;
    MapaCarregarDungeon(&gMapa, dungeonId, gCaminhoMapa, static_cast<int>(sizeof(gCaminhoMapa)));
    MatarTodosNpcs(inimigos, *quantidadeInimigos);
    *quantidadeInimigos = 0;
    LimparPedras(pedras, kMaxPedras);
    LimparBolasFogo(bolas, kMaxBolasFogo);
    *waveAtual = 0;
    *aguardandoProximaWave = true;
    *timerWave = 2.5f;
    *wavesCompletas = false;
    *retornoCasaTimer = 0.0f;
    jogador->posicao = Vector3{0.0f, 0.0f, 6.0f};
    jogador->velocidadeX = 0.0f;
    jogador->velocidadeZ = 0.0f;
    jogador->velocidadeY = 0.0f;
    jogador->knockbackX = 0.0f;
    jogador->knockbackZ = 0.0f;
    jogador->ataque = 0.0f;
    jogador->bloqueio = 0.0f;
    jogador->noChao = true;
    jogador->vida = kVidaJogadorMax;
    jogador->mana = kManaMax;
    orbita->yaw = 0.0f;
    orbita->pitch = -12.0f;
    ResolverColisaoMapa(&jogador->posicao, kRaioJogador);
}

void VoltarParaCasa(
    Personagem* jogador,
    Inimigo* inimigos,
    int* quantidadeInimigos,
    Pedra* pedras,
    BolaFogo* bolas,
    int* waveAtual,
    bool* aguardandoProximaWave,
    float* timerWave,
    bool* wavesCompletas,
    float* retornoCasaTimer,
    CameraOrbit* orbita) {
    gDungeonAtual = -1;
    MapaCarregarCasaBase(&gMapa, gCaminhoMapa, static_cast<int>(sizeof(gCaminhoMapa)));
    MatarTodosNpcs(inimigos, *quantidadeInimigos);
    LimparPedras(pedras, kMaxPedras);
    LimparBolasFogo(bolas, kMaxBolasFogo);
    SpawnerBonecoTreino(&inimigos[0]);
    *quantidadeInimigos = 1;
    *waveAtual = 0;
    *aguardandoProximaWave = false;
    *timerWave = 0.0f;
    *wavesCompletas = true;
    *retornoCasaTimer = 0.0f;
    jogador->posicao = Vector3{0.0f, 0.0f, 8.5f};
    jogador->velocidadeX = 0.0f;
    jogador->velocidadeZ = 0.0f;
    jogador->velocidadeY = 0.0f;
    jogador->knockbackX = 0.0f;
    jogador->knockbackZ = 0.0f;
    jogador->ataque = 0.0f;
    jogador->bloqueio = 0.0f;
    jogador->noChao = true;
    jogador->vida = kVidaJogadorMax;
    jogador->mana = kManaMax;
    orbita->yaw = 180.0f;
    orbita->pitch = -12.0f;
    ResolverColisaoMapa(&jogador->posicao, kRaioJogador);
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(kLarguraTela, kAlturaTela, "Jogo Novo - Casa Base");
    SetTargetFPS(60);
    DisableCursor();

    InicializarMapaDoArquivo();

    Personagem jogador{};
    jogador.posicao = Vector3{0.0f, 0.0f, 8.5f};
    jogador.yaw = 180.0f;
    jogador.noChao = true;
    jogador.vida = kVidaJogadorMax;
    jogador.xp = 0.0f;
    jogador.nivel = 1;
    jogador.temEspada = false;
    jogador.espadaMadeira = false;
    jogador.temMagiaFogo = false;
    jogador.usandoMagia = false;
    jogador.mana = kManaMax;
    jogador.cooldownMagia = 0.0f;
    jogador.pontosDisponiveis = 0;
    jogador.statMana = 0;
    jogador.statForca = 0;
    jogador.statResistencia = 0;

    Inimigo inimigos[kMaxInimigos]{};
    Pedra pedras[kMaxPedras]{};
    BolaFogo bolasFogo[kMaxBolasFogo]{};
    int quantidadeInimigos = 0;
    int waveAtual = 0;
    float timerWave = kTempoSpawnInimigo;
    bool aguardandoProximaWave = true;
    bool wavesCompletas = false;
    float retornoCasaTimer = 0.0f;
    int mundoPortal = 0;
    int dungeonEscolhida = -1;
    char msgStatus[96]{};
    float msgStatusTempo = 0.0f;

    Bau bau{};
    bau.posicao = Vector3{kBauX, 0.0f, kBauZ};
    bau.tampaAngulo = 0.0f;
    bau.aberto = false;

    EstacaoAberta estacao = EstacaoAberta::Nenhuma;
    AbaBau abaBau = AbaBau::CorpoACorpo;
    AbaMesa abaMesa = AbaMesa::Magias;
    bool menuStatsAberto = false;

    if (EstamosNaCasa()) {
        SpawnerBonecoTreino(&inimigos[0]);
        quantidadeInimigos = 1;
        aguardandoProximaWave = false;
        wavesCompletas = true;
        waveAtual = 0;
        gDungeonAtual = -1;
    }

    CameraOrbit orbita{};
    orbita.yaw = 180.0f;
    orbita.pitch = -12.0f;
    orbita.distancia = kDistanciaCamera;

    Camera3D camera{};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    AplicarCamera(&camera, jogador.posicao, orbita, 0.0f);

    bool cursorLivre = false;
    bool cursorEstavaLivre = false;
    bool gameOverAtivo = false;
    ConsoleComandos console{};
    ConsoleLimparTexto(&console);

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        const float tempo = static_cast<float>(GetTime());
        const bool morto = jogador.vida <= 0.0f;
        const bool emEditor = EditorMapaAtivo();
        const bool naCasa = EstamosNaCasa();
        const bool emDungeon = gDungeonAtual >= 0;
        msgStatusTempo = std::max(0.0f, msgStatusTempo - dt);

        if (morto && !gameOverAtivo) {
            gameOverAtivo = true;
            if (!naCasa) {
                MatarTodosNpcs(inimigos, quantidadeInimigos);
                quantidadeInimigos = 0;
            }
            LimparPedras(pedras, kMaxPedras);
            LimparBolasFogo(bolasFogo, kMaxBolasFogo);
            jogador.velocidadeX = 0.0f;
            jogador.velocidadeZ = 0.0f;
            jogador.velocidadeY = 0.0f;
            jogador.knockbackX = 0.0f;
            jogador.knockbackZ = 0.0f;
            jogador.ataque = 0.0f;
            jogador.bloqueio = 0.0f;
            jogador.agachar = 0.0f;
            jogador.intensidadeAndar = 0.0f;
            jogador.intensidadeCorrida = 0.0f;
            jogador.miraTravada = false;
            estacao = EstacaoAberta::Nenhuma;
            console.aberto = false;
            ConsoleLimparTexto(&console);
        }
        if (!morto) gameOverAtivo = false;

        bool pediuEditor = false;
        if (!morto && !emEditor) {
            AtualizarConsoleComandos(&console, dt, inimigos, &quantidadeInimigos, &waveAtual,
                                     &aguardandoProximaWave, &timerWave, &wavesCompletas,
                                     &retornoCasaTimer, &jogador, &pediuEditor);
        }
        if (pediuEditor && !naCasa) {
            AbrirEditorMapa(&jogador, &orbita, &cursorLivre, &console);
        }
        const bool consoleAberto = console.aberto;
        const bool uiEstacao = estacao != EstacaoAberta::Nenhuma;
        const bool entradaBloqueada = consoleAberto || morto || EditorMapaAtivo() || uiEstacao || menuStatsAberto;

        if (!entradaBloqueada && IsKeyPressed(KEY_X)) {
            cursorLivre = !cursorLivre;
        }
        if (!consoleAberto && !morto && !EditorMapaAtivo() && estacao == EstacaoAberta::Nenhuma &&
            IsKeyPressed(KEY_C)) {
            menuStatsAberto = !menuStatsAberto;
        }

        const bool mouseLivre = cursorLivre || consoleAberto || morto || EditorMapaAtivo() || uiEstacao ||
                                menuStatsAberto;
        if (mouseLivre != cursorEstavaLivre) {
            if (mouseLivre) EnableCursor();
            else DisableCursor();
            cursorEstavaLivre = mouseLivre;
        }

        // No editor o mouse fica sempre livre (cliques na UI / pecas).
        // Segurar o botao direito so gira a camera, sem travar o cursor.
        if (EditorMapaAtivo() && IsCursorHidden()) {
            EnableCursor();
        }

        if (!mouseLivre && !EditorMapaAtivo()) {
            const Vector2 mouse = GetMouseDelta();
            orbita.yaw -= mouse.x * kSensibilidadeMouse;
            orbita.pitch -= mouse.y * kSensibilidadeMouse;
            orbita.pitch = std::clamp(orbita.pitch, kPitchMin, kPitchMax);
        }

        // Zoom da camera: I aproxima, O afasta
        if (!entradaBloqueada || uiEstacao) {
            if (!consoleAberto && !morto && !EditorMapaAtivo()) {
                if (IsKeyPressed(KEY_I)) {
                    orbita.distancia = std::max(kDistanciaCameraMin, orbita.distancia - 0.45f);
                }
                if (IsKeyPressed(KEY_O)) {
                    orbita.distancia = std::min(kDistanciaCameraMax, orbita.distancia + 0.45f);
                }
            }
        }

        // Estacoes da casa (bau / mesa / armario / portal)
        const float distBau = ComprimentoXZ(Vector3{
            jogador.posicao.x - bau.posicao.x, 0.0f, jogador.posicao.z - bau.posicao.z});
        const float distMesa = ComprimentoXZ(Vector3{
            jogador.posicao.x - kMesaX, 0.0f, jogador.posicao.z - kMesaZ});
        const float distArmario = ComprimentoXZ(Vector3{
            jogador.posicao.x - kArmarioX, 0.0f, jogador.posicao.z - kArmarioZ});
        const float distPortal = ComprimentoXZ(Vector3{
            jogador.posicao.x - kPortalX, 0.0f, jogador.posicao.z - kPortalZ});
        const bool pertoBau = distBau <= kDistanciaInteracao;
        const bool pertoMesa = distMesa <= kDistanciaInteracao;
        const bool pertoArmario = distArmario <= kDistanciaInteracao;
        const bool pertoPortal = distPortal <= 3.2f;

        // E abre/fecha painel (nao fecha no mesmo frame do abrir)
        if (!consoleAberto && !morto && !EditorMapaAtivo() && IsKeyPressed(KEY_E)) {
            if (estacao != EstacaoAberta::Nenhuma) {
                estacao = EstacaoAberta::Nenhuma;
            } else {
                float melhor = kDistanciaInteracao + 0.01f;
                EstacaoAberta alvo = EstacaoAberta::Nenhuma;
                if (naCasa) {
                    if (distBau < melhor) { melhor = distBau; alvo = EstacaoAberta::Bau; }
                    if (distMesa < melhor) { melhor = distMesa; alvo = EstacaoAberta::MesaMagica; }
                    if (distArmario < melhor) { melhor = distArmario; alvo = EstacaoAberta::Armario; }
                }
                if (alvo != EstacaoAberta::Nenhuma) {
                    estacao = alvo;
                    if (alvo == EstacaoAberta::Bau) {
                        bau.aberto = true;
                        abaBau = AbaBau::CorpoACorpo;
                    } else if (alvo == EstacaoAberta::MesaMagica) {
                        abaMesa = AbaMesa::Magias;
                    }
                } else if (pertoPortal && naCasa) {
                    estacao = EstacaoAberta::Portal;
                    mundoPortal = 0;
                }
            }
        }
        if (estacao != EstacaoAberta::Bau) {
            bau.aberto = false;
        }
        const float tampaAlvo = bau.aberto ? 105.0f : 0.0f;
        bau.tampaAngulo += (tampaAlvo - bau.tampaAngulo) * (1.0f - std::exp(-8.0f * dt));

        // Mana
        jogador.mana = std::min(kManaMax, jogador.mana + kManaRegen * dt);
        jogador.cooldownMagia = std::max(0.0f, jogador.cooldownMagia - dt);

        // Atalhos de equipamento
        if (!entradaBloqueada) {
            if (IsKeyPressed(KEY_ONE) && jogador.temEspada) jogador.usandoMagia = false;
            if (IsKeyPressed(KEY_TWO) && jogador.temMagiaFogo) jogador.usandoMagia = true;
        }

        // Combate
        const bool querBloquear = !entradaBloqueada && jogador.temEspada && !jogador.usandoMagia &&
                                  IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && jogador.ataque <= 0.0f;
        const float bloqueioAlvo = querBloquear ? 1.0f : 0.0f;
        jogador.bloqueio += (bloqueioAlvo - jogador.bloqueio) * (1.0f - std::exp(-(querBloquear ? 22.0f : 14.0f) * dt));

        jogador.cooldownAtaque = std::max(0.0f, jogador.cooldownAtaque - dt);
        if (!entradaBloqueada && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && jogador.bloqueio < 0.4f) {
            if (jogador.usandoMagia && jogador.temMagiaFogo) {
                LancarBolaFogo(&jogador, bolasFogo, kMaxBolasFogo, camera);
            } else if (jogador.temEspada && jogador.ataque <= 0.0f && jogador.cooldownAtaque <= 0.0f) {
                jogador.ataque = 0.001f;
                jogador.jaAcertouGolpe = false;
                jogador.bloqueio = 0.0f;
            }
        }

        if (!morto && jogador.ataque > 0.0f) {
            jogador.ataque += dt / kDuracaoAtaque;
            if (!jogador.jaAcertouGolpe) {
                if (AplicarGolpeEspada(&jogador, inimigos, quantidadeInimigos) > 0) {
                    jogador.jaAcertouGolpe = true;
                }
            }
            if (jogador.ataque >= 1.0f) {
                jogador.ataque = 0.0f;
                jogador.cooldownAtaque = kCooldownAtaque;
            }
        }

        jogador.hitFlash = std::max(0.0f, jogador.hitFlash - dt);

        if (!entradaBloqueada && IsKeyPressed(KEY_K)) {
            jogador.miraTravada = !jogador.miraTravada;
        }

        const Vector3 movimento = entradaBloqueada ? Vector3{0.0f, 0.0f, 0.0f} : LerMovimento(orbita.yaw);
        const bool temInput = ComprimentoXZ(movimento) > 0.0001f;
        const bool combatendo = jogador.ataque > 0.0f || jogador.bloqueio > 0.45f;
        const bool querCorrer = !entradaBloqueada &&
                                (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
                                jogador.noChao && !combatendo;
        const bool querAgachar = !entradaBloqueada &&
                                 (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
                                 jogador.noChao && !querCorrer && !combatendo;
        const float agacharAlvo = querAgachar ? 1.0f : 0.0f;
        jogador.agachar += (agacharAlvo - jogador.agachar) * (1.0f - std::exp(-14.0f * dt));

        // Mira travada: corpo sempre olha pra frente da camera
        if (jogador.miraTravada && jogador.ataque <= 0.0f) {
            jogador.yaw = LerpAngulo(jogador.yaw, orbita.yaw, 1.0f - std::exp(-14.0f * dt));
        }

        if (jogador.noChao) {
            float velocidade = kVelocidadeJogador;
            if (jogador.bloqueio > 0.45f) velocidade *= 0.45f;
            else if (jogador.ataque > 0.0f) velocidade *= 0.35f;
            else if (jogador.agachar > 0.3f) {
                velocidade = kVelocidadeJogador + (kVelocidadeAgachado - kVelocidadeJogador) * jogador.agachar;
            } else if (querCorrer && !jogador.miraTravada) {
                velocidade = kVelocidadeCorrida;
            }
            if (jogador.miraTravada && jogador.ataque <= 0.0f && jogador.bloqueio < 0.45f) {
                velocidade *= 0.90f;
            }

            if (temInput) {
                jogador.velocidadeX = movimento.x * velocidade;
                jogador.velocidadeZ = movimento.z * velocidade;
                if (jogador.ataque <= 0.0f && !jogador.miraTravada) {
                    const float yawAlvo = atan2f(-movimento.x, -movimento.z) * RAD2DEG;
                    jogador.yaw = LerpAngulo(jogador.yaw, yawAlvo, 1.0f - std::exp(-kSuavidadeGiro * dt));
                }
                jogador.intensidadeAndar = std::min(1.0f, jogador.intensidadeAndar + dt * 8.0f);
                const float corridaAlvo = (querCorrer && jogador.agachar < 0.2f && !jogador.miraTravada) ? 1.0f : 0.0f;
                jogador.intensidadeCorrida += (corridaAlvo - jogador.intensidadeCorrida) * (1.0f - std::exp(-12.0f * dt));
                const float ritmo = 11.0f + 7.0f * jogador.intensidadeCorrida - 4.0f * jogador.agachar
                    + (jogador.miraTravada ? 1.5f : 0.0f);
                jogador.cicloPasso += dt * ritmo;
            } else {
                jogador.velocidadeX = 0.0f;
                jogador.velocidadeZ = 0.0f;
                jogador.intensidadeAndar = std::max(0.0f, jogador.intensidadeAndar - dt * 7.0f);
                jogador.intensidadeCorrida = std::max(0.0f, jogador.intensidadeCorrida - dt * 10.0f);
                jogador.cicloPasso += dt * 1.6f;
            }
        } else {
            jogador.intensidadeAndar = 0.0f;
            jogador.intensidadeCorrida = 0.0f;
            if (temInput) {
                const float alvoX = movimento.x * kVelocidadeAr;
                const float alvoZ = movimento.z * kVelocidadeAr;
                const float tAr = 1.0f - std::exp(-kAceleracaoAr * dt);
                jogador.velocidadeX += (alvoX - jogador.velocidadeX) * tAr * 0.45f;
                jogador.velocidadeZ += (alvoZ - jogador.velocidadeZ) * tAr * 0.45f;
            }
            const float freio = 1.0f - std::exp(-1.2f * dt);
            jogador.velocidadeX *= (1.0f - freio * 0.35f);
            jogador.velocidadeZ *= (1.0f - freio * 0.35f);

            const float speed = std::sqrt(jogador.velocidadeX * jogador.velocidadeX + jogador.velocidadeZ * jogador.velocidadeZ);
            const float maxAr = std::max(kVelocidadeAr, kVelocidadeJogador * 0.85f);
            if (speed > maxAr) {
                const float escala = maxAr / speed;
                jogador.velocidadeX *= escala;
                jogador.velocidadeZ *= escala;
            }
        }

        // Direcao local do movimento (pra animacao de strafe / costas)
        {
            float alvoF = 0.0f;
            float alvoL = 0.0f;
            const float speed = ComprimentoXZ(Vector3{jogador.velocidadeX, 0.0f, jogador.velocidadeZ});
            if (speed > 0.15f) {
                const Vector3 frente = FrenteYaw(jogador.yaw);
                const Vector3 direita = DireitaDaCamera(jogador.yaw);
                const float inv = 1.0f / std::max(speed, 0.001f);
                const float nF = (jogador.velocidadeX * frente.x + jogador.velocidadeZ * frente.z) * inv;
                const float nL = (jogador.velocidadeX * direita.x + jogador.velocidadeZ * direita.z) * inv;
                if (jogador.miraTravada) {
                    alvoF = std::clamp(nF, -1.0f, 1.0f);
                    alvoL = std::clamp(nL, -1.0f, 1.0f);
                } else {
                    // Modo normal: sempre "frente" na animacao de andar
                    alvoF = 1.0f;
                    alvoL = 0.0f;
                }
            }
            const float suav = 1.0f - std::exp(-16.0f * dt);
            jogador.moveFrente += (alvoF - jogador.moveFrente) * suav;
            jogador.moveLado += (alvoL - jogador.moveLado) * suav;
            if (!jogador.noChao) {
                jogador.moveFrente *= 1.0f - std::exp(-8.0f * dt);
                jogador.moveLado *= 1.0f - std::exp(-8.0f * dt);
            }
        }

        if (morto) {
            jogador.velocidadeX = 0.0f;
            jogador.velocidadeZ = 0.0f;
            jogador.knockbackX = 0.0f;
            jogador.knockbackZ = 0.0f;
            jogador.intensidadeAndar = 0.0f;
            jogador.intensidadeCorrida = 0.0f;
            jogador.moveFrente = 0.0f;
            jogador.moveLado = 0.0f;
            jogador.agachar = 0.0f;
            jogador.ataque = 0.0f;
            jogador.bloqueio = 0.0f;
        }

        jogador.posicao.x += jogador.velocidadeX * dt + jogador.knockbackX * dt;
        jogador.posicao.z += jogador.velocidadeZ * dt + jogador.knockbackZ * dt;
        {
            const float amortecimento = 1.0f - std::exp(-8.0f * dt);
            jogador.knockbackX *= (1.0f - amortecimento);
            jogador.knockbackZ *= (1.0f - amortecimento);
            if (std::fabs(jogador.knockbackX) < 0.05f) jogador.knockbackX = 0.0f;
            if (std::fabs(jogador.knockbackZ) < 0.05f) jogador.knockbackZ = 0.0f;
        }

        if (!entradaBloqueada && IsKeyPressed(KEY_SPACE) && jogador.noChao && jogador.ataque <= 0.0f) {
            const float impulsoAgachar = jogador.agachar;
            jogador.velocidadeY = kForcaPulo + (kForcaPuloAgachado - kForcaPulo) * impulsoAgachar;
            jogador.noChao = false;
            jogador.fasePulo = 0.0f;
            jogador.agachar = 0.0f;
            jogador.bloqueio = 0.0f;
            jogador.intensidadeAndar = 0.0f;
            jogador.intensidadeCorrida = 0.0f;
        }

        jogador.velocidadeY -= kGravidade * dt;
        jogador.posicao.y += jogador.velocidadeY * dt;

        const float solo = AlturaSoloEm(jogador.posicao.x, jogador.posicao.z, kRaioJogador, jogador.posicao.y);
        if (jogador.velocidadeY <= 0.0f && jogador.posicao.y <= solo) {
            jogador.posicao.y = solo;
            jogador.velocidadeY = 0.0f;
            jogador.noChao = true;
            jogador.fasePulo = 0.0f;
        } else if (jogador.posicao.y > solo) {
            jogador.noChao = false;
            const float alvoFase = 0.5f - 0.5f * std::clamp(jogador.velocidadeY / kForcaPulo, -1.0f, 1.0f);
            jogador.fasePulo += (alvoFase - jogador.fasePulo) * (1.0f - std::exp(-9.0f * dt));
        }

        const float limite = gMapa.tamanho / 2.0f - kRaioJogador - 0.5f;
        jogador.posicao.x = std::clamp(jogador.posicao.x, -limite, limite);
        jogador.posicao.z = std::clamp(jogador.posicao.z, -limite, limite);
        ResolverColisaoMapa(&jogador.posicao, kRaioJogador);
        // Reaplica solo apos empurrar XZ (pode ter saido da plataforma)
        {
            const float soloApos = AlturaSoloEm(jogador.posicao.x, jogador.posicao.z, kRaioJogador, jogador.posicao.y);
            if (jogador.noChao && jogador.posicao.y > soloApos + 0.05f) {
                jogador.noChao = false;
            } else if (jogador.velocidadeY <= 0.0f && jogador.posicao.y < soloApos) {
                jogador.posicao.y = soloApos;
                jogador.velocidadeY = 0.0f;
                jogador.noChao = true;
            }
        }

        if (!entradaBloqueada) {
            for (int i = 0; i < quantidadeInimigos; ++i) {
                AtualizarInimigo(&inimigos[i], &jogador, dt, pedras, kMaxPedras);
            }
            AtualizarPedras(pedras, kMaxPedras, &jogador, dt);
            AtualizarBolasFogo(bolasFogo, kMaxBolasFogo, inimigos, quantidadeInimigos, dt);
            AtualizarQueimaInimigos(inimigos, quantidadeInimigos, dt);
        }
        if (!morto) {
            ResolverColisaoEntidades(&jogador, inimigos, quantidadeInimigos);
        }

        // Mantem boneco vivo na casa
        if (naCasa && quantidadeInimigos > 0 && !inimigos[0].vivo) {
            SpawnerBonecoTreino(&inimigos[0]);
        }

        const int vivos = ContarInimigosVivos(inimigos, quantidadeInimigos);

        // Retorno automatico apos completar dungeon
        if (emDungeon && retornoCasaTimer > 0.0f) {
            retornoCasaTimer -= dt;
            if (retornoCasaTimer <= 0.0f) {
                VoltarParaCasa(&jogador, inimigos, &quantidadeInimigos, pedras, bolasFogo,
                              &waveAtual, &aguardandoProximaWave, &timerWave, &wavesCompletas,
                              &retornoCasaTimer, &orbita);
                std::snprintf(msgStatus, sizeof(msgStatus), "Voltando para a Casa Base...");
                msgStatusTempo = 3.0f;
            }
        }

        // Waves da dungeon
        if (emDungeon && retornoCasaTimer <= 0.0f && !entradaBloqueada && !wavesCompletas &&
            jogador.vida > 0.0f) {
            if (aguardandoProximaWave) {
                timerWave -= dt;
                const bool forcar = IsKeyPressed(KEY_R);
                if (timerWave <= 0.0f || forcar) {
                    if (waveAtual < kTotalWaves) {
                        waveAtual += 1;
                        quantidadeInimigos = IniciarWaveDungeon(inimigos, waveAtual, gDungeonAtual);
                        aguardandoProximaWave = false;
                    }
                }
            } else if (vivos == 0) {
                GanharXp(&jogador, XpPorWave(gDungeonAtual, waveAtual));
                std::snprintf(msgStatus, sizeof(msgStatus), "Wave %d completa! +%.0f XP",
                              waveAtual, XpPorWave(gDungeonAtual, waveAtual));
                msgStatusTempo = 2.5f;

                if (waveAtual >= kTotalWaves) {
                    wavesCompletas = true;
                    const float bonus = XpPorDungeon(gDungeonAtual);
                    GanharXp(&jogador, bonus);
                    std::snprintf(msgStatus, sizeof(msgStatus),
                                  "%s concluida! +%.0f XP - voltando pra casa...",
                                  NomeDungeon(gDungeonAtual), bonus);
                    msgStatusTempo = 4.0f;
                    retornoCasaTimer = 3.2f;
                } else {
                    aguardandoProximaWave = true;
                    timerWave = kDelayEntreWaves;
                }
            }
        }

        // Tela de morte
        bool pediuSair = false;
        if (morto) {
            const AcaoTelaMorte acao = AtualizarTelaMorte();
            if (acao == AcaoTelaMorte::Renascer) {
                jogador.vida = kVidaJogadorMax;
                jogador.mana = kManaMax;
                jogador.posicao = naCasa ? Vector3{0.0f, 0.0f, 8.5f} : Vector3{0.0f, 0.0f, 6.0f};
                jogador.velocidadeX = 0.0f;
                jogador.velocidadeZ = 0.0f;
                jogador.velocidadeY = 0.0f;
                jogador.knockbackX = 0.0f;
                jogador.knockbackZ = 0.0f;
                jogador.hitFlash = 0.0f;
                jogador.ataque = 0.0f;
                jogador.bloqueio = 0.0f;
                jogador.agachar = 0.0f;
                jogador.noChao = true;
                gameOverAtivo = false;
                if (naCasa) {
                    SpawnerBonecoTreino(&inimigos[0]);
                    quantidadeInimigos = 1;
                } else if (emDungeon && waveAtual > 0 && !wavesCompletas) {
                    quantidadeInimigos = IniciarWaveDungeon(inimigos, waveAtual, gDungeonAtual);
                    aguardandoProximaWave = false;
                } else {
                    quantidadeInimigos = 0;
                }
            } else if (acao == AcaoTelaMorte::Sair) {
                pediuSair = true;
            }
        }
        if (pediuSair) break;

        if (EditorMapaAtivo()) {
            AtualizarModoEditor(dt, &camera);
        } else {
            AplicarCamera(&camera, jogador.posicao, orbita, jogador.agachar);
            camera.fovy = 60.0f;
        }

        BeginDrawing();
        ClearBackground(MapaCorCeu(gMapa));

        BeginMode3D(camera);
        DesenharMapa(tempo);
        if (naCasa) {
            DesenharBau(bau);
            DesenharMesaMagica();
            DesenharArmario();
            DesenharPortalDungeon(tempo);
            DesenharLuzesCasa(tempo);
        }
        if (!EditorMapaAtivo()) {
            DesenharPersonagem(jogador, tempo);
            if (!morto) {
                for (int i = 0; i < quantidadeInimigos; ++i) {
                    DesenharInimigo(inimigos[i], tempo);
                }
                DesenharPedras(pedras, kMaxPedras);
                DesenharBolasFogo(bolasFogo, kMaxBolasFogo);
            }
        } else if (EditorEstaConstruindo(gEditor)) {
            DesenharGhostEditor(tempo);
        }
        EndMode3D();

        if (EditorMapaAtivo()) {
            bool sairEditor = false;
            DesenharUiEditor(&sairEditor);
            if (sairEditor) {
                FecharEditorMapa(&jogador, &orbita, &cursorLivre, &cursorEstavaLivre);
            }
            EndDrawing();
            continue;
        }

        if (!morto) {
            for (int i = 0; i < quantidadeInimigos; ++i) {
                DesenharBarraVidaInimigo2D(inimigos[i], camera);
            }
        }
        DesenharHudJogador(jogador);
        DesenharPainelEquipamento(jogador);
        if (!morto && !naCasa) {
            DesenharMinimapa(jogador, inimigos, quantidadeInimigos);
        }

        if (!morto) {
            dungeonEscolhida = -1;
            if (menuStatsAberto) {
                AtualizarUIStats(&jogador, &menuStatsAberto);
            } else if (estacao == EstacaoAberta::Portal) {
                AtualizarUIPortal(&estacao, &mundoPortal, &dungeonEscolhida);
                if (dungeonEscolhida >= 0) {
                    EntrarDungeon(dungeonEscolhida, &jogador, inimigos, &quantidadeInimigos,
                                 pedras, bolasFogo, &waveAtual, &aguardandoProximaWave, &timerWave,
                                 &wavesCompletas, &retornoCasaTimer, &orbita);
                    std::snprintf(msgStatus, sizeof(msgStatus), "Entrou em: %s", NomeDungeon(dungeonEscolhida));
                    msgStatusTempo = 3.0f;
                }
            } else {
                AtualizarUIEstacoes(&estacao, &abaBau, &abaMesa, &jogador);
            }
        }

        if (!morto) {
            DrawRectangle(0, 0, GetScreenWidth(), 56, Color{16, 18, 28, 210});
            if (naCasa) {
                DrawText("WASD | E interagir | C atributos | Portal | 1/2 arma-magia | I/O zoom | T console",
                         12, 18, 16, RAYWHITE);
            } else {
                DrawText("WASD | C atributos | 1/2 arma-magia | I/O zoom | K mira | Esq atacar | R wave | T console",
                         12, 18, 16, RAYWHITE);
            }

            if (naCasa && pertoBau && estacao == EstacaoAberta::Nenhuma) {
                DrawText("E - Abrir bau", GetScreenWidth() / 2 - 70, GetScreenHeight() / 2 + 40, 22, Color{255, 230, 140, 255});
            } else if (naCasa && pertoMesa && estacao == EstacaoAberta::Nenhuma) {
                DrawText("E - Mesa magica", GetScreenWidth() / 2 - 90, GetScreenHeight() / 2 + 40, 22, Color{200, 170, 255, 255});
            } else if (naCasa && pertoArmario && estacao == EstacaoAberta::Nenhuma) {
                DrawText("E - Armario", GetScreenWidth() / 2 - 70, GetScreenHeight() / 2 + 40, 22, Color{220, 200, 160, 255});
            } else if (naCasa && pertoPortal && estacao == EstacaoAberta::Nenhuma) {
                DrawText("E - Portal das dungeons", GetScreenWidth() / 2 - 130, GetScreenHeight() / 2 + 40, 22, Color{180, 150, 255, 255});
            }

            if (naCasa) {
                DrawText("Casa Base  |  prepare-se e entre no portal",
                         20, 64, 18, Color{180, 210, 255, 255});
            } else if (retornoCasaTimer > 0.0f) {
                DrawText(TextFormat("Dungeon completa! Voltando em %.0fs", std::ceil(retornoCasaTimer)),
                         20, 64, 22, Color{180, 255, 160, 255});
            } else if (wavesCompletas) {
                DrawText("Dungeon concluida!", 20, 64, 20, GOLD);
            } else if (aguardandoProximaWave) {
                const int seg = static_cast<int>(std::ceil(std::max(0.0f, timerWave)));
                if (waveAtual == 0) {
                    DrawText(TextFormat("%s | Wave 1 em %ds (R) - %d inimigos",
                                        NomeDungeon(gDungeonAtual), seg,
                                        ContarInimigosDaWave(1, gDungeonAtual)),
                             20, 64, 18, Color{255, 160, 120, 255});
                } else if (waveAtual + 1 >= kTotalWaves) {
                    DrawText(TextFormat("CHEFE em %ds (ou R)", seg),
                             20, 64, 20, Color{255, 120, 60, 255});
                } else {
                    DrawText(TextFormat("Wave %d em %ds (R) - %d inimigos",
                                        waveAtual + 1, seg,
                                        ContarInimigosDaWave(waveAtual + 1, gDungeonAtual)),
                             20, 64, 18, Color{255, 180, 100, 255});
                }
            } else if (waveAtual >= kTotalWaves) {
                DrawText(TextFormat("%s | Wave %d/%d  CHEFE", NomeDungeon(gDungeonAtual), waveAtual, kTotalWaves),
                         20, 64, 20, Color{255, 130, 70, 255});
            } else {
                DrawText(TextFormat("%s | Wave %d/%d  |  Inimigos: %d",
                                    NomeDungeon(gDungeonAtual), waveAtual, kTotalWaves, vivos),
                         20, 64, 20, Color{255, 200, 120, 255});
            }

            if (msgStatusTempo > 0.0f) {
                DrawText(msgStatus, 20, 88, 18, Color{160, 255, 180, 255});
            } else if (naCasa && !jogador.temEspada && !jogador.temMagiaFogo && estacao == EstacaoAberta::Nenhuma) {
                DrawText("Pegue a espada no bau e a magia na mesa", 20, 90, 18, Color{210, 190, 140, 255});
            } else if (jogador.usandoMagia) {
                DrawText("MAGIA DE FOGO", 20, 90, 22, Color{255, 140, 60, 255});
            } else if (jogador.miraTravada) {
                DrawText("MIRA TRAVADA", 20, 90, 22, Color{255, 210, 90, 255});
            } else if (jogador.bloqueio > 0.5f) {
                DrawText("BLOQUEANDO", 20, 90, 22, Color{120, 200, 255, 255});
            } else if (jogador.ataque > 0.0f) {
                DrawText("ATACANDO", 20, 90, 22, Color{255, 180, 80, 255});
            }

            if (jogador.hitFlash > 0.0f) {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                              Color{180, 20, 20, static_cast<unsigned char>(70.0f * (jogador.hitFlash / 0.28f))});
            }

            if (mouseLivre && estacao == EstacaoAberta::Nenhuma && !consoleAberto) {
                DrawText("Cursor livre", 20, 116, 18, GOLD);
            }

            DesenharConsoleComandos(console);
        } else {
            DesenharTelaMorte();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

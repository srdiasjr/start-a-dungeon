#include "raylib.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>

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
constexpr float kDistanciaCamera = 4.3f;
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
constexpr float kDanoAtaque = 25.0f;
constexpr float kVidaInimigoMax = 100.0f;
constexpr float kVelocidadeInimigo = 3.2f;
constexpr float kVidaJogadorMax = 100.0f;
constexpr float kDanoInimigo = 14.0f;
constexpr float kAlcanceAtaqueInimigo = 1.85f;
constexpr float kDuracaoAtaqueInimigo = 0.58f;
constexpr float kCooldownAtaqueInimigo = 1.15f;
constexpr float kXpPorKill = 40.0f;
constexpr float kXpBaseNivel = 100.0f;

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
const Color kCorMadeiraCabo{72, 42, 22, 255};
const Color kCorMadeiraGuarda{110, 72, 38, 255};
const Color kCorMadeiraLamina{148, 102, 55, 255};
const Color kCorMadeiraPonta{175, 128, 72, 255};
constexpr float kTempoSpawnInimigo = 10.0f;
constexpr float kDistanciaBau = 2.2f;

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
    bool jaAcertouGolpe;
    bool noChao;
};

struct Bau {
    Vector3 posicao;
    float tampaAngulo;
    bool aberto;
    bool lootColetado;
};

struct Inimigo {
    Vector3 posicao;
    float yaw;
    float vida;
    float hitFlash;
    float cicloPasso;
    float ataque;
    float cooldownAtaque;
    float knockbackX;
    float knockbackZ;
    float recuo;        // 0..1 reacao ao ser atingido
    bool jaAcertouGolpe;
    bool vivo;
};

struct CameraOrbit {
    float yaw;
    float pitch;
};

struct Obstaculo {
    float x;
    float z;
    float halfX;
    float halfZ;
};

// Colisao do mapa (fonte, pilares, rochas, muros)
constexpr Obstaculo kObstaculos[] = {
    // Fonte central
    {0.0f, 0.0f, 1.35f, 1.35f},
    // Pilares da praca
    {-7.5f, -7.5f, 0.55f, 0.55f},
    {7.5f, -7.5f, 0.55f, 0.55f},
    {-7.5f, 7.5f, 0.55f, 0.55f},
    {7.5f, 7.5f, 0.55f, 0.55f},
    // Muros quebrados
    {-3.0f, -11.5f, 3.2f, 0.45f},
    {4.5f, -11.5f, 2.4f, 0.45f},
    {11.5f, -2.0f, 0.45f, 2.8f},
    {11.5f, 5.5f, 0.45f, 2.2f},
    {-11.5f, 1.5f, 0.45f, 3.4f},
    // Rochas
    {-9.0f, 4.5f, 1.1f, 0.9f},
    {8.5f, -5.0f, 1.3f, 1.0f},
    {-5.5f, 10.0f, 1.0f, 1.2f},
    {10.0f, 9.0f, 0.9f, 0.8f},
    // Barricadas de madeira
    {-2.5f, 6.5f, 1.6f, 0.35f},
    {3.0f, -6.0f, 0.35f, 1.5f},
    // Troncos de arvores
    {-14.0f, -12.0f, 0.45f, 0.45f},
    {-16.0f, -6.0f, 0.50f, 0.50f},
    {-15.0f, 8.0f, 0.45f, 0.45f},
    {-13.5f, 14.0f, 0.55f, 0.55f},
    {14.0f, -13.0f, 0.45f, 0.45f},
    {16.0f, -4.0f, 0.50f, 0.50f},
    {15.0f, 7.0f, 0.45f, 0.45f},
    {13.0f, 14.5f, 0.50f, 0.50f},
    {-6.0f, -16.0f, 0.45f, 0.45f},
    {5.0f, -15.5f, 0.50f, 0.50f},
    {-8.0f, 16.0f, 0.45f, 0.45f},
    {6.5f, 15.5f, 0.50f, 0.50f},
};

float ComprimentoXZ(Vector3 v);
Vector3 FrenteYaw(float yawGraus);
void DesenharEspada(bool madeira);

void ResolverColisaoMapa(Vector3* pos, float raio) {
    for (const Obstaculo& o : kObstaculos) {
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
            continue;
        }

        if (d2 < raio * raio) {
            const float d = std::sqrt(d2);
            const float push = (raio - d) / d;
            pos->x += dx * push;
            pos->z += dz * push;
        }
    }
}

void DesenharArvore(float x, float z, float escala, float tom) {
    const Color tronco{
        static_cast<unsigned char>(78 + tom * 12),
        static_cast<unsigned char>(52 + tom * 4),
        32, 255};
    const Color folha{
        static_cast<unsigned char>(42 + tom * 18),
        static_cast<unsigned char>(98 + tom * 20),
        static_cast<unsigned char>(48 + tom * 8),
        255};
    const Color folhaEscura{
        static_cast<unsigned char>(28 + tom * 10),
        static_cast<unsigned char>(72 + tom * 12),
        static_cast<unsigned char>(36 + tom * 6),
        255};

    DrawCube(Vector3{x, 1.1f * escala, z}, 0.35f * escala, 2.2f * escala, 0.35f * escala, tronco);
    DrawSphere(Vector3{x, 2.6f * escala, z}, 1.15f * escala, folha);
    DrawSphere(Vector3{x + 0.55f * escala, 2.3f * escala, z - 0.2f * escala}, 0.75f * escala, folhaEscura);
    DrawSphere(Vector3{x - 0.45f * escala, 2.4f * escala, z + 0.35f * escala}, 0.70f * escala, folha);
}

void DesenharRocha(float x, float z, float sx, float sy, float sz, float yaw) {
    rlPushMatrix();
    rlTranslatef(x, sy * 0.5f, z);
    rlRotatef(yaw, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, sx, sy, sz, Color{92, 88, 80, 255});
    DrawCube(Vector3{sx * 0.2f, sy * 0.15f, -sz * 0.1f}, sx * 0.7f, sy * 0.7f, sz * 0.7f, Color{110, 106, 96, 255});
    rlPopMatrix();
}

void DesenharMapa(float tempo) {
    const float half = kTamanhoArena * 0.5f;

    // Grama externa
    DrawCube(Vector3{0.0f, -0.18f, 0.0f}, kTamanhoArena + 8.0f, 0.28f, kTamanhoArena + 8.0f, Color{58, 92, 48, 255});
    DrawCube(Vector3{0.0f, -0.05f, 0.0f}, kTamanhoArena, 0.12f, kTamanhoArena, Color{66, 104, 54, 255});

    // Praca de pedra central
    constexpr int kTiles = 18;
    const float praca = 18.0f;
    const float tile = praca / kTiles;
    const float origem = -praca * 0.5f;
    for (int z = 0; z < kTiles; ++z) {
        for (int x = 0; x < kTiles; ++x) {
            const bool claro = ((x + z) % 2) == 0;
            const bool borda = x == 0 || z == 0 || x == kTiles - 1 || z == kTiles - 1;
            Color cor = claro ? Color{168, 156, 132, 255} : Color{148, 136, 114, 255};
            if (borda) cor = Color{120, 110, 92, 255};
            DrawCube(
                Vector3{origem + tile * (x + 0.5f), 0.01f, origem + tile * (z + 0.5f)},
                tile * 0.98f, 0.08f, tile * 0.98f, cor);
        }
    }

    // Caminho de terra ate o sul
    DrawCube(Vector3{0.0f, 0.015f, 10.5f}, 3.2f, 0.06f, 12.0f, Color{120, 96, 68, 255});
    DrawCube(Vector3{0.0f, 0.02f, 10.5f}, 2.4f, 0.05f, 12.0f, Color{138, 110, 78, 255});

    // Anel de terra ao redor da praca
    DrawCube(Vector3{0.0f, 0.0f, -10.2f}, 20.0f, 0.05f, 1.8f, Color{110, 90, 62, 255});
    DrawCube(Vector3{0.0f, 0.0f, 10.2f}, 20.0f, 0.05f, 1.8f, Color{110, 90, 62, 255});
    DrawCube(Vector3{-10.2f, 0.0f, 0.0f}, 1.8f, 0.05f, 18.0f, Color{110, 90, 62, 255});
    DrawCube(Vector3{10.2f, 0.0f, 0.0f}, 1.8f, 0.05f, 18.0f, Color{110, 90, 62, 255});

    // Fonte central
    DrawCylinder(Vector3{0.0f, 0.0f, 0.0f}, 1.5f, 1.5f, 0.35f, 20, Color{130, 124, 112, 255});
    DrawCylinder(Vector3{0.0f, 0.35f, 0.0f}, 1.15f, 1.25f, 0.45f, 20, Color{150, 144, 130, 255});
    DrawCylinder(Vector3{0.0f, 0.55f, 0.0f}, 0.95f, 0.95f, 0.08f, 16, Color{70, 130, 170, 220});
    DrawCube(Vector3{0.0f, 1.0f, 0.0f}, 0.28f, 1.0f, 0.28f, Color{145, 138, 124, 255});
    DrawSphere(Vector3{0.0f, 1.55f, 0.0f}, 0.22f, Color{175, 168, 150, 255});
    // "Agua" animada simples
    const float onda = 0.04f + 0.02f * std::sin(tempo * 2.4f);
    DrawCylinder(Vector3{0.0f, 0.58f + onda * 0.5f, 0.0f}, 0.55f, 0.55f, 0.05f, 12, Color{110, 175, 210, 160});

    // Pilares
    const Vector3 pilares[] = {{-7.5f, 0, -7.5f}, {7.5f, 0, -7.5f}, {-7.5f, 0, 7.5f}, {7.5f, 0, 7.5f}};
    for (const Vector3& p : pilares) {
        DrawCube(Vector3{p.x, 1.5f, p.z}, 0.9f, 3.0f, 0.9f, Color{145, 136, 118, 255});
        DrawCube(Vector3{p.x, 3.15f, p.z}, 1.2f, 0.35f, 1.2f, Color{160, 150, 130, 255});
        DrawCube(Vector3{p.x, 0.15f, p.z}, 1.3f, 0.3f, 1.3f, Color{120, 112, 96, 255});
        // Tocha
        DrawCube(Vector3{p.x, 2.4f, p.z - 0.55f}, 0.12f, 0.5f, 0.12f, Color{70, 48, 30, 255});
        const float flicker = 0.85f + 0.15f * std::sin(tempo * 11.0f + p.x * 0.7f);
        DrawSphere(Vector3{p.x, 2.75f, p.z - 0.55f}, 0.18f * flicker, Color{255, 160, 60, 255});
        DrawSphere(Vector3{p.x, 2.85f, p.z - 0.55f}, 0.10f * flicker, Color{255, 230, 140, 230});
    }

    // Muros de ruina
    const Color pedra{128, 120, 104, 255};
    const Color pedraEscura{98, 92, 80, 255};
    DrawCube(Vector3{-3.0f, 1.1f, -11.5f}, 6.4f, 2.2f, 0.7f, pedra);
    DrawCube(Vector3{-4.5f, 2.4f, -11.5f}, 2.0f, 0.7f, 0.75f, pedraEscura);
    DrawCube(Vector3{4.5f, 0.9f, -11.5f}, 4.8f, 1.8f, 0.7f, pedra);
    DrawCube(Vector3{5.5f, 2.0f, -11.5f}, 1.6f, 0.9f, 0.75f, pedraEscura);

    DrawCube(Vector3{11.5f, 1.2f, -2.0f}, 0.7f, 2.4f, 5.6f, pedra);
    DrawCube(Vector3{11.5f, 1.0f, 5.5f}, 0.7f, 2.0f, 4.4f, pedraEscura);
    DrawCube(Vector3{-11.5f, 1.3f, 1.5f}, 0.7f, 2.6f, 6.8f, pedra);

    // Barricadas
    DrawCube(Vector3{-2.5f, 0.55f, 6.5f}, 3.2f, 1.1f, 0.45f, Color{96, 68, 42, 255});
    DrawCube(Vector3{-2.5f, 1.0f, 6.5f}, 3.0f, 0.25f, 0.55f, Color{110, 78, 48, 255});
    DrawCube(Vector3{3.0f, 0.55f, -6.0f}, 0.45f, 1.1f, 3.0f, Color{96, 68, 42, 255});

    // Rochas
    DesenharRocha(-9.0f, 4.5f, 2.2f, 1.3f, 1.8f, 25.0f);
    DesenharRocha(8.5f, -5.0f, 2.5f, 1.5f, 2.0f, -35.0f);
    DesenharRocha(-5.5f, 10.0f, 2.0f, 1.2f, 2.3f, 60.0f);
    DesenharRocha(10.0f, 9.0f, 1.8f, 1.1f, 1.6f, 10.0f);

    // Arvores ao redor
    DesenharArvore(-14.0f, -12.0f, 1.15f, 0.2f);
    DesenharArvore(-16.0f, -6.0f, 1.35f, 0.6f);
    DesenharArvore(-15.0f, 8.0f, 1.20f, 0.1f);
    DesenharArvore(-13.5f, 14.0f, 1.45f, 0.8f);
    DesenharArvore(14.0f, -13.0f, 1.25f, 0.4f);
    DesenharArvore(16.0f, -4.0f, 1.40f, 0.7f);
    DesenharArvore(15.0f, 7.0f, 1.15f, 0.3f);
    DesenharArvore(13.0f, 14.5f, 1.30f, 0.9f);
    DesenharArvore(-6.0f, -16.0f, 1.20f, 0.5f);
    DesenharArvore(5.0f, -15.5f, 1.35f, 0.2f);
    DesenharArvore(-8.0f, 16.0f, 1.25f, 0.6f);
    DesenharArvore(6.5f, 15.5f, 1.40f, 0.4f);
    DesenharArvore(-17.5f, 0.5f, 1.1f, 0.9f);
    DesenharArvore(17.5f, 2.0f, 1.2f, 0.3f);

    // Arbustos
    const Vector3 arbustos[] = {
        {-10.5f, 0, -8.0f}, {-4.0f, 0, 12.5f}, {6.0f, 0, 12.0f},
        {12.0f, 0, -8.5f}, {-12.0f, 0, 6.0f}, {2.5f, 0, -13.0f},
    };
    for (const Vector3& a : arbustos) {
        DrawSphere(Vector3{a.x, 0.45f, a.z}, 0.55f, Color{48, 96, 42, 255});
        DrawSphere(Vector3{a.x + 0.35f, 0.35f, a.z + 0.1f}, 0.40f, Color{40, 82, 36, 255});
    }

    // Limite / muralha baixa da arena
    const Color muroLimite{76, 88, 70, 255};
    DrawCube(Vector3{0.0f, 0.7f, -half - 0.4f}, kTamanhoArena + 2.0f, 1.4f, 0.7f, muroLimite);
    DrawCube(Vector3{0.0f, 0.7f, half + 0.4f}, kTamanhoArena + 2.0f, 1.4f, 0.7f, muroLimite);
    DrawCube(Vector3{-half - 0.4f, 0.7f, 0.0f}, 0.7f, 1.4f, kTamanhoArena + 2.0f, muroLimite);
    DrawCube(Vector3{half + 0.4f, 0.7f, 0.0f}, 0.7f, 1.4f, kTamanhoArena + 2.0f, muroLimite);

    // Colinas de fundo (silhueta)
    DrawCube(Vector3{-22.0f, 2.0f, -28.0f}, 14.0f, 6.0f, 8.0f, Color{70, 95, 78, 255});
    DrawCube(Vector3{18.0f, 1.6f, -30.0f}, 16.0f, 5.0f, 9.0f, Color{62, 88, 70, 255});
    DrawCube(Vector3{0.0f, 1.2f, 30.0f}, 22.0f, 4.0f, 7.0f, Color{68, 92, 74, 255});
}

void DesenharMinimapa(const Personagem& jogador, const Inimigo& inimigo) {
    const float mx = 18.0f;
    const float my = 70.0f;
    const float size = 168.0f;
    const float half = kTamanhoArena * 0.5f;

    DrawRectangleRounded(Rectangle{mx - 6, my - 6, size + 12, size + 12}, 0.08f, 6, Color{10, 14, 18, 210});
    DrawRectangle(static_cast<int>(mx), static_cast<int>(my), static_cast<int>(size), static_cast<int>(size), Color{48, 78, 42, 255});
    // Praca
    const float praca = 18.0f;
    const float px = mx + ((-praca * 0.5f) / kTamanhoArena + 0.5f) * size;
    const float pz = my + ((-praca * 0.5f) / kTamanhoArena + 0.5f) * size;
    const float pw = (praca / kTamanhoArena) * size;
    DrawRectangle(static_cast<int>(px), static_cast<int>(pz), static_cast<int>(pw), static_cast<int>(pw), Color{150, 140, 118, 255});

    auto paraMapa = [&](float wx, float wz) -> Vector2 {
        return Vector2{
            mx + (wx / kTamanhoArena + 0.5f) * size,
            my + (wz / kTamanhoArena + 0.5f) * size,
        };
    };

    for (const Obstaculo& o : kObstaculos) {
        const Vector2 c = paraMapa(o.x, o.z);
        const float w = (o.halfX * 2.0f / kTamanhoArena) * size;
        const float h = (o.halfZ * 2.0f / kTamanhoArena) * size;
        DrawRectangle(static_cast<int>(c.x - w * 0.5f), static_cast<int>(c.y - h * 0.5f),
                      std::max(2, static_cast<int>(w)), std::max(2, static_cast<int>(h)),
                      Color{70, 66, 58, 230});
    }

    if (inimigo.vivo) {
        const Vector2 ie = paraMapa(inimigo.posicao.x, inimigo.posicao.z);
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

    camera->position = Vector3{
        pivot.x - frente.x * kDistanciaCamera,
        pivot.y - frente.y * kDistanciaCamera,
        pivot.z - frente.z * kDistanciaCamera,
    };
    camera->target = Vector3{
        pivot.x + frente.x * 2.5f,
        pivot.y + frente.y * 2.5f,
        pivot.z + frente.z * 2.5f,
    };
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
    const Color corpo = flash > 0.0f
        ? Color{255, 220, 220, 255}
        : kCorInimigo;
    const Color detalhe = flash > 0.0f ? RAYWHITE : kCorInimigoEscuro;

    const float carrega = SmoothStep(std::clamp(inimigo.ataque / 0.30f, 0.0f, 1.0f));
    const float golpe = SmoothStep(std::clamp((inimigo.ataque - 0.28f) / 0.30f, 0.0f, 1.0f));
    const float volta = SmoothStep(std::clamp((inimigo.ataque - 0.65f) / 0.35f, 0.0f, 1.0f));
    const float noGolpe = (inimigo.ataque > 0.001f) ? 1.0f : 0.0f;
    const float r = inimigo.recuo;

    const float bob = std::fabs(std::sin(inimigo.cicloPasso)) * 0.05f * (1.0f - r) * (1.0f - noGolpe * 0.5f);
    const float swing = std::sin(inimigo.cicloPasso) * 28.0f * (1.0f - noGolpe) * (1.0f - r);
    const float inclinacao = -golpe * 12.0f * noGolpe + r * 22.0f;
    const float torsao = LerpF(8.0f * carrega, -14.0f * golpe, golpe) * noGolpe - r * 10.0f;

    rlPushMatrix();
    rlTranslatef(inimigo.posicao.x, inimigo.posicao.y + bob, inimigo.posicao.z);
    rlRotatef(inimigo.yaw, 0.0f, 1.0f, 0.0f);
    rlRotatef(torsao, 0.0f, 1.0f, 0.0f);
    rlRotatef(inclinacao, 1.0f, 0.0f, 0.0f);

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

    // Braco esquerdo
    const float pitchEsq = -swing * 0.8f + carrega * -35.0f * noGolpe + r * -25.0f;
    rlPushMatrix();
    rlTranslatef(-0.30f, 1.30f, 0.0f);
    rlRotatef(pitchEsq, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, -0.22f, 0.0f}, 0.12f, 0.42f, 0.12f, corpo);
    rlPopMatrix();

    // Braco direito (ataque)
    const float pitchDir = LerpF(
        LerpF(swing * 0.8f, 45.0f, carrega),
        LerpF(-95.0f, -20.0f, volta),
        golpe) * noGolpe
        + swing * 0.8f * (1.0f - noGolpe)
        + r * 30.0f;
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

    const Vector3 topo{inimigo.posicao.x, inimigo.posicao.y + 2.05f, inimigo.posicao.z};
    const Vector2 tela = GetWorldToScreen(topo, camera);
    if (tela.y < -40.0f || tela.y > GetScreenHeight() + 40.0f) return;
    if (tela.x < -80.0f || tela.x > GetScreenWidth() + 80.0f) return;

    const float w = 78.0f;
    const float h = 10.0f;
    const float x = tela.x - w * 0.5f;
    const float y = tela.y - 8.0f;
    const float pct = inimigo.vida / kVidaInimigoMax;
    DesenharBarra(x, y, w, h, pct, Color{220, 70, 70, 255}, Color{20, 20, 28, 220});
}

void DesenharHudJogador(const Personagem& jogador) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    const float painelW = 268.0f;
    const float painelH = 96.0f;
    const float x = static_cast<float>(sw) - painelW - 18.0f;
    const float y = static_cast<float>(sh) - painelH - 18.0f;

    DrawRectangleRounded(Rectangle{x, y, painelW, painelH}, 0.12f, 8, Color{12, 14, 22, 220});
    DrawRectangleRoundedLines(Rectangle{x, y, painelW, painelH}, 0.12f, 8, Color{70, 90, 130, 180});

    DrawText(TextFormat("Nv. %d", jogador.nivel), static_cast<int>(x + 14), static_cast<int>(y + 10), 18, Color{200, 210, 230, 255});

    const float vidaPct = jogador.vida / kVidaJogadorMax;
    DrawText("HP", static_cast<int>(x + 14), static_cast<int>(y + 36), 16, Color{255, 140, 140, 255});
    DesenharBarra(x + 46, y + 38, 170, 14, vidaPct, Color{210, 55, 65, 255}, Color{35, 28, 36, 255});
    DrawText(TextFormat("%.0f/%.0f", jogador.vida, kVidaJogadorMax),
             static_cast<int>(x + 222), static_cast<int>(y + 36), 14, Color{230, 200, 200, 255});

    const float xpMax = kXpBaseNivel * static_cast<float>(jogador.nivel);
    const float xpPct = (xpMax > 0.0f) ? (jogador.xp / xpMax) : 0.0f;
    DrawText("XP", static_cast<int>(x + 14), static_cast<int>(y + 64), 16, Color{140, 210, 255, 255});
    DesenharBarra(x + 46, y + 66, 170, 14, xpPct, Color{70, 160, 230, 255}, Color{28, 34, 48, 255});
    DrawText(TextFormat("%.0f/%.0f", jogador.xp, xpMax),
             static_cast<int>(x + 222), static_cast<int>(y + 64), 14, Color{190, 220, 245, 255});
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
    if (jogador.temEspada) {
        DesenharSlotEquipamento(xMeio, y0, size, "Madeira", true);
        DesenharIconeEspadaHud(xMeio + size * 0.5f, y0 + size * 0.42f, jogador.espadaMadeira);
    } else {
        DesenharSlotEquipamento(xMeio, y0, size, "Arma", false);
        DrawText("-", static_cast<int>(xMeio + size * 0.5f - 4), static_cast<int>(y0 + size * 0.35f), 22, Color{90, 100, 115, 180});
    }

    const float xDir = x0 + (size + gap) * 2.0f;
    DesenharSlotEquipamento(xDir, y0, size, "Magia", false);
    DrawText("-", static_cast<int>(xDir + size * 0.5f - 4), static_cast<int>(y0 + size * 0.35f), 22, Color{90, 100, 115, 180});
}

void DesenharBau(const Bau& bau) {
    rlPushMatrix();
    rlTranslatef(bau.posicao.x, bau.posicao.y, bau.posicao.z);

    DrawCube(Vector3{0.0f, 0.28f, 0.0f}, 1.1f, 0.55f, 0.75f, Color{118, 78, 38, 255});
    DrawCube(Vector3{0.0f, 0.28f, 0.0f}, 1.18f, 0.08f, 0.82f, Color{92, 58, 28, 255});
    DrawCube(Vector3{0.0f, 0.12f, 0.38f}, 0.18f, 0.18f, 0.08f, Color{180, 150, 60, 255});

    rlPushMatrix();
    rlTranslatef(0.0f, 0.55f, -0.35f);
    rlRotatef(-bau.tampaAngulo, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.08f, 0.35f}, 1.12f, 0.14f, 0.78f, Color{138, 92, 48, 255});
    DrawCube(Vector3{0.0f, 0.08f, 0.35f}, 1.0f, 0.06f, 0.68f, Color{160, 110, 60, 255});
    rlPopMatrix();

    if (bau.aberto && !bau.lootColetado) {
        rlPushMatrix();
        rlTranslatef(0.0f, 0.85f, 0.05f);
        rlRotatef(25.0f, 0.0f, 0.0f, 1.0f);
        rlRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        rlScalef(0.55f, 0.55f, 0.55f);
        DesenharEspada(true);
        rlPopMatrix();
    }

    rlPopMatrix();
}

// Retorna true se coletou a espada neste frame
bool AtualizarLootBau(Bau* bau) {
    if (!bau->aberto || bau->lootColetado) return false;

    const float pw = 280.0f;
    const float ph = 220.0f;
    const float px = (GetScreenWidth() - pw) * 0.5f;
    const float py = (GetScreenHeight() - ph) * 0.5f;
    const Rectangle painel{px, py, pw, ph};
    const Rectangle areaEspada{px + 90.0f, py + 50.0f, 100.0f, 110.0f};

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 110});
    DrawRectangleRounded(painel, 0.08f, 8, Color{18, 20, 28, 240});
    DrawRectangleRoundedLines(painel, 0.08f, 8, Color{180, 150, 80, 220});
    DrawText("Bau aberto", static_cast<int>(px + 84), static_cast<int>(py + 16), 22, Color{240, 220, 160, 255});
    DrawText("Clique na espada para coletar", static_cast<int>(px + 28), static_cast<int>(py + 175), 18, Color{200, 205, 220, 255});

    const bool hover = CheckCollisionPointRec(GetMousePosition(), areaEspada);
    DrawRectangleRounded(areaEspada, 0.12f, 6, hover ? Color{45, 38, 28, 255} : Color{30, 26, 20, 255});
    DrawRectangleRoundedLines(areaEspada, 0.12f, 6, hover ? Color{230, 190, 100, 255} : Color{140, 110, 60, 220});
    DesenharIconeEspadaHud(px + 140.0f, py + 100.0f, true);
    DrawText("Espada de madeira", static_cast<int>(px + 68), static_cast<int>(py + 148), 16, Color{210, 170, 110, 255});

    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        bau->lootColetado = true;
        return true;
    }
    return false;
}

void SpawnerInimigo(Inimigo* inimigo) {
    inimigo->posicao = Vector3{
        static_cast<float>(GetRandomValue(-8, 8)),
        0.0f,
        static_cast<float>(GetRandomValue(-8, 8)),
    };
    if (std::fabs(inimigo->posicao.x) < 2.5f && std::fabs(inimigo->posicao.z) < 2.5f) {
        inimigo->posicao.x = 6.0f;
        inimigo->posicao.z = -5.0f;
    }
    ResolverColisaoMapa(&inimigo->posicao, kRaioInimigo);
    inimigo->vida = kVidaInimigoMax;
    inimigo->vivo = true;
    inimigo->hitFlash = 0.0f;
    inimigo->ataque = 0.0f;
    inimigo->cooldownAtaque = 0.5f;
    inimigo->knockbackX = 0.0f;
    inimigo->knockbackZ = 0.0f;
    inimigo->recuo = 0.0f;
    inimigo->yaw = 180.0f;
}

float XpNecessario(int nivel) {
    return kXpBaseNivel * static_cast<float>(std::max(1, nivel));
}

void GanharXp(Personagem* jogador, float quantidade) {
    jogador->xp += quantidade;
    while (jogador->xp >= XpNecessario(jogador->nivel)) {
        jogador->xp -= XpNecessario(jogador->nivel);
        jogador->nivel += 1;
        jogador->vida = std::min(kVidaJogadorMax, jogador->vida + 25.0f);
    }
}

void AplicarDanoJogador(Personagem* jogador, float dano, Vector3 origemKnock) {
    float danoFinal = dano;
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

void AtualizarInimigo(Inimigo* inimigo, Personagem* jogador, float dt) {
    inimigo->hitFlash = std::max(0.0f, inimigo->hitFlash - dt);
    inimigo->recuo = std::max(0.0f, inimigo->recuo - dt * 2.4f);
    inimigo->cooldownAtaque = std::max(0.0f, inimigo->cooldownAtaque - dt);

    // Knockback suave
    inimigo->posicao.x += inimigo->knockbackX * dt;
    inimigo->posicao.z += inimigo->knockbackZ * dt;
    const float amortecimento = 1.0f - std::exp(-7.0f * dt);
    inimigo->knockbackX *= (1.0f - amortecimento);
    inimigo->knockbackZ *= (1.0f - amortecimento);
    if (std::fabs(inimigo->knockbackX) < 0.05f) inimigo->knockbackX = 0.0f;
    if (std::fabs(inimigo->knockbackZ) < 0.05f) inimigo->knockbackZ = 0.0f;

    if (!inimigo->vivo) return;

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

    const bool emRecuo = inimigo->recuo > 0.15f || ComprimentoXZ(Vector3{inimigo->knockbackX, 0, inimigo->knockbackZ}) > 0.8f;

    if (inimigo->ataque > 0.0f) {
        inimigo->ataque += dt / kDuracaoAtaqueInimigo;
        inimigo->cicloPasso += dt * 2.0f;

        if (!inimigo->jaAcertouGolpe &&
            inimigo->ataque >= 0.35f && inimigo->ataque <= 0.62f &&
            dist <= kAlcanceAtaqueInimigo && jogador->vida > 0.0f) {
            AplicarDanoJogador(jogador, kDanoInimigo, inimigo->posicao);
            inimigo->jaAcertouGolpe = true;
        }

        if (inimigo->ataque >= 1.0f) {
            inimigo->ataque = 0.0f;
            inimigo->cooldownAtaque = kCooldownAtaqueInimigo;
        }
    } else if (!emRecuo) {
        if (dist > 1.55f) {
            inimigo->posicao.x += paraJogador.x * kVelocidadeInimigo * dt;
            inimigo->posicao.z += paraJogador.z * kVelocidadeInimigo * dt;
            inimigo->cicloPasso += dt * 9.0f;
        } else {
            inimigo->cicloPasso += dt * 2.0f;
            if (inimigo->cooldownAtaque <= 0.0f && dist <= kAlcanceAtaqueInimigo + 0.25f) {
                inimigo->ataque = 0.001f;
                inimigo->jaAcertouGolpe = false;
            }
        }
    } else {
        inimigo->cicloPasso += dt * 1.5f;
    }

    const float limite = kTamanhoArena / 2.0f - 1.0f;
    inimigo->posicao.x = std::clamp(inimigo->posicao.x, -limite, limite);
    inimigo->posicao.z = std::clamp(inimigo->posicao.z, -limite, limite);
    ResolverColisaoMapa(&inimigo->posicao, kRaioInimigo);
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
    const Vector3 centroInimigo{
        inimigo.posicao.x,
        inimigo.posicao.y + 1.0f,
        inimigo.posicao.z,
    };

    const float dx = pontoAtaque.x - centroInimigo.x;
    const float dy = pontoAtaque.y - centroInimigo.y;
    const float dz = pontoAtaque.z - centroInimigo.z;
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist > kAlcanceAtaque) return false;

    // Precisa estar na frente do jogador
    const float dot = (centroInimigo.x - origem.x) * frente.x + (centroInimigo.z - origem.z) * frente.z;
    return dot > 0.35f;
}
}  // namespace

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(kLarguraTela, kAlturaTela, "Jogo Novo - Combate");
    SetTargetFPS(60);
    DisableCursor();

    Personagem jogador{};
    jogador.posicao = Vector3{0.0f, 0.0f, 5.5f};
    jogador.yaw = 0.0f;
    jogador.noChao = true;
    jogador.vida = kVidaJogadorMax;
    jogador.xp = 0.0f;
    jogador.nivel = 1;
    jogador.temEspada = false;
    jogador.espadaMadeira = false;

    Inimigo inimigo{};
    inimigo.vivo = false;
    inimigo.vida = 0.0f;

    Bau bau{};
    bau.posicao = Vector3{2.8f, 0.0f, 4.2f};
    bau.tampaAngulo = 0.0f;
    bau.aberto = false;
    bau.lootColetado = false;

    CameraOrbit orbita{};
    orbita.yaw = 0.0f;
    orbita.pitch = -12.0f;

    Camera3D camera{};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    AplicarCamera(&camera, jogador.posicao, orbita, 0.0f);

    bool cursorLivre = false;
    bool cursorEstavaLivre = false;
    float timerSpawnInimigo = kTempoSpawnInimigo;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        const float tempo = static_cast<float>(GetTime());

        if (IsKeyPressed(KEY_X) && !(bau.aberto && !bau.lootColetado)) {
            cursorLivre = !cursorLivre;
        }

        const bool mouseLivre = cursorLivre || (bau.aberto && !bau.lootColetado);
        if (mouseLivre != cursorEstavaLivre) {
            if (mouseLivre) EnableCursor();
            else DisableCursor();
            cursorEstavaLivre = mouseLivre;
        }

        if (!mouseLivre) {
            const Vector2 mouse = GetMouseDelta();
            orbita.yaw -= mouse.x * kSensibilidadeMouse;
            orbita.pitch -= mouse.y * kSensibilidadeMouse;
            orbita.pitch = std::clamp(orbita.pitch, kPitchMin, kPitchMax);
        }

        // Bau
        const float distBau = ComprimentoXZ(Vector3{
            jogador.posicao.x - bau.posicao.x,
            0.0f,
            jogador.posicao.z - bau.posicao.z,
        });
        const bool pertoBau = distBau <= kDistanciaBau;
        if (pertoBau && !bau.aberto && IsKeyPressed(KEY_E)) {
            bau.aberto = true;
        }
        const float tampaAlvo = bau.aberto ? 105.0f : 0.0f;
        bau.tampaAngulo += (tampaAlvo - bau.tampaAngulo) * (1.0f - std::exp(-8.0f * dt));

        // Combate (precisa de espada; loot do bau bloqueia clique)
        const bool lootAberto = bau.aberto && !bau.lootColetado;
        const bool querBloquear = jogador.temEspada && !lootAberto &&
                                  IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && jogador.ataque <= 0.0f;
        const float bloqueioAlvo = querBloquear ? 1.0f : 0.0f;
        jogador.bloqueio += (bloqueioAlvo - jogador.bloqueio) * (1.0f - std::exp(-(querBloquear ? 22.0f : 14.0f) * dt));

        jogador.cooldownAtaque = std::max(0.0f, jogador.cooldownAtaque - dt);
        if (jogador.temEspada && !lootAberto &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && jogador.ataque <= 0.0f &&
            jogador.cooldownAtaque <= 0.0f && jogador.bloqueio < 0.4f) {
            jogador.ataque = 0.001f;
            jogador.jaAcertouGolpe = false;
            jogador.bloqueio = 0.0f;
        }

        if (jogador.ataque > 0.0f) {
            jogador.ataque += dt / kDuracaoAtaque;
            if (!jogador.jaAcertouGolpe && GolpeAcertaInimigo(jogador, inimigo)) {
                inimigo.vida -= kDanoAtaque;
                inimigo.hitFlash = 0.35f;
                inimigo.recuo = 1.0f;
                jogador.jaAcertouGolpe = true;

                // Empurrao suave com velocidade (nao teleporte)
                const Vector3 frente = FrenteYaw(jogador.yaw);
                inimigo.knockbackX += frente.x * 9.5f;
                inimigo.knockbackZ += frente.z * 9.5f;
                // Cancela ataque do inimigo se estiver carregando
                if (inimigo.ataque > 0.0f && inimigo.ataque < 0.35f) {
                    inimigo.ataque = 0.0f;
                    inimigo.cooldownAtaque = 0.35f;
                }

                if (inimigo.vida <= 0.0f) {
                    inimigo.vida = 0.0f;
                    inimigo.vivo = false;
                    inimigo.hitFlash = 0.7f;
                    inimigo.ataque = 0.0f;
                    inimigo.knockbackX += frente.x * 4.0f;
                    inimigo.knockbackZ += frente.z * 4.0f;
                    GanharXp(&jogador, kXpPorKill);
                    timerSpawnInimigo = kTempoSpawnInimigo;
                }
            }
            if (jogador.ataque >= 1.0f) {
                jogador.ataque = 0.0f;
                jogador.cooldownAtaque = kCooldownAtaque;
            }
        }

        jogador.hitFlash = std::max(0.0f, jogador.hitFlash - dt);

        if (IsKeyPressed(KEY_K)) {
            jogador.miraTravada = !jogador.miraTravada;
        }

        const Vector3 movimento = LerMovimento(orbita.yaw);
        const bool temInput = ComprimentoXZ(movimento) > 0.0001f;
        const bool combatendo = jogador.ataque > 0.0f || jogador.bloqueio > 0.45f;
        const bool querCorrer = (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
                                jogador.noChao && !combatendo;
        const bool querAgachar = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
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

        jogador.posicao.x += jogador.velocidadeX * dt + jogador.knockbackX * dt;
        jogador.posicao.z += jogador.velocidadeZ * dt + jogador.knockbackZ * dt;
        {
            const float amortecimento = 1.0f - std::exp(-8.0f * dt);
            jogador.knockbackX *= (1.0f - amortecimento);
            jogador.knockbackZ *= (1.0f - amortecimento);
            if (std::fabs(jogador.knockbackX) < 0.05f) jogador.knockbackX = 0.0f;
            if (std::fabs(jogador.knockbackZ) < 0.05f) jogador.knockbackZ = 0.0f;
        }

        if (IsKeyPressed(KEY_SPACE) && jogador.noChao && jogador.ataque <= 0.0f) {
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
        if (jogador.posicao.y <= 0.0f) {
            jogador.posicao.y = 0.0f;
            jogador.velocidadeY = 0.0f;
            jogador.noChao = true;
            jogador.fasePulo = 0.0f;
        } else if (!jogador.noChao) {
            const float alvoFase = 0.5f - 0.5f * std::clamp(jogador.velocidadeY / kForcaPulo, -1.0f, 1.0f);
            jogador.fasePulo += (alvoFase - jogador.fasePulo) * (1.0f - std::exp(-9.0f * dt));
        }

        const float limite = kTamanhoArena / 2.0f - kRaioJogador - 0.5f;
        jogador.posicao.x = std::clamp(jogador.posicao.x, -limite, limite);
        jogador.posicao.z = std::clamp(jogador.posicao.z, -limite, limite);
        ResolverColisaoMapa(&jogador.posicao, kRaioJogador);

        AtualizarInimigo(&inimigo, &jogador, dt);
        if (inimigo.vivo) {
            ResolverColisaoMapa(&inimigo.posicao, kRaioInimigo);
        }

        // Spawn do inimigo: R instantaneo ou apos 10s (nao compete com revive)
        if (!inimigo.vivo && jogador.vida > 0.0f) {
            timerSpawnInimigo -= dt;
            if ((inimigo.hitFlash <= 0.0f && IsKeyPressed(KEY_R)) || timerSpawnInimigo <= 0.0f) {
                SpawnerInimigo(&inimigo);
                timerSpawnInimigo = kTempoSpawnInimigo;
            }
        }

        // Jogador morto: revive simples
        if (jogador.vida <= 0.0f && IsKeyPressed(KEY_R)) {
            jogador.vida = kVidaJogadorMax;
            jogador.posicao = Vector3{0.0f, 0.0f, 5.5f};
            jogador.knockbackX = 0.0f;
            jogador.knockbackZ = 0.0f;
            jogador.hitFlash = 0.0f;
        }

        AplicarCamera(&camera, jogador.posicao, orbita, jogador.agachar);

        BeginDrawing();
        ClearBackground(Color{92, 128, 158, 255});

        BeginMode3D(camera);
        DesenharMapa(tempo);
        DesenharBau(bau);
        DesenharPersonagem(jogador, tempo);
        DesenharInimigo(inimigo, tempo);
        EndMode3D();

        DesenharBarraVidaInimigo2D(inimigo, camera);
        DesenharHudJogador(jogador);
        DesenharPainelEquipamento(jogador);
        DesenharMinimapa(jogador, inimigo);

        if (AtualizarLootBau(&bau)) {
            jogador.temEspada = true;
            jogador.espadaMadeira = true;
        }

        DrawRectangle(0, 0, GetScreenWidth(), 56, Color{16, 18, 28, 210});
        DrawText("WASD | E bau | K mira | Ctrl correr | Espaco pular | Shift agachar | Esq atacar | Dir bloquear | X cursor | R inimigo",
                 12, 18, 16, RAYWHITE);

        if (pertoBau && !bau.aberto) {
            DrawText("E - Abrir bau", GetScreenWidth() / 2 - 70, GetScreenHeight() / 2 + 40, 22, Color{255, 230, 140, 255});
        }

        if (!inimigo.vivo) {
            const int seg = static_cast<int>(std::ceil(std::max(0.0f, timerSpawnInimigo)));
            DrawText(TextFormat("Inimigo em %ds (ou R)", seg), 20, 64, 18, Color{255, 160, 120, 255});
        }
        if (jogador.vida <= 0.0f) {
            DrawText("Voce morreu - R para reviver", GetScreenWidth() / 2 - 140, GetScreenHeight() / 2, 24, RED);
        } else if (!jogador.temEspada && !lootAberto) {
            DrawText("Pegue a espada no bau", 20, 90, 18, Color{210, 190, 140, 255});
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

        if (mouseLivre && !lootAberto) {
            DrawText("Cursor livre", 20, 116, 18, GOLD);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "mapa.hpp"

#include "rlgl.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <io.h>
#endif

namespace {

const PresetEditor kPresets[] = {
    {"Chao", TipoPecaMapa::Chao, 1.50f, 1.50f, 0.25f, 1.0f},
    {"Bloco", TipoPecaMapa::Bloco, 0.75f, 0.75f, 1.50f, 1.0f},
    {"Muro", TipoPecaMapa::Muro, 2.0f, 0.45f, 2.20f, 1.0f},
    {"Rocha", TipoPecaMapa::Rocha, 1.1f, 0.90f, 1.30f, 1.0f},
    {"Arvore", TipoPecaMapa::Arvore, 0.45f, 0.45f, 3.50f, 1.20f},
    {"Pilar", TipoPecaMapa::Pilar, 0.55f, 0.55f, 3.15f, 1.0f},
    {"Fonte", TipoPecaMapa::Fonte, 1.35f, 1.35f, 1.55f, 1.0f},
    {"Madeira", TipoPecaMapa::Madeira, 1.60f, 0.35f, 1.10f, 1.0f},
    {"Caixa", TipoPecaMapa::Caixa, 0.80f, 0.80f, 1.00f, 1.0f},
    {"Arbusto", TipoPecaMapa::Arbusto, 0.50f, 0.50f, 0.70f, 1.0f},
};

const Color kCoresRapidas[] = {
    {66, 104, 54, 255},
    {120, 96, 68, 255},
    {148, 136, 114, 255},
    {128, 120, 104, 255},
    {96, 68, 42, 255},
    {70, 130, 170, 255},
    {180, 70, 60, 255},
    {40, 44, 58, 255},
    {220, 210, 190, 255},
    {90, 140, 80, 255},
};

float Snap(float v, float grid) {
    if (grid <= 0.001f) return v;
    return std::round(v / grid) * grid;
}

Color ComAlpha(Color c, float alpha) {
    c.a = static_cast<unsigned char>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
    return c;
}

Color CorDaPeca(const PecaMapa& p, float alpha) {
    return ComAlpha(Color{p.r, p.g, p.b, 255}, alpha);
}

void CopiarStr(char* dest, int cap, const char* src) {
    if (!dest || cap <= 0) return;
    std::strncpy(dest, src ? src : "", cap - 1);
    dest[cap - 1] = '\0';
}

bool ArquivoExiste(const char* caminho) {
    FILE* f = std::fopen(caminho, "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

bool PontoNoChaoDoRay(const Ray& ray, float* outX, float* outZ) {
    if (std::fabs(ray.direction.y) < 0.0001f) return false;
    const float t = -ray.position.y / ray.direction.y;
    if (t < 0.0f) return false;
    *outX = ray.position.x + ray.direction.x * t;
    *outZ = ray.position.z + ray.direction.z * t;
    return true;
}

int IndicePecaSobPonto(const MapaMundo& mapa, float x, float z) {
    int melhor = -1;
    float melhorArea = 1.0e9f;
    for (int i = 0; i < mapa.quantidade; ++i) {
        const PecaMapa& p = mapa.pecas[i];
        if (x < p.x - p.halfX || x > p.x + p.halfX) continue;
        if (z < p.z - p.halfZ || z > p.z + p.halfZ) continue;
        const float area = p.halfX * p.halfZ;
        if (area < melhorArea) {
            melhorArea = area;
            melhor = i;
        }
    }
    return melhor;
}

Rectangle BotaoCentro(float y, float w, float h) {
    const float x = (static_cast<float>(GetScreenWidth()) - w) * 0.5f;
    return Rectangle{x, y, w, h};
}

void DesenharBotaoUI(Rectangle r, const char* texto, bool hover, Color base) {
    Color c = hover ? Color{
        static_cast<unsigned char>(std::min(255, base.r + 30)),
        static_cast<unsigned char>(std::min(255, base.g + 30)),
        static_cast<unsigned char>(std::min(255, base.b + 30)),
        255} : base;
    DrawRectangleRounded(r, 0.16f, 8, c);
    DrawRectangleRoundedLines(r, 0.16f, 8, Color{220, 230, 240, 120});
    const int tw = MeasureText(texto, 22);
    DrawText(texto, static_cast<int>(r.x + (r.width - tw) * 0.5f),
             static_cast<int>(r.y + (r.height - 22) * 0.5f), 22, RAYWHITE);
}

bool Clicou(Rectangle r) {
    return CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool Hover(Rectangle r) {
    return CheckCollisionPointRec(GetMousePosition(), r);
}

void DesenharRochaLocal(float x, float y, float z, float sx, float sy, float sz, float yaw, Color cor, float alpha) {
    rlPushMatrix();
    rlTranslatef(x, y + sy * 0.5f, z);
    rlRotatef(yaw, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{0.0f, 0.0f, 0.0f}, sx, sy, sz, ComAlpha(cor, alpha));
    DrawCube(Vector3{sx * 0.18f, sy * 0.12f, -sz * 0.1f}, sx * 0.55f, sy * 0.7f, sz * 0.55f,
             ComAlpha(Color{
                 static_cast<unsigned char>(cor.r * 0.8f),
                 static_cast<unsigned char>(cor.g * 0.8f),
                 static_cast<unsigned char>(cor.b * 0.8f), 255}, alpha));
    rlPopMatrix();
}

void DesenharArvoreLocal(float x, float y, float z, float escala, Color tom, float alpha) {
    const Color tronco = ComAlpha(Color{78, 52, 32, 255}, alpha);
    const Color folha = ComAlpha(tom.r ? tom : Color{42, 98, 48, 255}, alpha);
    DrawCube(Vector3{x, y + 1.1f * escala, z}, 0.35f * escala, 2.2f * escala, 0.35f * escala, tronco);
    DrawSphere(Vector3{x, y + 2.6f * escala, z}, 1.15f * escala, folha);
    DrawSphere(Vector3{x + 0.55f * escala, y + 2.3f * escala, z - 0.2f * escala}, 0.75f * escala,
               ComAlpha(Color{28, 72, 36, 255}, alpha));
}

void AdicionarItemLista(ListaMapas* lista, const char* nome, const char* caminho, bool builtin) {
    if (lista->quantidade >= kMaxMapasLista) return;
    for (int i = 0; i < lista->quantidade; ++i) {
        if (std::strcmp(lista->itens[i].caminho, caminho) == 0) return;
    }
    MapaListaItem& it = lista->itens[lista->quantidade++];
    CopiarStr(it.nome, kMaxNomeMapa, nome);
    CopiarStr(it.caminho, kMaxCaminhoMapa, caminho);
    it.builtin = builtin;
}

void NomeArquivoSemExt(const char* path, char* out, int cap) {
    const char* base = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }
    CopiarStr(out, cap, base);
    char* dot = std::strrchr(out, '.');
    if (dot) *dot = '\0';
}

}  // namespace

const PresetEditor* EditorPresets(int* quantidade) {
    if (quantidade) *quantidade = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));
    return kPresets;
}

const char* NomeTipoPeca(TipoPecaMapa tipo) {
    switch (tipo) {
        case TipoPecaMapa::Chao: return "chao";
        case TipoPecaMapa::Bloco: return "bloco";
        case TipoPecaMapa::Muro: return "muro";
        case TipoPecaMapa::Rocha: return "rocha";
        case TipoPecaMapa::Arvore: return "arvore";
        case TipoPecaMapa::Pilar: return "pilar";
        case TipoPecaMapa::Fonte: return "fonte";
        case TipoPecaMapa::Madeira: return "madeira";
        case TipoPecaMapa::Caixa: return "caixa";
        case TipoPecaMapa::Arbusto: return "arbusto";
        default: return "bloco";
    }
}

TipoPecaMapa TipoPecaDeNome(const char* nome) {
    char buf[32]{};
    int n = 0;
    for (; nome && nome[n] && n < 31; ++n) {
        buf[n] = static_cast<char>(std::tolower(static_cast<unsigned char>(nome[n])));
    }
    if (std::strcmp(buf, "chao") == 0) return TipoPecaMapa::Chao;
    if (std::strcmp(buf, "bloco") == 0) return TipoPecaMapa::Bloco;
    if (std::strcmp(buf, "muro") == 0) return TipoPecaMapa::Muro;
    if (std::strcmp(buf, "rocha") == 0) return TipoPecaMapa::Rocha;
    if (std::strcmp(buf, "arvore") == 0) return TipoPecaMapa::Arvore;
    if (std::strcmp(buf, "pilar") == 0) return TipoPecaMapa::Pilar;
    if (std::strcmp(buf, "fonte") == 0) return TipoPecaMapa::Fonte;
    if (std::strcmp(buf, "madeira") == 0) return TipoPecaMapa::Madeira;
    if (std::strcmp(buf, "arbusto") == 0) return TipoPecaMapa::Arbusto;
    if (std::strcmp(buf, "caixa") == 0) return TipoPecaMapa::Caixa;
    return TipoPecaMapa::Bloco;
}

void MapaLimpar(MapaMundo* mapa) {
    mapa->quantidade = 0;
    CopiarStr(mapa->nome, kMaxNomeMapa, "Mapa");
    mapa->tamanho = 40.0f;
    mapa->noite = false;
    mapa->corChao = Color{66, 104, 54, 255};
    mapa->corCeu = Color{92, 128, 158, 255};
}

void MapaNovoEmBranco(MapaMundo* mapa, const char* nome, float tamanho, bool noite, Color corChao) {
    MapaLimpar(mapa);
    CopiarStr(mapa->nome, kMaxNomeMapa, nome);
    mapa->tamanho = std::clamp(tamanho, 16.0f, 80.0f);
    mapa->noite = noite;
    mapa->corChao = corChao;
    mapa->corCeu = noite ? Color{18, 22, 40, 255} : Color{92, 128, 158, 255};

    // Chao base central
    if (mapa->quantidade < kMaxPecasMapa) {
        PecaMapa& p = mapa->pecas[mapa->quantidade++];
        p.tipo = TipoPecaMapa::Chao;
        p.x = 0;
        p.y = 0;
        p.z = 0;
        p.halfX = mapa->tamanho * 0.25f;
        p.halfZ = mapa->tamanho * 0.25f;
        p.altura = 0.18f;
        p.yaw = 0;
        p.escala = 1;
        p.r = corChao.r;
        p.g = corChao.g;
        p.b = corChao.b;
    }
}

void MapaCarregarPadrao(MapaMundo* mapa) {
    MapaLimpar(mapa);
    CopiarStr(mapa->nome, kMaxNomeMapa, "Arena Inicial");
    mapa->tamanho = 40.0f;
    mapa->noite = false;
    mapa->corChao = Color{66, 104, 54, 255};
    mapa->corCeu = Color{92, 128, 158, 255};

    const struct {
        TipoPecaMapa tipo;
        float x, z, hx, hz, h, yaw, esc;
        unsigned char r, g, b;
    } padrao[] = {
        {TipoPecaMapa::Fonte, 0, 0, 1.35f, 1.35f, 1.55f, 0, 1, 145, 138, 124},
        {TipoPecaMapa::Pilar, -7.5f, -7.5f, 0.55f, 0.55f, 3.15f, 0, 1, 145, 136, 118},
        {TipoPecaMapa::Pilar, 7.5f, -7.5f, 0.55f, 0.55f, 3.15f, 0, 1, 145, 136, 118},
        {TipoPecaMapa::Pilar, -7.5f, 7.5f, 0.55f, 0.55f, 3.15f, 0, 1, 145, 136, 118},
        {TipoPecaMapa::Pilar, 7.5f, 7.5f, 0.55f, 0.55f, 3.15f, 0, 1, 145, 136, 118},
        {TipoPecaMapa::Muro, -3.0f, -11.5f, 3.2f, 0.45f, 2.40f, 0, 1, 128, 120, 104},
        {TipoPecaMapa::Muro, 4.5f, -11.5f, 2.4f, 0.45f, 2.00f, 0, 1, 128, 120, 104},
        {TipoPecaMapa::Muro, 11.5f, -2.0f, 0.45f, 2.8f, 2.40f, 0, 1, 128, 120, 104},
        {TipoPecaMapa::Muro, 11.5f, 5.5f, 0.45f, 2.2f, 2.00f, 0, 1, 98, 92, 80},
        {TipoPecaMapa::Muro, -11.5f, 1.5f, 0.45f, 3.4f, 2.60f, 0, 1, 128, 120, 104},
        {TipoPecaMapa::Rocha, -9.0f, 4.5f, 1.1f, 0.9f, 1.30f, 25, 1, 110, 104, 96},
        {TipoPecaMapa::Rocha, 8.5f, -5.0f, 1.3f, 1.0f, 1.50f, -35, 1, 110, 104, 96},
        {TipoPecaMapa::Rocha, -5.5f, 10.0f, 1.0f, 1.2f, 1.20f, 60, 1, 110, 104, 96},
        {TipoPecaMapa::Rocha, 10.0f, 9.0f, 0.9f, 0.8f, 1.10f, 10, 1, 110, 104, 96},
        {TipoPecaMapa::Madeira, -2.5f, 6.5f, 1.6f, 0.35f, 1.10f, 0, 1, 96, 68, 42},
        {TipoPecaMapa::Madeira, 3.0f, -6.0f, 0.35f, 1.5f, 1.10f, 0, 1, 96, 68, 42},
        {TipoPecaMapa::Arvore, -14.0f, -12.0f, 0.45f, 0.45f, 3.50f, 0, 1.15f, 42, 98, 48},
        {TipoPecaMapa::Arvore, -16.0f, -6.0f, 0.50f, 0.50f, 3.80f, 0, 1.35f, 42, 98, 48},
        {TipoPecaMapa::Arvore, -15.0f, 8.0f, 0.45f, 0.45f, 3.50f, 0, 1.20f, 42, 98, 48},
        {TipoPecaMapa::Arvore, -13.5f, 14.0f, 0.55f, 0.55f, 4.00f, 0, 1.45f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 14.0f, -13.0f, 0.45f, 0.45f, 3.50f, 0, 1.25f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 16.0f, -4.0f, 0.50f, 0.50f, 3.80f, 0, 1.40f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 15.0f, 7.0f, 0.45f, 0.45f, 3.50f, 0, 1.15f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 13.0f, 14.5f, 0.50f, 0.50f, 3.80f, 0, 1.30f, 42, 98, 48},
        {TipoPecaMapa::Arvore, -6.0f, -16.0f, 0.45f, 0.45f, 3.50f, 0, 1.20f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 5.0f, -15.5f, 0.50f, 0.50f, 3.80f, 0, 1.35f, 42, 98, 48},
        {TipoPecaMapa::Arvore, -8.0f, 16.0f, 0.45f, 0.45f, 3.50f, 0, 1.25f, 42, 98, 48},
        {TipoPecaMapa::Arvore, 6.5f, 15.5f, 0.50f, 0.50f, 3.80f, 0, 1.40f, 42, 98, 48},
        {TipoPecaMapa::Arbusto, -10.5f, -8.0f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
        {TipoPecaMapa::Arbusto, -4.0f, 12.5f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
        {TipoPecaMapa::Arbusto, 6.0f, 12.0f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
        {TipoPecaMapa::Arbusto, 12.0f, -8.5f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
        {TipoPecaMapa::Arbusto, -12.0f, 6.0f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
        {TipoPecaMapa::Arbusto, 2.5f, -13.0f, 0.5f, 0.5f, 0.7f, 0, 1, 48, 96, 42},
    };

    for (const auto& p : padrao) {
        if (mapa->quantidade >= kMaxPecasMapa) break;
        PecaMapa& d = mapa->pecas[mapa->quantidade++];
        d.tipo = p.tipo;
        d.x = p.x; d.y = 0.0f; d.z = p.z;
        d.halfX = p.hx; d.halfZ = p.hz; d.altura = p.h;
        d.yaw = p.yaw; d.escala = p.esc;
        d.r = p.r; d.g = p.g; d.b = p.b;
    }
}

bool MapaCarregarDeArquivo(MapaMundo* mapa, const char* caminho) {
    FILE* f = std::fopen(caminho, "rb");
    if (!f) return false;

    MapaLimpar(mapa);
    char linha[320];
    bool cabecalhoOk = false;
    int versao = 1;

    while (std::fgets(linha, sizeof(linha), f)) {
        char* p = linha;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '\0' || *p == '\n' || *p == '\r' || *p == '#') continue;

        if (std::strncmp(p, "JNMAP", 5) == 0) {
            std::sscanf(p + 5, "%d", &versao);
            cabecalhoOk = true;
            continue;
        }
        if (std::strncmp(p, "name ", 5) == 0) {
            char nome[kMaxNomeMapa]{};
            std::sscanf(p + 5, "%63[^\r\n]", nome);
            CopiarStr(mapa->nome, kMaxNomeMapa, nome);
            continue;
        }
        if (std::strncmp(p, "size ", 5) == 0) {
            std::sscanf(p + 5, "%f", &mapa->tamanho);
            continue;
        }
        if (std::strncmp(p, "night ", 6) == 0) {
            int n = 0;
            std::sscanf(p + 6, "%d", &n);
            mapa->noite = n != 0;
            mapa->corCeu = mapa->noite ? Color{18, 22, 40, 255} : Color{92, 128, 158, 255};
            continue;
        }
        if (std::strncmp(p, "ground ", 7) == 0) {
            int r = 66, g = 104, b = 54;
            std::sscanf(p + 7, "%d %d %d", &r, &g, &b);
            mapa->corChao = Color{
                static_cast<unsigned char>(r),
                static_cast<unsigned char>(g),
                static_cast<unsigned char>(b), 255};
            continue;
        }

        cabecalhoOk = true;
        char tipoStr[32]{};
        float x = 0, z = 0, hx = 0.5f, hz = 0.5f, h = 1.0f, yaw = 0, esc = 1.0f, yBase = 0.0f;
        int r = 140, g = 130, b = 110;
        const int lidos = std::sscanf(p, "%31s %f %f %f %f %f %f %f %d %d %d %f",
                                      tipoStr, &x, &z, &hx, &hz, &h, &yaw, &esc, &r, &g, &b, &yBase);
        if (lidos < 6) continue;
        if (mapa->quantidade >= kMaxPecasMapa) break;

        PecaMapa& dest = mapa->pecas[mapa->quantidade++];
        dest.tipo = TipoPecaDeNome(tipoStr);
        dest.x = x;
        dest.z = z;
        dest.y = (lidos >= 12) ? yBase : 0.0f;
        dest.halfX = hx; dest.halfZ = hz; dest.altura = h;
        dest.yaw = (lidos >= 7) ? yaw : 0.0f;
        dest.escala = (lidos >= 8) ? esc : 1.0f;
        dest.r = static_cast<unsigned char>((lidos >= 9) ? std::clamp(r, 0, 255) : 140);
        dest.g = static_cast<unsigned char>((lidos >= 10) ? std::clamp(g, 0, 255) : 130);
        dest.b = static_cast<unsigned char>((lidos >= 11) ? std::clamp(b, 0, 255) : 110);
        (void)versao;
    }

    std::fclose(f);
    if (mapa->nome[0] == '\0') {
        NomeArquivoSemExt(caminho, mapa->nome, kMaxNomeMapa);
    }
    return mapa->quantidade > 0 || cabecalhoOk;
}

bool MapaSalvarEmArquivo(const MapaMundo& mapa, const char* caminho) {
    FILE* f = std::fopen(caminho, "wb");
    if (!f) return false;

    std::fprintf(f, "JNMAP 2\n");
    std::fprintf(f, "name %s\n", mapa.nome);
    std::fprintf(f, "size %.3f\n", mapa.tamanho);
    std::fprintf(f, "night %d\n", mapa.noite ? 1 : 0);
    std::fprintf(f, "ground %d %d %d\n", mapa.corChao.r, mapa.corChao.g, mapa.corChao.b);
    std::fprintf(f, "# tipo x z halfX halfZ altura yaw escala r g b yBase\n");
    for (int i = 0; i < mapa.quantidade; ++i) {
        const PecaMapa& p = mapa.pecas[i];
        std::fprintf(f, "%s %.3f %.3f %.3f %.3f %.3f %.3f %.3f %d %d %d %.3f\n",
                     NomeTipoPeca(p.tipo), p.x, p.z, p.halfX, p.halfZ, p.altura, p.yaw, p.escala,
                     p.r, p.g, p.b, p.y);
    }
    std::fclose(f);
    return true;
}

void MapaResolverPastaMaps(char* out, int capacidade) {
    const char* candidatos[] = {"maps", "../maps", "../../maps"};
    for (const char* c : candidatos) {
        char teste[kMaxCaminhoMapa];
        std::snprintf(teste, sizeof(teste), "%s/mapa_atual.map", c);
        if (ArquivoExiste(teste) || ArquivoExiste(c)) {
            CopiarStr(out, capacidade, c);
            return;
        }
#ifdef _WIN32
        char pattern[kMaxCaminhoMapa];
        std::snprintf(pattern, sizeof(pattern), "%s/*", c);
        _finddata_t info{};
        const intptr_t h = _findfirst(pattern, &info);
        if (h != -1) {
            _findclose(h);
            CopiarStr(out, capacidade, c);
            return;
        }
#endif
    }
    CopiarStr(out, capacidade, "maps");
}

bool MapaResolverCaminhoSalvar(char* out, int capacidade) {
    char pasta[kMaxCaminhoMapa];
    MapaResolverPastaMaps(pasta, sizeof(pasta));
    std::snprintf(out, capacidade, "%s/mapa_atual.map", pasta);
    return true;
}

bool MapaEhCasaBase(const char* caminhoOuNome) {
    if (!caminhoOuNome || caminhoOuNome[0] == '\0') return false;
    return std::strstr(caminhoOuNome, "casa_base") != nullptr ||
           std::strstr(caminhoOuNome, "Casa Base") != nullptr;
}

static void AddPecaMapa(MapaMundo* mapa, TipoPecaMapa tipo, float x, float z,
                        float hx, float hz, float h, float yaw, float esc,
                        unsigned char r, unsigned char g, unsigned char b, float yBase = 0.0f) {
    if (!mapa || mapa->quantidade >= kMaxPecasMapa) return;
    PecaMapa& p = mapa->pecas[mapa->quantidade++];
    p.tipo = tipo;
    p.x = x; p.y = yBase; p.z = z;
    p.halfX = hx; p.halfZ = hz; p.altura = h;
    p.yaw = yaw; p.escala = esc;
    p.r = r; p.g = g; p.b = b;
}

int QuantidadeDungeonsMundo1() { return 5; }

const char* NomeDungeon(int dungeonId) {
    switch (dungeonId) {
        case 0: return "Arena de batalha";
        case 1: return "Nether";
        case 2: return "Caverna";
        case 3: return "Floresta";
        case 4: return "Arena final";
        default: return "Dungeon";
    }
}

bool MapaCarregarCasaBase(MapaMundo* mapa, char* caminhoUsado, int caminhoCapacidade) {
    const char* caminhos[] = {
        "maps/casa_base.map",
        "../maps/casa_base.map",
        "../../maps/casa_base.map",
    };
    for (const char* caminho : caminhos) {
        if (!ArquivoExiste(caminho)) continue;
        if (MapaCarregarDeArquivo(mapa, caminho)) {
            if (caminhoUsado && caminhoCapacidade > 0) {
                CopiarStr(caminhoUsado, caminhoCapacidade, caminho);
            }
            return true;
        }
    }
    return false;
}

bool MapaCarregarDungeon(MapaMundo* mapa, int dungeonId, char* caminhoUsado, int caminhoCapacidade) {
    if (!mapa) return false;
    dungeonId = std::clamp(dungeonId, 0, QuantidadeDungeonsMundo1() - 1);

    // Arena de batalha = mapa padrao / arena_inicial
    if (dungeonId == 0) {
        const char* caminhos[] = {
            "maps/arena_inicial.map",
            "../maps/arena_inicial.map",
            "maps/arena_batalha.map",
            "../maps/arena_batalha.map",
        };
        for (const char* caminho : caminhos) {
            if (!ArquivoExiste(caminho)) continue;
            if (MapaCarregarDeArquivo(mapa, caminho)) {
                CopiarStr(mapa->nome, kMaxNomeMapa, "Arena de batalha");
                if (caminhoUsado && caminhoCapacidade > 0) {
                    CopiarStr(caminhoUsado, caminhoCapacidade, caminho);
                }
                return true;
            }
        }
        MapaCarregarPadrao(mapa);
        CopiarStr(mapa->nome, kMaxNomeMapa, "Arena de batalha");
        if (caminhoUsado && caminhoCapacidade > 0) {
            CopiarStr(caminhoUsado, caminhoCapacidade, "maps/arena_batalha.map");
        }
        return true;
    }

    MapaLimpar(mapa);
    CopiarStr(mapa->nome, kMaxNomeMapa, NomeDungeon(dungeonId));
    mapa->tamanho = 42.0f;

    if (dungeonId == 1) { // Nether
        mapa->noite = true;
        mapa->corChao = Color{70, 18, 18, 255};
        mapa->corCeu = Color{40, 8, 10, 255};
        AddPecaMapa(mapa, TipoPecaMapa::Chao, 0, 0, 12, 12, 0.2f, 0, 1, 90, 25, 20);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -8, -8, 0.7f, 0.7f, 4.0f, 0, 1, 80, 20, 20);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 8, -8, 0.7f, 0.7f, 4.0f, 0, 1, 80, 20, 20);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -8, 8, 0.7f, 0.7f, 4.0f, 0, 1, 80, 20, 20);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 8, 8, 0.7f, 0.7f, 4.0f, 0, 1, 80, 20, 20);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, -10, 2, 1.4f, 1.1f, 1.8f, 20, 1, 60, 20, 18);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 10, -3, 1.3f, 1.2f, 1.6f, -30, 1, 60, 20, 18);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 3, 11, 1.2f, 1.0f, 1.5f, 40, 1, 55, 18, 16);
        AddPecaMapa(mapa, TipoPecaMapa::Muro, 0, -14, 8, 0.5f, 2.8f, 0, 1, 70, 22, 22);
        AddPecaMapa(mapa, TipoPecaMapa::Muro, 0, 14, 8, 0.5f, 2.8f, 0, 1, 70, 22, 22);
        AddPecaMapa(mapa, TipoPecaMapa::Bloco, -5, 0, 1.2f, 1.2f, 0.5f, 0, 1, 120, 40, 20, 0);
        AddPecaMapa(mapa, TipoPecaMapa::Bloco, 5, 4, 1.0f, 1.0f, 0.5f, 0, 1, 120, 40, 20, 0);
        AddPecaMapa(mapa, TipoPecaMapa::Fonte, 0, 0, 1.2f, 1.2f, 1.4f, 0, 1, 160, 40, 30);
    } else if (dungeonId == 2) { // Caverna
        mapa->noite = true;
        mapa->corChao = Color{48, 44, 40, 255};
        mapa->corCeu = Color{10, 10, 14, 255};
        AddPecaMapa(mapa, TipoPecaMapa::Chao, 0, 0, 11, 11, 0.18f, 0, 1, 55, 50, 46);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, -9, -6, 2.0f, 1.6f, 2.4f, 15, 1, 70, 66, 62);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 9, -5, 1.8f, 1.7f, 2.2f, -25, 1, 70, 66, 62);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, -7, 8, 1.6f, 1.8f, 2.0f, 50, 1, 65, 62, 58);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 8, 7, 1.7f, 1.5f, 2.3f, -10, 1, 65, 62, 58);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 0, -11, 2.2f, 1.4f, 2.6f, 0, 1, 60, 58, 54);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, -11, 1, 1.5f, 2.0f, 2.5f, 30, 1, 60, 58, 54);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 11, 2, 1.5f, 2.0f, 2.5f, -30, 1, 60, 58, 54);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -4, -3, 0.45f, 0.45f, 3.5f, 0, 1, 90, 85, 78);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 4, 3, 0.45f, 0.45f, 3.5f, 0, 1, 90, 85, 78);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -3, 5, 0.4f, 0.4f, 3.2f, 0, 1, 85, 80, 74);
        AddPecaMapa(mapa, TipoPecaMapa::Bloco, 2, -2, 1.5f, 1.0f, 0.7f, 0, 1, 50, 48, 44);
    } else if (dungeonId == 3) { // Floresta
        mapa->noite = false;
        mapa->corChao = Color{42, 78, 36, 255};
        mapa->corCeu = Color{70, 110, 90, 255};
        AddPecaMapa(mapa, TipoPecaMapa::Chao, 0, 0, 10, 10, 0.12f, 0, 1, 50, 90, 42);
        for (int i = 0; i < 16; ++i) {
            const float ang = i * 22.5f * DEG2RAD;
            const float rr = 11.0f + (i % 3) * 1.5f;
            AddPecaMapa(mapa, TipoPecaMapa::Arvore,
                       std::sin(ang) * rr, std::cos(ang) * rr,
                       0.5f, 0.5f, 3.8f, 0, 1.2f + (i % 4) * 0.1f, 30, 90, 40);
        }
        AddPecaMapa(mapa, TipoPecaMapa::Arbusto, -5, 3, 0.5f, 0.5f, 0.7f, 0, 1, 40, 90, 35);
        AddPecaMapa(mapa, TipoPecaMapa::Arbusto, 6, -4, 0.5f, 0.5f, 0.7f, 0, 1, 40, 90, 35);
        AddPecaMapa(mapa, TipoPecaMapa::Arbusto, -3, -7, 0.5f, 0.5f, 0.7f, 0, 1, 40, 90, 35);
        AddPecaMapa(mapa, TipoPecaMapa::Arbusto, 4, 8, 0.5f, 0.5f, 0.7f, 0, 1, 40, 90, 35);
        AddPecaMapa(mapa, TipoPecaMapa::Madeira, -2, 5, 1.8f, 0.35f, 1.0f, 25, 1, 80, 55, 30);
        AddPecaMapa(mapa, TipoPecaMapa::Rocha, 7, 2, 1.0f, 0.9f, 1.1f, 10, 1, 90, 88, 80);
    } else { // Arena final
        mapa->noite = false;
        mapa->corChao = Color{210, 205, 195, 255};
        mapa->corCeu = Color{180, 200, 230, 255};
        AddPecaMapa(mapa, TipoPecaMapa::Chao, 0, 0, 13, 13, 0.16f, 0, 1, 230, 228, 220);
        AddPecaMapa(mapa, TipoPecaMapa::Chao, 0, 0, 8, 8, 0.08f, 0, 1, 245, 245, 250, 0.16f);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -10, -10, 0.7f, 0.7f, 4.5f, 0, 1, 240, 240, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 10, -10, 0.7f, 0.7f, 4.5f, 0, 1, 240, 240, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, -10, 10, 0.7f, 0.7f, 4.5f, 0, 1, 240, 240, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 10, 10, 0.7f, 0.7f, 4.5f, 0, 1, 240, 240, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 0, -12, 0.55f, 0.55f, 3.8f, 0, 1, 235, 235, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Pilar, 0, 12, 0.55f, 0.55f, 3.8f, 0, 1, 235, 235, 245);
        AddPecaMapa(mapa, TipoPecaMapa::Muro, -14, 0, 0.4f, 6, 2.5f, 0, 1, 220, 220, 230);
        AddPecaMapa(mapa, TipoPecaMapa::Muro, 14, 0, 0.4f, 6, 2.5f, 0, 1, 220, 220, 230);
        AddPecaMapa(mapa, TipoPecaMapa::Fonte, 0, 0, 1.5f, 1.5f, 1.8f, 0, 1, 200, 210, 230);
        AddPecaMapa(mapa, TipoPecaMapa::Bloco, -6, -6, 1.2f, 1.2f, 0.4f, 0, 1, 250, 250, 255);
        AddPecaMapa(mapa, TipoPecaMapa::Bloco, 6, 6, 1.2f, 1.2f, 0.4f, 0, 1, 250, 250, 255);
    }

    if (caminhoUsado && caminhoCapacidade > 0) {
        std::snprintf(caminhoUsado, caminhoCapacidade, "maps/dungeon_%d.map", dungeonId);
    }
    return true;
}

bool MapaTentarCarregar(MapaMundo* mapa, char* caminhoUsado, int caminhoCapacidade) {
    if (MapaCarregarCasaBase(mapa, caminhoUsado, caminhoCapacidade)) return true;
    MapaCarregarPadrao(mapa);
    if (caminhoUsado && caminhoCapacidade > 0) {
        CopiarStr(caminhoUsado, caminhoCapacidade, "maps/casa_base.map");
    }
    return false;
}

void MapaListarDisponiveis(ListaMapas* lista) {
    lista->quantidade = 0;
    AdicionarItemLista(lista, "Arena Inicial", "__builtin_arena__", true);

    char pasta[kMaxCaminhoMapa];
    MapaResolverPastaMaps(pasta, sizeof(pasta));

#ifdef _WIN32
    char pattern[kMaxCaminhoMapa];
    std::snprintf(pattern, sizeof(pattern), "%s/*.map", pasta);
    _finddata_t fd{};
    const intptr_t h = _findfirst(pattern, &fd);
    if (h != -1) {
        do {
            if (fd.attrib & _A_SUBDIR) continue;
            char full[kMaxCaminhoMapa];
            std::snprintf(full, sizeof(full), "%s/%s", pasta, fd.name);
            char nome[kMaxNomeMapa];
            NomeArquivoSemExt(fd.name, nome, sizeof(nome));
            // Casa base e protegida: nao aparece para edicao
            if (std::strcmp(nome, "casa_base") == 0) continue;
            if (std::strcmp(nome, "arena_inicial") == 0) {
                CopiarStr(nome, sizeof(nome), "Arena Inicial (arquivo)");
            }
            AdicionarItemLista(lista, nome, full, false);
        } while (_findnext(h, &fd) == 0);
        _findclose(h);
    }
#else
    (void)pasta;
#endif

    // Fallbacks comuns
    if (ArquivoExiste("maps/mapa_atual.map")) {
        AdicionarItemLista(lista, "mapa_atual", "maps/mapa_atual.map", false);
    }
    if (ArquivoExiste("../maps/mapa_atual.map")) {
        AdicionarItemLista(lista, "mapa_atual", "../maps/mapa_atual.map", false);
    }
}

ObstaculoColisao PecaParaColisao(const PecaMapa& peca) {
    return ObstaculoColisao{peca.x, peca.z, peca.halfX, peca.halfZ, peca.y, peca.y + peca.altura};
}

bool PecaTemColisao(const PecaMapa& peca) {
    return peca.tipo != TipoPecaMapa::Arbusto;
}

bool AabbSobrepoeXZ(float ax, float az, float ahx, float ahz,
                    float bx, float bz, float bhx, float bhz) {
    return std::fabs(ax - bx) < (ahx + bhx) && std::fabs(az - bz) < (ahz + bhz);
}

bool AabbSobrepoe3D(const PecaMapa& a, const PecaMapa& b) {
    if (!AabbSobrepoeXZ(a.x, a.z, a.halfX, a.halfZ, b.x, b.z, b.halfX, b.halfZ)) return false;
    const float aMin = a.y;
    const float aMax = a.y + a.altura;
    const float bMin = b.y;
    const float bMax = b.y + b.altura;
    return aMin < bMax - 0.001f && aMax > bMin + 0.001f;
}

float AlturaTopoSobPeca(const MapaMundo& mapa, float x, float z, float halfX, float halfZ) {
    float topo = 0.0f;
    for (int i = 0; i < mapa.quantidade; ++i) {
        const PecaMapa& p = mapa.pecas[i];
        if (!PecaTemColisao(p)) continue;
        if (!AabbSobrepoeXZ(x, z, halfX, halfZ, p.x, p.z, p.halfX, p.halfZ)) continue;
        topo = std::max(topo, p.y + p.altura);
    }
    return topo;
}

bool PecaColideComMapa(const MapaMundo& mapa, const PecaMapa& candidata) {
    for (int i = 0; i < mapa.quantidade; ++i) {
        if (!PecaTemColisao(mapa.pecas[i])) continue;
        if (AabbSobrepoe3D(candidata, mapa.pecas[i])) return true;
    }
    return false;
}

Color MapaCorCeu(const MapaMundo& mapa) {
    return mapa.noite ? Color{8, 10, 22, 255} : mapa.corCeu;
}

void MapaDesenharBase(const MapaMundo& mapa, float tempo) {
    (void)tempo;
    const float size = mapa.tamanho;
    const float half = size * 0.5f;

    Color grama = mapa.corChao;
    if (mapa.noite) {
        grama = Color{
            static_cast<unsigned char>(grama.r * 0.55f),
            static_cast<unsigned char>(grama.g * 0.55f),
            static_cast<unsigned char>(grama.b * 0.60f), 255};
    }
    Color gramaExt{
        static_cast<unsigned char>(grama.r * 0.80f),
        static_cast<unsigned char>(grama.g * 0.80f),
        static_cast<unsigned char>(grama.b * 0.80f), 255};

    DrawCube(Vector3{0.0f, -0.18f, 0.0f}, size + 8.0f, 0.28f, size + 8.0f, gramaExt);
    DrawCube(Vector3{0.0f, -0.05f, 0.0f}, size, 0.12f, size, grama);

    if (!mapa.noite) {
        const float praca = std::min(18.0f, size * 0.45f);
        constexpr int kTiles = 18;
        const float tile = praca / kTiles;
        const float origem = -praca * 0.5f;
        for (int z = 0; z < kTiles; ++z) {
            for (int x = 0; x < kTiles; ++x) {
                const bool claro = ((x + z) % 2) == 0;
                Color cor = claro ? Color{168, 156, 132, 255} : Color{148, 136, 114, 255};
                DrawCube(Vector3{origem + tile * (x + 0.5f), 0.01f, origem + tile * (z + 0.5f)},
                         tile * 0.98f, 0.08f, tile * 0.98f, cor);
            }
        }
    }

    const Color muroLimite = mapa.noite ? Color{22, 28, 40, 255} : Color{76, 88, 70, 255};
    DrawCube(Vector3{0.0f, 0.7f, -half - 0.4f}, size + 2.0f, 1.4f, 0.7f, muroLimite);
    DrawCube(Vector3{0.0f, 0.7f, half + 0.4f}, size + 2.0f, 1.4f, 0.7f, muroLimite);
    DrawCube(Vector3{-half - 0.4f, 0.7f, 0.0f}, 0.7f, 1.4f, size + 2.0f, muroLimite);
    DrawCube(Vector3{half + 0.4f, 0.7f, 0.0f}, 0.7f, 1.4f, size + 2.0f, muroLimite);

    if (mapa.noite) {
        DrawSphere(Vector3{half * 0.35f, 24.0f, -half * 0.55f}, 2.2f, Color{235, 235, 210, 255});
        DrawSphere(Vector3{half * 0.35f, 24.0f, -half * 0.55f}, 3.4f, Color{180, 190, 230, 40});
        // Estrelas simples
        for (int i = 0; i < 28; ++i) {
            const float sx = -half + (i * 97 % static_cast<int>(size));
            const float sz = -half + ((i * 53 + 17) % static_cast<int>(size));
            DrawSphere(Vector3{sx, 18.0f + (i % 5), sz}, 0.08f, Color{220, 225, 255, 200});
        }
    }
}

void MapaDesenharPeca(const PecaMapa& peca, float tempo, float alpha) {
    const Color cor = CorDaPeca(peca, alpha);
    const float by = peca.y;
    switch (peca.tipo) {
        case TipoPecaMapa::Chao: {
            rlPushMatrix();
            rlTranslatef(peca.x, by + peca.altura * 0.5f, peca.z);
            rlRotatef(peca.yaw, 0.0f, 1.0f, 0.0f);
            DrawCube(Vector3{0, 0, 0}, peca.halfX * 2.0f, peca.altura, peca.halfZ * 2.0f, cor);
            DrawCubeWires(Vector3{0, 0, 0}, peca.halfX * 2.0f, peca.altura, peca.halfZ * 2.0f,
                          ComAlpha(Color{0, 0, 0, 80}, alpha));
            rlPopMatrix();
            break;
        }
        case TipoPecaMapa::Bloco:
        case TipoPecaMapa::Caixa: {
            rlPushMatrix();
            rlTranslatef(peca.x, by + peca.altura * 0.5f, peca.z);
            rlRotatef(peca.yaw, 0.0f, 1.0f, 0.0f);
            DrawCube(Vector3{0, 0, 0}, peca.halfX * 2.0f, peca.altura, peca.halfZ * 2.0f, cor);
            rlPopMatrix();
            break;
        }
        case TipoPecaMapa::Fonte: {
            DrawCylinder(Vector3{peca.x, by + 0.0f, peca.z}, 1.5f, 1.5f, 0.35f, 20, cor);
            DrawCylinder(Vector3{peca.x, by + 0.35f, peca.z}, 1.15f, 1.25f, 0.45f, 20, cor);
            DrawCylinder(Vector3{peca.x, by + 0.55f, peca.z}, 0.95f, 0.95f, 0.08f, 16,
                         ComAlpha(Color{70, 130, 170, 220}, alpha));
            DrawCube(Vector3{peca.x, by + 1.0f, peca.z}, 0.28f, 1.0f, 0.28f, cor);
            const float onda = 0.04f + 0.02f * std::sin(tempo * 2.4f);
            DrawCylinder(Vector3{peca.x, by + 0.58f + onda * 0.5f, peca.z}, 0.55f, 0.55f, 0.05f, 12,
                         ComAlpha(Color{110, 175, 210, 160}, alpha));
            break;
        }
        case TipoPecaMapa::Pilar: {
            DrawCube(Vector3{peca.x, by + 1.5f, peca.z}, 0.9f, 3.0f, 0.9f, cor);
            DrawCube(Vector3{peca.x, by + 3.15f, peca.z}, 1.2f, 0.35f, 1.2f, cor);
            DrawCube(Vector3{peca.x, by + 0.15f, peca.z}, 1.3f, 0.3f, 1.3f, cor);
            break;
        }
        case TipoPecaMapa::Muro:
        case TipoPecaMapa::Madeira: {
            rlPushMatrix();
            rlTranslatef(peca.x, by + peca.altura * 0.5f, peca.z);
            rlRotatef(peca.yaw, 0.0f, 1.0f, 0.0f);
            DrawCube(Vector3{0, 0, 0}, peca.halfX * 2.0f, peca.altura, peca.halfZ * 2.0f, cor);
            rlPopMatrix();
            break;
        }
        case TipoPecaMapa::Rocha: {
            DesenharRochaLocal(peca.x, by, peca.z, peca.halfX * 2.0f, peca.altura,
                               peca.halfZ * 2.0f, peca.yaw, Color{peca.r, peca.g, peca.b, 255}, alpha);
            break;
        }
        case TipoPecaMapa::Arvore: {
            DesenharArvoreLocal(peca.x, by, peca.z, peca.escala, Color{peca.r, peca.g, peca.b, 255}, alpha);
            break;
        }
        case TipoPecaMapa::Arbusto: {
            DrawSphere(Vector3{peca.x, by + 0.45f, peca.z}, 0.55f, cor);
            DrawSphere(Vector3{peca.x + 0.35f, by + 0.35f, peca.z + 0.1f}, 0.40f,
                       ComAlpha(Color{40, 82, 36, 255}, alpha));
            break;
        }
        default: break;
    }
}

void MapaDesenharPecas(const MapaMundo& mapa, float tempo) {
    for (int i = 0; i < mapa.quantidade; ++i) {
        MapaDesenharPeca(mapa.pecas[i], tempo, 1.0f);
    }
}

void EditorDefinirStatus(EditorMapa* editor, const char* msg) {
    CopiarStr(editor->status, sizeof(editor->status), msg);
    editor->statusTempo = 3.0f;
}

void EditorIniciar(EditorMapa* editor, const char* caminho) {
    editor->ativo = true;
    editor->fase = EditorFase::Menu;
    editor->preset = 0;
    editor->grid = 0.5f;
    editor->yaw = 0.0f;
    editor->halfX = 1.5f;
    editor->halfZ = 1.5f;
    editor->altura = 0.25f;
    editor->corPeca = Color{148, 136, 114, 255};
    editor->cursorValido = false;
    editor->snap = true;
    editor->encaixeSolido = true;
    editor->sujo = false;
    editor->cursorY = 0.0f;
    editor->statusTempo = 0.0f;
    editor->novoTamanho = 40.0f;
    editor->novoNoite = false;
    editor->novoCorChao = Color{66, 104, 54, 255};
    CopiarStr(editor->novoNome, kMaxNomeMapa, "Meu Mapa");
    editor->listaSelecionada = 0;
    MapaListarDisponiveis(&editor->lista);
    if (caminho) CopiarStr(editor->caminhoArquivo, kMaxCaminhoMapa, caminho);
    else MapaResolverCaminhoSalvar(editor->caminhoArquivo, kMaxCaminhoMapa);
    EditorDefinirStatus(editor, "escolha criar ou modificar");
}

bool EditorEstaConstruindo(const EditorMapa& editor) {
    return editor.ativo && editor.fase == EditorFase::Construindo;
}

void EditorAtualizar(
    EditorMapa* editor,
    MapaMundo* mapa,
    const Camera3D& camera,
    bool mouseSobreUi) {
    if (!editor->ativo || editor->fase != EditorFase::Construindo) return;
    editor->statusTempo = std::max(0.0f, editor->statusTempo - GetFrameTime());

    int qPresets = 0;
    const PresetEditor* presets = EditorPresets(&qPresets);
    for (int i = 0; i < qPresets && i < 9; ++i) {
        if (IsKeyPressed(KEY_ONE + i)) {
            editor->preset = i;
            editor->halfX = presets[i].halfX;
            editor->halfZ = presets[i].halfZ;
            editor->altura = presets[i].altura;
            EditorDefinirStatus(editor, TextFormat("peca: %s", presets[i].nome));
        }
    }

    if (IsKeyPressed(KEY_G)) {
        editor->snap = !editor->snap;
        EditorDefinirStatus(editor, editor->snap ? "snap ligado" : "snap desligado");
    }
    if (IsKeyPressed(KEY_F)) {
        editor->encaixeSolido = !editor->encaixeSolido;
        EditorDefinirStatus(editor, editor->encaixeSolido
            ? "encaixe solido: blocos empilham (sem atravessar)"
            : "encaixe livre: pode colocar dentro de blocos");
    }
    if (IsKeyPressed(KEY_Q)) editor->yaw -= 15.0f;
    if (IsKeyPressed(KEY_E)) editor->yaw += 15.0f;

    // Tamanho da peca
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (IsKeyPressed(KEY_LEFT_BRACKET)) editor->halfX = std::max(0.25f, editor->halfX - 0.25f);
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) editor->halfX = std::min(8.0f, editor->halfX + 0.25f);
        if (IsKeyPressed(KEY_MINUS)) editor->halfZ = std::max(0.25f, editor->halfZ - 0.25f);
        if (IsKeyPressed(KEY_EQUAL)) editor->halfZ = std::min(8.0f, editor->halfZ + 0.25f);
        if (IsKeyPressed(KEY_COMMA)) editor->altura = std::max(0.1f, editor->altura - 0.1f);
        if (IsKeyPressed(KEY_PERIOD)) editor->altura = std::min(8.0f, editor->altura + 0.1f);
    } else {
        if (IsKeyPressed(KEY_LEFT_BRACKET)) editor->grid = std::clamp(editor->grid - 0.25f, 0.25f, 2.0f);
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) editor->grid = std::clamp(editor->grid + 0.25f, 0.25f, 2.0f);
    }

    // Cores rapidas 0-9 no numpad / F1-F10 style: Z cycles
    if (IsKeyPressed(KEY_Z)) {
        static int corIdx = 0;
        corIdx = (corIdx + 1) % static_cast<int>(sizeof(kCoresRapidas) / sizeof(kCoresRapidas[0]));
        editor->corPeca = kCoresRapidas[corIdx];
        EditorDefinirStatus(editor, "cor alterada");
    }
    if (IsKeyPressed(KEY_N)) {
        mapa->noite = !mapa->noite;
        mapa->corCeu = mapa->noite ? Color{18, 22, 40, 255} : Color{92, 128, 158, 255};
        editor->sujo = true;
        EditorDefinirStatus(editor, mapa->noite ? "modo noite" : "modo dia");
    }

    const Ray ray = GetMouseRay(GetMousePosition(), camera);
    float hx = 0.0f, hz = 0.0f;
    editor->cursorValido = PontoNoChaoDoRay(ray, &hx, &hz);
    if (editor->cursorValido) {
        if (editor->snap) {
            hx = Snap(hx, editor->grid);
            hz = Snap(hz, editor->grid);
        }
        const float lim = mapa->tamanho * 0.5f - 1.0f;
        editor->cursorX = std::clamp(hx, -lim, lim);
        editor->cursorZ = std::clamp(hz, -lim, lim);
        if (editor->encaixeSolido) {
            editor->cursorY = AlturaTopoSobPeca(*mapa, editor->cursorX, editor->cursorZ,
                                               editor->halfX, editor->halfZ);
        } else {
            editor->cursorY = 0.0f;
        }
    }

    if (mouseSobreUi || !editor->cursorValido) return;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) return;
    // Evita clicar UI lateral
    if (GetMousePosition().x < 190.0f) return;
    if (GetMousePosition().y > GetScreenHeight() - 70.0f) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (mapa->quantidade >= kMaxPecasMapa) {
            EditorDefinirStatus(editor, "limite de pecas");
            return;
        }
        const PresetEditor& pr = presets[std::clamp(editor->preset, 0, qPresets - 1)];
        PecaMapa candidata{};
        candidata.tipo = pr.tipo;
        candidata.x = editor->cursorX;
        candidata.y = editor->cursorY;
        candidata.z = editor->cursorZ;
        candidata.halfX = editor->halfX;
        candidata.halfZ = editor->halfZ;
        candidata.altura = editor->altura;
        candidata.yaw = editor->yaw;
        candidata.escala = pr.escala;
        candidata.r = editor->corPeca.r;
        candidata.g = editor->corPeca.g;
        candidata.b = editor->corPeca.b;

        if (editor->encaixeSolido && PecaColideComMapa(*mapa, candidata)) {
            // Recalcula topo e tenta de novo (garante encaixe por cima)
            candidata.y = AlturaTopoSobPeca(*mapa, candidata.x, candidata.z,
                                            candidata.halfX, candidata.halfZ);
            if (PecaColideComMapa(*mapa, candidata)) {
                EditorDefinirStatus(editor, "nao cabe aqui (F = encaixe livre)");
                return;
            }
        }

        mapa->pecas[mapa->quantidade++] = candidata;
        editor->sujo = true;
        EditorDefinirStatus(editor, TextFormat("colocou %s em y=%.2f", pr.nome, candidata.y));
    }

    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) {
        const int idx = IndicePecaSobPonto(*mapa, editor->cursorX, editor->cursorZ);
        if (idx >= 0) {
            for (int i = idx; i < mapa->quantidade - 1; ++i) mapa->pecas[i] = mapa->pecas[i + 1];
            --mapa->quantidade;
            editor->sujo = true;
            EditorDefinirStatus(editor, "removeu peca");
        }
    }
}

void EditorDesenharGhost(const EditorMapa& editor, float tempo) {
    if (!EditorEstaConstruindo(editor) || !editor.cursorValido) return;
    int q = 0;
    const PresetEditor* presets = EditorPresets(&q);
    const PresetEditor& pr = presets[std::clamp(editor.preset, 0, q - 1)];
    PecaMapa ghost{};
    ghost.tipo = pr.tipo;
    ghost.x = editor.cursorX;
    ghost.y = editor.cursorY;
    ghost.z = editor.cursorZ;
    ghost.halfX = editor.halfX;
    ghost.halfZ = editor.halfZ;
    ghost.altura = editor.altura;
    ghost.yaw = editor.yaw;
    ghost.escala = pr.escala;
    ghost.r = editor.corPeca.r;
    ghost.g = editor.corPeca.g;
    ghost.b = editor.corPeca.b;
    MapaDesenharPeca(ghost, tempo, 0.45f);
}

static void DesenharMenuPrincipal(EditorMapa* editor, bool* pediuSair) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{10, 14, 22, 240});
    const char* titulo = "EDITOR DE MAPAS";
    DrawText(titulo, (sw - MeasureText(titulo, 42)) / 2, sh / 2 - 160, 42, Color{230, 240, 255, 255});

    Rectangle rCriar = BotaoCentro(static_cast<float>(sh) * 0.42f, 320, 56);
    Rectangle rMod = BotaoCentro(static_cast<float>(sh) * 0.42f + 72, 320, 56);
    Rectangle rVoltar = BotaoCentro(static_cast<float>(sh) * 0.42f + 144, 320, 48);

    DesenharBotaoUI(rCriar, "Criar novo", Hover(rCriar), Color{40, 90, 70, 255});
    DesenharBotaoUI(rMod, "Modificar ja existente", Hover(rMod), Color{50, 70, 110, 255});
    DesenharBotaoUI(rVoltar, "Voltar ao jogo", Hover(rVoltar), Color{70, 40, 45, 255});

    if (Clicou(rCriar)) editor->fase = EditorFase::CriarNovo;
    if (Clicou(rMod)) {
        MapaListarDisponiveis(&editor->lista);
        editor->listaSelecionada = 0;
        editor->fase = EditorFase::ModificarLista;
    }
    if (Clicou(rVoltar) || IsKeyPressed(KEY_ESCAPE)) {
        editor->ativo = false;
        if (pediuSair) *pediuSair = true;
    }
}

static void DesenharCriarNovo(EditorMapa* editor, MapaMundo* mapa, bool* pediuSair) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{10, 14, 22, 240});
    DrawText("CRIAR NOVO MAPA", (sw - MeasureText("CRIAR NOVO MAPA", 34)) / 2, 60, 34, RAYWHITE);

    DrawText(TextFormat("Nome: %s", editor->novoNome), 80, 140, 22, Color{200, 210, 230, 255});
    DrawText(TextFormat("Tamanho do espaco: %.0f", editor->novoTamanho), 80, 190, 22, RAYWHITE);

    Rectangle rMenor = {80, 230, 120, 40};
    Rectangle rMaior = {220, 230, 120, 40};
    DesenharBotaoUI(rMenor, "- 5", Hover(rMenor), Color{50, 55, 70, 255});
    DesenharBotaoUI(rMaior, "+ 5", Hover(rMaior), Color{50, 55, 70, 255});
    if (Clicou(rMenor)) editor->novoTamanho = std::max(16.0f, editor->novoTamanho - 5.0f);
    if (Clicou(rMaior)) editor->novoTamanho = std::min(80.0f, editor->novoTamanho + 5.0f);

    DrawText(TextFormat("Periodo: %s", editor->novoNoite ? "NOITE" : "DIA"), 80, 300, 22, RAYWHITE);
    Rectangle rDia = {80, 340, 140, 42};
    Rectangle rNoite = {240, 340, 140, 42};
    DesenharBotaoUI(rDia, "Dia", Hover(rDia), editor->novoNoite ? Color{40, 45, 55, 255} : Color{70, 120, 180, 255});
    DesenharBotaoUI(rNoite, "Noite", Hover(rNoite), editor->novoNoite ? Color{40, 50, 90, 255} : Color{40, 45, 55, 255});
    if (Clicou(rDia)) editor->novoNoite = false;
    if (Clicou(rNoite)) editor->novoNoite = true;

    DrawText("Cor dos blocos / chao:", 80, 410, 22, RAYWHITE);
    for (int i = 0; i < 10; ++i) {
        Rectangle c{static_cast<float>(80 + i * 46), 450, 40, 40};
        DrawRectangleRec(c, kCoresRapidas[i]);
        if (editor->novoCorChao.r == kCoresRapidas[i].r &&
            editor->novoCorChao.g == kCoresRapidas[i].g &&
            editor->novoCorChao.b == kCoresRapidas[i].b) {
            DrawRectangleLinesEx(c, 3, RAYWHITE);
        }
        if (Clicou(c)) {
            editor->novoCorChao = kCoresRapidas[i];
            editor->corPeca = kCoresRapidas[i];
        }
    }

    Rectangle rOk = BotaoCentro(static_cast<float>(sh) - 120, 280, 52);
    Rectangle rVoltar = BotaoCentro(static_cast<float>(sh) - 60, 280, 44);
    DesenharBotaoUI(rOk, "Comecar a construir", Hover(rOk), Color{40, 110, 70, 255});
    DesenharBotaoUI(rVoltar, "Voltar", Hover(rVoltar), Color{70, 40, 45, 255});

    if (Clicou(rOk)) {
        MapaNovoEmBranco(mapa, editor->novoNome, editor->novoTamanho, editor->novoNoite, editor->novoCorChao);
        editor->corPeca = editor->novoCorChao;
        editor->preset = 0;
        editor->halfX = 1.5f;
        editor->halfZ = 1.5f;
        editor->altura = 0.25f;
        editor->fase = EditorFase::Construindo;
        editor->sujo = true;
        char pasta[kMaxCaminhoMapa];
        MapaResolverPastaMaps(pasta, sizeof(pasta));
        std::snprintf(editor->caminhoArquivo, sizeof(editor->caminhoArquivo), "%s/%s.map", pasta, "novo_mapa");
        EditorDefinirStatus(editor, "mapa novo criado");
    }
    if (Clicou(rVoltar) || IsKeyPressed(KEY_ESCAPE)) {
        editor->fase = EditorFase::Menu;
    }
    (void)pediuSair;
}

static void DesenharModificarLista(EditorMapa* editor, MapaMundo* mapa, bool* pediuSair) {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{10, 14, 22, 240});
    DrawText("MODIFICAR MAPA EXISTENTE", (sw - MeasureText("MODIFICAR MAPA EXISTENTE", 32)) / 2, 50, 32, RAYWHITE);

    for (int i = 0; i < editor->lista.quantidade; ++i) {
        Rectangle r{80, static_cast<float>(120 + i * 52), static_cast<float>(sw - 160), 44};
        const bool sel = i == editor->listaSelecionada;
        DesenharBotaoUI(r, editor->lista.itens[i].nome, Hover(r) || sel,
                        sel ? Color{60, 100, 150, 255} : Color{35, 42, 55, 255});
        if (Clicou(r)) editor->listaSelecionada = i;
    }

    Rectangle rAbrir = BotaoCentro(static_cast<float>(sh) - 120, 260, 50);
    Rectangle rVoltar = BotaoCentro(static_cast<float>(sh) - 60, 260, 44);
    DesenharBotaoUI(rAbrir, "Abrir mapa", Hover(rAbrir), Color{40, 110, 70, 255});
    DesenharBotaoUI(rVoltar, "Voltar", Hover(rVoltar), Color{70, 40, 45, 255});

    if (Clicou(rAbrir) && editor->lista.quantidade > 0) {
        const MapaListaItem& it = editor->lista.itens[std::clamp(editor->listaSelecionada, 0, editor->lista.quantidade - 1)];
        if (MapaEhCasaBase(it.caminho) || MapaEhCasaBase(it.nome)) {
            EditorDefinirStatus(editor, "casa base protegida");
        } else if (it.builtin) {
            MapaCarregarPadrao(mapa);
            CopiarStr(editor->caminhoArquivo, kMaxCaminhoMapa, "maps/arena_inicial.map");
            editor->fase = EditorFase::Construindo;
            editor->sujo = false;
            EditorDefinirStatus(editor, TextFormat("editando: %s", mapa->nome));
        } else {
            MapaCarregarDeArquivo(mapa, it.caminho);
            CopiarStr(editor->caminhoArquivo, kMaxCaminhoMapa, it.caminho);
            editor->fase = EditorFase::Construindo;
            editor->sujo = false;
            EditorDefinirStatus(editor, TextFormat("editando: %s", mapa->nome));
        }
    }
    if (Clicou(rVoltar) || IsKeyPressed(KEY_ESCAPE)) editor->fase = EditorFase::Menu;
    (void)pediuSair;
}

static void DesenharUIConstrucao(EditorMapa* editor, MapaMundo* mapa, bool* pediuSair) {
    const int sw = GetScreenWidth();
    DrawRectangle(0, 0, sw, 52, Color{12, 18, 28, 230});
    DrawText("Dir olha | WASD voa | Esq coloca | Del remove | Q/E gira | F encaixe | Z cor | N dia/noite",
             12, 16, 15, RAYWHITE);

    DrawRectangleRounded(Rectangle{8, 60, 178, 430}, 0.08f, 6, Color{10, 14, 22, 230});
    DrawText("Pecas", 24, 70, 18, Color{180, 200, 230, 255});

    int q = 0;
    const PresetEditor* presets = EditorPresets(&q);
    const Vector2 mouse = GetMousePosition();
    for (int i = 0; i < q; ++i) {
        Rectangle r{16, 96.0f + i * 34.0f, 160, 30};
        const bool sel = i == editor->preset;
        DrawRectangleRounded(r, 0.15f, 6, sel ? Color{50, 90, 140, 255} : Color{22, 28, 38, 255});
        DrawText(TextFormat("%d %s", i + 1, presets[i].nome), static_cast<int>(r.x + 8),
                 static_cast<int>(r.y + 6), 16, RAYWHITE);
        if (Hover(r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            editor->preset = i;
            editor->halfX = presets[i].halfX;
            editor->halfZ = presets[i].halfZ;
            editor->altura = presets[i].altura;
        }
    }

    DrawText(TextFormat("%s", mapa->nome), 200, 64, 18, Color{220, 230, 240, 255});
    DrawText(TextFormat("Tam: %.0f | %s", mapa->tamanho, mapa->noite ? "Noite" : "Dia"), 200, 86, 16,
             Color{180, 190, 210, 255});
    DrawText(TextFormat("Peca %.2fx%.2fx%.2f  yaw %.0f  |  %s",
                        editor->halfX, editor->halfZ, editor->altura, editor->yaw,
                        editor->encaixeSolido ? "ENCAIXE SOLIDO" : "ENCAIXE LIVRE"),
             200, 108, 16, editor->encaixeSolido ? Color{160, 255, 180, 255} : Color{255, 180, 120, 255});
    DrawRectangle(200, 132, 28, 18, editor->corPeca);
    DrawRectangleLines(200, 132, 28, 18, RAYWHITE);
    if (editor->sujo) DrawText("* nao salvo", 240, 132, 16, Color{255, 180, 90, 255});
    if (editor->statusTempo > 0.0f) DrawText(editor->status, 200, 156, 16, Color{160, 255, 150, 255});

    // Cores
    for (int i = 0; i < 10; ++i) {
        Rectangle c{16.0f + (i % 5) * 32.0f, 450.0f + (i / 5) * 28.0f, 28, 24};
        DrawRectangleRec(c, kCoresRapidas[i]);
        if (Clicou(c)) editor->corPeca = kCoresRapidas[i];
    }

    Rectangle rSalvar{static_cast<float>(sw) * 0.5f - 160, static_cast<float>(GetScreenHeight()) - 56, 150, 40};
    Rectangle rSair{static_cast<float>(sw) * 0.5f + 10, static_cast<float>(GetScreenHeight()) - 56, 150, 40};
    DesenharBotaoUI(rSalvar, "Salvar", Hover(rSalvar), Color{30, 90, 55, 255});
    DesenharBotaoUI(rSair, "Sair", Hover(rSair), Color{90, 35, 40, 255});

    if (Clicou(rSalvar)) {
        char caminho[kMaxCaminhoMapa];
        CopiarStr(caminho, sizeof(caminho), editor->caminhoArquivo);
        if (std::strstr(caminho, "__builtin") || caminho[0] == '\0') {
            MapaResolverCaminhoSalvar(caminho, sizeof(caminho));
        }
        if (MapaEhCasaBase(caminho) || MapaEhCasaBase(mapa->nome)) {
            EditorDefinirStatus(editor, "casa base protegida: nao pode salvar");
        } else if (MapaSalvarEmArquivo(*mapa, caminho)) {
            MapaSalvarEmArquivo(*mapa, "maps/mapa_atual.map");
            MapaSalvarEmArquivo(*mapa, "../maps/mapa_atual.map");
            CopiarStr(editor->caminhoArquivo, kMaxCaminhoMapa, caminho);
            editor->sujo = false;
            EditorDefinirStatus(editor, TextFormat("salvo: %s", caminho));
        } else {
            EditorDefinirStatus(editor, "falha ao salvar");
        }
    }
    if (Clicou(rSair) || IsKeyPressed(KEY_ESCAPE)) {
        editor->fase = EditorFase::Menu;
    }
    (void)mouse;
    (void)pediuSair;
}

void EditorDesenharUI(EditorMapa* editor, MapaMundo* mapa, bool* pediuSair) {
    if (!editor->ativo) return;
    if (pediuSair) *pediuSair = false;

    switch (editor->fase) {
        case EditorFase::Menu: DesenharMenuPrincipal(editor, pediuSair); break;
        case EditorFase::CriarNovo: DesenharCriarNovo(editor, mapa, pediuSair); break;
        case EditorFase::ModificarLista: DesenharModificarLista(editor, mapa, pediuSair); break;
        case EditorFase::Construindo: DesenharUIConstrucao(editor, mapa, pediuSair); break;
    }
}

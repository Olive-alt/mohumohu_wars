//=============================================================================
//
// 地面表示 ＋ フィールドマスク減速 [field.cpp]
// Author :
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "field.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include "player.h"       // PLAYER（prevPos / pos を使う）
#include "stageobject.h"  // ステージ配置の外接矩形推定に使う（pos.x,z）

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;

#undef max            // Windowsマクロを無効化
#undef min

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX     (1)                 // 地面テクスチャの数（1枚だけ）
#define MAX_POLYGON     (1)                 // 1枚の四角ポリゴン
#define SIZE_X          (2000.0f)           // 地面の横幅（ワールドX）
#define SIZE_Z          (2000.0f)           // 地面の奥行（ワールドZ）

// 画像ファイルパス（お好みで変更）
static const char* kFieldTexPath = "data/TEXTURE/beach_mask.png"; // 非タイリングの大きい画像
static const char* kDefaultMask = "data/beach_mask.pgm";        // 減速マスク(PGM 1枚)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
static HRESULT MakeVertexField(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;                     // 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };         // テクスチャ情報

static FIELD                       g_Field;                                   // 地面パラメータ
static int                         g_TexNo;                                   // テクスチャ番号

// 非タイリングの1枚画像を 0..1 のUVで貼る四角形
static VERTEX_3D g_VertexArray[4] = {
    //  ３Ｄ座標                                   法線                 RGBA                       UV
    { XMFLOAT3(-SIZE_X * 0.5f, 0.0f,  SIZE_Z * 0.5f), XMFLOAT3(0,1,0), XMFLOAT4(1,1,1,1), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(SIZE_X * 0.5f, 0.0f,  SIZE_Z * 0.5f), XMFLOAT3(0,1,0), XMFLOAT4(1,1,1,1), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-SIZE_X * 0.5f, 0.0f, -SIZE_Z * 0.5f), XMFLOAT3(0,1,0), XMFLOAT4(1,1,1,1), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(SIZE_X * 0.5f, 0.0f, -SIZE_Z * 0.5f), XMFLOAT3(0,1,0), XMFLOAT4(1,1,1,1), XMFLOAT2(1.0f, 1.0f) },
};

// 外部（ステージ配置）参照
extern std::vector<STAGE_OBJECT> g_StageObjects;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitField(void)
{
    // 頂点バッファの作成
    MakeVertexField();

    // テクスチャ生成（1枚だけ）
    D3DX11CreateShaderResourceViewFromFile(GetDevice(), kFieldTexPath, NULL, NULL, &g_Texture[0], NULL);

    // 位置・回転・スケールの初期設定（回転0前提でUV->XZを対応）
    g_Field.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    g_Field.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
    g_Field.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
    XMStoreFloat4x4(&g_Field.mtxWorld, DirectX::XMMatrixIdentity());

    g_TexNo = 0;

    // （任意）既定マスクを読みたいならここで
    // Field_LoadMaskPGM(kDefaultMask);

    return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitField(void)
{
    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; }

    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (g_Texture[i]) { g_Texture[i]->Release(); g_Texture[i] = NULL; }
    }
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateField(void)
{
#ifdef _DEBUG
    if (GetKeyboardPress(DIK_R)) { g_Field.rot = XMFLOAT3(0, 0, 0); }
    PrintDebugProc("Field: non-tiling single image (%.0fx%.0f)\n", SIZE_X, SIZE_Z);
#endif
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawField(void)
{
    // 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

    // プリミティブトポロジ設定（TRIANGLESTRIP, 4頂点）
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // マテリアル設定
    MATERIAL material; ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1, 1, 1, 1);
    SetMaterial(material);

    // テクスチャ設定
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

    // ワールド行列
    using namespace DirectX;
    XMMATRIX mtxS = XMMatrixScaling(g_Field.scl.x, g_Field.scl.y, g_Field.scl.z);
    XMMATRIX mtxR = XMMatrixRotationRollPitchYaw(g_Field.rot.x, g_Field.rot.y, g_Field.rot.z);
    XMMATRIX mtxT = XMMatrixTranslation(g_Field.pos.x, g_Field.pos.y, g_Field.pos.z);
    XMMATRIX mtxW = XMMatrixIdentity();
    mtxW = XMMatrixMultiply(mtxW, mtxS);
    mtxW = XMMatrixMultiply(mtxW, mtxR);
    mtxW = XMMatrixMultiply(mtxW, mtxT);
    SetWorldMatrix(&mtxW);
    XMStoreFloat4x4(&g_Field.mtxWorld, mtxW);

    // 描画
    GetDeviceContext()->Draw(4, 0);
}

//=============================================================================
// 頂点生成
//=============================================================================
static HRESULT MakeVertexField(void)
{
    D3D11_BUFFER_DESC bd; ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sd; ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = g_VertexArray;

    GetDevice()->CreateBuffer(&bd, &sd, &g_VertexBuffer);
    return S_OK;
}

//=============================================================================
// フィールド減速：PGM 1枚マスク
//=============================================================================
namespace {
    struct AREA_MASK {
        int   w = 0, h = 0;
        float minX = -SIZE_X * 0.5f;   // マッピング範囲（ワールド）
        float maxX = SIZE_X * 0.5f;
        float minZ = -SIZE_Z * 0.5f;
        float maxZ = SIZE_Z * 0.5f;
        std::vector<unsigned char> pix;
        bool  ready = false;
    } gMask;

    // 依存：ステージ配置から大まかなXZ範囲を推定（必要なら呼ぶ）
    static void ComputeStageBoundsXZ(float& outMinX, float& outMaxX, float& outMinZ, float& outMaxZ)
    {
        bool inited = false;
        for (const auto& o : g_StageObjects) {
            if (!inited) { outMinX = outMaxX = o.pos.x; outMinZ = outMaxZ = o.pos.z; inited = true; }
            else {
                outMinX = std::min(outMinX, o.pos.x); outMaxX = std::max(outMaxX, o.pos.x);
                outMinZ = std::min(outMinZ, o.pos.z); outMaxZ = std::max(outMaxZ, o.pos.z);
            }
        }
        if (!inited) { outMinX = -SIZE_X * 0.5f; outMaxX = SIZE_X * 0.5f; outMinZ = -SIZE_Z * 0.5f; outMaxZ = SIZE_Z * 0.5f; }
    }

    static bool LoadPGM(const char* path, int& w, int& h, std::vector<unsigned char>& buf)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;
        std::string magic; f >> magic; if (magic != "P5") return false;
        auto skipC = [&]() { while (f.peek() == '#') { std::string d; std::getline(f, d); } };
        skipC(); f >> w; skipC(); f >> h; skipC(); int maxv; f >> maxv; if (maxv <= 0 || maxv > 255) return false; f.get();
        buf.resize((size_t)w * h);
        f.read((char*)buf.data(), buf.size());
        return (bool)f;
    }
}

// 範囲は地面四角（SIZE_X/Z）に揃える想定。必要なら ComputeStageBoundsXZ で自動推定に差し替え可。
bool Field_LoadMaskPGM(const char* pgmPath)
{
    gMask = AREA_MASK{};
    if (!pgmPath) return false;
    if (!LoadPGM(pgmPath, gMask.w, gMask.h, gMask.pix)) return false;

    // ステージに置かれているオブジェクトの XZ から外接矩形を推定し、少し広げる
    float minX, maxX, minZ, maxZ;
    ComputeStageBoundsXZ(minX, maxX, minZ, maxZ);            // 既存の静的ヘルパー
    const float padX = (maxX - minX) * 0.05f;                // 5% だけ余白
    const float padZ = (maxZ - minZ) * 0.05f;

    gMask.minX = minX - padX;
    gMask.maxX = maxX + padX;
    gMask.minZ = minZ - padZ;
    gMask.maxZ = maxZ + padZ;

    gMask.ready = true;
    PrintDebugProc("FieldMask loaded: %s (%dx%d)  bounds:[%.1f..%.1f]x[%.1f..%.1f]\n",
        pgmPath, gMask.w, gMask.h, gMask.minX, gMask.maxX, gMask.minZ, gMask.maxZ);
    return true;
}

// 指定座標の速度係数（白=1, 黒=minScale）  ※V軸を画像都合で反転
float Field_SpeedScaleAt(const XMFLOAT3& pos, float minScale)
{
    if (!gMask.ready || gMask.w <= 0 || gMask.h <= 0) return 1.0f;

    const float u = (pos.x - gMask.minX) / (gMask.maxX - gMask.minX);
    float v = (pos.z - gMask.minZ) / (gMask.maxZ - gMask.minZ);
    v = 1.0f - v;                                           // ← 画像のYは上→下なので反転

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return 1.0f;

    const int ix = (int)std::floor(u * (gMask.w - 1));
    const int iy = (int)std::floor(v * (gMask.h - 1));
    const unsigned char val = gMask.pix[(size_t)iy * gMask.w + ix];

    const float t = val / 255.0f;
    const float ms = std::max(0.0f, std::min(minScale, 1.0f));
    return ms + (1.0f - ms) * t;
}

// 1フレームの移動量(XZ)に減速を適用
void Field_ApplySlowdown(PLAYER* p, float minScale)
{
    if (!p || !p->use) return;

    const float s = Field_SpeedScaleAt(p->pos, minScale);
    const float dx = p->pos.x - p->prevPos.x;
    const float dz = p->pos.z - p->prevPos.z;

    p->pos.x = p->prevPos.x + dx * s;
    p->pos.z = p->prevPos.z + dz * s;
}

// 黒ほど沈む量（maxSink）。白は0。※V軸反転をこちらにも適用
float Field_SinkOffsetAt(const XMFLOAT3& pos, float maxSink)
{
    if (!gMask.ready || gMask.w <= 0 || gMask.h <= 0) return 0.0f;

    const float u = (pos.x - gMask.minX) / (gMask.maxX - gMask.minX);
    float v = (pos.z - gMask.minZ) / (gMask.maxZ - gMask.minZ);
    v = 1.0f - v;                                           // 画像のYを反転

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return 0.0f;

    const int ix = (int)std::floor(u * (gMask.w - 1));
    const int iy = (int)std::floor(v * (gMask.h - 1));
    const unsigned char val = gMask.pix[(size_t)iy * gMask.w + ix];

    const float t = val / 255.0f;                           // 白=1, 黒=0
    return (1.0f - t) * std::max(0.0f, maxSink);            // 黒ほど沈む
}

// 疑似吸着＋減速（レイ不使用・縁の誤判定を抑える）
void Field_ApplyPseudoSnapAndSlow(PLAYER* p,
    float baseY,       // 水側の基準高さ（例：GetWaterLevel()+半径）
    float minScale,    // 水100%での最小速度係数
    float maxSink)     // 水100%での最大沈み込み量
{
    if (!p || !p->use) return;

    // 近傍床の取得（XZ距離と高さ近さで“床扱い”にする）
    const float centerOff = (p->radius > 0.01f) ? p->radius : 14.0f;
    DirectX::XMFLOAT3 nf{};
    const bool haveFloor = StageGetNearestFloorPoint(p->pos, &nf, centerOff);

    // 1) “床扱い”の基準（ヒステリシス無しの単純近接）
    //    ENTER_XZ 以内 & Y差 ENTER_DY 以内 → 床として即スナップ
    const float ENTER_XZ = 28.0f;   // 島の縁を確実に拾うため少し広め
    const float ENTER_DY = 18.0f;   // 高さ差が小さければ床
    if (haveFloor) {
        const float dx = p->pos.x - nf.x;
        const float dz = p->pos.z - nf.z;
        const float distXZ = std::sqrt(dx * dx + dz * dz);
        const float dy = std::fabs(p->pos.y - nf.y);
        if (distXZ <= ENTER_XZ && dy <= ENTER_DY) {
            // ★吸着：床上は即スナップ＆スロー/沈み込みは一切かけない
            p->pos.y = nf.y;
            return;
        }
    }

    // 2) ここから“床の外”のみ適用。まずマスクの水率を取る
    float waterRatio = 1.0f;  // 既定は水
    if (gMask.ready && gMask.w > 0 && gMask.h > 0) {
        // 足元プローブ矩形をUVに写し、画素の水割合（0=地面,1=水）を求める
        const float PROBE = 60.0f;
        const float half = PROBE * 0.5f;

        auto clamp01 = [](float a) { return (a < 0.f) ? 0.f : (a > 1.f ? 1.f : a); };
        float u0 = ((p->pos.x - half) - gMask.minX) / (gMask.maxX - gMask.minX);
        float u1 = ((p->pos.x + half) - gMask.minX) / (gMask.maxX - gMask.minX);
        float v0 = ((p->pos.z - half) - gMask.minZ) / (gMask.maxZ - gMask.minZ);
        float v1 = ((p->pos.z + half) - gMask.minZ) / (gMask.maxZ - gMask.minZ);
        float vv0 = clamp01(1.0f - v1), vv1 = clamp01(1.0f - v0);
        u0 = clamp01(u0); u1 = clamp01(u1);

        int x0 = (int)std::floor(u0 * (gMask.w - 1));
        int x1 = (int)std::ceil(u1 * (gMask.w - 1));
        int y0 = (int)std::floor(vv0 * (gMask.h - 1));
        int y1 = (int)std::ceil(vv1 * (gMask.h - 1));
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        x0 = std::max(0, std::min(gMask.w - 1, x0));
        x1 = std::max(0, std::min(gMask.w - 1, x1));
        y0 = std::max(0, std::min(gMask.h - 1, y0));
        y1 = std::max(0, std::min(gMask.h - 1, y1));

        const unsigned char THRESH = 128; // これ以下を水とみなす
        const int step = 2;
        int waterCnt = 0, totalCnt = 0;
        for (int y = y0; y <= y1; y += step) {
            const unsigned char* row = &gMask.pix[(size_t)y * gMask.w];
            for (int x = x0; x <= x1; x += step) {
                if (row[x] <= THRESH) waterCnt++;
                totalCnt++;
            }
        }
        if (totalCnt > 0) waterRatio = (float)waterCnt / (float)totalCnt;
    }

    // 3) “床の縁”救済：床に近いほど水率を減衰させる（距離でフェード）
    //    EDGE_FREE までは完全に地面扱い、そこから EDGE_FADE で水率へ遷移
    if (haveFloor) {
        const float dx = p->pos.x - nf.x;
        const float dz = p->pos.z - nf.z;
        const float distXZ = std::sqrt(dx * dx + dz * dz);
        const float EDGE_FREE = 10.0f; // ここまでは水率=0
        const float EDGE_FADE = 20.0f; // ここから先で徐々に水率に
        float k = 0.0f;
        if (distXZ <= EDGE_FREE) k = 0.0f;
        else if (distXZ >= EDGE_FREE + EDGE_FADE) k = 1.0f;
        else k = (distXZ - EDGE_FREE) / EDGE_FADE;
        waterRatio *= k; // 近いほど水率を弱める
    }

    // 4) しきい値でスイッチ＋ブレンド（ヒステリシスっぽく）
    const float FLOOR_EPS = 0.20f;
    const float WATER_EPS = 0.80f;
    float w = waterRatio;
    if (w <= FLOOR_EPS) w = 0.0f;
    else if (w >= WATER_EPS) w = 1.0f;
    else                     w = (w - FLOOR_EPS) / (WATER_EPS - FLOOR_EPS);

    // (A) 減速：今フレームのXZ移動量に係数
    const float s_ground = 1.0f;
    const float s_water = std::max(0.0f, std::min(minScale, 1.0f));
    const float speed = (1.0f - w) * s_ground + w * s_water;
    const float dx = p->pos.x - p->prevPos.x;
    const float dz = p->pos.z - p->prevPos.z;
    p->pos.x = p->prevPos.x + dx * speed;
    p->pos.z = p->prevPos.z + dz * speed;

    // (B) 疑似吸着：水面基準へ寄せ、強さ w で沈める
    const float sink = std::max(0.0f, maxSink) * w;
    const float targetY = baseY - sink;
    const float lerp = 0.35f;
    p->pos.y = p->pos.y + (targetY - p->pos.y) * (1.0f - lerp);
}
//    プローブ＆しきい値は内部の疑似吸着に集約して扱う。
//    レイは使わない。床上判定は OBB(XZ) で行い、水側はマスクで減速/沈み込む。
void Field_ApplyImageProbeSnapAndSlow(PLAYER* p,
    float /*probeSizeWorld*/,
    unsigned char /*waterThreshold*/,
    float minScale,
    float maxSink)
{
    if (!p || !p->use) return;

    // 基準高さは「水面 + プレイヤー半径（無ければ14）」にする
    const float centerOff = (p->radius > 0.01f) ? p->radius : 14.0f;
    const float baseY = GetWaterLevel() + centerOff;

    // 実処理は疑似吸着版に委譲（床上＝通常／水域＝減速＋沈み込み）
    Field_ApplyPseudoSnapAndSlow(p, baseY, minScale, maxSink);
}

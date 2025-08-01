//=============================================================================
//
// タイマー管理 [time.cpp]
// Author : (Your Name)
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "time.h"
#include "sprite.h"
#include "fade.h"
#include <windows.h>

//-----------------------------------------------------------------------------
// ローカル構造体: タイマー管理に関する情報を1箇所で保持
//-----------------------------------------------------------------------------
struct TimerManager
{
    ID3D11Buffer* vertexBuffer = nullptr;           // 数字スプライト描画用の頂点バッファ
    ID3D11ShaderResourceView* texture = nullptr;    // 数字画像テクスチャ
    float width = TIME_TEXTURE_WIDTH;               // スプライト1桁分の幅
    float height = TIME_TEXTURE_HEIGHT;             // スプライト1桁分の高さ
    XMFLOAT3 pos = { 900.0f, 20.0f, 0.0f };         // 右上表示位置
    float timeLimit = TIME_LIMIT_DEFAULT;           // 制限時間（初期値）
    float timeRemaining = TIME_LIMIT_DEFAULT;       // 残り秒数
    bool  isTimeOver = false;                       // タイムオーバーか
    int   texNo = 0;                                // テクスチャ番号（拡張時用）
    bool  use = true;                               // 表示有効
};
static TimerManager g_Timer; // 静的な管理変数

static int s_lastDisplayedSeconds = -1;
static int s_digits[4] = { 0 };
//-----------------------------------------------------------------------------
// Δタイム管理：ゲーム全体で1つだけ必要
//-----------------------------------------------------------------------------
static LARGE_INTEGER g_Frequency;
static LARGE_INTEGER g_PrevTime;
static float g_DeltaTime = 0.0f;

//-----------------------------------------------------------------------------
// 数字テクスチャ（現状1枚のみ）
//-----------------------------------------------------------------------------
static const char* TEXTURE_PATH = "data/TEXTURE/Score/ui_num.png";

//=============================================================================
// 経過時間（Δ秒）管理
//=============================================================================
// 高精度タイマーの初期化（ゲーム開始時1回呼ぶ）
void InitDeltaTime(void)
{
    QueryPerformanceFrequency(&g_Frequency);
    QueryPerformanceCounter(&g_PrevTime);
    g_DeltaTime = 0.0f;
}

// 前フレームとの経過秒数を毎フレーム先頭で計測
void UpdateDeltaTime(void)
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    g_DeltaTime = (float)(currentTime.QuadPart - g_PrevTime.QuadPart) / (float)g_Frequency.QuadPart;
    g_PrevTime = currentTime;
}

// 現在のΔ秒数を取得（主に時間加算・減算用）
float GetDeltaTime(void)
{
    return g_DeltaTime;
}

//=============================================================================
// タイマー管理の初期化
//=============================================================================
HRESULT InitTime(void)
{
    ID3D11Device* pDevice = GetDevice();

    // テクスチャ読み込み（1枚だけだが、将来の拡張も考慮して構造体で管理）
    if (g_Timer.texture == nullptr) {
        D3DX11CreateShaderResourceViewFromFile(pDevice, TEXTURE_PATH, NULL, NULL, &g_Timer.texture, NULL);
    }

    // 頂点バッファ作成（2Dスプライト描画用）
    if (g_Timer.vertexBuffer == nullptr) {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(VERTEX_3D) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(&bd, NULL, &g_Timer.vertexBuffer);
    }

    // 時間・状態リセット
    g_Timer.timeLimit = TIME_LIMIT_DEFAULT;
    g_Timer.timeRemaining = g_Timer.timeLimit;
    g_Timer.isTimeOver = false;
    g_Timer.use = true;

    InitDeltaTime(); // Δ秒計測も初期化

    return S_OK;
}

//=============================================================================
// リソース解放
//=============================================================================
void UninitTime(void)
{
    if (g_Timer.vertexBuffer) {
        g_Timer.vertexBuffer->Release();
        g_Timer.vertexBuffer = nullptr;
    }
    if (g_Timer.texture) {
        g_Timer.texture->Release();
        g_Timer.texture = nullptr;
    }
}

//=============================================================================
// 残り時間の更新
//=============================================================================
// 「ゲーム進行のタイミング管理」のため、Δ秒分だけ減算。
// タイムオーバー時はリザルト画面へ遷移要求。
void UpdateTime(void)
{
    if (!g_Timer.isTimeOver) {
        g_Timer.timeRemaining -= GetDeltaTime();
        if (g_Timer.timeRemaining <= 0.0f) {
            g_Timer.timeRemaining = 0.0f;
            g_Timer.isTimeOver = true;
        }
    }
    else {
        // タイムオーバー後にリザルトへ遷移（1度だけ発火したいならフラグ追加を推奨）
        SetFade(FADE_OUT, MODE_RESULT);
    }
}

//=============================================================================
// 残り時間の描画
//=============================================================================
// 1桁ごとに数字スプライトを右から左へ順に並べる（MMSS想定）。
// ゲーム中のプレイヤーUI等の邪魔にならない右上付近に表示。
void DrawTime(void)
{
    if (!g_Timer.use) return;

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_Timer.vertexBuffer, &stride, &offset);
    SetWorldViewProjection2D();
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    MATERIAL material = {};
    material.Diffuse = XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Timer.texture);

    int totalSeconds = (int)g_Timer.timeRemaining;
    if (totalSeconds != s_lastDisplayedSeconds) {
        // 秒数が変化したときだけ分解
        for (int i = 0; i < TIME_DIGITS; ++i) {
            s_digits[i] = totalSeconds % 10;
            totalSeconds /= 10;
        }
        s_lastDisplayedSeconds = (int)g_Timer.timeRemaining;
    }

    // 数字配列から描画
    for (int i = 0; i < TIME_DIGITS; ++i) {
        int digit = s_digits[i];
        float px = g_Timer.pos.x - g_Timer.width * i;
        float py = g_Timer.pos.y;
        float tw = 0.1f;
        float tx = static_cast<float>(digit) * tw;
        SetSpriteColor(g_Timer.vertexBuffer, px, py, g_Timer.width, g_Timer.height, tx, 0.0f, tw, 1.0f, XMFLOAT4(1, 1, 1, 1));
        GetDeviceContext()->Draw(4, 0);
    }
}

//=============================================================================
// 外部API：時間設定・状態取得
//=============================================================================
// ゲーム開始/再スタート時に制限秒数をセット
void SetTimeLimit(float sec)
{
    g_Timer.timeLimit = sec;
    g_Timer.timeRemaining = sec;
    g_Timer.isTimeOver = false;
}

// 残り時間（秒）を取得
float GetRemainingTime(void)
{
    return g_Timer.timeRemaining;
}

// タイムオーバーか判定
bool IsTimeOver(void)
{
    return g_Timer.isTimeOver;
}

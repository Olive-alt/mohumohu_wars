//=============================================================================
//
// スコア処理 [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"
#include "player.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH        (16)    // キャラサイズ
#define TEXTURE_HEIGHT       (32)    // 
#define TEXTURE_MAX          (1)     // テクスチャの数
#define SCORE_PLAYER_MAX     (2)     // プレイヤーの数（2人用に変更）
#define SCORE_DIGIT          (6)     // 表示桁数
#define SCORE_MAX            (999999)

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;      // 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };    // テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
    "data/TEXTURE/Score/ui_num.png",
};

// 各プレイヤーのスコア
static int    g_Score[SCORE_PLAYER_MAX];    // スコア（複数人分）

static BOOL   g_Use;            // TRUE:使っている  FALSE:未使用
static float  g_w, g_h;         // 幅と高さ
static int    g_TexNo;          // テクスチャ番号

static BOOL   g_Load = FALSE;

// 各プレイヤーのスコア表示座標
static XMFLOAT3 g_Pos[SCORE_PLAYER_MAX] = {
    XMFLOAT3(500.0f, 20.0f, 0.0f),   // プレイヤー1
    XMFLOAT3(700.0f, 20.0f, 0.0f),   // プレイヤー2
    // 3人目以降はここに追加
};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitScore(void)
{
    ID3D11Device* pDevice = GetDevice();

    //テクスチャ生成
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_TexturName[i],
            NULL,
            NULL,
            &g_Texture[i],
            NULL);
    }

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    // プレイヤーの初期化
    g_Use = TRUE;
    g_w = TEXTURE_WIDTH;
    g_h = TEXTURE_HEIGHT;
    g_TexNo = 0;

    //// 各プレイヤーのスコア初期化
    //for (int i = 0; i < SCORE_PLAYER_MAX; i++)
    //    g_Score[i] = 0;

    g_Load = TRUE;
    return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitScore(void)
{
    if (g_Load == FALSE) return;

    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (g_Texture[i])
        {
            g_Texture[i]->Release();
            g_Texture[i] = NULL;
        }
    }

    g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateScore(void)
{
#ifdef _DEBUG   // デバッグ情報を表示する
    // 必要ならここにデバッグ出力を書く
#endif
}

//=============================================================================
// 描画処理（全プレイヤー分のスコアを描画）
//=============================================================================
void DrawScore(void)
{
    if (GetMode() == MODE_RESULT)
    {
        DrawScoreAbovePlayers();
        return;
    }

    // 頂点バッファ設定
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

    // マトリクス設定
    SetWorldViewProjection2D();

    // プリミティブトポロジ設定
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // テクスチャ設定
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

    // 各プレイヤーごとにスコアを描画
    for (int p = 0; p < SCORE_PLAYER_MAX; p++)
    {
        int number = g_Score[p];
        for (int i = 0; i < SCORE_DIGIT; i++)
        {
            // 今回表示する桁の数字
            float x = (float)(number % 10);

            // スコアの位置やテクスチャー座標を反映
            float px = g_Pos[p].x - g_w * i; // スコアの表示位置X
            float py = g_Pos[p].y;           // スコアの表示位置Y
            float pw = g_w;                  // スコアの表示幅
            float ph = g_h;                  // スコアの表示高さ

            float tw = 1.0f / 10;            // テクスチャの幅
            float th = 1.0f / 1;             // テクスチャの高さ
            float tx = x * tw;               // テクスチャの左上X座標
            float ty = 0.0f;                 // テクスチャの左上Y座標

            // １枚のポリゴンの頂点とテクスチャ座標を設定
            SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
                XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

            // ポリゴン描画
            GetDeviceContext()->Draw(4, 0);

            // 次の桁へ
            number /= 10;
        }
    }
}

//=============================================================================
// スコアを加算する
// 引数: playerIndex : プレイヤー番号
//      add         : 追加する点数。マイナスも可能
//=============================================================================
void AddScore(int playerIndex, int add)
{
    if (playerIndex < 0 || playerIndex >= SCORE_PLAYER_MAX) return;
    g_Score[playerIndex] += add;
    if (g_Score[playerIndex] > SCORE_MAX)
    {
        g_Score[playerIndex] = SCORE_MAX;
    }
}

//=============================================================================
// 指定プレイヤーのスコアを取得する
//=============================================================================
int GetScore(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= SCORE_PLAYER_MAX) return 0;
    return g_Score[playerIndex];
}

//スコアをリセットする
void ResetScore(void)
{
    for (int i = 0; i < SCORE_PLAYER_MAX; i++)
        g_Score[i] = 0;
}

//プレイヤーの上にスコアを表示する
void DrawScoreAbovePlayers(void)
{
    CAMERA* cam = GetCamera();
    XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);
    PLAYER* players = GetPlayer();

    // 数字一桁のサイズ（ワールド座標系）
    const float DIGIT_WIDTH = 4.0f;
    const float DIGIT_HEIGHT = 8.0f;

    // スコア表示用テクスチャのマテリアル設定
    MATERIAL mat = {};
    mat.Ambient = mat.Diffuse = XMFLOAT4(1, 1, 1, 1);
    mat.noTexSampling = 0;
    SetMaterial(mat);

    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

    for (int p = 0; p < SCORE_PLAYER_MAX; p++)
    {
        if (!players[p].use) continue;

        XMFLOAT3 scorePos = players[p].pos;
        scorePos.y += 28.0f; // HPバーの上に表示

        int score = g_Score[p];

        // スコアを左から右に描画するための数字配列へ変換
        char digits[16];
        int numDigits = 0;
        if (score == 0) {
            digits[0] = 0;
            numDigits = 1;
        }
        else {
            int temp = score;
            char tempDigits[16];
            int count = 0;
            while (temp > 0) {
                tempDigits[count++] = temp % 10;
                temp /= 10;
            }
            // 配列を反転して左から右に並べる
            for (int i = 0; i < count; i++) {
                digits[i] = tempDigits[count - i - 1];
            }
            numDigits = count;
        }

        float totalWidth = DIGIT_WIDTH * numDigits;
        for (int d = 0; d < numDigits; d++)
        {
            int digit = digits[d];

            // ビルボード行列を計算（カメラの方向を向く）
            XMMATRIX mtxBillboard = XMMatrixIdentity();
            mtxBillboard.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0.0f);
            mtxBillboard.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0.0f);
            mtxBillboard.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0.0f);

            // スコア全体を中央揃えするためのXオフセット
            float digitOffsetX = -totalWidth / 2 + DIGIT_WIDTH / 2 + DIGIT_WIDTH * d;

            // ワールド行列作成（スケール・ビルボード・位置）
            XMMATRIX mtxScale = XMMatrixIdentity();
            XMMATRIX mtxTrans = XMMatrixTranslation(scorePos.x + digitOffsetX, scorePos.y, scorePos.z);
            XMMATRIX mtxWorld = mtxScale * mtxBillboard * mtxTrans;
            SetWorldMatrix(&mtxWorld);

            // 数字一桁分の四角形（ポリゴン）を作成
            struct VERTEX_3D {
                XMFLOAT3 Position;
                XMFLOAT3 Normal;
                XMFLOAT4 Diffuse;
                XMFLOAT2 TexCoord;
            };
            VERTEX_3D vtx[4];

            float left = -DIGIT_WIDTH / 2.0f;
            float right = +DIGIT_WIDTH / 2.0f;
            float top = DIGIT_HEIGHT / 2.0f;
            float bottom = -DIGIT_HEIGHT / 2.0f;

            float tw = 1.0f / 10.0f;
            float tx = digit * tw;
            float th = 1.0f;

            vtx[0].Position = XMFLOAT3(left, top, 0);     vtx[0].TexCoord = XMFLOAT2(tx, 0);
            vtx[1].Position = XMFLOAT3(right, top, 0);    vtx[1].TexCoord = XMFLOAT2(tx + tw, 0);
            vtx[2].Position = XMFLOAT3(left, bottom, 0);  vtx[2].TexCoord = XMFLOAT2(tx, th);
            vtx[3].Position = XMFLOAT3(right, bottom, 0); vtx[3].TexCoord = XMFLOAT2(tx + tw, th);

            for (int i = 0; i < 4; i++) {
                vtx[i].Normal = XMFLOAT3(0, 0, -1);
                vtx[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
            }

            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(vtx);
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA init = { vtx, 0, 0 };
            ID3D11Buffer* vb = NULL;
            GetDevice()->CreateBuffer(&bd, &init, &vb);

            UINT stride = sizeof(VERTEX_3D);
            UINT offset = 0;
            GetDeviceContext()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
            GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            GetDeviceContext()->Draw(4, 0);

            if (vb) vb->Release();
        }
    }
}

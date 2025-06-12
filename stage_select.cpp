// stage_select.cpp
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "stage_select.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
//stage制作したら変更
#define MAX_STAGES 2
#define TEXTURE_WIDTH_STAGE_ICON (128)
#define TEXTURE_HEIGHT_STAGE_ICON (128)
#define TEXTURE_WIDTH_STAGE_TITLE (400)
#define TEXTURE_HEIGHT_STAGE_TITLE (100)

static int g_SelectedStage = 1;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_BackgroundTexture = NULL;
static ID3D11ShaderResourceView* g_StageTitleTexture = NULL;
static ID3D11ShaderResourceView* g_StageIconsTexture = NULL;

static const char* g_BackgroundTextureName = "data/TEXTURE/stage_select_bg.jpg";
static const char* g_StageTitleTextureName = "data/TEXTURE/stage_title.png";
static const char* g_StageIconsTextureName = "data/TEXTURE/stage_icons.png";

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

HRESULT InitStageSelect(void)
{
    ID3D11Device* pDevice = GetDevice();

    // 背景テクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_BackgroundTextureName,
        NULL,
        NULL,
        &g_BackgroundTexture,
        NULL);

    // ステージタイトルテクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_StageTitleTextureName,
        NULL,
        NULL,
        &g_StageTitleTexture,
        NULL);

    // ステージアイコンテクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_StageIconsTextureName,
        NULL,
        NULL,
        &g_StageIconsTexture,
        NULL);

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_SelectedStage = 1;
    alpha = 1.0f;
    flag_alpha = TRUE;
    g_Load = TRUE;

    return S_OK;
}

void UninitStageSelect(void)
{
    if (g_Load == FALSE) return;

    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    if (g_BackgroundTexture)
    {
        g_BackgroundTexture->Release();
        g_BackgroundTexture = NULL;
    }

    if (g_StageTitleTexture)
    {
        g_StageTitleTexture->Release();
        g_StageTitleTexture = NULL;
    }

    if (g_StageIconsTexture)
    {
        g_StageIconsTexture->Release();
        g_StageIconsTexture = NULL;
    }

    g_Load = FALSE;
}

void UpdateStageSelect(void)
{
    // ステージ選択（← → キー）
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedStage--;
        if (g_SelectedStage < 1) g_SelectedStage = MAX_STAGES;
        // ここで選択音を再生
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedStage++;
        if (g_SelectedStage > MAX_STAGES) g_SelectedStage = 1;
        // ここで選択音を再生
    }

    // 決定キー
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        // 選択されたステージを設定してゲーム開始
        SetFade(FADE_OUT, MODE_GAME);
    }
    // キャンセルキー
    else if (GetKeyboardTrigger(DIK_ESCAPE))
    {
        SetFade(FADE_OUT, MODE_PLAYER_SELECT);
    }

    // 点滅効果
    if (flag_alpha == TRUE)
    {
        alpha -= 0.02f;
        if (alpha <= 0.0f)
        {
            alpha = 0.0f;
            flag_alpha = FALSE;
        }
    }
    else
    {
        alpha += 0.02f;
        if (alpha >= 1.0f)
        {
            alpha = 1.0f;
            flag_alpha = TRUE;
        }
    }
}

void DrawStageSelect(void)
{
    // 深度テストとライティングを無効化
    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);

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

    // 背景描画
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_BackgroundTexture);
        SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // ステージタイトル描画
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_StageTitleTexture);
        SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4,
            TEXTURE_WIDTH_STAGE_TITLE, TEXTURE_HEIGHT_STAGE_TITLE,
            0.0f, 0.0f, 1.0f, 1.0f,
            XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
        GetDeviceContext()->Draw(4, 0);
    }

    // ステージアイコン描画
    if (g_StageIconsTexture)
    {
        float iconSpacing = SCREEN_WIDTH / (MAX_STAGES + 1);
        float iconY = SCREEN_HEIGHT / 2;

        for (int i = 1; i <= MAX_STAGES; i++)
        {
            float iconX = iconSpacing * i;
            float iconAlpha = (i == g_SelectedStage) ? alpha : 0.5f;

            GetDeviceContext()->PSSetShaderResources(0, 1, &g_StageIconsTexture);
            SetSpriteColor(g_VertexBuffer, iconX, iconY,
                TEXTURE_WIDTH_STAGE_ICON, TEXTURE_HEIGHT_STAGE_ICON,
                (i - 1) * (1.0f / MAX_STAGES), 0.0f, 1.0f / MAX_STAGES, 1.0f,
                XMFLOAT4(1.0f, 1.0f, 1.0f, iconAlpha));
            GetDeviceContext()->Draw(4, 0);
        }
    }

    // 深度テストとライティングを有効化
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}
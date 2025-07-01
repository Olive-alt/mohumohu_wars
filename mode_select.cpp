#include "main.h"
#include "renderer.h"
#include "input.h"
#include "mode_select.h"
#include "fade.h"
#include "sprite.h"
#include "sound.h"
#include "debugproc.h"

#define MAX_MODES 2
#define TEXTURE_WIDTH_MODE_ICON (250)
#define TEXTURE_HEIGHT_MODE_ICON (250)

static int g_SelectedMode = 1;
static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[4] = { NULL }; // 背景 + 2 モード

static const char* g_TextureName[4] = {
    "data/TEXTURE/bg002.jpg", // 背景
    "data/TEXTURE/Select_mode/mode_1.png",
    "data/TEXTURE/Select_mode/mode_2.png",
    "data/TEXTURE/Select_mode/title.png"
};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Loaded = FALSE;

HRESULT InitModeSelect(void)
{
    ID3D11Device* pDevice = GetDevice();

    for (int i = 0; i <4; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_TextureName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_SelectedMode = 1;
    alpha = 1.0f;
    flag_alpha = TRUE;
    g_Loaded = TRUE;

    return S_OK;
}

void UninitModeSelect(void)
{
    if (!g_Loaded) return;

    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    for (int i = 0; i < 4; i++)
    {
        if (g_Texture[i])
        {
            g_Texture[i]->Release();
            g_Texture[i] = NULL;
        }
    }

    g_Loaded = FALSE;
}

void UpdateModeSelect(void)
{
    // ← → で選択
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedMode--;
        if (g_SelectedMode < 1) g_SelectedMode = MAX_MODES;
        PrintDebugProc("Selected Mode: %d\n", g_SelectedMode);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedMode++;
        if (g_SelectedMode > MAX_MODES) g_SelectedMode = 1;
        PrintDebugProc("Selected Mode: %d\n", g_SelectedMode);
    }

    // 決定（Enter）
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        if (g_SelectedMode == 1)
        {
            PrintDebugProc("スコアアタックモード 選択\n");
        }
        else if (g_SelectedMode == 2)
        {
            PrintDebugProc("サバイバルモード 選択\n");
        }

        SetFade(FADE_OUT, MODE_GAME);  // 遷移先は必要に応じて変更
    }
    // キャンセル（スペース）
    else if (GetKeyboardTrigger(DIK_SPACE))
    {
        SetFade(FADE_OUT, MODE_STAGE_SELECT);  // タイトル画面に戻る
    }

    // アルファ点滅
    if (flag_alpha)
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

void DrawModeSelect(void)
{
    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    SetWorldViewProjection2D();
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // 背景
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
        SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
            SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    {
        float titleWidth = 500.0f;
        float titleHeight = 100.0f;
        float titlePosX = SCREEN_WIDTH / 2;
        float titlePosY = 60.0f;

        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);  // タイトルはg_Texture[3]
        SetSprite(g_VertexBuffer,
            titlePosX, titlePosY,
            titleWidth, titleHeight,
            0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }
    // 両方のモードアイコンを描画（選択中は拡大）
    for (int i = 1; i <= MAX_MODES; ++i)
    {
        float scale = (i == g_SelectedMode) ? 1.2f : 1.0f;
        float iconWidth = TEXTURE_WIDTH_MODE_ICON * scale;
        float iconHeight = TEXTURE_HEIGHT_MODE_ICON * scale;

        // 配置：左(1) 中央-200px、右(2) 中央+200px（距離調整可）
        float posX = SCREEN_WIDTH / 2 + (i == 1 ? -200 : 200);
        float posY = SCREEN_HEIGHT / 2;

        // alpha：選択中は点滅、非選択はやや透明
        float iconAlpha = (i == g_SelectedMode) ? alpha : 0.6f;

        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);
        SetSpriteColor(g_VertexBuffer,
            posX, posY,
            iconWidth, iconHeight,
            0.0f, 0.0f, 1.0f, 1.0f,
            XMFLOAT4(1.0f, 1.0f, 1.0f, iconAlpha));
        GetDeviceContext()->Draw(4, 0);
    }

    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}

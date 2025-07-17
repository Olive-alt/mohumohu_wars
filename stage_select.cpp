#include "main.h"
#include "renderer.h"
#include "input.h"
#include "stage_select.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "player.h"
#include "debugproc.h"
#include "sound.h"


#define MAX_STAGES 2
#define TEXTURE_WIDTH_STAGE_ICON (200)
#define TEXTURE_HEIGHT_STAGE_ICON (300)

static int g_SelectedStage = 1;
char g_SelectedStageFile[32] = "stage1.txt";

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[4] = { NULL };

static const char* g_TexturName[4] = {
    "data/TEXTURE/bg003.jpg",
    "data/TEXTURE/Select_stage/stage1.png",
    "data/TEXTURE/Select_stage/stage2.png",
    "data/TEXTURE/Select_stage/stage_title.png"
};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

HRESULT InitStageSelect(void)
{
    ID3D11Device* pDevice = GetDevice();
    PlaySound(SOUND_LABEL_BGM_bgm_a);

    for (int i = 0; i < 4; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(
            pDevice, g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
    }

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
    if (!g_Load) return;

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

    g_Load = FALSE;
}

void UpdateStageSelect(void)
{
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedStage--;
        if (g_SelectedStage < 1) g_SelectedStage = MAX_STAGES;
        PrintDebugProc("Selected Stage: %d\n", g_SelectedStage);
        PlaySound(SOUND_LABEL_SE_switch01);

    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedStage++;
        if (g_SelectedStage > MAX_STAGES) g_SelectedStage = 1;
        PrintDebugProc("Selected Stage: %d\n", g_SelectedStage);
        PlaySound(SOUND_LABEL_SE_switch01);

    }

    // 決定キー（Enter）
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        // 選択したステージ番号に応じたファイル名をセット
        sprintf(g_SelectedStageFile, "stage%d.txt", g_SelectedStage);

        PrintDebugProc("Loading %s\n", g_SelectedStageFile);

        SetFade(FADE_OUT, MODE_GAME);
    }
    else if (GetKeyboardTrigger(DIK_SPACE))
    {
        SetFade(FADE_OUT, MODE_PLAYER_SELECT);
    }

    // 点滅
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

void DrawStageSelect(void)
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

    // タイトル
    {
        float titleWidth = 500.0f;
        float titleHeight = 100.0f;
        float titlePosX = SCREEN_WIDTH / 2;
        float titlePosY = 60.0f;

        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);  // タイトル
        SetSprite(g_VertexBuffer,
            titlePosX, titlePosY,
            titleWidth, titleHeight,
            0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // ステージアイコン
    for (int i = 1; i <= MAX_STAGES; ++i)
    {
        float scale = (i == g_SelectedStage) ? 1.2f : 1.0f;
        float iconWidth = TEXTURE_WIDTH_STAGE_ICON * scale;
        float iconHeight = TEXTURE_HEIGHT_STAGE_ICON * scale;

        float posX = SCREEN_WIDTH / 2 + (i == 1 ? -200 : 200);
        float posY = SCREEN_HEIGHT / 2 + 50.0f;

        float iconAlpha = (i == g_SelectedStage) ? alpha : 0.6f;

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

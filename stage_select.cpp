#include "main.h"
#include "renderer.h"
#include "input.h"
#include "stage_select.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "debugproc.h"

using namespace DirectX;

char g_SelectedStageFile[64] = "stage_road.txt";

#define MAX_STAGES 2
#define TEXTURE_WIDTH_STAGE_ICON  (300)
#define TEXTURE_HEIGHT_STAGE_ICON (300)

static int  g_SelectedStage = 1;
static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[4] = { NULL };

// [0]=背景, [1]=ロード, [2]=ビーチ(水上アスレ), [3]=タイトル
static const char* g_TexturName[4] = {
    "data/TEXTURE/bg003.jpg",
    "data/TEXTURE/Select_stage/stage3.png",
    "data/TEXTURE/Select_stage/stage4.png",
    "data/TEXTURE/Select_stage/stage_title.png"
};

static float alpha;
static BOOL  flag_alpha;
static BOOL  g_Load = FALSE;

static void MapStageToFile()
{
    if (g_SelectedStage == 1) {
        sprintf_s(g_SelectedStageFile, "%s", "stage_road.txt");
    }
    else {
        sprintf_s(g_SelectedStageFile, "%s", "stage_beach.txt");
    }
    PrintDebugProc("Selected file: %s\n", g_SelectedStageFile);
}

HRESULT InitStageSelect(void)
{
    ID3D11Device* pDevice = GetDevice();
    PlaySound(SOUND_LABEL_BGM_bgm_a);

    for (int i = 0; i < 4; i++) {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(
            pDevice, g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_SelectedStage = 1;
    alpha = 1.0f;
    flag_alpha = TRUE;
    g_Load = TRUE;

    MapStageToFile();
    return S_OK;
}

void UninitStageSelect(void)
{
    if (!g_Load) return;

    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; }
    for (int i = 0; i < 4; i++) {
        if (g_Texture[i]) { g_Texture[i]->Release(); g_Texture[i] = NULL; }
    }
    g_Load = FALSE;
}

void UpdateStageSelect(void)
{
    if (GetKeyboardTrigger(DIK_LEFT) || IsButtonTriggered(0, BUTTON_LEFT)) {
        g_SelectedStage--; if (g_SelectedStage < 1) g_SelectedStage = MAX_STAGES;
        MapStageToFile(); PlaySound(SOUND_LABEL_SE_switch01);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT) || IsButtonTriggered(0, BUTTON_RIGHT)) {
        g_SelectedStage++; if (g_SelectedStage > MAX_STAGES) g_SelectedStage = 1;
        MapStageToFile(); PlaySound(SOUND_LABEL_SE_switch01);
    }

    if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A)) {
        PrintDebugProc("Loading %s\n", g_SelectedStageFile);
        SetFade(FADE_OUT, MODE_MODE_SELECT);
    }
    else if (GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_B)) {
        SetFade(FADE_OUT, MODE_PLAYER_SELECT);
    }

    if (flag_alpha) { alpha -= 0.02f; if (alpha <= 0.0f) { alpha = 0.0f; flag_alpha = FALSE; } }
    else { alpha += 0.02f; if (alpha >= 1.0f) { alpha = 1.0f; flag_alpha = TRUE; } }
}

void DrawStageSelect(void)
{
    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D), offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    SetWorldViewProjection2D();
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    MATERIAL material = {}; material.Diffuse = XMFLOAT4(1, 1, 1, 1);
    SetMaterial(material);

    // 背景
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // タイトル
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, 60.0f, 500.0f, 100.0f, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // 左=ロード(1)・右=ビーチ(2)
    for (int i = 1; i <= MAX_STAGES; ++i) {
        float scale = (i == g_SelectedStage) ? 1.2f : 1.0f;
        float w = TEXTURE_WIDTH_STAGE_ICON * scale;
        float h = TEXTURE_HEIGHT_STAGE_ICON * scale;
        float x = SCREEN_WIDTH / 2 + (i == 1 ? -200.0f : 200.0f);
        float y = SCREEN_HEIGHT / 2 + 50.0f;
        float a = (i == g_SelectedStage) ? alpha : 0.6f;

        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[i]);
        SetSpriteColor(g_VertexBuffer, x, y, w, h, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, a));
        GetDeviceContext()->Draw(4, 0);
    }

    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}

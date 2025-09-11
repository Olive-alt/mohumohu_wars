// ===== player_select.cpp =====
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "player_select.h"
#include "player.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"

#define MAX_PLAYERS 4
#define MAX_CHARACTERS 4

#define TEXTURE_WIDTH_LOGO (480)
#define TEXTURE_HEIGHT_LOGO (80)
#define TEXTURE_WIDTH_PLAYER_ICON (128)
#define TEXTURE_HEIGHT_PLAYER_ICON (128)

static int g_SelectedPlayer = 0;
static int g_SelectingPlayerIndex = 0;
static int g_SelectedCharIndex[MAX_PLAYERS] = { -1, -1, -1, -1 };

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[4] = { NULL };
static ID3D11ShaderResourceView* g_PlayerIcons[MAX_CHARACTERS] = { NULL };
static ID3D11ShaderResourceView* g_SelectFrameTex[MAX_PLAYERS] = { NULL };

static const char* g_TexturName[4] = {
    "data/TEXTURE/bg003.jpg",                     // 背景
    "data/TEXTURE/copy.png",                      // ロゴ
    "data/TEXTURE/Select_player/player_title.png", // タイトル
    "data/TEXTURE/Select_player/title1.png"      // 操作説明（画面最上部固定）
};

static const char* g_PlayerIconNames[MAX_CHARACTERS] = {
    "data/TEXTURE/Select_player/player1.png",
    "data/TEXTURE/Select_player/player2.png",
    "data/TEXTURE/Select_player/player3.png",
    "data/TEXTURE/Select_player/player4.png"
};

static const char* g_FrameTexNames[4] = {
    "data/TEXTURE/Select_player/1p.png",
    "data/TEXTURE/Select_player/2p.png",
    "data/TEXTURE/Select_player/3p.png",
    "data/TEXTURE/Select_player/4p.png"
};

static BOOL g_Load = FALSE;

// 使用済みキャラクターか判定
static BOOL IsTaken(int charIdx)
{
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (p == g_SelectingPlayerIndex) continue;
        if (g_SelectedCharIndex[p] == charIdx) return TRUE;
    }
    return FALSE;
}

// 次のプレイヤーに進む（全員決定でステージへ）
static void AdvanceToNextPlayer(void)
{
    g_SelectingPlayerIndex++;
    if (g_SelectingPlayerIndex >= MAX_PLAYERS) {
        SetFade(FADE_OUT, MODE_STAGE_SELECT);
        return;
    }

    g_SelectedPlayer = 0;
    int guard = 0;
    while (IsTaken(g_SelectedPlayer) && guard < MAX_CHARACTERS) {
        g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS;
        ++guard;
    }
}

// 初期化
HRESULT InitPlayerSelect(void)
{
    ID3D11Device* pDevice = GetDevice();
    PlaySound(SOUND_LABEL_BGM_bgm_a);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        g_IsCPU[i] = false;
    }

    for (int i = 0; i < 4; i++) {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    for (int i = 0; i < MAX_CHARACTERS; i++) {
        g_PlayerIcons[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_PlayerIconNames[i], NULL, NULL, &g_PlayerIcons[i], NULL);
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        g_SelectFrameTex[i] = NULL;
        HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pDevice, g_FrameTexNames[i], NULL, NULL, &g_SelectFrameTex[i], NULL);
        if (FAILED(hr)) {
            D3DX11CreateShaderResourceViewFromFile(pDevice, g_FrameTexNames[1], NULL, NULL, &g_SelectFrameTex[i], NULL);
        }
    }

    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; }
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer))) return E_FAIL;

    g_SelectedPlayer = 0;
    g_SelectingPlayerIndex = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) g_SelectedCharIndex[i] = -1;

    g_Load = TRUE;
    return S_OK;
}

// 終了処理
void UninitPlayerSelect(void)
{
    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; }

    for (int i = 0; i < 4; i++) { if (g_Texture[i]) { g_Texture[i]->Release(); g_Texture[i] = NULL; } }
    for (int i = 0; i < MAX_CHARACTERS; i++) { if (g_PlayerIcons[i]) { g_PlayerIcons[i]->Release(); g_PlayerIcons[i] = NULL; } }
    for (int i = 0; i < MAX_PLAYERS; i++) { if (g_SelectFrameTex[i]) { g_SelectFrameTex[i]->Release(); g_SelectFrameTex[i] = NULL; } }

    g_Load = FALSE;
    g_SelectedPlayer = 0;
    g_SelectingPlayerIndex = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) { g_SelectedCharIndex[i] = -1; g_IsCPU[i] = false; }
}

// 更新
void UpdatePlayerSelect(void)
{
    if (g_SelectingPlayerIndex >= MAX_PLAYERS) return;

    if (GetKeyboardTrigger(DIK_LEFT) || IsButtonTriggered(0, BUTTON_LEFT)) {
        int guard = 0;
        do { g_SelectedPlayer = (g_SelectedPlayer - 1 + MAX_CHARACTERS) % MAX_CHARACTERS; } while (IsTaken(g_SelectedPlayer) && ++guard < MAX_CHARACTERS);
        PlaySound(SOUND_LABEL_SE_switch01);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT) || IsButtonTriggered(0, BUTTON_RIGHT)) {
        int guard = 0;
        do { g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS; } while (IsTaken(g_SelectedPlayer) && ++guard < MAX_CHARACTERS);
        PlaySound(SOUND_LABEL_SE_switch01);
    }

    if (GetKeyboardTrigger(DIK_TAB) || IsButtonTriggered(0, BUTTON_X)) {
        for (int c = 0; c < MAX_CHARACTERS; ++c) { if (!IsTaken(c)) { g_SelectedCharIndex[g_SelectingPlayerIndex] = c; break; } }
        g_IsCPU[g_SelectingPlayerIndex] = true;
        AdvanceToNextPlayer();
        return;
    }

    if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A)) {
        if (!IsTaken(g_SelectedPlayer)) { g_SelectedCharIndex[g_SelectingPlayerIndex] = g_SelectedPlayer; AdvanceToNextPlayer(); }
    }
    else if (GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_B)) {
        SetFade(FADE_OUT, MODE_TITLE);
    }
}

// 描画
void DrawPlayerSelect(void)
{
    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);

    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    SetWorldViewProjection2D();
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    MATERIAL material = {};
    material.Diffuse = XMFLOAT4(1, 1, 1, 1);
    SetMaterial(material);

    // 背景
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // ロゴ
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 6, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // タイトル
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2 + 30, 70, 400, 80, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // 操作説明
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, 70, SCREEN_WIDTH, 150, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // キャラクターアイコン
    float startX = SCREEN_WIDTH / 2 - (MAX_CHARACTERS * (TEXTURE_WIDTH_PLAYER_ICON + 20)) / 2 + TEXTURE_WIDTH_PLAYER_ICON / 2;
    float yPos = SCREEN_HEIGHT - 150;

    for (int i = 0; i < MAX_CHARACTERS; i++)
    {
        float x = startX + i * (TEXTURE_WIDTH_PLAYER_ICON + 20);

        if (g_PlayerIcons[i])
        {
            GetDeviceContext()->PSSetShaderResources(0, 1, &g_PlayerIcons[i]);
            float width = TEXTURE_WIDTH_PLAYER_ICON;
            float height = TEXTURE_HEIGHT_PLAYER_ICON;
            if (i == g_SelectedPlayer) { width += 20; height += 20; }
            SetSprite(g_VertexBuffer, x, yPos, width, height, 0, 0, 1, 1);
            GetDeviceContext()->Draw(4, 0);
        }

        for (int p = 0; p < MAX_PLAYERS; p++)
        {
            if (g_SelectedCharIndex[p] == i && g_SelectFrameTex[p])
            {
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_SelectFrameTex[p]);
                SetSprite(g_VertexBuffer, x, yPos - 15, TEXTURE_WIDTH_PLAYER_ICON * 1.2f, TEXTURE_HEIGHT_PLAYER_ICON * 1.5f, 0, 0, 1, 1);
                GetDeviceContext()->Draw(4, 0);
            }
        }
    }

    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}

void SetSelectedCharIndex(int playerIndex, int charIndex) { g_SelectedCharIndex[playerIndex] = charIndex; }
int GetSelectedCharIndex(int playerIndex) { return g_SelectedCharIndex[playerIndex]; }

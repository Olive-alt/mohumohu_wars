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
static int g_SelectedCharIndex[MAX_PLAYERS] = { -1, -1, -1 ,-1};

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[3] = { NULL };
static ID3D11ShaderResourceView* g_PlayerIcons[MAX_CHARACTERS] = { NULL };
static ID3D11ShaderResourceView* g_SelectFrameTex[MAX_PLAYERS] = { NULL }; // 1P/2P/3P

static const char* g_TexturName[3] = {
    "data/TEXTURE/bg003.jpg",
    "data/TEXTURE/copy.png",
    "data/TEXTURE/Select_player/player_title.png"
};

static const char* g_PlayerIconNames[MAX_CHARACTERS] = {
    "data/TEXTURE/Select_player/player1.png",
    "data/TEXTURE/Select_player/player2.png",
    "data/TEXTURE/Select_player/player3.png",
    "data/TEXTURE/Select_player/player4.png",
};

static const char* g_FrameTexNames[4] = {
    "data/TEXTURE/Select_player/1p.png",
    "data/TEXTURE/Select_player/2p.png",
    "data/TEXTURE/Select_player/3p.png",
    "data/TEXTURE/Select_player/4p.png",
};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

// 使用済みキャラか判定
static BOOL IsTaken(int charIdx)
{
    for (int p = 0; p < MAX_PLAYERS; ++p) {
        if (p == g_SelectingPlayerIndex) continue;
        if (g_SelectedCharIndex[p] == charIdx) return TRUE;
    }
    return FALSE;
}

// 次のプレイヤーへ進める（全員決定でステージへ）
static void AdvanceToNextPlayer(void)
{
    g_SelectingPlayerIndex++;
    if (g_SelectingPlayerIndex >= MAX_PLAYERS) {
        SetFade(FADE_OUT, MODE_STAGE_SELECT);
        return;
    }

    // その時点で未使用のキャラから初期選択候補を探す（無限ループ防止）
    g_SelectedPlayer = 0;
    int guard = 0;
    while (IsTaken(g_SelectedPlayer) && guard < MAX_CHARACTERS) {
        g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS;
        ++guard;
    }
}


HRESULT InitPlayerSelect(void)
{
    ID3D11Device* pDevice = GetDevice();
    PlaySound(SOUND_LABEL_BGM_bgm_a);

    // ★ MAX_PLAYER → MAX_PLAYERS に統一
    for (int i = 0; i < MAX_PLAYERS; i++) {
        g_IsCPU[i] = false;
    }

    for (int i = 0; i < 3; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    for (int i = 0; i < MAX_CHARACTERS; i++)
    {
        g_PlayerIcons[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_PlayerIconNames[i], NULL, NULL, &g_PlayerIcons[i], NULL);
    }

    // 1p/2p/3p/4p 枠（存在しない場合は 2p をフォールバック）
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        g_SelectFrameTex[i] = NULL;
        HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pDevice,
            g_FrameTexNames[i], NULL, NULL, &g_SelectFrameTex[i], NULL);
        if (FAILED(hr)) {
            D3DX11CreateShaderResourceViewFromFile(pDevice, g_FrameTexNames[1], NULL, NULL, &g_SelectFrameTex[i], NULL);
        }
    }

    // 頂点バッファ
    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; } // ★念のため再生成前に解放
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT hrVB = pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);
    if (FAILED(hrVB)) return E_FAIL;

    // ★再入時のために毎回選択状態を初期化
    g_SelectedPlayer = 0;
    g_SelectingPlayerIndex = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) g_SelectedCharIndex[i] = -1;

    alpha = 1.0f;
    flag_alpha = TRUE;
    g_Load = TRUE;

    return S_OK;
}


void UninitPlayerSelect(void)
{
    // ★常に解放処理を通す（再入時の未解放リソースが残らないように）
    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    for (int i = 0; i < 3; i++)
    {
        if (g_Texture[i]) { g_Texture[i]->Release(); g_Texture[i] = NULL; }
    }

    for (int i = 0; i < MAX_CHARACTERS; i++)
    {
        if (g_PlayerIcons[i]) { g_PlayerIcons[i]->Release(); g_PlayerIcons[i] = NULL; }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (g_SelectFrameTex[i]) { g_SelectFrameTex[i]->Release(); g_SelectFrameTex[i] = NULL; }
    }

    g_Load = FALSE;

    // ★安全のため、抜ける時に状態も初期化（戻り先がそのまま再入してもOK）
    g_SelectedPlayer = 0;
    g_SelectingPlayerIndex = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        g_SelectedCharIndex[i] = -1;
        g_IsCPU[i] = false;
    }
}

void UpdatePlayerSelect(void)
{
    // 進行済み（全員決定）なら何もしない防御
    if (g_SelectingPlayerIndex >= MAX_PLAYERS) return;

    // ← →
    if (GetKeyboardTrigger(DIK_LEFT) || IsButtonTriggered(0, BUTTON_LEFT))
    {
        int guard = 0;
        do {
            g_SelectedPlayer = (g_SelectedPlayer - 1 + MAX_CHARACTERS) % MAX_CHARACTERS;
        } while (IsTaken(g_SelectedPlayer) && ++guard < MAX_CHARACTERS);
        PlaySound(SOUND_LABEL_SE_switch01);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT) || IsButtonTriggered(0, BUTTON_RIGHT))
    {
        int guard = 0;
        do {
            g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS;
        } while (IsTaken(g_SelectedPlayer) && ++guard < MAX_CHARACTERS);
        PlaySound(SOUND_LABEL_SE_switch01);
    }

    // TABキー：CPUにする（余りキャラを自動割当）
    if (GetKeyboardTrigger(DIK_TAB) || IsButtonTriggered(0, BUTTON_X))
    {
        for (int c = 0; c < MAX_CHARACTERS; ++c) {
            if (!IsTaken(c)) { g_SelectedCharIndex[g_SelectingPlayerIndex] = c; break; }
        }
        g_IsCPU[g_SelectingPlayerIndex] = true;
        AdvanceToNextPlayer();
        return;
    }

    // Enter：キャラ決定
    if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A))
    {
        // 念のため二重確保防止
        if (!IsTaken(g_SelectedPlayer)) {
            g_SelectedCharIndex[g_SelectingPlayerIndex] = g_SelectedPlayer;
            AdvanceToNextPlayer();
        }
    }
    else if (GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_B))
    {
        SetFade(FADE_OUT, MODE_TITLE);
    }

    // 点滅
    alpha += flag_alpha ? -0.02f : 0.02f;
    if (alpha <= 0.0f) { alpha = 0.0f; flag_alpha = FALSE; }
    if (alpha >= 1.0f) { alpha = 1.0f; flag_alpha = TRUE; }
}


// ===== 置き換え：DrawPlayerSelect =====
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
    SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 6, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO,
        0, 0, 1, 1, XMFLOAT4(1, 1, 1, alpha));
    GetDeviceContext()->Draw(4, 0);

    // タイトル
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
    SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, 60, 500, 100, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // キャラアイコン
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

            if (i == g_SelectedPlayer)
            {
                width += 20.0f;
                height += 20.0f;
            }

            SetSpriteColor(g_VertexBuffer, x, yPos, width, height,
                0, 0, 1, 1, XMFLOAT4(1, 1, 1, 1));
            GetDeviceContext()->Draw(4, 0);
        }

        // 選択済みプレイヤーの枠
        for (int p = 0; p < MAX_PLAYERS; p++)
        {
            if (g_SelectedCharIndex[p] == i && g_SelectFrameTex[p])
            {
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_SelectFrameTex[p]);
                SetSpriteColor(g_VertexBuffer, x, yPos - 15.0f,
                    TEXTURE_WIDTH_PLAYER_ICON * 1.2f, TEXTURE_HEIGHT_PLAYER_ICON * 1.5f,
                    0, 0, 1, 1, XMFLOAT4(1, 1, 1, 1));
                GetDeviceContext()->Draw(4, 0);
            }
        }
    }

    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}

void SetSelectedCharIndex(int playerIndex, int charIndex)
{
    g_SelectedCharIndex[playerIndex] = charIndex;
}

int GetSelectedCharIndex(int playerIndex)
{
    return g_SelectedCharIndex[playerIndex];
}

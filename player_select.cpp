#include "main.h"
#include "renderer.h"
#include "input.h"
#include "player_select.h"
#include "player.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"

#define MAX_PLAYERS 2
#define MAX_CHARACTERS 2

#define TEXTURE_WIDTH_LOGO (480)
#define TEXTURE_HEIGHT_LOGO (80)
#define TEXTURE_WIDTH_PLAYER_ICON (128)
#define TEXTURE_HEIGHT_PLAYER_ICON (128)

static int g_SelectedPlayer = 0;
static int g_SelectingPlayerIndex = 0;
static int g_SelectedCharIndex[MAX_PLAYERS] = { -1, -1 };

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[3] = { NULL };
static ID3D11ShaderResourceView* g_PlayerIcons[MAX_CHARACTERS] = { NULL };
static ID3D11ShaderResourceView* g_SelectFrameTex[2] = { NULL }; // 1P/2P 

static const char* g_TexturName[3] = {
    "data/TEXTURE/bg003.jpg",
    "data/TEXTURE/copy.png",
    "data/TEXTURE/Select_player/player_title.png"
};

static const char* g_PlayerIconNames[MAX_CHARACTERS] = {
    "data/TEXTURE/Select_player/player1.png",
    "data/TEXTURE/Select_player/player2.png",
    //"data/TEXTURE/Select_player/player3.png",
    //"data/TEXTURE/Select_player/player4.png"
};

static const char* g_FrameTexNames[2] = {
    "data/TEXTURE/Select_player/1p.png",
    "data/TEXTURE/Select_player/2p.png"
};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

HRESULT InitPlayerSelect(void)
{
    ID3D11Device* pDevice = GetDevice();
    PlaySound(SOUND_LABEL_BGM_bgm_a);

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

    for (int i = 0; i < 2; i++)
    {
        g_SelectFrameTex[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(pDevice, g_FrameTexNames[i], NULL, NULL, &g_SelectFrameTex[i], NULL);
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_SelectedPlayer = 0;
    g_SelectingPlayerIndex = 0;
    g_SelectedCharIndex[0] = -1;
    g_SelectedCharIndex[1] = -1;
    alpha = 1.0f;
    flag_alpha = TRUE;
    g_Load = TRUE;

    return S_OK;
}

void UninitPlayerSelect(void)
{
    if (g_Load == FALSE) return;

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

    for (int i = 0; i < 2; i++)
    {
        if (g_SelectFrameTex[i]) { g_SelectFrameTex[i]->Release(); g_SelectFrameTex[i] = NULL; }
    }

    g_Load = FALSE;
}

void UpdatePlayerSelect(void)
{
    // ← →
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        do {
            g_SelectedPlayer = (g_SelectedPlayer - 1 + MAX_CHARACTERS) % MAX_CHARACTERS;
        } while (g_SelectedPlayer == g_SelectedCharIndex[1 - g_SelectingPlayerIndex]);

        PlaySound(SOUND_LABEL_SE_switch01);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        do {
            g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS;
        } while (g_SelectedPlayer == g_SelectedCharIndex[1 - g_SelectingPlayerIndex]);

        PlaySound(SOUND_LABEL_SE_switch01);
    }

    // Enter 決定
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        g_SelectedCharIndex[g_SelectingPlayerIndex] = g_SelectedPlayer;

        if (g_SelectingPlayerIndex == 0)
        {
            g_SelectingPlayerIndex = 1;
            g_SelectedPlayer = 0;
            while (g_SelectedPlayer == g_SelectedCharIndex[0])
            {
                g_SelectedPlayer = (g_SelectedPlayer + 1) % MAX_CHARACTERS;
            }
        }
        else
        {
            SetFade(FADE_OUT, MODE_STAGE_SELECT);

        }
    }
    else if (GetKeyboardTrigger(DIK_SPACE))
    {
        SetFade(FADE_OUT, MODE_TITLE);
    }

    // 点滅
    alpha += flag_alpha ? -0.02f : 0.02f;
    if (alpha <= 0.0f) { alpha = 0.0f; flag_alpha = FALSE; }
    if (alpha >= 1.0f) { alpha = 1.0f; flag_alpha = TRUE; }
}

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

        for (int p = 0; p < MAX_PLAYERS; p++)
        {
            if (g_SelectedCharIndex[p] == i && g_SelectFrameTex[p])
            {
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_SelectFrameTex[p]);
                SetSpriteColor(g_VertexBuffer, x, yPos-15.0f, TEXTURE_WIDTH_PLAYER_ICON*1.2, TEXTURE_HEIGHT_PLAYER_ICON*1.5,
                    0, 0, 1, 1, XMFLOAT4(1, 1, 1, 0.6f));
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

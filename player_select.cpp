#include "main.h"
#include "renderer.h"
#include "input.h"
#include "player_select.h"
#include "player.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"

#define MAX_PLAYERS 4
#define TEXTURE_WIDTH_LOGO (480)
#define TEXTURE_HEIGHT_LOGO (80)
#define TEXTURE_WIDTH_PLAYER_ICON (128)
#define TEXTURE_HEIGHT_PLAYER_ICON (128)
#define SELECTION_BORDER_SIZE (15)

static int g_SelectedPlayer = 0; // 0-3 for 4 players

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[2] = { NULL };
static ID3D11ShaderResourceView* g_PlayerIcons[MAX_PLAYERS] = { NULL };

static const char* g_TexturName[2] = {
    "data/TEXTURE/bg001.jpg",
    "data/TEXTURE/copy.png"
};

static const char* g_PlayerIconNames[MAX_PLAYERS] = {
    "data/TEXTURE/Select_player/player1.png",
    "data/TEXTURE/Select_player/player2.png",
    "data/TEXTURE/Select_player/player3.png",
    "data/TEXTURE/Select_player/player4.png"
};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

HRESULT InitPlayerSelect(void)
{
    ID3D11Device* pDevice = GetDevice();

    // テクスチャ生成（背景とロゴ）
    for (int i = 0; i < 2; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_TexturName[i],
            NULL,
            NULL,
            &g_Texture[i],
            NULL);
    }

    // プレイヤーアイコン読み込み
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        g_PlayerIcons[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_PlayerIconNames[i],
            NULL,
            NULL,
            &g_PlayerIcons[i],
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

    g_SelectedPlayer = 0;
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

    for (int i = 0; i < 2; i++)
    {
        if (g_Texture[i])
        {
            g_Texture[i]->Release();
            g_Texture[i] = NULL;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (g_PlayerIcons[i])
        {
            g_PlayerIcons[i]->Release();
            g_PlayerIcons[i] = NULL;
        }
    }

    g_Load = FALSE;
}

void UpdatePlayerSelect(void)
{
    // プレイヤー選択（← → キー）
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedPlayer--;
        if (g_SelectedPlayer < 0) g_SelectedPlayer = MAX_PLAYERS - 1;
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedPlayer++;
        if (g_SelectedPlayer >= MAX_PLAYERS) g_SelectedPlayer = 0;
    }

    // 決定キー
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        SetFade(FADE_OUT, MODE_STAGE_SELECT);
    }
    // キャンセルキー
    else if (GetKeyboardTrigger(DIK_SPACE))
    {
        SetFade(FADE_OUT, MODE_TITLE);
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

void DrawPlayerSelect(void)
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
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
        SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // ロゴ描画
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);
        SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 6, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO,
            0.0f, 0.0f, 1.0f, 1.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
        GetDeviceContext()->Draw(4, 0);
    }

    // プレイヤーアイコン描画 (横並び)
    {
        float startX = SCREEN_WIDTH / 2 - (MAX_PLAYERS * (TEXTURE_WIDTH_PLAYER_ICON + 20)) / 2 + TEXTURE_WIDTH_PLAYER_ICON / 2;
        float yPos = SCREEN_HEIGHT - 150;

        for (int i = 0; i <MAX_PLAYERS; i++)
        {
            if (g_PlayerIcons[i])
            {
             // プレイヤーアイコンを描画
                GetDeviceContext()->PSSetShaderResources(0, 1, &g_PlayerIcons[i]);
                SetSpriteColor(g_VertexBuffer,
                    startX + i * (TEXTURE_WIDTH_PLAYER_ICON + 20), yPos,
                    TEXTURE_WIDTH_PLAYER_ICON, TEXTURE_HEIGHT_PLAYER_ICON,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
                GetDeviceContext()->Draw(4, 0);

                // 選択中のプレイヤーには青い枠を描画
                if (i == g_SelectedPlayer)
                {
                    // 青い枠を描画 
                    SetSpriteColor(g_VertexBuffer, 
                        startX + i * (TEXTURE_WIDTH_PLAYER_ICON + 20), yPos,
                        TEXTURE_WIDTH_PLAYER_ICON + SELECTION_BORDER_SIZE * 2, 
                        TEXTURE_HEIGHT_PLAYER_ICON + SELECTION_BORDER_SIZE * 2,
                        0.0f, 0.0f, 1.0f, 1.0f,
                        XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f));
                    GetDeviceContext()->Draw(4, 0);
                }

            }
        }
    }

    // 深度テストとライティングを有効化
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}
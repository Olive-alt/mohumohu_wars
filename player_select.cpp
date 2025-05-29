#include "main.h"
#include "renderer.h"
#include "input.h"
#include "player_select.h"
#include "player.h"
#include "player2.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
//#include "pet.h"   

#define MAX_PLAYERS 4
#define MAX_PET 4
#define TEXTURE_WIDTH_LOGO (480)
#define TEXTURE_HEIGHT_LOGO (80)
#define TEXTURE_WIDTH_PET_LOGO (320)
#define TEXTURE_HEIGHT_PET_LOGO (64)

static int g_SelectedPlayers = 1;
static int g_SelectedPet = 1;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[2] = { NULL };
static ID3D11ShaderResourceView* g_PetTexture = NULL;

static const char* g_TexturName[2] = {
    "data/TEXTURE/bg000.jpg",
    "data/TEXTURE/copy.png"
};

static const char* g_PetTextureName = "data/TEXTURE/pet_logo.png";

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

    // ペットロゴのテクスチャ読み込み
    D3DX11CreateShaderResourceViewFromFile(GetDevice(),
        g_PetTextureName,
        NULL,
        NULL,
        &g_PetTexture,
        NULL);

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    g_SelectedPlayers = 1;
    g_SelectedPet = 1;
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

    if (g_PetTexture)
    {
        g_PetTexture->Release();
        g_PetTexture = NULL;
    }

    g_Load = FALSE;
}

void UpdatePlayerSelect(void)
{
    // プレイヤー数選択（1～4）
    if (GetKeyboardTrigger(DIK_1))
    {
        g_SelectedPlayers = 1;
    }
    else if (GetKeyboardTrigger(DIK_2))
    {
        g_SelectedPlayers = 2;
    }
    else if (GetKeyboardTrigger(DIK_3))
    {
        g_SelectedPlayers = 3;
    }
    else if (GetKeyboardTrigger(DIK_4))
    {
        g_SelectedPlayers = 4;
    }

    // ペット選択（← → キー）
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedPet--;
        if (g_SelectedPet < 1) g_SelectedPet = MAX_PET;
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedPet++;
        if (g_SelectedPet > MAX_PET) g_SelectedPet = 1;
    }

    // 決定キー
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        SetFade(FADE_OUT, MODE_GAME);
    }
    // キャンセルキー
    else if (GetKeyboardTrigger(DIK_ESCAPE))
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
        SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO,
            0.0f, 0.0f, 1.0f, 1.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
        GetDeviceContext()->Draw(4, 0);
    }

    // ペットロゴ描画
    {
        if (g_PetTexture)
        {
            GetDeviceContext()->PSSetShaderResources(0, 1, &g_PetTexture);
            SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT * 2 / 3,
                TEXTURE_WIDTH_PET_LOGO, TEXTURE_HEIGHT_PET_LOGO,
                0.0f, 0.0f, 1.0f, 1.0f,
                XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
            GetDeviceContext()->Draw(4, 0);
        }
    }

    // 深度テストとライティングを有効化
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}

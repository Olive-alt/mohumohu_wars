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
#include "sprite.h"

#define MAX_STAGES 2
#define TEXTURE_WIDTH_STAGE_ICON (128)
#define TEXTURE_HEIGHT_STAGE_ICON (128)
#define TEXTURE_WIDTH_STAGE_TITLE (400)
#define TEXTURE_HEIGHT_STAGE_TITLE (100)
#define TEXTURE_WIDTH_LOGO (200)
#define TEXTURE_HEIGHT_LOGO (200)

static int g_SelectedStage = 1;

static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[3] = { NULL };

static const char* g_TexturName[3] = {
    "data/TEXTURE/bg002.jpg",
    "data/TEXTURE/Select_stage/stage1.png",
    "data/TEXTURE/Select_stage/stage2.png"

};

static float alpha;
static BOOL flag_alpha;
static BOOL g_Load = FALSE;

HRESULT InitStageSelect(void)
{
    ID3D11Device* pDevice = GetDevice();

    // �e�N�X�`�������i�w�i�ƃ��S�j
    for (int i = 0; i < 3; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_TexturName[i],
            NULL,
            NULL,
            &g_Texture[i],
            NULL);
    }


    // ���_�o�b�t�@����
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

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

    for (int i = 0; i < 3; i++)
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
    // �X�e�[�W�I���i�� �� �L�[�j
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedStage--;
        if (g_SelectedStage < 1) g_SelectedStage = MAX_STAGES;
        PrintDebugProc("Selected Stage: %d\n", g_SelectedStage);
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedStage++;
        if (g_SelectedStage > MAX_STAGES) g_SelectedStage = 1;
        PrintDebugProc("Selected Stage: %d\n", g_SelectedStage);
    }

    // ����L�[�iEnter�j
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        if (g_SelectedStage == 1)
        {
            PrintDebugProc("Loading Model 1\n");
        }
        else if (g_SelectedStage == 2)
        {
            PrintDebugProc("Loading Model 2\n");
        }

        SetFade(FADE_OUT, MODE_MODE_SELECT);
    }
    // �L�����Z���L�[�iESC�j
    else if (GetKeyboardTrigger(DIK_SPACE))
    {
        SetFade(FADE_OUT, MODE_PLAYER_SELECT);
    }

    // �_�Ō���
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
    // �[�x�e�X�g�ƃ��C�e�B���O�𖳌���
    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);

    // ���_�o�b�t�@�ݒ�
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

    // �}�g���N�X�ݒ�
    SetWorldViewProjection2D();

    // �v���~�e�B�u�g�|���W�ݒ�
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // �}�e���A���ݒ�
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // �w�i�`��
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);
        SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // ���S�`��
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_SelectedStage]);
        SetSpriteColor(g_VertexBuffer,
            SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
            TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO,
            0.0f, 0.0f, 1.0f, 1.0f,
            XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
        GetDeviceContext()->Draw(4, 0);
    }


    // �[�x�e�X�g�ƃ��C�e�B���O��L����
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}
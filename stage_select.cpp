// stage_select.cpp
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "stage_select.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
//stage���삵����ύX
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

    // �w�i�e�N�X�`���ǂݍ���
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_BackgroundTextureName,
        NULL,
        NULL,
        &g_BackgroundTexture,
        NULL);

    // �X�e�[�W�^�C�g���e�N�X�`���ǂݍ���
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_StageTitleTextureName,
        NULL,
        NULL,
        &g_StageTitleTexture,
        NULL);

    // �X�e�[�W�A�C�R���e�N�X�`���ǂݍ���
    D3DX11CreateShaderResourceViewFromFile(pDevice,
        g_StageIconsTextureName,
        NULL,
        NULL,
        &g_StageIconsTexture,
        NULL);

    // ���_�o�b�t�@����
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
    // �X�e�[�W�I���i�� �� �L�[�j
    if (GetKeyboardTrigger(DIK_LEFT))
    {
        g_SelectedStage--;
        if (g_SelectedStage < 1) g_SelectedStage = MAX_STAGES;
        // �����őI�������Đ�
    }
    else if (GetKeyboardTrigger(DIK_RIGHT))
    {
        g_SelectedStage++;
        if (g_SelectedStage > MAX_STAGES) g_SelectedStage = 1;
        // �����őI�������Đ�
    }

    // ����L�[
    if (GetKeyboardTrigger(DIK_RETURN))
    {
        // �I�����ꂽ�X�e�[�W��ݒ肵�ăQ�[���J�n
        SetFade(FADE_OUT, MODE_GAME);
    }
    // �L�����Z���L�[
    else if (GetKeyboardTrigger(DIK_ESCAPE))
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
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_BackgroundTexture);
        SetSprite(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
        GetDeviceContext()->Draw(4, 0);
    }

    // �X�e�[�W�^�C�g���`��
    {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_StageTitleTexture);
        SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4,
            TEXTURE_WIDTH_STAGE_TITLE, TEXTURE_HEIGHT_STAGE_TITLE,
            0.0f, 0.0f, 1.0f, 1.0f,
            XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
        GetDeviceContext()->Draw(4, 0);
    }

    // �X�e�[�W�A�C�R���`��
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

    // �[�x�e�X�g�ƃ��C�e�B���O��L����
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
}
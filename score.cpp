//=============================================================================
//
// �X�R�A���� [score.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "score.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH        (16)    // �L�����T�C�Y
#define TEXTURE_HEIGHT       (32)    // 
#define TEXTURE_MAX          (1)     // �e�N�X�`���̐�
#define SCORE_PLAYER_MAX     (2)     // �v���C���[�̐��i2�l�p�ɕύX�j
#define SCORE_DIGIT          (6)     // �\������
#define SCORE_MAX            (999999)

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;      // ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };    // �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
    "data/TEXTURE/ui_num.png",
};

// �e�v���C���[�̃X�R�A
static int    g_Score[SCORE_PLAYER_MAX];    // �X�R�A�i�����l���j

static BOOL   g_Use;            // TRUE:�g���Ă���  FALSE:���g�p
static float  g_w, g_h;         // ���ƍ���
static int    g_TexNo;          // �e�N�X�`���ԍ�

static BOOL   g_Load = FALSE;

// �e�v���C���[�̃X�R�A�\�����W
static XMFLOAT3 g_Pos[SCORE_PLAYER_MAX] = {
    XMFLOAT3(500.0f, 20.0f, 0.0f),   // �v���C���[1
    XMFLOAT3(700.0f, 20.0f, 0.0f),   // �v���C���[2
    // 3�l�ڈȍ~�͂����ɒǉ�
};


//=============================================================================
// ����������
//=============================================================================
HRESULT InitScore(void)
{
    ID3D11Device* pDevice = GetDevice();

    //�e�N�X�`������
    for (int i = 0; i < TEXTURE_MAX; i++)
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

    // �v���C���[�̏�����
    g_Use = TRUE;
    g_w = TEXTURE_WIDTH;
    g_h = TEXTURE_HEIGHT;
    g_TexNo = 0;

    // �e�v���C���[�̃X�R�A������
    for (int i = 0; i < SCORE_PLAYER_MAX; i++)
        g_Score[i] = 0;

    g_Load = TRUE;
    return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitScore(void)
{
    if (g_Load == FALSE) return;

    if (g_VertexBuffer)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        if (g_Texture[i])
        {
            g_Texture[i]->Release();
            g_Texture[i] = NULL;
        }
    }

    g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateScore(void)
{
#ifdef _DEBUG   // �f�o�b�O����\������
    // �K�v�Ȃ炱���Ƀf�o�b�O�o�͂�����
#endif
}

//=============================================================================
// �`�揈���i�S�v���C���[���̃X�R�A��`��j
//=============================================================================
void DrawScore(void)
{
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

    // �e�N�X�`���ݒ�
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

    // �e�v���C���[���ƂɃX�R�A��`��
    for (int p = 0; p < SCORE_PLAYER_MAX; p++)
    {
        int number = g_Score[p];
        for (int i = 0; i < SCORE_DIGIT; i++)
        {
            // ����\�����錅�̐���
            float x = (float)(number % 10);

            // �X�R�A�̈ʒu��e�N�X�`���[���W�𔽉f
            float px = g_Pos[p].x - g_w * i; // �X�R�A�̕\���ʒuX
            float py = g_Pos[p].y;           // �X�R�A�̕\���ʒuY
            float pw = g_w;                  // �X�R�A�̕\����
            float ph = g_h;                  // �X�R�A�̕\������

            float tw = 1.0f / 10;            // �e�N�X�`���̕�
            float th = 1.0f / 1;             // �e�N�X�`���̍���
            float tx = x * tw;               // �e�N�X�`���̍���X���W
            float ty = 0.0f;                 // �e�N�X�`���̍���Y���W

            // �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
            SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
                XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

            // �|���S���`��
            GetDeviceContext()->Draw(4, 0);

            // ���̌���
            number /= 10;
        }
    }
}

//=============================================================================
// �X�R�A�����Z����
// ����: playerIndex : �v���C���[�ԍ�
//      add         : �ǉ�����_���B�}�C�i�X���\
//=============================================================================
void AddScore(int playerIndex, int add)
{
    if (playerIndex < 0 || playerIndex >= SCORE_PLAYER_MAX) return;
    g_Score[playerIndex] += add;
    if (g_Score[playerIndex] > SCORE_MAX)
    {
        g_Score[playerIndex] = SCORE_MAX;
    }
}

//=============================================================================
// �w��v���C���[�̃X�R�A���擾����
//=============================================================================
int GetScore(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= SCORE_PLAYER_MAX) return 0;
    return g_Score[playerIndex];
}

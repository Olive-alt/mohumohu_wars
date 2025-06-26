//=============================================================================
//
// �^�C�g����ʏ��� [title.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"
#include "meshfield.h"
#include "score.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// �w�i�T�C�Y
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)				// �e�N�X�`���̐�

#define TEXTURE_WIDTH_LOGO			(480)			// ���S�T�C�Y
#define TEXTURE_HEIGHT_LOGO			(80)			// 

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/Title/ui_title_bg1280x720.png",
	"data/TEXTURE/Title/ui_title_bt_start_off.png",
	"data/TEXTURE/Title/ui_title_bt_start_on.png",
	"data/TEXTURE/Title/ui_title_bt_set_off.png",
	"data/TEXTURE/Title/ui_title_bt_set_on.png",
	"data/TEXTURE/Title/ui_title_bt_end_off.png",
	"data/TEXTURE/Title/ui_title_bt_end_on.png",
	"data/TEXTURE/Title/002.png",

	"data/TEXTURE/effect000.jpg",
};


static BOOL						g_Use;						// TRUE:�g���Ă���  FALSE:���g�p
static float					g_w, g_h;					// ���ƍ���
static XMFLOAT3					g_Pos;						// �|���S���̍��W
static XMFLOAT3					g_RogPos;					// �^�C�g�����S�̍��W
static XMFLOAT3					g_StartPos;					// �^�C�g�����S�̍��W
static XMFLOAT3					g_ExitPos;					// �^�C�g�����S�̍��W
static int						g_TexNo;					// �e�N�X�`���ԍ�

static int						selectedMenuItem;			// �^�C�g���ł̃��[�h�I��

float	alpha;
BOOL	flag_alpha;

BOOL	flag_settings;

static BOOL						g_Load = FALSE;

static bool isExitDialogShown = false;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTitle(void)
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


	// �ϐ��̏�����
	g_Use = TRUE;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
	g_RogPos = XMFLOAT3(g_w / 2, g_h / 5, 0.0f);
	//g_StartPos = XMFLOAT3(g_w/4, g_h/1.5, 0.0f);
	//g_ExitPos = XMFLOAT3(g_w/4 * 3, g_h / 1.5, 0.0f);

	selectedMenuItem = 0;

	g_TexNo = 0;

	alpha = 1.0f;
	flag_alpha = TRUE;

	flag_settings = FALSE;

	//�X�R�A�����Z�b�g����
	ResetScore();

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_title);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTitle(void)
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
void UpdateTitle(void)
{
	if (GetKeyboardTrigger(DIK_DOWN))
	{
		selectedMenuItem += 1;
	}
	else if (selectedMenuItem >= 3)
	{
		selectedMenuItem = 0;
	}

	if (GetKeyboardTrigger(DIK_UP))
	{
		selectedMenuItem -= 1;
	}
	else if (selectedMenuItem <= -1)
	{
		selectedMenuItem = 2;
	}

	if (selectedMenuItem == 0 && GetKeyboardTrigger(DIK_RETURN))
	{// �Q�[���J�n�A�C�R����Enter��������A�Q�[���X�^�[�g
		SetFade(FADE_OUT, MODE_PLAYER_SELECT);
		//SetFade(FADE_OUT, MODE_RESULT);
	}
	if (selectedMenuItem == 1 && GetKeyboardTrigger(DIK_RETURN))
	{// �ݒ�A�C�R����Enter��������A�ݒ�E�B���h�E���o��
		flag_settings == TRUE;

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[8]);

		SetSprite(g_VertexBuffer, g_Pos.x / 2, g_Pos.y / 2, g_w / 2, g_h / 2, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);




	}
	//if (selectedMenuItem == 2 && GetKeyboardTrigger(DIK_RETURN))
	//{
	//	int id = MessageBox(NULL, "�Q�[�����I�����܂����H", "�N�����[�h", MB_YESNOCANCEL | MB_ICONQUESTION);
	//	switch (id)
	//	{
	//	case IDYES:        // Yes�Ȃ�I��
	//	std::exit(-1);
	//	case IDNO:        // No�Ȃ�^�C�g���ɖ߂�
	//		return;
	//	case IDCANCEL:    // CANCEL�ł��^�C�g����
	//	default:
	//		break;
	//	}
	//}

	if (selectedMenuItem == 2 && GetKeyboardTrigger(DIK_RETURN) && !isExitDialogShown)
	{
		//isExitDialogShown = true; // ��x�����\��

		//int id = MessageBox(NULL, "�Q�[�����I�����܂����H", "�N�����[�h", MB_YESNOCANCEL | MB_ICONQUESTION);
		//switch (id)
		//{
		//case IDYES:        // Yes�Ȃ�I��
		std::exit(-1);
		//	break;

		//case IDNO:	// No�Ȃ�^�C�g���ɖ߂�
		//	SetMode(MODE_TITLE);
		//	selectedMenuItem = 2;
		//default:
		//	SetMode(MODE_TITLE);
		//	selectedMenuItem = 2;

		//	isExitDialogShown = false; // �_�C�A���O������ĕ\��OK��
		//	break;
		//}
	}

	// �Q�[���p�b�h�œ��͏���
	else if (selectedMenuItem == 0 && IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_GAME);
	}
	else if (selectedMenuItem == 0 && IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_GAME);
	}

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






#ifdef _DEBUG	// �f�o�b�O����\������
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTitle(void)
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

	// �^�C�g���̔w�i��`��
	{
		// �e�N�X�`���ݒ�
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// �|���S���`��
		GetDeviceContext()->Draw(4, 0);
	}

	// �^�C�g���̃��S��`��
	//{
	//	// �e�N�X�`���ݒ�
	//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[ ]);

	//	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//	SetSprite(g_VertexBuffer, g_RogPos.x, g_RogPos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);
	//	//SetSpriteColor(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
	//	//				XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

	//	// �|���S���`��
	//	GetDeviceContext()->Draw(4, 0);
	//}

	// �X�^�[�g���S��`��
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 12.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	// �ݒ胍�S��`��
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 15, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	// �Q�[���I�����S��`��
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 17.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	if (selectedMenuItem == 0)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 12.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	if (selectedMenuItem == 1)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[4]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 15, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	if (selectedMenuItem == 2)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[6]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 17.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	// �ݒ�t���O����������A�ݒ�E�B���h�E��\��
	if (flag_settings == TRUE)
	{

	}

	//	// �����Z�̃e�X�g
	//	SetBlendState(BLEND_MODE_ADD);		// ���Z����
	////	SetBlendState(BLEND_MODE_SUBTRACT);	// ���Z����
	//	for(int i=0; i<30; i++)
	//	{
	//		// �e�N�X�`���ݒ�
	//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
	//
	//		// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//		float dx = 100.0f;
	//		float dy = 100.0f;
	//		float sx = (float)(rand() % 100);
	//		float sy = (float)(rand() % 100);
	//
	//
	//		SetSpriteColor(g_VertexBuffer, dx+sx, dy+sy, 50, 50, 0.0f, 0.0f, 1.0f, 1.0f,
	//			XMFLOAT4(0.3f, 0.3f, 1.0f, 0.5f));
	//
	//		// �|���S���`��
	//		GetDeviceContext()->Draw(4, 0);
	//	}
	//	SetBlendState(BLEND_MODE_ALPHABLEND);	// ���������������ɖ߂�

}


#include "main.h"
#include "input.h"
#include "IT_ball.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_BALL  	"data/MODEL/sphere.obj"			// �ǂݍ��ރ��f����

#define	VALUE_MOVE		(10.0f)					// �ړ���
#define	VALUE_ROTATE	(XM_PI * 0.02f)			// ��]��

#define BALL_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define BALL_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��
#define	BALL_DAMAGE			(5.0f)					// �_���[�W��


//�A�C�R���p
#define TEXTURE_MAX			(1)				// �e�N�X�`���̐�

#define	ICON_WIDTH			(10.0f)			// ���_�T�C�Y
#define	ICON_HEIGHT			(10.0f)			// ���_�T�C�Y
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexITballIcon(void);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����
static BOOL					g_bAlpaTest;		// �A���t�@�e�X�gON/OFF

static char* g_TextureName[] =
{
	"data/TEXTURE/tree001.png",
};

//=============================================================================
// ����������
//=============================================================================
HRESULT BALL::InitITball(void)
{
	load = TRUE;
	LoadModel(MODEL_BALL, &model);

	use = FALSE;
	to_throw = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	move = XMFLOAT3(VALUE_MOVE, 1.0f, VALUE_MOVE);
	size = BALL_SIZE;
	count = 0.0f;
	PlayerIndex = -1;

	MakeVertexITballIcon();

	// �e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	// �؃��[�N�̏�����
	ZeroMemory(&icon_material, sizeof(icon_material));
	icon_material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	icon_pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	icon_fWidth = ICON_WIDTH;
	icon_fHeight = ICON_HEIGHT;
	icon_scl = XMFLOAT3(1.0f / ICON_WIDTH, 1.0f / ICON_HEIGHT, 1.0f / ICON_HEIGHT);
	icon_use = FALSE;
	g_bAlpaTest = FALSE;

	return S_OK;
}


void BALL::UninitITball(void)
{
	use = FALSE;
	// ���f���̉������
	if (load == TRUE)
	{
		UnloadModel(&model);
		load = FALSE;
	}

	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{// �e�N�X�`���̉��
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}

void BALL::UpdateITball(void)
{
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE : TRUE;
	}

	if (use)
	{
		if (!load) return;

		if (to_throw && !pick)
		{
			count += 1;    // ���Ԃ�i�߂Ă���

			pos.x -= sinf(rot.y) * move.x;
			pos.z -= cosf(rot.y) * move.z;


			// �t�B�[���h�̊O�ɏo����e����������
			if (pos.x < MAP_LEFT
				|| pos.x > MAP_RIGHT
				|| pos.z < MAP_DOWN
				|| pos.z > MAP_TOP)
			{
				use = FALSE;
				pick = FALSE;
				to_throw = FALSE;
				//ReleaseShadow(shadowIdx);
			}
		}
		else if (pick)
		{
			PLAYER* player = GetPlayer(PlayerIndex);
			icon_pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			icon_pos.y = player->pos.y * 2;
			icon_pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			pos.y = player->pos.y;
			pos.z = player->pos.z - cos(XM_PI) * 1.0f;

		}


	}
#ifdef _DEBUG
	PrintDebugProc(
		"ballPos:(%f,%f,%f)\n",
		pos.x,
		pos.y,
		pos.z
	);
#endif

}

void BALL::DrawITball(void)
{
	// ���e�X�g�ݒ�
	if (g_bAlpaTest == TRUE)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(TRUE);
	}

	if (!use)return;

	if (!pick)
	{
		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

		// �J�����O����
		SetCullingMode(CULL_MODE_NONE);

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&m_mtxWorld, mtxWorld);

		// ���f���`��
		DrawModel(&model);

		// �J�����O�ݒ��߂�
		SetCullingMode(CULL_MODE_BACK);
	}
	else if (pick)
	{
		// ���C�e�B���O�𖳌�
		SetLightEnable(FALSE);

		XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
		CAMERA* cam = GetCamera();

		// ���_�o�b�t�@�ݒ�
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// �v���~�e�B�u�g�|���W�ݒ�
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �r���[�}�g���b�N�X���擾
		mtxView = XMLoadFloat4x4(&cam->mtxView);


		// �֐��g������
		mtxWorld = XMMatrixInverse(nullptr, mtxView);
		mtxWorld.r[3].m128_f32[0] = 0.0f;
		mtxWorld.r[3].m128_f32[1] = 0.0f;
		mtxWorld.r[3].m128_f32[2] = 0.0f;

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(icon_scl.x, icon_scl.y, icon_scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(icon_pos.x, icon_pos.y, icon_pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);


		// �}�e���A���ݒ�
		SetMaterial(icon_material);

		//// �e�N�X�`���ݒ�
		//int texNo = i % TEXTURE_MAX;
		//if (i == 4)	// �T�Ԃ̖؂����e�N�X�`���A�j�������Ă݂�
		//{
		//	// �e�N�X�`���A�j���͂���Ȋ����ŗǂ���
		//	g_TexAnim++;
		//	if ((g_TexAnim % 16) == 0)
		//	{
		//		g_TexNo = (g_TexNo + 1) % TEXTURE_MAX;
		//	}
		//	texNo = g_TexNo;
		//}
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[texNo]);

		// �|���S���̕`��
		GetDeviceContext()->Draw(4, 0);


		// ���C�e�B���O��L����
		SetLightEnable(TRUE);

		// ���e�X�g�𖳌���
		SetAlphaTestEnable(FALSE);

	}
}

void BALL::SetITballObject(XMFLOAT3 set_pos)
{
	use = TRUE;
	pick = FALSE;
	to_throw = FALSE;

	pos = set_pos;
}

void BALL::SetITball(XMFLOAT3 set_pos, XMFLOAT3 p_rot)
{
	pick = FALSE;
	to_throw = TRUE;
	PLAYER* player = GetPlayer(PlayerIndex);
	pos = player->pos;
	rot = player->rot;
	player->haveWeapon = FALSE;

}

void BALL::HitITball(int p_Index)
{
	if (p_Index == PlayerIndex)return;

	use = FALSE;
	to_throw = FALSE;
	PLAYER* player = GetPlayer(p_Index);
	player->use = FALSE;
	PlayerIndex = -1;
}

void BALL::PickITball(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);
	if (player->haveWeapon)return;

	pick = TRUE;

	player->haveWeapon = TRUE;
	PlayerIndex = p_Index;
	pos.x = player->pos.x - sin(XM_PI) * 1.0f;
	pos.y = player->pos.y;
	pos.z = player->pos.z - cos(XM_PI) * 1.0f;
}


//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexITballIcon(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = 60.0f;
	float fHeight = 90.0f;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}


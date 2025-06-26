#include "main.h"
#include "input.h"
#include "IT_boomerang.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_BOOM  	"data/MODEL/sphere.obj"			// �ǂݍ��ރ��f����

#define	VALUE_MOVE		(10.0f)					// �ړ���
#define	VALUE_ROTATE	(XM_PI * 0.02f)			// ��]��

#define BOOM_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define BOOM_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��
#define	BOOM_DAMAGE			(5.0f)					// �_���[�W��


//�A�C�R���p
#define TEXTURE_MAX			(1)				// �e�N�X�`���̐�

#define	ICON_WIDTH			(10.0f)			// ���_�T�C�Y
#define	ICON_HEIGHT			(10.0f)			// ���_�T�C�Y
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT MakeVertexITboomIcon(void);

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
HRESULT BOOM::InitITboom(void)
{
	load = TRUE;
	LoadModel(MODEL_BOOM, &model);

	use = FALSE;
	to_throw = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	move = XMFLOAT3(VALUE_MOVE, 1.0f, VALUE_MOVE);
	size = BOOM_SIZE;
	count = 0.0f;
	PlayerIndex = -1;

	boomerangArcRadius = 50.0f;   
	boomerangSpeed = 1.5f;        
	boomerangTime = 0.0f;

	MakeVertexITboomIcon();

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


void BOOM::UninitITboom(void)
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

void BOOM::UpdateITboom(void)
{
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE : TRUE;
	}

	if (use)
	{
		if (!load) return;

		if (to_throw)
		{
			boomerangTime += boomerangSpeed * 0.032f;

			PLAYER* player = GetPlayer(PlayerIndex);

			if (!returning)
			{
				// �O�����̋O��: startPos ���� endPos ��
				float t = boomerangTime / 1.8f; // 0 ���� 1 �܂�
				if (t >= 1.0f)
				{
					t = 1.0f;
					returning = true;
					boomerangTime = 0.0f; // �߂�̃^�C�}�[���Z�b�g
				}

				// �O�����̑ȉ~�O��
				XMFLOAT3 center;
				center.x = (startPos.x + endPos.x) * 0.5f;
				center.y = (startPos.y + endPos.y) * 0.5f;
				center.z = (startPos.z + endPos.z) * 0.5f;

				XMFLOAT3 major;
				major.x = (startPos.x - endPos.x) * 0.5f;
				major.y = 0.0f;
				major.z = (startPos.z - endPos.z) * 0.5f;

				XMFLOAT3 minor;
				minor.x = -(major.z);
				minor.y = 0.0f;
				minor.z = major.x;

				float majorLen = sqrtf(major.x * major.x + major.z * major.z);
				float arcHeight = 0.2f * majorLen;
				float minorLen = sqrtf(minor.x * minor.x + minor.z * minor.z);
				if (minorLen > 0.01f)
				{
					minor.x = (minor.x / minorLen) * arcHeight;
					minor.z = (minor.z / minorLen) * arcHeight;
				}

				float angle = XM_PI * t; // 0 ���� PI �܂�

				pos.x = center.x + major.x * cosf(angle) + minor.x * sinf(angle);
				pos.y = center.y;
				pos.z = center.z + major.z * cosf(angle) + minor.z * sinf(angle);
			}
			else
			{
				// �������̋O��: endPos ����v���C���[�̌��݈ʒu�ցi���t���[���Čv�Z�j
				PLAYER* player = GetPlayer(PlayerIndex);

				// ���t���[���A�^�[�Q�b�g���v���C���[�̌��݈ʒu�ɂ���
				XMFLOAT3 dynamicTarget;
				dynamicTarget.x = player->pos.x - sin(XM_PI) * 1.0f;
				dynamicTarget.y = player->pos.y;
				dynamicTarget.z = player->pos.z - cos(XM_PI) * 1.0f;

				float t = boomerangTime / 1.8f; // 0 ���� 1 �܂�
				if (t >= 1.0f) t = 1.0f;

				// �������̑ȉ~�O��
				XMFLOAT3 center;
				center.x = (endPos.x + dynamicTarget.x) * 0.5f;
				center.y = (endPos.y + dynamicTarget.y) * 0.5f;
				center.z = (endPos.z + dynamicTarget.z) * 0.5f;

				XMFLOAT3 major;
				major.x = (endPos.x - dynamicTarget.x) * 0.5f;
				major.y = 0.0f;
				major.z = (endPos.z - dynamicTarget.z) * 0.5f;

				XMFLOAT3 minor;
				minor.x = -(major.z);
				minor.y = 0.0f;
				minor.z = major.x;

				float majorLen = sqrtf(major.x * major.x + major.z * major.z);
				float arcHeight = 0.2f * majorLen;
				float minorLen = sqrtf(minor.x * minor.x + minor.z * minor.z);
				if (minorLen > 0.01f)
				{
					minor.x = (minor.x / minorLen) * arcHeight;
					minor.z = (minor.z / minorLen) * arcHeight;
				}

				float angle = XM_PI * t; // 0 ���� PI �܂�

				pos.x = center.x + major.x * cosf(angle) + minor.x * sinf(angle);
				pos.y = center.y;
				pos.z = center.z + major.z * cosf(angle) + minor.z * sinf(angle);

				// �ڕW�ɏ\���߂���΃A�[�N�I��
				float dx = pos.x - dynamicTarget.x;
				float dy = pos.y - dynamicTarget.y;
				float dz = pos.z - dynamicTarget.z;
				float dist = sqrtf(dx * dx + dy * dy + dz * dz);
				if (t >= 1.0f || dist < 2.0f)
				{
					pos = dynamicTarget;
					to_throw = FALSE;
					returning = FALSE;
					pick = TRUE;
					count = 0.0f;
					player->haveWeapon = TRUE;
				}
			}
		}
		if (pick)
		{
			PLAYER* player = GetPlayer(PlayerIndex);

			icon_pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			icon_pos.y = player->pos.y * 2;
			icon_pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			pos.y = player->pos.y;
			pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			if (GetKeyboardTrigger(DIK_NUMPAD0))
			{
				rot = player->rot;
				pick = FALSE;
				to_throw = TRUE;
				count = 0.0f;
				returning = false;
				boomerangTime = 0.0f;

				// �X�^�[�g�ʒu = �v���C���[�̎�
				startPos.x = player->pos.x - sinf(XM_PI) * 1.0f;
				startPos.y = player->pos.y;
				startPos.z = player->pos.z - cosf(XM_PI) * 1.0f;

				// �I�_�ʒu = �v���C���[�O��
				float throwDistance = 160.0f;
				endPos.x = player->pos.x + -sinf(rot.y) * throwDistance;
				endPos.y = player->pos.y;
				endPos.z = player->pos.z + -cosf(rot.y) * throwDistance;

				pos = startPos;
			}

		}
	}
#ifdef _DEBUG
	PrintDebugProc(
		"boomPos:(%f,%f,%f)\n",
		pos.x,
		pos.y,
		pos.z
	);
#endif
}

void BOOM::DrawITboom(void)
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

void BOOM::SetITboomObject(XMFLOAT3 set_pos, int playerIndex)
{
	use = TRUE;
	pos = set_pos;
	pick = FALSE;
	to_throw = FALSE; 
	returning = false;
	PlayerIndex = playerIndex;
	PLAYER* player = GetPlayer(PlayerIndex);
	pos = set_pos;
	startPos = player->pos;
	rot = player->rot;
	player->haveWeapon = FALSE;
}


void BOOM::HitITboom(int p_Index)
{
	// �������g�ɓ�����̂�h��
	if (p_Index == PlayerIndex) return;

	// ���������v���C���[���畐����O��
	GetPlayer(p_Index)->haveWeapon = FALSE;

	// ���������v���C���[�������i�������j
	GetPlayer(p_Index)->use = FALSE;

}




void BOOM::PickITboom(int p_Index)
{
	pick = TRUE;
	PLAYER* player = GetPlayer(p_Index);

	if (player->haveWeapon) return; 

	player->haveWeapon = TRUE;
	PlayerIndex = p_Index;
	pos.x = player->pos.x - sin(XM_PI) * 1.0f;
	pos.y = player->pos.y;
	pos.z = player->pos.z - cos(XM_PI) * 1.0f;
}



//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT MakeVertexITboomIcon(void)
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


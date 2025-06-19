#include "main.h"
#include "input.h"
#include "IT_invisible.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_INVISIBLE	"data/MODEL/sphere.obj"			// �ǂݍ��ރ��f����

#define INVISIBLE_TIME			(600.0f)							// ���剻�̎���

#define INVISIBLE_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define INVISIBLE_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************

//=============================================================================
// ����������
//=============================================================================
HRESULT INVISIBLE::InitITinvisible(void)
{
	load = TRUE;
	LoadModel(MODEL_INVISIBLE, &model);

	use = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	invisibleUse = FALSE;
	invisibleTimer = 0;
	PlayerIndex = -1;

	return S_OK;
}


void INVISIBLE::UninitITinvisible(void)
{
	use = FALSE;
	// ���f���̉������
	if (load == TRUE)
	{
		UnloadModel(&model);
		load = FALSE;
	}
}

void INVISIBLE::UpdateITinvisible(void)
{
	if (use)
	{

	}

	if (invisibleUse)
	{
		invisibleTimer += 1;
		//PrintDebugProc("invisibleTimer %d\n", invisibleTimer);
		if (invisibleTimer > INVISIBLE_TIME)
		{
			FinishITinvisible();
		}
	}
}

void INVISIBLE::DrawITinvisible(void)
{
	if (!use)return;

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

void INVISIBLE::SetITinvisible(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}

void INVISIBLE::FinishITinvisible(void)
{
	invisibleUse = FALSE;
	use = FALSE;
	invisibleTimer = 0;
	PLAYER* player = GetPlayer(PlayerIndex);

	for (int j = 0; j < player->model.SubsetNum; j++)
	{
		SetModelDiffuse(&player->model, j, old_diffuse[j]);
	}
	PlayerIndex = -1;
}

void INVISIBLE::PickITinvisible(int p_Index)
{
	invisibleUse = TRUE;
	use = FALSE;
	PLAYER* player = GetPlayer(p_Index);
	GetModelDiffuse(&player->model, &old_diffuse[0]);
	PlayerIndex = p_Index;
	for (int j = 0; j < player->model.SubsetNum; j++)
	{
		SetModelDiffuse(&player->model, j, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f));
	}
}

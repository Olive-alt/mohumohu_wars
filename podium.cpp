#include "main.h"
#include "renderer.h"
#include "podium.h"
#include "camera.h"


// ���f���p�X
#define MODEL_PODIUM "data/MODEL/podium.obj"

static PODIUM		g_Podium[MAX_PODIUM];						// �v���C���[


//=============================================================================
// ����������
//=============================================================================
HRESULT InitPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		g_Podium[i].load = TRUE;
		LoadModel(MODEL_PODIUM, &g_Podium[i].model);

		g_Podium[i].pos = XMFLOAT3(20.0f, 0.0f, 0.0f);
		g_Podium[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Podium[i].scl = XMFLOAT3(4.0f, 4.0f, 4.0f);


		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Podium[i].model, &g_Podium[i].diffuse[0]);
		g_Podium[i].use = TRUE;

	}


	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		// ���f���̉������
		if (g_Podium[i].load == TRUE)
		{
			UnloadModel(&g_Podium[i].model);
			g_Podium[i].load = FALSE;
		}
	}



}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePodium(void)
{

}
//=============================================================================
// �`�揈��
//=============================================================================
void DrawPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		// �J�����O����
		SetCullingMode(CULL_MODE_NONE);

		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Podium[i].scl.x - 0.7f, g_Podium[i].scl.y - 0.7f, g_Podium[i].scl.z - 0.7f);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Podium[i].rot.x, g_Podium[i].rot.y + XM_PI, g_Podium[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Podium[i].pos.x, g_Podium[i].pos.y, g_Podium[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Podium[i].mtxWorld, mtxWorld);


		// ���f���`��
		DrawModel(&g_Podium[i].model);

		// �J�����O�ݒ��߂�
		SetCullingMode(CULL_MODE_BACK);
	}
}

// �����R�[�h�݊��p�F�����Ȃ��ŌĂԂƎ����I�� index=0 ��Ԃ�
PODIUM* GetPodium(void)
{
	return &(g_Podium[0]);
}

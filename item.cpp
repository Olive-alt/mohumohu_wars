//=============================================================================
//
// �Q�[����ʏ��� [game.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "game.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"

#include "item.h"
#include "player.h"
#include "collision.h"
#include "debugproc.h"

//�A�C�e���p
#include "IT_giant.h"
#include "IT_invisible.h"

//�f�o�b�O�\��
#include "debugline.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
//�X�e�[�W�M�~�b�N�p
//�A�C�e���p


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// �|�[�YON/OFF


//�A�C�e���p
GIANT* giant[10] = { nullptr };
int giantMax = 10;
int giantCnt = 0;

INVISIBLE invisible;
BALL ball;
BALL* GetBall()  // �A�N�Z�X�p�̊֐����쐬
{
	return &ball;
}
BOOM boom;  // �u�[�������̃C���X�^���X���쐬
BOOM* GetBoomerang()  // �A�N�Z�X�p�̊֐����쐬
{
	return &boom;  // �{�[���̒��ɂ���u�[��������Ԃ�
}

//=============================================================================
// ����������
//=============================================================================
HRESULT InitItem(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		//���剻�A�C�e���̏�����
		giant[i]->InitITgiant();
	}



	//�������A�C�e���̏�����
	invisible.InitITinvisible();
	invisible.SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));

	//�{�[���A�C�e���̏�����
	ball.InitITball();
	ball.SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));

	boom.InitITboom();
	boom.SetITboomObject(XMFLOAT3(-150.0f, 0.0f, -150.0f), 0);

	// BGM�Đ�
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitItem(void)
{
	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;
		// ���剻�A�C�e���̏I������
		giant[i]->UninitITgiant();
		giant[i] = nullptr;
	}

	// �������A�C�e���̏I������
	invisible.UninitITinvisible();

	//�{�[���A�C�e���̏I������
	ball.UninitITball();

	// �u�[�������̏I������
	boom.UninitITboom();

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateItem(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause = g_bPause ? FALSE : TRUE;
	}

	if (GetKeyboardTrigger(DIK_K))
	{
		for (int i = 0; i < 10; i++)
		{
			if (giant[i]) continue;

			giant[i] = new GIANT;
			giant[i]->InitITgiant();
			giant[i]->SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));

			if (giant[i])break;
		}

	}

#endif

	if (g_bPause == FALSE)
		return;

	// �����蔻�菈��
	CheckHitItem();

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// ���剻�A�C�e���̍X�V����
		giant[i]->UpdateITgiant();
	}


	// �������A�C�e���̍X�V����
	invisible.UpdateITinvisible();

	// �{�[���A�C�e���̍X�V����
	ball.UpdateITball();

	// �u�[�������̍X�V����
	boom.UpdateITboom();


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawItem(void)
{
	// �{�[���A�C�e���̕`�揈��
	ball.DrawITball();

	// �u�[�������̕`�揈��
	boom.DrawITboom();

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// ���剻�A�C�e���̕`�揈��
		giant[i]->DrawITgiant();
	}
	// �������A�C�e���̕`�揈��
	invisible.DrawITinvisible();

	// 2D�̕���`�悷�鏈��
	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);


	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r����
	SetDepthEnable(TRUE);
}


//=============================================================================
// �����蔻�菈��
//=============================================================================
void CheckHitItem(void)
{
	PLAYER* player = GetPlayer(); // �v���C���[1 (index=0)

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// ���剻�A�C�e��
		if (bool use = giant[i]->IsUsedITgiant())
		{
			XMFLOAT3 gi_pos = giant[i]->GetPositionITgiant();
			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, gi_pos, player[j].size, GIANT_SIZE))
				{
					giant[i]->PickITgiant(j);
				}
			}
		}
	}


	//������
	if (bool use = invisible.IsUsedITinvisible())
	{
		XMFLOAT3 invi_pos = invisible.GetPositionITinvisible();

		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (CollisionBC(player[i].pos, invi_pos, player[i].size, INVISIBLE_SIZE))
			{
				invisible.PickITinvisible(i);
			}
		}
	}

	//�{�[��
	if (bool use = ball.IsUsedITball())
	{
		XMFLOAT3 ball_pos = ball.GetPositionITball();
		BOOL pick = ball.IsPickedITball();
		BOOL to_throw = ball.IsThrewITball();

		if (!pick && !to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, ball_pos, player[i].size, BALL_SIZE))
				{
					ball.PickITball(i);
				}
			}
		}
		else if (to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, ball_pos, player[i].size, BALL_SIZE))
				{
					ball.HitITball(i);
				}
			}
		}
	}

	// �u�[������
	if (bool use = boom.IsUsedITboom())
	{
		XMFLOAT3 boom_pos = boom.GetPositionITboom();
		BOOL pick = boom.IsPickedITboom();
		BOOL to_throw = boom.IsThrewITboom();
		if (!pick && !to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, boom_pos, player[i].size, BOOM_SIZE))
				{
					boom.PickITboom(i);
				}
			}
		}
		else if (to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, boom_pos, player[i].size, BOOM_SIZE))
				{
					boom.HitITboom(i);
				}
			}
		}
	}

}


//// ���̃��C���[�t���[����`�悷��֐�
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices)
//{
//	// XY���ʂ̉~��`��
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius, center.z);
//		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius, center.z);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//
//	// XZ���ʂ̉~��`��
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y, center.z + sinf(theta1) * radius);
//		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y, center.z + sinf(theta2) * radius);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//
//	// YZ���ʂ̉~��`��
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x, center.y + cosf(theta1) * radius, center.z + sinf(theta1) * radius);
//		XMFLOAT3 p2(center.x, center.y + cosf(theta2) * radius, center.z + sinf(theta2) * radius);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//}

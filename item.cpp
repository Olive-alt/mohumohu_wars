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
#define ITEM_MAX	(10)


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
//�A�C�e���p
GIANT giant[ITEM_MAX];

INVISIBLE invisible[1];

BALL ball[ITEM_MAX];
BALL* GetBall()  // �A�N�Z�X�p�̊֐����쐬
{
	return &ball[0];
}

BOOM boom;  // �u�[�������̃C���X�^���X���쐬
BOOM* GetBoomerang()  // �A�N�Z�X�p�̊֐����쐬
{
	return &boom;  // �{�[���̒��ɂ���u�[��������Ԃ�
}

HAMR hamr;
HAMR* GetHammer()  // �A�N�Z�X�p�̊֐����쐬
{
	return &hamr;  // �n���}�[�̃C���X�^���X��Ԃ�
}

//=============================================================================
// ����������
//=============================================================================
HRESULT InitItem(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//���剻�A�C�e���̏�����
		giant[i].InitITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		//�������A�C�e���̏�����
		invisible[i].InitITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//�{�[���A�C�e���̏�����
		ball[i].InitITball();
	}

	// �u�[�������̏�����
	boom.InitITboom();
	boom.SetITboomObject(XMFLOAT3(-150.0f, 0.0f, -150.0f), 0);

	// �n���}�[�̏�����
	hamr.InitITHamr();
	hamr.SetITHamrObject(XMFLOAT3(100.0f, 0.0f, -100.0f));
	// BGM�Đ�
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ���剻�A�C�e���̏I������
		giant[i].UninitITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		// �������A�C�e���̏I������
		invisible[i].UninitITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//�{�[���A�C�e���̏I������
		ball[i].UninitITball();
	}

	// �u�[�������̏I������
	boom.UninitITboom();

	// �n���}�[�̏I������
	hamr.UninitITHamr();

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

	if (GetKeyboardTrigger(DIK_6))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!giant[i].IsUsedITgiant())
			{
				giant[i].SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_7))
	{
		for (int i = 0; i < 1; i++)
		{
			if (!invisible[i].IsUsedITinvisible())
			{
				invisible[i].SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_8))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!ball[i].IsUsedITball())
			{
				ball[i].SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));
				break;
			}
		}
	}

#endif

	if (g_bPause == FALSE)
		return;

	// �����蔻�菈��
	CheckHitItem();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ���剻�A�C�e���̍X�V����
		giant[i].UpdateITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		// �������A�C�e���̍X�V����
		invisible[i].UpdateITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// �{�[���A�C�e���̍X�V����
		ball[i].UpdateITball();
	}

	// �u�[�������̍X�V����
	boom.UpdateITboom();

	// �n���}�[�̍X�V����
	hamr.UpdateITHamr();




}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// �{�[���A�C�e���̕`�揈��
		ball[i].DrawITball();
	}

	// �u�[�������̕`�揈��
	boom.DrawITboom();

	// �n���}�[�̕`�揈��
	hamr.DrawITHamr();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ���剻�A�C�e���̕`�揈��
		giant[i].DrawITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		// �������A�C�e���̕`�揈��
		invisible[i].DrawITinvisible();
	}

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

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ���剻�A�C�e��
		if (giant[i].IsUsedITgiant() && !giant[i].IsPickedITgiant())
		{
			XMFLOAT3 gi_pos = giant[i].GetPositionITgiant();
			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, gi_pos, player[j].size, GIANT_SIZE))
				{
					giant[i].PickITgiant(j);
				}
			}
		}
	}

	for (int i = 0; i < 1
		; i++)
	{
		//������
		if (invisible[i].IsUsedITinvisible() && !invisible[i].IsPickedITinvisible())
		{
			XMFLOAT3 invi_pos = invisible[i].GetPositionITinvisible();

			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, invi_pos, player[j].size, INVISIBLE_SIZE))
				{
					invisible[i].PickITinvisible(j);
				}
			}
		}
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//�{�[��
		if (bool use = ball[i].IsUsedITball())
		{
			XMFLOAT3 ball_pos = ball[i].GetPositionITball();
			BOOL pick = ball[i].IsPickedITball();
			BOOL to_throw = ball[i].IsThrewITball();

			if (!pick && !to_throw)
			{
				for (int j = 0; j < MAX_PLAYER; j++)
				{
					if (CollisionBC(player[j].pos, ball_pos, player[j].size, BALL_SIZE) && !player[j].haveWeapon)
					{
						ball[i].PickITball(j);
					}
				}
			}
			else if (to_throw && !pick)
			{
				for (int j = 0; j < MAX_PLAYER; j++)
				{
					if (CollisionBC(player[j].pos, ball_pos, player[j].size, BALL_SIZE))
					{
						ball[i].HitITball(j);
					}
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

	// �n���}�[
	if (bool use = hamr.IsUsedITHamr())
	{
		BOOL pick = hamr.IsPickedITHamr();
		XMFLOAT3 test_pos;

		if (pick)
		{
			test_pos = hamr.GetHeadWorldPosition();  // <-- This MUST be here
		}
		else
		{
			test_pos = hamr.GetPositionITHamr();
		}


		for (int i = 0; i < MAX_PLAYER; i++)
		{

			if (CollisionBC(player[i].pos, test_pos, player[i].size, HAMR_SIZE))
			{
				if (!hamr.IsPickedITHamr())
				{
					hamr.PickITHamr(i);
				}
				else
				{
					// Hammer is being held: hit, but do NOT give it to the player hit
					hamr.HitITHamr(i);
				}
			}
		}



		// This will draw the sphere at the swinging head position
		DrawDebugSphereOutline(test_pos, HAMR_SIZE, XMFLOAT4(1, 0.5f, 0, 1));

		DebugLine_Render(GetCameraViewProjMatrix());

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

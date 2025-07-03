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

#include "player.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "score.h"
#include "particle.h"
#include "collision.h"
#include "debugproc.h"
#include "player_select.h"

//�X�e�[�W�M�~�b�N�p
#include "SG_wind.h"
#include "SG_warpgate.h"
//�A�C�e���p
#include "item.h"

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
void CheckHit(void);



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// �|�[�YON/OFF


//�X�e�[�W�M�~�b�N�p
WIND wind;
WARPGATE warpgate[2];

//=============================================================================
// ����������
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// �t�B�[���h�̏�����
	InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ���C�g��L����	// �e�̏���������
	InitShadow();

	// �v���C���[�̏�����
	InitPlayer();

	// �ǂ̏�����
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f,  XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);

	// ��(�����p�̔�����)
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f,    XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f,   XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);


	// �X�R�A�̏�����
	InitScore();

	//// �p�[�e�B�N���̏�����
	//InitParticle();

	//���̏�����
	wind.InitSGwind();
	//wind.SetSGwind();

	//���[�v�Q�[�g�̏�����
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].InitSGwarpgate();
		warpgate[i].SetSGwarpgate(XMFLOAT3(0.0f + (300.0f * i), 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f));
	}

	//�A�C�e���̏�����
	InitItem();

	// BGM�Đ�
	PlaySound(SOUND_LABEL_BGM_mofu_1);

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitGame(void)
{
	//// �p�[�e�B�N���̏I������
	//UninitParticle();

	// �X�R�A�̏I������
	UninitScore();

	// �ǂ̏I������
	UninitMeshWall();

	// �n�ʂ̏I������
	UninitMeshField();

	// �v���C���[�̏I������
	UninitPlayer();

	// �e�̏I������
	UninitShadow();

	// ���̏I������
	wind.UninitSGwind();

	// ���[�v�Q�[�g�̏I������
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UninitSGwarpgate();
	}

	//�A�C�e���̏I������
	UninitItem();

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateGame(void)
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


#endif

	if(g_bPause == FALSE)
		return;

	// �n�ʏ����̍X�V
	UpdateMeshField();

	// �v���C���[�̍X�V����
	UpdatePlayer();

	// �Ǐ����̍X�V
	UpdateMeshWall();

	//�A�C�e���̍X�V����
	UpdateItem();

	//// �p�[�e�B�N���̍X�V����
	//UpdateParticle();

	// �e�̍X�V����
	UpdateShadow();

	// �����蔻�菈��
	CheckHit();

	//�A�C�e���̓����蔻�菈��
	CheckHitItem();

	// �X�R�A�̍X�V����
	UpdateScore();

	// ���̍X�V����
	wind.UpdateSGwind();

	// ���[�v�Q�[�g�̍X�V����
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UpdateSGwarpgate();
	}


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawGame0(void)
{
	// 3D�̕���`�悷�鏈��
	// �n�ʂ̕`�揈��
	DrawMeshField();

	// �e�̕`�揈��
	DrawShadow();

	// �v���C���[�̕`�揈��
	DrawPlayer();

	//�A�C�e���̕`�揈��
	DrawItem();

	// �ǂ̕`�揈��
	DrawMeshWall();

	// ���[�v�Q�[�g�̕`�揈��
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].DrawSGwarpgate();
	}

	//// �p�[�e�B�N���̕`�揈��
	//DrawParticle();


	// 2D�̕���`�悷�鏈��
	// Z��r�Ȃ�
	SetDepthEnable(FALSE);

	// ���C�e�B���O�𖳌�
	SetLightEnable(FALSE);

	// �X�R�A�̕`�揈��
	DrawScore();


	// ���C�e�B���O��L����
	SetLightEnable(TRUE);

	// Z��r����
	SetDepthEnable(TRUE);
}


void DrawGame(void)
{
	XMFLOAT3 pos;


#ifdef _DEBUG
	// �f�o�b�O�\��
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif
	DebugLine_BeginFrame();

	XMFLOAT3 playerPos = GetPlayer(0)->pos;
	XMFLOAT3 player2Pos = GetPlayer(1)->pos;
	EnsureCameraFramesTargets(playerPos, player2Pos, 0.35f); // 20%��O����J�����㏸

	// �v���C���[���_
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

		// �G�l�~�[���_
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		DrawGame0();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame0();

		// �G�l�~�[���_
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawGame0();
		break;

	}
#ifdef _DEBUG
	// �v���C���[�A�v���C���[�Q�A�G�l�~�[�̃f�o�b�O�p�����蔻�苅��`��
	DrawDebugSphereOutline(GetPlayer(0)->pos, GetPlayer(0)->size, XMFLOAT4(1, 0, 0, 1));


	DrawDebugCapsuleOutline(
		GetPlayer(0)->capsuleA,
		GetPlayer(0)->capsuleB,
		GetPlayer(0)->size,
		XMFLOAT4(1, 1, 1, 1), // color
		20,  // slices (roundness)
		4    // hemiRings (cap smoothness)
	);



	DrawDebugSphereOutline(GetPlayer(1)->pos, GetPlayer(1)->size, XMFLOAT4(1, 0, 0, 1));

	// �A�C�e���̃f�o�b�O�p�����蔻�苅��`��
	//if (giant.IsUsedITgiant())
	//	DrawDebugSphereOutline(giant.GetPositionITgiant(), GIANT_SIZE, XMFLOAT4(1, 1, 0, 1)); // Yellow


	//DrawDebugSphereOutline(invisible.GetPositionITinvisible(), INVISIBLE_SIZE, XMFLOAT4(0, 1, 0, 1)); // Green

	//if (ball.IsUsedITball())
	//	DrawDebugSphereOutline(ball.GetPositionITball(), BALL_SIZE, XMFLOAT4(1, 1, 1, 1)); // White

	//DebugLine_Render(GetCameraViewProjMatrix());
#endif
}


//=============================================================================
// �����蔻�菈��
//=============================================================================
void CheckHit(void)
{
	PLAYER* player = GetPlayer(); // �v���C���[1 (index=0)

	// ���[�v����
	for (int i = 0; i < MAX_WG; i++)
	{
		bool use = warpgate[i].IsUsed();
		//�G�̗L���t���O���`�F�b�N����
		if (use == FALSE)
			continue;

		XMFLOAT3 wg_pos = warpgate[i].GetPosition();
		XMFLOAT3 wg_hitscl = warpgate[i].GetHitScl();

		for (int j = 0; j < MAX_PLAYER; j++)
		{
			if (CollisionBB(player[j].pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player[j].gateUse == FALSE)
			{
				int n = i + 1;
				if (n > MAX_WG)n = 0;
				player[j].pos = warpgate[n].GetPosition();
				player[j].gateUse = TRUE;
				PrintDebugProc("warpgateHIT!!!:No%d\n", j);
			}
		}
	}

	// �G�l�~�[���S�����S�������ԑJ��
	int enemy_count = 0;
	for(int i=0;i<MAX_PLAYER;i++)
	{
		// �G�l�~�[���O�C�H
		if (player[i].use ==false)
		{
			SetFade(FADE_OUT, MODE_RESULT);
		}
	}
}



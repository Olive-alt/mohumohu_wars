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
#include "enemy.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "tree.h"
#include "bullet.h"
#include "score.h"
#include "particle.h"
#include "collision.h"
#include "debugproc.h"
#include "player_select.h"

//�X�e�[�W�M�~�b�N�p
#include "SG_wind.h"
#include "SG_warpgate.h"
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
void CheckHit(void);
void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);



//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// �|�[�YON/OFF


//�X�e�[�W�M�~�b�N�p
WIND wind;
WARPGATE warpgate[2];
//�A�C�e���p
GIANT giant;
INVISIBLE invisible;
BALL ball;
BALL* GetBall()  // �A�N�Z�X�p�̊֐����쐬
{
	return &ball;
}

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

	// �G�l�~�[�̏�����
	InitEnemy();

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

	// �؂𐶂₷
	InitTree();

	// �e�̏�����
	InitBullet();

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

	//���剻�A�C�e���̏�����
	giant.InitITgiant();
	giant.SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));

	//�������A�C�e���̏�����
	invisible.InitITinvisible();
	invisible.SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));

	//�{�[���A�C�e���̏�����
	ball.InitITball();
	ball.SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));

	// BGM�Đ�
	//PlaySound(SOUND_LABEL_BGM_sample001);

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

	// �e�̏I������
	UninitBullet();

	// �؂̏I������
	UninitTree();

	// �ǂ̏I������
	UninitMeshWall();

	// �n�ʂ̏I������
	UninitMeshField();

	// �G�l�~�[�̏I������
	UninitEnemy();

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

	// ���剻�A�C�e���̏I������
	giant.UninitITgiant();

	// �������A�C�e���̏I������
	invisible.UninitITinvisible();

	//�{�[���A�C�e���̏I������
	ball.UninitITball();

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

	// �G�l�~�[�̍X�V����
	UpdateEnemy();

	// �Ǐ����̍X�V
	UpdateMeshWall();

	// �؂̍X�V����
	UpdateTree();

	// �e�̍X�V����
	UpdateBullet();

	//// �p�[�e�B�N���̍X�V����
	//UpdateParticle();

	// �e�̍X�V����
	UpdateShadow();

	// �����蔻�菈��
	CheckHit();

	// �X�R�A�̍X�V����
	UpdateScore();

	// ���̍X�V����
	wind.UpdateSGwind();

	// ���[�v�Q�[�g�̍X�V����
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UpdateSGwarpgate();
	}

	// ���剻�A�C�e���̍X�V����
	giant.UpdateITgiant();

	// �������A�C�e���̍X�V����
	invisible.UpdateITinvisible();

	// �{�[���A�C�e���̍X�V����
	ball.UpdateITball();

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

	// �G�l�~�[�̕`�揈��
	DrawEnemy();

	// �v���C���[�̕`�揈��
	DrawPlayer();

	// �e�̕`�揈��
	DrawBullet();

	// �{�[���A�C�e���̕`�揈��
	ball.DrawITball();

	// ���剻�A�C�e���̕`�揈��
	giant.DrawITgiant();

	// �������A�C�e���̕`�揈��
	invisible.DrawITinvisible();

	// �ǂ̕`�揈��
	DrawMeshWall();

	// �؂̕`�揈��
	DrawTree();

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
		pos = GetEnemy()->pos;
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
		pos = GetEnemy()->pos;
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
	DrawDebugSphereOutline(GetPlayer(1)->pos, GetPlayer(1)->size, XMFLOAT4(1, 0, 0, 1));
	ENEMY* enemy = GetEnemy();
	for (int i = 0; i < MAX_ENEMY; ++i) {
		if (enemy[i].use)
			DrawDebugSphereOutline(enemy[i].pos, enemy[i].size, XMFLOAT4(0, 0, 1, 1));
	}

	// �A�C�e���̃f�o�b�O�p�����蔻�苅��`��
	if (giant.IsUsedITgiant())
		DrawDebugSphereOutline(giant.GetPositionITgiant(), GIANT_SIZE, XMFLOAT4(1, 1, 0, 1)); // Yellow


	DrawDebugSphereOutline(invisible.GetPositionITinvisible(), INVISIBLE_SIZE, XMFLOAT4(0, 1, 0, 1)); // Green

	if (ball.IsUsedITball())
		DrawDebugSphereOutline(ball.GetPositionITball(), BALL_SIZE, XMFLOAT4(1, 1, 1, 1)); // White

	DebugLine_Render(GetCameraViewProjMatrix());
#endif
}


//=============================================================================
// �����蔻�菈��
//=============================================================================
void CheckHit(void)
{
	ENEMY *enemy = GetEnemy();		// �G�l�~�[�̃|�C���^�[��������
	PLAYER* player0 = GetPlayer(0); // �v���C���[1 (index=0)
	PLAYER* player1 = GetPlayer(1); // �v���C���[2 (index=1)
	BULLET *bullet = GetBullet();	// �e�̃|�C���^�[��������

	// �G�ƃv���C���[�L����
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		//�G�̗L���t���O���`�F�b�N����
		if (enemy[i].use == FALSE)
			continue;

		// �v���C���[1 �ƓG�̏Փ�
		if (CollisionBC(player0->pos, enemy[i].pos, player0->size, enemy[i].size))
		{
			enemy[i].use = FALSE;
			ReleaseShadow(enemy[i].shadowIdx);
			player0->scl.x = min(player0->scl.x * 1.2f, 4.0f);
			player0->scl.y = min(player0->scl.y * 1.2f, 4.0f);
			player0->scl.z = min(player0->scl.z * 1.2f, 4.0f);
			AddScore(0, 100);
		}

		// �v���C���[2 �ƓG�̏Փˁi�K�v�Ȃ�L���t���O�`�F�b�N��ɓ��l�̏�����ǉ��j
		if (player1 && player1->use &&
			CollisionBC(player1->pos, enemy[i].pos, player1->size, enemy[i].size))
		{
			enemy[i].use = FALSE;
			ReleaseShadow(enemy[i].shadowIdx);
			player1->scl.x = min(player1->scl.x * 1.2f, 4.0f);
			player1->scl.y = min(player1->scl.y * 1.2f, 4.0f);
			player1->scl.z = min(player1->scl.z * 1.2f, 4.0f);
			AddScore(1, 100);
		}
	}


	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].use) continue;

		// �v���C���[2 �̒e (owner==2) ���v���C���[1 �ɓ���������
		if (bullet[i].owner == 2 &&
			CollisionBC(bullet[i].pos, player0->pos, bullet[i].fWidth, player0->size))
		{
			player0->hp -= 1.0f;
			if (player0->hp <= 0.0f) {
				player0->use = FALSE;
				ReleaseShadow(player0->shadowIdx);
				SetMode(MODE_RESULT);
			}
			bullet[i].use = FALSE;
			ReleaseShadow(bullet[i].shadowIdx);
			AddScore(1, 10);
		}

		// �v���C���[1 �̒e (owner==1) ���v���C���[2 �ɓ���������
		if (player1 && bullet[i].owner == 1 &&
			CollisionBC(bullet[i].pos, player1->pos, bullet[i].fWidth, player1->size))
		{
			player1->hp -= 1.0f;
			if (player1->hp <= 0.0f) {
				player1->use = FALSE;
				ReleaseShadow(player1->shadowIdx);
				SetMode(MODE_RESULT);
			}
			bullet[i].use = FALSE;
			ReleaseShadow(bullet[i].shadowIdx);
			AddScore(0, 10);
		}
	}


	// ���[�v����
	for (int j = 0; j < MAX_WG; j++)
	{
		bool use = warpgate[j].IsUsed();
		//�G�̗L���t���O���`�F�b�N����
		if (use == FALSE)
			continue;

		XMFLOAT3 wg_pos = warpgate[j].GetPosition();
		XMFLOAT3 wg_hitscl = warpgate[j].GetHitScl();

		//�@�v���C���[1
		//BC�̓����蔻��
		if (CollisionBB(player0->pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player0->gateUse == FALSE)
		{
			int n = j + 1;
			if (n > MAX_WG)n = 0;
			player0->pos = warpgate[n].GetPosition();
			player0->gateUse = TRUE;
			PrintDebugProc("warpgateHIT!!!:No%d\n", j);
		}

		//�@�v���C���[2
		//BC�̓����蔻��
		if (CollisionBB(player1->pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player1->gateUse == FALSE)
		{
			int n = j + 1;
			if (n > MAX_WG)n = 0;
			player1->pos = warpgate[n].GetPosition();
			player1->gateUse = TRUE;
			PrintDebugProc("warpgateHIT!!!:No%d\n", j);
		}
	}

	// ���剻�A�C�e��
	if (bool use = giant.IsUsedITgiant())
	{
		XMFLOAT3 gi_pos = giant.GetPositionITgiant();

		//�@�v���C���[1
		//BC�̓����蔻��
		if (CollisionBC(player0->pos, gi_pos, player0->size, GIANT_SIZE))
		{
			giant.PickITgiant();
		}

		//�@�v���C���[2
		if (CollisionBC(player1->pos, gi_pos, player1->size, GIANT_SIZE))
		{
			giant.PickITgiant();
		}

	}

	//������
	if (bool use = invisible.IsUsedITinvisible())
	{
		XMFLOAT3 invi_pos = invisible.GetPositionITinvisible();
		
		//�@�v���C���[1
		//BC�̓����蔻��
		if (CollisionBC(player0->pos, invi_pos, player0->size, INVISIBLE_SIZE))
		{
			invisible.PickITinvisible();
		}

		//�@�v���C���[2
		if (CollisionBC(player1->pos, invi_pos, player1->size, INVISIBLE_SIZE))
		{
			invisible.PickITinvisible();
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
			//�@�v���C���[1
			//BC�̓����蔻��
			if (CollisionBC(player1->pos, ball_pos, player1->size, BALL_SIZE))
			{
				ball.PickITball();
			}

			//�@�v���C���[2
			if (CollisionBC(player1->pos, ball_pos, player1->size, BALL_SIZE))
			{
				ball.PickITball();
			}


		}
	}

	// �G�l�~�[���S�����S�������ԑJ��
	int enemy_count = 0;
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (enemy[i].use == FALSE) continue;
		enemy_count++;
	}

	// �G�l�~�[���O�C�H
	if (enemy_count == 0)
	{
		SetFade(FADE_OUT, MODE_RESULT);
	}

}


// ���̃��C���[�t���[����`�悷��֐�
void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices)
{
	// XY���ʂ̉~��`��
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius, center.z);
		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius, center.z);
		DebugLine_DrawLine(p1, p2, color);
	}

	// XZ���ʂ̉~��`��
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y, center.z + sinf(theta1) * radius);
		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y, center.z + sinf(theta2) * radius);
		DebugLine_DrawLine(p1, p2, color);
	}

	// YZ���ʂ̉~��`��
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x, center.y + cosf(theta1) * radius, center.z + sinf(theta1) * radius);
		XMFLOAT3 p2(center.x, center.y + cosf(theta2) * radius, center.z + sinf(theta2) * radius);
		DebugLine_DrawLine(p1, p2, color);
	}
}

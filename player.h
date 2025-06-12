//=============================================================================
//
// ���f������ [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PLAYER		(2)					// �v���C���[�̐�

#define	PLAYER_SIZE		(15.0f)				// �����蔻��̑傫��

#define PLAYER_HEIGHT   (20.0f)  // �J�v�Z���̍����i���f���̓����瑫�܂ł̒����j
//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct PLAYER
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	float				spd;				// �ړ��X�s�[�h
	
	BOOL				load;
	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
	float					hp;
	//���[�v�Q�[�g�N�[���^�C���p
	BOOL				gateUse;
	int					gateCoolTime;

	//����d���h�~�p
	BOOL haveWeapon;

	BOOL				use;

	float				size;

	// �K�w�A�j���[�V�����p�̃����o�[�ϐ�
	float				time;				// ���`��ԗp
	int					tblNo;				// �s���f�[�^�̃e�[�u���ԍ�
	int					tblMax;				// ���̃e�[�u���̃f�[�^��

	// �e�́ANULL�A�q���͐e�̃A�h���X������
	PLAYER				*parent;			// �������e�Ȃ�NULL�A�������q���Ȃ�e��player�A�h���X

	// �N�H�[�^�j�I��
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// �����������Ă��鏊

	// �J�v�Z�������蔻��p
	XMFLOAT3 capsuleA;  // �J�v�Z�����[
	XMFLOAT3 capsuleB;  // �J�v�Z����[

};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

// �v���C���[�擾�i�݊��Ł{�C���f�b�N�X�Łj
PLAYER* GetPlayer(void);         // �݊��Findex=0
PLAYER* GetPlayer(int index);    // index = 0�`PLAYER_MAX-1

void DrawPlayerHpBar();

void HandlePlayerInput(void);	//�e���ˏ���
void MovePlayers(void);			//�ړ�����


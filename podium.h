#pragma once

#include "model.h"

#define MAX_PODIUM		(2)					// �v���C���[�̐�

typedef struct
{
	XMFLOAT4X4	mtxWorld;		// ���[���h�}�g���b�N�X
	XMFLOAT3	pos;			// �ʒu
	XMFLOAT3	rot;			// �p�x
	XMFLOAT3	scl;			// �X�P�[��
	MATERIAL	material;		// �}�e���A��
	int			shadowIdx;		// �eID
	BOOL		use;			// �g�p���Ă��邩�ǂ���

	DX11_MODEL			model;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F
	BOOL				load;

} PODIUM;


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitPodium(void);
void UninitPodium(void);
void UpdatePodium(void);
void DrawPodium(void);

// �v���C���[�擾�i�݊��Ł{�C���f�b�N�X�Łj
PODIUM* GetPodium(void);         // �݊��Findex=0

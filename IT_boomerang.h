#pragma once
//=============================================================================
//
// �n�ʏ��� [field.h]
// Author : 
//
//=============================================================================
#pragma once

#include "renderer.h"
#include <random>
#include "model.h"

#define BOOM_SIZE	(10.0f)
#define BOOM_MAX		(1)

//*****************************************************************************
// �}�N����`
//*****************************************************************************
class BOOM
{
private:
	XMFLOAT3		pos;		// �|���S���̈ʒu
	XMFLOAT3		rot;		// �|���S���̌���(��])
	XMFLOAT3		scl;		// �|���S���̑傫��(�X�P�[��)
	XMFLOAT3		move;		// �|���S���̈ʒu
	int				PlayerIndex;		// �|���S���̈ʒu

	BOOL			use;
	BOOL			to_throw;
	BOOL			pick;
	BOOL			load;
	DX11_MODEL		model;				// ���f�����
	XMFLOAT4		diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float			spd;				// �ړ��X�s�[�h
	float			size;				// �����蔻��̑傫��
	int				shadowIdx;			// �e�̃C���f�b�N�X�ԍ�
	float			count;

	//�A�C�R���p
	XMFLOAT3		icon_pos;			// �ʒu
	XMFLOAT3		icon_scl;			// �X�P�[��
	MATERIAL		icon_material;		// �}�e���A��
	float			icon_fWidth;			// ��
	float			icon_fHeight;		// ����
	BOOL			icon_use;			// �g�p���Ă��邩�ǂ���

	XMFLOAT4X4		m_mtxWorld;	// ���[���h�}�g���b�N�X
	bool returning;      // �߂��Ă��邩�ǂ���
	XMFLOAT3 startPos;   // �����n�߂��ʒu
	XMFLOAT3 endPos;     // �����I���ʒu�i�߂�Ƃ��̃v���C���[�ʒu�j
	XMFLOAT3 throwDir;   // ����������i���K���ς݁j
	float boomerangArcRadius; // �J�[�u�̕��i�����p�j
	float boomerangTime;      // ���݂̎��ԁi0=�J�n, 1=�I���j
	float boomerangSpeed;     // �u�[�������̑����i�����p�j

	float spinAngle; // Radians
	float spinSpeed; // Radians per frame (or per time)

public:
	HRESULT InitITboom(void);
	void UninitITboom(void);
	void UpdateITboom(void);
	void DrawITboom(void);

	void SetITboomObject(XMFLOAT3 set_pos, int playerIndex);

	void SetITboom(XMFLOAT3 set_pos, XMFLOAT3 p_rot);
	void HitITboom(int p_Index);
	void PickITboom(int p_Index);

	bool IsUsedITboom() const { return use; }
	bool IsPickedITboom() const { return pick; }
	bool IsThrewITboom() const { return to_throw; }
	XMFLOAT3 GetPositionITboom() const { return pos; }
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

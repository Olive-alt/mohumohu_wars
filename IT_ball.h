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

#define BALL_SIZE	(15.0f)
#define BALL_MAX		(1)

//*****************************************************************************
// �}�N����`
//*****************************************************************************
class BALL
{
private:
	XMFLOAT3		pos;		// �|���S���̈ʒu
	XMFLOAT3		rot;		// �|���S���̌���(��])
	XMFLOAT3		scl;		// �|���S���̑傫��(�X�P�[��)
	XMFLOAT3		move;		// �|���S���̈ʒu
	//XMFLOAT3		Bmove;		// �|���S���̈ʒu

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

public:
	HRESULT InitITball(void);
	void UninitITball(void);
	void UpdateITball(void);
	void DrawITball(void);

	void SetITballObject(XMFLOAT3 set_pos);
	void SetITball(XMFLOAT3 set_pos, XMFLOAT3 p_rot);
	void HitITball(void);
	void PickITball(void);

	bool IsUsedITball() const { return use; }
	bool IsPickedITball() const { return pick; }
	bool IsThrewITball() const { return to_throw; }
	XMFLOAT3 GetPositionITball() const { return pos; }
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

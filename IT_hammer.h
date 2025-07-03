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

#define HAMR_SIZE	(10.0f)
#define HAMR_MAX		(1)

//*****************************************************************************
// �}�N����`
//*****************************************************************************
class HAMR
{
private:
	XMFLOAT3		pos;		// �|���S���̈ʒu
	XMFLOAT3		rot;		// �|���S���̌���(��])
	XMFLOAT3		scl;		// �|���S���̑傫��(�X�P�[��)
	XMFLOAT3		move;		// �|���S���̈ʒu
	int				PlayerIndex;		// �|���S���̈ʒu

	BOOL			use;
	BOOL			to_swing;
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
	float swingAngle = 0.0f;   // ��]�p�x�i�����l0�j
	const float swingMax = XM_PIDIV2; // 90�x (��/2)

public:
	HRESULT InitITHamr(void);
	void UninitITHamr(void);
	void UpdateITHamr(void);
	void DrawITHamr(void);

	void SetITHamrObject(XMFLOAT3 set_pos);
	void HitITHamr(int p_Index);
	void PickITHamr(int p_Index);

	XMFLOAT3 GetHeadWorldPosition() const;

	bool IsUsedITHamr() const { return use; }
	bool IsPickedITHamr() const { return pick; }

	XMFLOAT3 GetPositionITHamr() const { return pos; }
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

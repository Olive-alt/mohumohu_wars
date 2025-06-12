#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
//#define	MAX_GIANT			(2)			// �؍ő吔

#define	GIANT_SIZE		(15.0f)				// �����蔻��̑傫��

class GIANT
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ���[���h�}�g���b�N�X
	BOOL		use;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	XMFLOAT3	old_scl;
	MATERIAL	material;		// �}�e���A��
	BOOL		load;
	DX11_MODEL	model;				// ���f�����
	int			shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	BOOL		giantUse;
	int			giantTimer;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}

	HRESULT InitITgiant(void);
	void UninitITgiant(void);
	void UpdateITgiant(void);
	void DrawITgiant(void);

	void SetITgiant(XMFLOAT3 set_pos);
	void FinishITgiant(void);
	void PickITgiant(void);

	bool IsUsedITgiant() const { return use; }
	XMFLOAT3 GetPositionITgiant() const { return pos; }
};

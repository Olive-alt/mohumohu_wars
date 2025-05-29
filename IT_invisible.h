#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
#define	MAX_INVISIBLE			(1)			// �؍ő吔

#define	INVISIBLE_SIZE		(5.0f)				// �����蔻��̑傫��

class INVISIBLE
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ���[���h�}�g���b�N�X
	BOOL		use;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	MATERIAL	material;		// �}�e���A��
	BOOL		load;
	DX11_MODEL	model;				// ���f�����
	XMFLOAT4	old_diffuse[MODEL_MAX_MATERIAL];
	int			shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

	BOOL		invisibleUse;
	int			invisibleTimer;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}

	HRESULT InitITinvisible(void);
	void UninitITinvisible(void);
	void UpdateITinvisible(void);
	void DrawITinvisible(void);

	void SetITinvisible(XMFLOAT3 set_pos);
	void FinishITinvisible(void);
	void PickITinvisible(void);

	bool IsUsedITinvisible() const { return use; }
	XMFLOAT3 GetPositionITinvisible() const { return pos; }
};

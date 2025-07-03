#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
//#define	MAX_GIANT			(2)			// 木最大数

#define	GIANT_SIZE		(15.0f)				// 当たり判定の大きさ

class GIANT
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ワールドマトリックス
	BOOL		use = FALSE;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	XMFLOAT3	old_scl;
	float      old_size;
	MATERIAL	material;		// マテリアル
	int			shadowIdx;			// 影のインデックス番号

	BOOL		giantUse = FALSE;
	int			giantTimer;
	int         PlayerIndex;
	BOOL		pick;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}
	HRESULT InitITgiant(void);
	void UninitITgiant(void);
	void UpdateITgiant(void);
	void DrawITgiant(void);

	void SetITgiant(XMFLOAT3 set_pos);
	void FinishITgiant(void);
	void PickITgiant(int p_Index);

	bool IsUsedITgiant() const { return use; }
	bool IsPickedITgiant() const { return pick; }
	XMFLOAT3 GetPositionITgiant() const { return pos; }
};

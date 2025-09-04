#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
//#define	MAX_GIANT			(2)			// 木最大数

#define	HEAL_SIZE		(15.0f)				// 当たり判定の大きさ

class HEAL
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ワールドマトリックス
	BOOL		use = FALSE;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	MATERIAL	material;		// マテリアル
	int			shadowIdx;			// 影のインデックス番号

	int         PlayerIndex;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}
	HRESULT InitITheal(void);
	void UninitITheal(void);
	void UpdateITheal(void);
	void DrawITheal(void);

	void SetITheal(XMFLOAT3 set_pos);
	void FinishITheal(void);
	void PickITheal(int p_Index);

	bool IsUsedITheal() const { return use; }
	XMFLOAT3 GetPositionITheal() const { return pos; }
};

#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

#define	MUTEKI_SIZE		(15.0f)				// 当たり判定の大きさ

class MUTEKI
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ワールドマトリックス
	BOOL		use;
	BOOL		mutekiUse;
	BOOL		pick;
	int			mutekiCount;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	MATERIAL	material;		// マテリアル
	int			shadowIdx;			// 影のインデックス番号

	int         PlayerIndex;

public:
	HRESULT InitITmuteki(void);
	void UninitITmuteki(void);
	void UpdateITmuteki(void);
	void DrawITmuteki(void);

	void SetITmuteki(XMFLOAT3 set_pos);
	void FinishITmuteki(void);
	void PickITmuteki(int p_Index);

	bool IsUsedITmuteki() const { return use; }
	bool IsPickedITmuteki() const { return pick; }
	XMFLOAT3 GetPositionITmuteki() const { return pos; }
};

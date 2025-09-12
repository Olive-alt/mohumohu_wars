#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

#define	ENADORI_SIZE		(15.0f)				// 当たり判定の大きさ
#define ENADORI_OFFSET_Y		(12.0f)							// プレイヤーの足元をあわせる

class ENADORI
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ワールドマトリックス
	BOOL		use;
	BOOL		enadoriUse;
	BOOL		pick;
	int			enadoriCount;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	MATERIAL	material;		// マテリアル
	int			shadowIdx;			// 影のインデックス番号

	int         PlayerIndex;

public:
	HRESULT InitITenadori(void);
	void UninitITenadori(void);
	void UpdateITenadori(void);
	void DrawITenadori(void);

	void SetITenadori(XMFLOAT3 set_pos);
	void FinishITenadori(void);
	void PickITenadori(int p_Index);

	bool IsUsedITenadori() const { return use; }
	bool IsPickedITenadori() const { return pick; }
	XMFLOAT3 GetPositionITenadori() const { return pos; }
};

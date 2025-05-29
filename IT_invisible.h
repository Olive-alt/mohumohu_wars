#pragma once
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
#define	MAX_INVISIBLE			(1)			// 木最大数

#define	INVISIBLE_SIZE		(5.0f)				// 当たり判定の大きさ

class INVISIBLE
{
private:
	XMFLOAT4X4	m_mtxWorld;			// ワールドマトリックス
	BOOL		use;
	XMFLOAT3	pos;
	XMFLOAT3	rot;
	XMFLOAT3	scl;
	MATERIAL	material;		// マテリアル
	BOOL		load;
	DX11_MODEL	model;				// モデル情報
	XMFLOAT4	old_diffuse[MODEL_MAX_MATERIAL];
	int			shadowIdx;			// 影のインデックス番号

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

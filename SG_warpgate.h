#pragma once
#include "renderer.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
#define	MAX_WG			(2)			// 木最大数

class WARPGATE
{
private:
	BOOL use;
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMFLOAT3 scl;
	MATERIAL material;		// マテリアル
	XMFLOAT3 hit_scl;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}

	HRESULT InitSGwarpgate(void);
	void UninitSGwarpgate(void);
	void UpdateSGwarpgate(void);
	void DrawSGwarpgate(void) const;

	void WARPGATE::SetSGwarpgate(XMFLOAT3 set_pos, XMFLOAT3 set_rot, XMFLOAT3 set_scl);
	bool IsUsed() const { return use; }
	XMFLOAT3 GetPosition() const { return pos; }
	XMFLOAT3 GetHitScl() const { return hit_scl; }
};

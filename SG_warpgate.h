#pragma once
#include "renderer.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
#define	MAX_WG			(3)			// 木最大数
#define WG_OFFSET_Y		(25.0f)							// プレイヤーの足元をあわせる

class WARPGATE
{
private:
	BOOL use;
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMFLOAT3 scl;
	MATERIAL material;		// マテリアル
	XMFLOAT3 hit_scl;
	int RespawnCount;

public:
	//WARPGATE(XMFLOAT3& init_pos) :  pos(init_pos) {}

	HRESULT InitSGwarpgate(void);
	void UninitSGwarpgate(void);
	void UpdateSGwarpgate(void);
	void DrawSGwarpgate(void) const;

	void SetSGwarpgate(XMFLOAT3 set_pos, XMFLOAT3 set_rot, XMFLOAT3 set_scl);
	void SetSGwarpgatePos(XMFLOAT3 set_pos);
	bool IsUsed() const { return use; }
	XMFLOAT3 GetPosition() const { return pos; }
	XMFLOAT3 GetHitScl() const { return hit_scl; }
};

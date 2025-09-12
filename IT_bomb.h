//=============================================================================
//
// 地面処理 [field.h]
// Author : 
//
//=============================================================================
#pragma once

#include "renderer.h"
#include <random>
#include "model.h"

#define BOMB_SIZE	(10.0f)
#define BOMB_MAX		(1)
#define BOMB_OFFSET_Y		(9.0f)							// プレイヤーの足元をあわせる

//*****************************************************************************
// マクロ定義
//*****************************************************************************
class BOMB
{
private:
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)
	XMFLOAT3		move;		// ポリゴンの位置
	int				PlayerIndex;		// ポリゴンの位置

	BOOL			use;
	BOOL			to_throw;
	BOOL			pick;
	XMFLOAT4		diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float			spd;				// 移動スピード
	float			size;				// 当たり判定の大きさ
	int				shadowIdx;			// 影のインデックス番号
	float			count;

	float			expSize;				// 当たり判定の大きさ
	BOOL			expUse;

	//アイコン用
	XMFLOAT3		icon_pos;			// 位置
	XMFLOAT3		icon_scl;			// スケール
	MATERIAL		icon_material;		// マテリアル
	float			icon_fWidth;			// 幅
	float			icon_fHeight;		// 高さ
	BOOL			icon_use;			// 使用しているかどうか

	XMFLOAT4X4		m_mtxWorld;	// ワールドマトリックス

public:
	HRESULT InitITbomb(void);
	void UninitITbomb(void);
	void UpdateITbomb(void);
	void DrawITbomb(void);

	void SetITbombObject(XMFLOAT3 set_pos);
	void SetITbomb(XMFLOAT3 set_pos, XMFLOAT3 p_rot);
	void HitITbomb(int p_Index);
	void PickITbomb(int p_Index);
	void FinishITbomb(void);


	bool IsUsedITbomb() const { return use; }
	bool IsPickedITbomb() const { return pick; }
	bool IsThrewITbomb() const { return to_throw; }
	bool IsExpUseITbomb() const { return expUse; }
	XMFLOAT3 GetPositionITbomb() const { return pos; }
	float GetExpSizeITbomb() const { return expSize; }
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

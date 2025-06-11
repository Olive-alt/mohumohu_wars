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

#define BALL_SIZE	(15.0f)
#define BALL_MAX		(1)

//*****************************************************************************
// マクロ定義
//*****************************************************************************
class BALL
{
private:
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)
	XMFLOAT3		move;		// ポリゴンの位置
	//XMFLOAT3		Bmove;		// ポリゴンの位置

	BOOL			use;
	BOOL			to_throw;
	BOOL			pick;
	BOOL			load;
	DX11_MODEL		model;				// モデル情報
	XMFLOAT4		diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	float			spd;				// 移動スピード
	float			size;				// 当たり判定の大きさ
	int				shadowIdx;			// 影のインデックス番号
	float			count;

	//アイコン用
	XMFLOAT3		icon_pos;			// 位置
	XMFLOAT3		icon_scl;			// スケール
	MATERIAL		icon_material;		// マテリアル
	float			icon_fWidth;			// 幅
	float			icon_fHeight;		// 高さ
	BOOL			icon_use;			// 使用しているかどうか

	XMFLOAT4X4		m_mtxWorld;	// ワールドマトリックス

public:
	HRESULT InitITball(void);
	void UninitITball(void);
	void UpdateITball(void);
	void DrawITball(void);

	void SetITballObject(XMFLOAT3 set_pos);
	void SetITball(XMFLOAT3 set_pos, XMFLOAT3 p_rot);
	void HitITball(void);
	void PickITball(void);

	bool IsUsedITball() const { return use; }
	bool IsPickedITball() const { return pick; }
	bool IsThrewITball() const { return to_throw; }
	XMFLOAT3 GetPositionITball() const { return pos; }
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

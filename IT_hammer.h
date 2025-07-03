#pragma once
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

#define HAMR_SIZE	(10.0f)
#define HAMR_MAX		(1)

//*****************************************************************************
// マクロ定義
//*****************************************************************************
class HAMR
{
private:
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)
	XMFLOAT3		move;		// ポリゴンの位置
	int				PlayerIndex;		// ポリゴンの位置

	BOOL			use;
	BOOL			to_swing;
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
	float swingAngle = 0.0f;   // 回転角度（初期値0）
	const float swingMax = XM_PIDIV2; // 90度 (π/2)

public:
	HRESULT InitITHamr(void);
	void UninitITHamr(void);
	void UpdateITHamr(void);
	void DrawITHamr(void);

	void SetITHamrObject(XMFLOAT3 set_pos);
	void HitITHamr(int p_Index);
	void PickITHamr(int p_Index);

	XMFLOAT3 GetHeadWorldPosition() const;

	bool IsUsedITHamr() const { return use; }
	bool IsPickedITHamr() const { return pick; }

	XMFLOAT3 GetPositionITHamr() const { return pos; }
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

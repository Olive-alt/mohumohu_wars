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

#define BOOM_SIZE	(10.0f)
#define BOOM_MAX		(1)

//*****************************************************************************
// マクロ定義
//*****************************************************************************
class BOOM
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
	bool returning;      // 戻っているかどうか
	XMFLOAT3 startPos;   // 投げ始めた位置
	XMFLOAT3 endPos;     // 投げ終わる位置（戻るときのプレイヤー位置）
	XMFLOAT3 throwDir;   // 投げる方向（正規化済み）
	float boomerangArcRadius; // カーブの幅（調整用）
	float boomerangTime;      // 現在の時間（0=開始, 1=終了）
	float boomerangSpeed;     // ブーメランの速さ（調整用）

	float spinAngle; // Radians
	float spinSpeed; // Radians per frame (or per time)

public:
	HRESULT InitITboom(void);
	void UninitITboom(void);
	void UpdateITboom(void);
	void DrawITboom(void);

	void SetITboomObject(XMFLOAT3 set_pos, int playerIndex);

	void SetITboom(XMFLOAT3 set_pos, XMFLOAT3 p_rot);
	void HitITboom(int p_Index);
	void PickITboom(int p_Index);

	bool IsUsedITboom() const { return use; }
	bool IsPickedITboom() const { return pick; }
	bool IsThrewITboom() const { return to_throw; }
	XMFLOAT3 GetPositionITboom() const { return pos; }
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

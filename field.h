//=============================================================================
//
// 地面＆フィールドマスク処理 [field.h]
// Author :
//
//=============================================================================
#pragma once

#include "renderer.h"

//*****************************************************************************
// 構造体
//*****************************************************************************
struct FIELD
{
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)

	XMFLOAT4X4		mtxWorld;	// ワールドマトリックス
};

// PLAYER（他ヘッダ）を前方宣言
struct PLAYER;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitField(void);
void    UninitField(void);
void    UpdateField(void);
void    DrawField(void);

// ---- フィールド減速用マスク（PGM 1枚）-------------------------------
// ※ InitField() の後、ステージ読み込み直後などで一度だけ呼んでください。
bool    Field_LoadMaskPGM(const char* pgmPath);

// 指定座標の速度係数（0..1）。範囲外は 1.0。minScale=黒のときの最低速度。
float   Field_SpeedScaleAt(const XMFLOAT3& pos, float minScale = 0.50f);

// プレイヤーの移動量(XZ)に減速を適用（prevPos 必須）
void    Field_ApplySlowdown(PLAYER* p, float minScale = 0.50f);

// 黒ほど沈む量（maxSink を上限）を返す。範囲外は 0。
float   Field_SinkOffsetAt(const XMFLOAT3& pos, float maxSink = 16.0f);

// ★ 疑似吸着＋減速をまとめて適用（レイ不使用）
void    Field_ApplyPseudoSnapAndSlow(PLAYER* p,
	float baseY = 0.0f,    // 乾いた地面の高さ
	float minScale = 0.50f,// 黒の地点の速度係数
	float maxSink = 16.0f  // 黒の地点での沈み量
);
void Field_ApplyImageProbeSnapAndSlow(PLAYER* p,
	float probeSizeWorld,
	unsigned char waterThreshold,
	float minScale,
	float maxSink);
// 手打ちでマスクのワールド対応範囲を指定（minX<maxX, minZ<maxZ）
bool Field_SetMaskWorldBounds(float minX, float maxX, float minZ, float maxZ);

void Field_ApplyImageProbeSnapAndSlow(PLAYER* p,
	float /*probeSizeWorld*/,
	unsigned char /*waterThreshold*/,
	float minScale,
	float maxSink);

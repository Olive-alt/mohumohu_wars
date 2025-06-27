#pragma once

#include "model.h"

#define MAX_PODIUM		(2)					// プレイヤーの数

typedef struct
{
	XMFLOAT4X4	mtxWorld;		// ワールドマトリックス
	XMFLOAT3	pos;			// 位置
	XMFLOAT3	rot;			// 角度
	XMFLOAT3	scl;			// スケール
	MATERIAL	material;		// マテリアル
	int			shadowIdx;		// 影ID
	BOOL		use;			// 使用しているかどうか

	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色
	BOOL				load;

} PODIUM;


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPodium(void);
void UninitPodium(void);
void UpdatePodium(void);
void DrawPodium(void);

// プレイヤー取得（互換版＋インデックス版）
PODIUM* GetPodium(void);         // 互換：index=0

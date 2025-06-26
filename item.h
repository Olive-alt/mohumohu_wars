//=============================================================================
//
// ゲーム画面処理 [game.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IT_ball.h"
#include "IT_boomerang.h"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitItem(void);
void UninitItem(void);
void UpdateItem(void);
void DrawItem(void);

void CheckHitItem(void);
BALL* GetBall();  // アクセス用の関数を作成
BOOM* GetBoomerang();  // アクセス用の関数を作成
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices); //debug用の関数

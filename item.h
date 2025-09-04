//=============================================================================
//
// ゲーム画面処理 [game.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IT_giant.h"
#include "IT_invisible.h"
#include "IT_ball.h"
#include "IT_bomb.h"
#include "IT_boomerang.h"
#include "IT_hammer.h"


#define ITEM_MAX	(10)
#define ITEM_TOTAL_MAX	(6)

extern GIANT giant[ITEM_MAX];
extern INVISIBLE invisible[1];
extern BALL ball[ITEM_MAX];
extern BOOM boom[ITEM_MAX];
extern HAMR hamr[ITEM_MAX];
extern BOMB bomb[ITEM_MAX];

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
HAMR* GetHammer();  // アクセス用の関数を作成	
BOMB* GetBomb();  // アクセス用の関数を作成
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices); //debug用の関数

//=============================================================================
//
// スコア処理 [time.h]
// Author : 
//
//=============================================================================
#pragma once

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH (16) // キャラサイズ
#define TEXTURE_HEIGHT (32)
#define TEXTURE_MAX (1) // テクスチャの数
#define TIME_DIGIT (4) // MMSS形式で4桁表示
#define TIME_LIMIT_DEFAULT (100) // デフォルト制限時間（秒）

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTime(void);
void UninitTime(void);
void UpdateTime(void);
void DrawTime(void);

void SetTimeLimit(float sec); // 制限時間を設定
float GetRemainingTime(void); // 残り時間を取得
bool IsTimeOver(void); // 時間切れ判定

// 経過時間（DeltaTime）管理
void InitDeltaTime(void); // 経過時間初期化（InitTime内で呼ばれる）
void UpdateDeltaTime(void); // 毎フレームの冒頭で呼び出す
float GetDeltaTime(void); // 前のフレームとの経過時間（秒）を取得
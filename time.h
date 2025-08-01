//=============================================================================
//
// タイマー・残り時間管理 [time.h]
// Author : (Your Name)
//
//=============================================================================
#pragma once

//*****************************************************************************
// 定数定義（マジックナンバーは極力定数化）
//*****************************************************************************
#define TIME_TEXTURE_WIDTH    (16)      // 1桁の数字画像サイズ（px）
#define TIME_TEXTURE_HEIGHT   (32)
#define TIME_TEXTURE_MAX      (1)       // 使用テクスチャ数
#define TIME_DIGITS          (4)        // 表示桁数（MMSS形式: 99分59秒まで対応）
#define TIME_LIMIT_DEFAULT   (10.0f)    // デフォルト制限時間（秒）

//*****************************************************************************
// タイマー管理の関数プロトタイプ
//*****************************************************************************
HRESULT InitTime(void);       // タイマー機能初期化
void    UninitTime(void);     // リソース解放
void    UpdateTime(void);     // 残り時間の更新
void    DrawTime(void);       // 残り時間の描画

void    SetTimeLimit(float sec);    // 制限時間（秒）を設定し、リセット
float   GetRemainingTime(void);     // 残り時間（秒）取得
bool    IsTimeOver(void);           // 時間切れか？

// Δ時間（経過時間）管理
void    InitDeltaTime(void);   // 経過時間初期化（内部で呼ぶ）
void    UpdateDeltaTime(void); // 毎フレームの最初に呼び出す
float   GetDeltaTime(void);    // 前フレームとの経過秒数取得

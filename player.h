//=============================================================================
//
// モデル処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(2)					// プレイヤーの数

#define	PLAYER_SIZE		(15.0f)				// 当たり判定の大きさ

#define PLAYER_HEIGHT   (20.0f)  // カプセルの高さ（モデルの頭から足までの長さ）
//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
	XMFLOAT4X4			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	float				spd;				// 移動スピード
	
	BOOL				load;
	DX11_MODEL			model;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色

	int					shadowIdx;			// 影のインデックス番号
	float					hp;
	//ワープゲートクールタイム用
	BOOL				gateUse;
	int					gateCoolTime;

	//武器重複防止用
	BOOL haveWeapon;

	BOOL				use;

	float				size;

	// 階層アニメーション用のメンバー変数
	float				time;				// 線形補間用
	int					tblNo;				// 行動データのテーブル番号
	int					tblMax;				// そのテーブルのデータ数

	// 親は、NULL、子供は親のアドレスを入れる
	PLAYER				*parent;			// 自分が親ならNULL、自分が子供なら親のplayerアドレス

	// クォータニオン
	XMFLOAT4			Quaternion;

	XMFLOAT3			UpVector;			// 自分が立っている所

	// カプセル当たり判定用
	XMFLOAT3 capsuleA;  // カプセル下端
	XMFLOAT3 capsuleB;  // カプセル上端

};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

// プレイヤー取得（互換版＋インデックス版）
PLAYER* GetPlayer(void);         // 互換：index=0
PLAYER* GetPlayer(int index);    // index = 0～PLAYER_MAX-1

void DrawPlayerHpBar();

void HandlePlayerInput(void);	//弾発射処理
void MovePlayers(void);			//移動処理


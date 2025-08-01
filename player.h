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
#define PLAYER_PARTS_MAX	(5)								// プレイヤーのパーツの数

#define	PLAYER_SIZE		(15.0f)				// 当たり判定の大きさ

#define PLAYER_HEIGHT   (20.0f)  // カプセルの高さ（モデルの頭から足までの長さ）

static bool g_PlayerIsMoving[MAX_PLAYER] = { false, false };

// 状態管理（移動開始の瞬間を検出）
static bool g_PlayerPrevMoving[MAX_PLAYER] = { false };
static int  g_PlayerAnimBlendTimer[MAX_PLAYER] = { 0 }; // blend進行度
static int  g_PlayerAnimBlendMode[MAX_PLAYER] = { 0 };  // 0:通常, 1:ブレンドIN, 2:ブレンドOUT
static float g_PlayerAnimBlendFrom[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 }; // ブレンドOUT用

typedef enum
{
	PLAYER_NORMAL,
	PLAYER_WALK,
	PLAYER_HIT,
	PLAYER_DEAD,
	PLAYER_RESULT_WIN,
	PLAYER_RESULT_LOSE,
} PLAYER_STATE;


extern PLAYER_STATE g_PlayerState[MAX_PLAYER];
extern int g_PlayerAnimTimer[MAX_PLAYER];
extern XMFLOAT3 g_PlayerKnockback[MAX_PLAYER]; // 吹っ飛び速度

static float partAngle[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 };    // バネ用の一時値
static float partVelocity[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 }; // バネ用速度

// プレイヤーがCPUかどうか
extern bool g_IsCPU[MAX_PLAYER];

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

	//巨大化用
	BOOL				big;
	//透明化用
	BOOL				invisible;

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

	bool squished;
	float squishTimer;
	XMFLOAT3 originalScl;

	// アニメーション状態
	PLAYER_STATE currentAnimation;

	float stunTimer;      // スタン中か（0なら非スタン）
	XMFLOAT3 knockbackVel;// ノックバック速度
	float radius;         // 当たり判定半径
	XMFLOAT3 prevPos; // 前フレームの座標

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

void UpdatePlayerPartsAnimation(int playerIndex);
void UpdatePlayerKnockback(int playerIndex);
void OnPlayerHit(int i, const XMFLOAT3& hitDirection);
float GetNoise(float strength = 0.12f);
float GetPeriodicNoise(int seed, float time, float strength = 0.12f);
float GetSmoothNoise(float& last, float strength = 0.12f);
void PSetAnimation(int playerIndex, PLAYER_STATE animation);
float NormalizeAngle(float angle);
float SmoothAngle(float current, float target, float smoothFactor = 0.1f);
float turning(float target, float current);
void PlayerAttack(int playerIndex);

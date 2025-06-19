//=============================================================================
//
// ゲーム画面処理 [game.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "game.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"

#include "player.h"
#include "enemy.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "tree.h"
#include "bullet.h"
#include "score.h"
#include "particle.h"
#include "collision.h"
#include "debugproc.h"
#include "player_select.h"

//ステージギミック用
#include "SG_wind.h"
#include "SG_warpgate.h"
//アイテム用
#include "item.h"

//デバッグ表示
#include "debugline.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
//ステージギミック用
//アイテム用


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void CheckHit(void);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF


//ステージギミック用
WIND wind;
WARPGATE warpgate[2];

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// フィールドの初期化
	InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ライトを有効化	// 影の初期化処理
	InitShadow();

	// プレイヤーの初期化
	InitPlayer();

	// エネミーの初期化
	InitEnemy();

	// 壁の初期化
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f,  XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);

	// 壁(裏側用の半透明)
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f,    XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f,   XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);

	// 木を生やす
	InitTree();

	// 弾の初期化
	InitBullet();

	// スコアの初期化
	InitScore();

	//// パーティクルの初期化
	//InitParticle();

	//風の初期化
	wind.InitSGwind();
	//wind.SetSGwind();

	//ワープゲートの初期化
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].InitSGwarpgate();
		warpgate[i].SetSGwarpgate(XMFLOAT3(0.0f + (300.0f * i), 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f));
	}

	//アイテムの初期化
	InitItem();

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGame(void)
{
	//// パーティクルの終了処理
	//UninitParticle();

	// スコアの終了処理
	UninitScore();

	// 弾の終了処理
	UninitBullet();

	// 木の終了処理
	UninitTree();

	// 壁の終了処理
	UninitMeshWall();

	// 地面の終了処理
	UninitMeshField();

	// エネミーの終了処理
	UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();

	// 影の終了処理
	UninitShadow();

	// 風の終了処理
	wind.UninitSGwind();

	// ワープゲートの終了処理
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UninitSGwarpgate();
	}

	//アイテムの終了処理
	UninitItem();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGame(void)
{
#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V))
	{
		g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE;
		SetViewPort(g_ViewPortType_Game);
	}

	if (GetKeyboardTrigger(DIK_P))
	{
		g_bPause = g_bPause ? FALSE : TRUE;
	}


#endif

	if(g_bPause == FALSE)
		return;

	// 地面処理の更新
	UpdateMeshField();

	// プレイヤーの更新処理
	UpdatePlayer();

	// エネミーの更新処理
	UpdateEnemy();

	// 壁処理の更新
	UpdateMeshWall();

	// 木の更新処理
	UpdateTree();

	// 弾の更新処理
	UpdateBullet();

	//アイテムの更新処理
	UpdateItem();

	//// パーティクルの更新処理
	//UpdateParticle();

	// 影の更新処理
	UpdateShadow();

	// 当たり判定処理
	CheckHit();

	//アイテムの当たり判定処理
	CheckHitItem();

	// スコアの更新処理
	UpdateScore();

	// 風の更新処理
	wind.UpdateSGwind();

	// ワープゲートの更新処理
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UpdateSGwarpgate();
	}


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGame0(void)
{
	// 3Dの物を描画する処理
	// 地面の描画処理
	DrawMeshField();

	// 影の描画処理
	DrawShadow();

	// エネミーの描画処理
	DrawEnemy();

	// プレイヤーの描画処理
	DrawPlayer();

	// 弾の描画処理
	DrawBullet();

	//アイテムの描画処理
	DrawItem();

	// 壁の描画処理
	DrawMeshWall();

	// 木の描画処理
	DrawTree();

	// ワープゲートの描画処理
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].DrawSGwarpgate();
	}

	//// パーティクルの描画処理
	//DrawParticle();


	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);

	// スコアの描画処理
	DrawScore();


	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}


void DrawGame(void)
{
	XMFLOAT3 pos;


#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif
	DebugLine_BeginFrame();

	XMFLOAT3 playerPos = GetPlayer(0)->pos;
	XMFLOAT3 player2Pos = GetPlayer(1)->pos;
	EnsureCameraFramesTargets(playerPos, player2Pos, 0.35f); // 20%手前からカメラ上昇

	// プレイヤー視点
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawGame0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawGame0();

		// エネミー視点
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		DrawGame0();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawGame0();

		// エネミー視点
		pos = GetEnemy()->pos;
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawGame0();
		break;

	}
#ifdef _DEBUG
	// プレイヤー、プレイヤー２、エネミーのデバッグ用当たり判定球を描画
	DrawDebugSphereOutline(GetPlayer(0)->pos, GetPlayer(0)->size, XMFLOAT4(1, 0, 0, 1));


	DrawDebugCapsuleOutline(
		GetPlayer(0)->capsuleA,
		GetPlayer(0)->capsuleB,
		GetPlayer(0)->size,
		XMFLOAT4(1, 1, 1, 1), // color
		20,  // slices (roundness)
		4    // hemiRings (cap smoothness)
	);



	DrawDebugSphereOutline(GetPlayer(1)->pos, GetPlayer(1)->size, XMFLOAT4(1, 0, 0, 1));
	ENEMY* enemy = GetEnemy();
	for (int i = 0; i < MAX_ENEMY; ++i) {
		if (enemy[i].use)
			DrawDebugSphereOutline(enemy[i].pos, enemy[i].size, XMFLOAT4(0, 0, 1, 1));
	}

	// アイテムのデバッグ用当たり判定球を描画
	//if (giant.IsUsedITgiant())
	//	DrawDebugSphereOutline(giant.GetPositionITgiant(), GIANT_SIZE, XMFLOAT4(1, 1, 0, 1)); // Yellow


	//DrawDebugSphereOutline(invisible.GetPositionITinvisible(), INVISIBLE_SIZE, XMFLOAT4(0, 1, 0, 1)); // Green

	//if (ball.IsUsedITball())
	//	DrawDebugSphereOutline(ball.GetPositionITball(), BALL_SIZE, XMFLOAT4(1, 1, 1, 1)); // White

	//DebugLine_Render(GetCameraViewProjMatrix());
#endif
}


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	ENEMY* enemy = GetEnemy();		// エネミーのポインターを初期化
	BULLET* bullet = GetBullet();	// 弾のポインターを初期化
	PLAYER* player = GetPlayer(); // プレイヤー1 (index=0)

	// 敵とプレイヤーキャラ
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		//敵の有効フラグをチェックする
		if (enemy[i].use == FALSE)
			continue;

		for (int j = 0; j < MAX_PLAYER; j++)
		{
			// プレイヤー1 と敵の衝突
			if (CollisionBC(player[j].pos, enemy[i].pos, player[j].size, enemy[i].size))
			{
				enemy[i].use = FALSE;
				ReleaseShadow(enemy[i].shadowIdx);
				AddScore(0, 100);
			}
		}
	}


	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].use) continue;

		for (int j = 0; j < MAX_PLAYER; j++)
		{
			// プレイヤー1 と敵の衝突
			if (bullet[i].owner == 2 && CollisionBC(bullet[i].pos, player[j].pos, bullet[i].fWidth, player[j].size))
			{
				player[j].hp -= 1.0f;
				if (player[j].hp <= 0.0f) {
					player[j].use = FALSE;
					ReleaseShadow(player[j].shadowIdx);
					SetMode(MODE_RESULT);
				}
				bullet[i].use = FALSE;
				ReleaseShadow(bullet[i].shadowIdx);
				AddScore(1, 10);
			}
		}
	}


	// ワープ処理
	for (int i = 0; i < MAX_WG; i++)
	{
		bool use = warpgate[i].IsUsed();
		//敵の有効フラグをチェックする
		if (use == FALSE)
			continue;

		XMFLOAT3 wg_pos = warpgate[i].GetPosition();
		XMFLOAT3 wg_hitscl = warpgate[i].GetHitScl();

		for (int j = 0; j < MAX_PLAYER; j++)
		{
			if (CollisionBB(player[j].pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player[j].gateUse == FALSE)
			{
				int n = i + 1;
				if (n > MAX_WG)n = 0;
				player[j].pos = warpgate[n].GetPosition();
				player[j].gateUse = TRUE;
				PrintDebugProc("warpgateHIT!!!:No%d\n", j);
			}
		}
	}

	// エネミーが全部死亡したら状態遷移
	int enemy_count = 0;
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (enemy[i].use == FALSE) continue;
		enemy_count++;
	}

	// エネミーが０匹？
	if (enemy_count == 0)
	{
		SetFade(FADE_OUT, MODE_RESULT);
	}

}



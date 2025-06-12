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
#include "IT_giant.h"
#include "IT_invisible.h"

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
void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF


//ステージギミック用
WIND wind;
WARPGATE warpgate[2];
//アイテム用
GIANT giant;
INVISIBLE invisible;
BALL ball;
BALL* GetBall()  // アクセス用の関数を作成
{
	return &ball;
}

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

	//巨大化アイテムの初期化
	giant.InitITgiant();
	giant.SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));

	//透明化アイテムの初期化
	invisible.InitITinvisible();
	invisible.SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));

	//ボールアイテムの初期化
	ball.InitITball();
	ball.SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));

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

	// 巨大化アイテムの終了処理
	giant.UninitITgiant();

	// 透明化アイテムの終了処理
	invisible.UninitITinvisible();

	//ボールアイテムの終了処理
	ball.UninitITball();

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

	//// パーティクルの更新処理
	//UpdateParticle();

	// 影の更新処理
	UpdateShadow();

	// 当たり判定処理
	CheckHit();

	// スコアの更新処理
	UpdateScore();

	// 風の更新処理
	wind.UpdateSGwind();

	// ワープゲートの更新処理
	for (int i = 0; i < MAX_WG; i++)
	{
		warpgate[i].UpdateSGwarpgate();
	}

	// 巨大化アイテムの更新処理
	giant.UpdateITgiant();

	// 透明化アイテムの更新処理
	invisible.UpdateITinvisible();

	// ボールアイテムの更新処理
	ball.UpdateITball();

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

	// ボールアイテムの描画処理
	ball.DrawITball();

	// 巨大化アイテムの描画処理
	giant.DrawITgiant();

	// 透明化アイテムの描画処理
	invisible.DrawITinvisible();

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
	DrawDebugSphereOutline(GetPlayer(1)->pos, GetPlayer(1)->size, XMFLOAT4(1, 0, 0, 1));
	ENEMY* enemy = GetEnemy();
	for (int i = 0; i < MAX_ENEMY; ++i) {
		if (enemy[i].use)
			DrawDebugSphereOutline(enemy[i].pos, enemy[i].size, XMFLOAT4(0, 0, 1, 1));
	}

	// アイテムのデバッグ用当たり判定球を描画
	if (giant.IsUsedITgiant())
		DrawDebugSphereOutline(giant.GetPositionITgiant(), GIANT_SIZE, XMFLOAT4(1, 1, 0, 1)); // Yellow


	DrawDebugSphereOutline(invisible.GetPositionITinvisible(), INVISIBLE_SIZE, XMFLOAT4(0, 1, 0, 1)); // Green

	if (ball.IsUsedITball())
		DrawDebugSphereOutline(ball.GetPositionITball(), BALL_SIZE, XMFLOAT4(1, 1, 1, 1)); // White

	DebugLine_Render(GetCameraViewProjMatrix());
#endif
}


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	ENEMY *enemy = GetEnemy();		// エネミーのポインターを初期化
	PLAYER* player0 = GetPlayer(0); // プレイヤー1 (index=0)
	PLAYER* player1 = GetPlayer(1); // プレイヤー2 (index=1)
	BULLET *bullet = GetBullet();	// 弾のポインターを初期化

	// 敵とプレイヤーキャラ
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		//敵の有効フラグをチェックする
		if (enemy[i].use == FALSE)
			continue;

		// プレイヤー1 と敵の衝突
		if (CollisionBC(player0->pos, enemy[i].pos, player0->size, enemy[i].size))
		{
			enemy[i].use = FALSE;
			ReleaseShadow(enemy[i].shadowIdx);
			player0->scl.x = min(player0->scl.x * 1.2f, 4.0f);
			player0->scl.y = min(player0->scl.y * 1.2f, 4.0f);
			player0->scl.z = min(player0->scl.z * 1.2f, 4.0f);
			AddScore(0, 100);
		}

		// プレイヤー2 と敵の衝突（必要なら有効フラグチェック後に同様の処理を追加）
		if (player1 && player1->use &&
			CollisionBC(player1->pos, enemy[i].pos, player1->size, enemy[i].size))
		{
			enemy[i].use = FALSE;
			ReleaseShadow(enemy[i].shadowIdx);
			player1->scl.x = min(player1->scl.x * 1.2f, 4.0f);
			player1->scl.y = min(player1->scl.y * 1.2f, 4.0f);
			player1->scl.z = min(player1->scl.z * 1.2f, 4.0f);
			AddScore(1, 100);
		}
	}


	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].use) continue;

		// プレイヤー2 の弾 (owner==2) がプレイヤー1 に当たったら
		if (bullet[i].owner == 2 &&
			CollisionBC(bullet[i].pos, player0->pos, bullet[i].fWidth, player0->size))
		{
			player0->hp -= 1.0f;
			if (player0->hp <= 0.0f) {
				player0->use = FALSE;
				ReleaseShadow(player0->shadowIdx);
				SetMode(MODE_RESULT);
			}
			bullet[i].use = FALSE;
			ReleaseShadow(bullet[i].shadowIdx);
			AddScore(1, 10);
		}

		// プレイヤー1 の弾 (owner==1) がプレイヤー2 に当たったら
		if (player1 && bullet[i].owner == 1 &&
			CollisionBC(bullet[i].pos, player1->pos, bullet[i].fWidth, player1->size))
		{
			player1->hp -= 1.0f;
			if (player1->hp <= 0.0f) {
				player1->use = FALSE;
				ReleaseShadow(player1->shadowIdx);
				SetMode(MODE_RESULT);
			}
			bullet[i].use = FALSE;
			ReleaseShadow(bullet[i].shadowIdx);
			AddScore(0, 10);
		}
	}


	// ワープ処理
	for (int j = 0; j < MAX_WG; j++)
	{
		bool use = warpgate[j].IsUsed();
		//敵の有効フラグをチェックする
		if (use == FALSE)
			continue;

		XMFLOAT3 wg_pos = warpgate[j].GetPosition();
		XMFLOAT3 wg_hitscl = warpgate[j].GetHitScl();

		//　プレイヤー1
		//BCの当たり判定
		if (CollisionBB(player0->pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player0->gateUse == FALSE)
		{
			int n = j + 1;
			if (n > MAX_WG)n = 0;
			player0->pos = warpgate[n].GetPosition();
			player0->gateUse = TRUE;
			PrintDebugProc("warpgateHIT!!!:No%d\n", j);
		}

		//　プレイヤー2
		//BCの当たり判定
		if (CollisionBB(player1->pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) && player1->gateUse == FALSE)
		{
			int n = j + 1;
			if (n > MAX_WG)n = 0;
			player1->pos = warpgate[n].GetPosition();
			player1->gateUse = TRUE;
			PrintDebugProc("warpgateHIT!!!:No%d\n", j);
		}
	}

	// 巨大化アイテム
	if (bool use = giant.IsUsedITgiant())
	{
		XMFLOAT3 gi_pos = giant.GetPositionITgiant();

		//　プレイヤー1
		//BCの当たり判定
		if (CollisionBC(player0->pos, gi_pos, player0->size, GIANT_SIZE))
		{
			giant.PickITgiant();
		}

		//　プレイヤー2
		if (CollisionBC(player1->pos, gi_pos, player1->size, GIANT_SIZE))
		{
			giant.PickITgiant();
		}

	}

	//透明化
	if (bool use = invisible.IsUsedITinvisible())
	{
		XMFLOAT3 invi_pos = invisible.GetPositionITinvisible();
		
		//　プレイヤー1
		//BCの当たり判定
		if (CollisionBC(player0->pos, invi_pos, player0->size, INVISIBLE_SIZE))
		{
			invisible.PickITinvisible();
		}

		//　プレイヤー2
		if (CollisionBC(player1->pos, invi_pos, player1->size, INVISIBLE_SIZE))
		{
			invisible.PickITinvisible();
		}

	}

	//ボール
	if (bool use = ball.IsUsedITball())
	{
		XMFLOAT3 ball_pos = ball.GetPositionITball();
		BOOL pick = ball.IsPickedITball();
		BOOL to_throw = ball.IsThrewITball();

		if (!pick && !to_throw)
		{
			//　プレイヤー1
			//BCの当たり判定
			if (CollisionBC(player1->pos, ball_pos, player1->size, BALL_SIZE))
			{
				ball.PickITball();
			}

			//　プレイヤー2
			if (CollisionBC(player1->pos, ball_pos, player1->size, BALL_SIZE))
			{
				ball.PickITball();
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


// 球のワイヤーフレームを描画する関数
void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices)
{
	// XY平面の円を描画
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius, center.z);
		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius, center.z);
		DebugLine_DrawLine(p1, p2, color);
	}

	// XZ平面の円を描画
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y, center.z + sinf(theta1) * radius);
		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y, center.z + sinf(theta2) * radius);
		DebugLine_DrawLine(p1, p2, color);
	}

	// YZ平面の円を描画
	for (int i = 0; i < slices; ++i) {
		float theta1 = XM_2PI * i / slices;
		float theta2 = XM_2PI * (i + 1) / slices;
		XMFLOAT3 p1(center.x, center.y + cosf(theta1) * radius, center.z + sinf(theta1) * radius);
		XMFLOAT3 p2(center.x, center.y + cosf(theta2) * radius, center.z + sinf(theta2) * radius);
		DebugLine_DrawLine(p1, p2, color);
	}
}

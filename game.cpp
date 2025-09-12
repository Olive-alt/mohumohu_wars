//=============================================================================
// ゲーム画面処理 [game.cpp]
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
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "score.h"
#include "particle.h"
#include "collision.h"
#include "debugproc.h"
#include "player_select.h"
#include "stageobject.h"
#include "stage_select.h"   // ★ 追加：g_SelectedStageFile

// ステージギミック
#include "SG_wind.h"
#include "SG_warpgate.h"
#include "SG_car.h"
// アイテム
#include "item.h"
// タイム
#include "time.h"
// デバッグ表示
#include "debugline.h"

//*****************************************************************************
// マクロ定義 / プロトタイプ
//*****************************************************************************
void CheckHit(void);

//*****************************************************************************
// グローバル
//*****************************************************************************
static int  g_ViewPortType_Game = TYPE_FULL_SCREEN;
static BOOL g_bPause = TRUE;

WIND     wind;
WARPGATE warpgate[MAX_WG];

//=============================================================================
// 初期化
//=============================================================================
HRESULT InitGame(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// フィールド
	InitMeshField(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), 100, 100, 13.0f, 13.0f);

	// 影
	InitShadow();

	// ステージ（モデル→インスタンス）
	InitStageModels();
	LoadStageObjects(g_SelectedStageFile);

	// ★ 水上アスレ対応：ファイル名で水面Yを決定
	if (strstr(g_SelectedStageFile, "beach")) {
		// たとえば床（bg_1.obj）の高さを 0 と想定（必要なら値は調整）
		SetWaterLevel(0.0f);
	}
	else {
		// 無効化
		SetWaterLevel(-1.0e9f);
	}

	// プレイヤー
	InitPlayer();

	// 壁
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1, 1, 1, 1), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f), XMFLOAT4(1, 1, 1, 1), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f), XMFLOAT4(1, 1, 1, 1), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, XM_PI, 0.0f), XMFLOAT4(1, 1, 1, 1), 16, 2, 80.0f, 80.0f);
	// 裏面半透明
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, XM_PI, 0.0f), XMFLOAT4(1, 1, 1, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f), XMFLOAT4(1, 1, 1, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f), XMFLOAT4(1, 1, 1, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1, 1, 1, 0.25f), 16, 2, 80.0f, 80.0f);

	// スコア・タイム
	InitScore();
	InitTime();

	// 風
	wind.InitSGwind();

	// ワープゲート
	for (int i = 0; i < MAX_WG; i++) {
		warpgate[i].InitSGwarpgate();
		warpgate[i].SetSGwarpgate(XMFLOAT3(GetRandf(-500.0f, 500.0f), 0.0f, GetRandf(-500.0f, 500.0f)),
			XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	}

	// 車
	g_CarSystem.Init();

	// アイテム
	InitItem();

	// BGM
	PlaySound(SOUND_LABEL_BGM_party);

	return S_OK;
}

//=============================================================================
// 終了
//=============================================================================
void UninitGame(void)
{
	UninitScore();
	UninitTime();
	UninitMeshWall();
	UninitMeshField();
	UninitPlayer();
	UninitShadow();

	wind.UninitSGwind();
	for (int i = 0; i < MAX_WG; i++) { warpgate[i].UninitSGwarpgate(); }

	g_CarSystem.Uninit();
	UninitItem();
}

//=============================================================================
// 更新
//=============================================================================
void UpdateGame(void)
{
	UpdateDeltaTime();

#ifdef _DEBUG
	if (GetKeyboardTrigger(DIK_V)) { g_ViewPortType_Game = (g_ViewPortType_Game + 1) % TYPE_NONE; SetViewPort(g_ViewPortType_Game); }
	if (GetKeyboardTrigger(DIK_P)) { g_bPause = g_bPause ? FALSE : TRUE; }
#endif
	if (g_bPause == FALSE) return;

	UpdateMeshField();
	UpdatePlayer();
	UpdateMeshWall();
	UpdateItem();

	g_CarSystem.Update();
	g_CarSystem.CheckCarHitPlayers();

	UpdateShadow();

	CheckHit();       // 既存の当たり判定群
	CheckHitItem();

	UpdateScore();
	UpdateTime();

	wind.UpdateSGwind();
	for (int i = 0; i < MAX_WG; i++) { warpgate[i].UpdateSGwarpgate(); }
}

//=============================================================================
// 描画
//=============================================================================
void DrawGame0(void)
{
	DrawMeshField();
	DrawShadow();
	DrawStageObjects();
	DrawPlayer();
	g_CarSystem.Draw();
	DrawItem();
	DrawMeshWall();
	for (int i = 0; i < MAX_WG; i++) { warpgate[i].DrawSGwarpgate(); }

	SetDepthEnable(FALSE);
	SetLightEnable(FALSE);
	DrawScore();
	DrawTime();
	SetLightEnable(TRUE);
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

	// --- プレイヤーの位置を集めてフレーミング ---
	XMFLOAT3 pts[4];
	int n = 0;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		PLAYER* p = GetPlayer(i);
		if (p && p->use) {
			pts[n++] = p->pos;
		}
	}
	if (n > 0) {
		EnsureCameraFramesTargetsN(pts, n, 0.15f); // 0.15f は画面端の余裕
	}

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
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawGame0();
		break;

	}
#ifdef _DEBUG
	//// プレイヤー、プレイヤー２、エネミーのデバッグ用当たり判定球を描画
	//DrawDebugSphereOutline(GetPlayer(0)->pos, GetPlayer(0)->size, XMFLOAT4(1, 0, 0, 1));


	//DrawDebugCapsuleOutline(
	//	GetPlayer(0)->capsuleA,
	//	GetPlayer(0)->capsuleB,
	//	GetPlayer(0)->size,
	//	XMFLOAT4(1, 1, 1, 1), // color
	//	20,  // slices (roundness)
	//	4    // hemiRings (cap smoothness)
	//);



	//DrawDebugSphereOutline(GetPlayer(1)->pos, GetPlayer(1)->size, XMFLOAT4(1, 0, 0, 1));
	//ENEMY* enemy = GetEnemy();
	//for (int i = 0; i < MAX_ENEMY; ++i) {
	//	if (enemy[i].use)
	//		DrawDebugSphereOutline(enemy[i].pos, enemy[i].size, XMFLOAT4(0, 0, 1, 1));
	//}

	//if (giant.IsUsedITgiant())
	//	DrawDebugSphereOutline(giant.GetPositionITgiant(), GIANT_SIZE, XMFLOAT4(1, 1, 0, 1)); // Yellow


	//DrawDebugSphereOutline(invisible.GetPositionITinvisible(), INVISIBLE_SIZE, XMFLOAT4(0, 1, 0, 1)); // Green

	if (GetHammer()->IsUsedITHamr())
	{
		XMFLOAT3 test_pos = GetHammer()->GetHeadWorldPosition();  // Always use this
		DrawDebugSphereOutline(test_pos, HAMR_SIZE, XMFLOAT4(1, 0.5f, 0, 1)); // Orange
	}


	DebugLine_Render(GetCameraViewProjMatrix());
#endif
}


//=============================================================================
// 当たり判定処理
//=============================================================================
void CheckHit(void)
{
	// ワープ処理
	for (int i = 0; i < MAX_WG; i++)
	{
		if (!warpgate[i].IsUsed()) continue;

		const XMFLOAT3 wg_pos = warpgate[i].GetPosition();
		const XMFLOAT3 wg_hitscl = warpgate[i].GetHitScl();

		for (int j = 0; j < MAX_PLAYER; j++)
		{
			PLAYER* pj = GetPlayer(j);
			if (!pj || !pj->use) continue;

			if (CollisionBB(pj->pos, wg_pos, XMFLOAT3(50.0f, 50.0f, 50.0f), wg_hitscl) &&
				pj->gateUse == FALSE)
			{
				int n = i + 1;
				if (n >= MAX_WG) n = 0;     //範囲ガード

				// テレポート
				pj->pos = warpgate[n].GetPosition();
				pj->gateUse = TRUE;
				PrintDebugProc("warpgateHIT!!!:No%d\n", j);
			}
		}
	}

	// 終了条件チェック
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		const PLAYER* pj = GetPlayer(i);
		if (!pj || !pj->use)
		{
			SetFade(FADE_OUT, MODE_RESULT);
		}
	}
}



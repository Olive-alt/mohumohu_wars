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

#include "item.h"
#include "player.h"
#include "collision.h"
#include "debugproc.h"

//アイテム用
#include "IT_giant.h"
#include "IT_invisible.h"
#include "IT_heal.h"

//デバッグ表示
#include "debugline.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ITEM_MAX	(10)
#define ITEM_TOTAL_MAX	(6)


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);
void ItemSpown(void);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF
int SpownTime = 0;

//アイテム用
GIANT giant[ITEM_MAX];

HEAL heal[ITEM_MAX];

INVISIBLE invisible[1];

BALL ball[ITEM_MAX];
BALL* GetBall()  // アクセス用の関数を作成
{
	return &ball[0];
}

BOOM boom[ITEM_MAX];  // ブーメランのインスタンスを作成
BOOM* GetBoomerang()  // アクセス用の関数を作成
{
	return &boom[0];  // ボールの中にあるブーメランを返す
}

HAMR hamr[ITEM_MAX];
HAMR* GetHammer()  // アクセス用の関数を作成
{
	return &hamr[0];  // ハンマーのインスタンスを返す
}

BOMB bomb[ITEM_MAX];
BOMB* GetBomb()  // アクセス用の関数を作成
{
	return &bomb[0];
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitItem(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;
	SpownTime = 0;

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] 巨大化アイテムの初期化（全要素をInitITgiantで初期化する）
		giant[i].InitITgiant();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		//回復アイテムの初期化
		heal[i].InitITheal();
	}
	for (int i = 0; i < 1; i++)
	{
		// [目的] 透明化アイテムの初期化（全要素をInitITinvisibleで初期化する）
		invisible[i].InitITinvisible();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボールアイテムの初期化（全要素をInitITballで初期化する）
		ball[i].InitITball();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ブーメランアイテムの初期化（全要素をInitITboomで初期化する）
		boom[i].InitITboom();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ハンマーアイテムの初期化（全要素をInitITHamrで初期化する）
		hamr[i].InitITHamr();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボムアイテムの初期化（全要素をInitITbombで初期化する）
		bomb[i].InitITbomb();
	}

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 巨大化アイテムの終了処理
		giant[i].UninitITgiant();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 回復アイテムの終了処理
		heal[i].UninitITheal();
	}

	for (int i = 0; i < 1; i++)
	{
		// 透明化アイテムの終了処理
		invisible[i].UninitITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//ボールアイテムの終了処理
		ball[i].UninitITball();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//ボムアイテムの終了処理
		bomb[i].UninitITbomb();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ブーメランの終了処理
		boom[i].UninitITboom();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ハンマーの終了処理
		hamr[i].UninitITHamr();
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateItem(void)
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

	if (GetKeyboardTrigger(DIK_5))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!heal[i].IsUsedITheal())
			{
				heal[i].SetITheal(XMFLOAT3(100.0f, 0.0f, 0.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_6))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!giant[i].IsUsedITgiant())
			{
				giant[i].SetITgiant(XMFLOAT3(200.0f, 0.0f, 0.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_7))
	{
		for (int i = 0; i < 1; i++)
		{
			if (!invisible[i].IsUsedITinvisible())
			{
				invisible[i].SetITinvisible(XMFLOAT3(300.0f, 0.0f, 0.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_8))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!ball[i].IsUsedITball())
			{
				ball[i].SetITballObject(XMFLOAT3(400.0f, 0.0f, 0.0f));
				break;
			}
		}
	}

	if (GetKeyboardTrigger(DIK_9))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!bomb[i].IsUsedITbomb())
			{
				bomb[i].SetITbombObject(XMFLOAT3(500.0f, 0.0f, 0.0f));
				break;
			}
		}
	}

#endif

	if (g_bPause == FALSE)
		return;

	SpownTime += 1;
	if (SpownTime >= 300)
	{
		ItemSpown();
		SpownTime = 0;
	}

	// 当たり判定処理
	CheckHitItem();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] 巨大化アイテムの更新（全要素をUpdateITgiantで更新する）
		giant[i].UpdateITgiant();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 回復アイテムの更新処理
		heal[i].UpdateITheal();
	}
	for (int i = 0; i < 1; i++)
	{
		// [目的] 透明化アイテムの更新（全要素をUpdateITinvisibleで更新する）
		invisible[i].UpdateITinvisible();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボールアイテムの更新（全要素をUpdateITballで更新する）
		ball[i].UpdateITball();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ブーメランアイテムの更新（全要素をUpdateITboomで更新する）
		boom[i].UpdateITboom();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ハンマーアイテムの更新（全要素をUpdateITHamrで更新する）
		hamr[i].UpdateITHamr();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボムアイテムの更新（全要素をUpdateITbombで更新する）
		bomb[i].UpdateITbomb();
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] 巨大化アイテムの描画（全要素をDrawITgiantで描画する）
		giant[i].DrawITgiant();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 回復アイテムの描画処理
		heal[i].DrawITheal();
	}
	for (int i = 0; i < 1; i++)
	{
		// [目的] 透明化アイテムの描画（全要素をDrawITinvisibleで描画する）
		invisible[i].DrawITinvisible();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボールアイテムの描画（全要素をDrawITballで描画する）
		ball[i].DrawITball();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ブーメランアイテムの描画（全要素をDrawITboomで描画する）
		boom[i].DrawITboom();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ハンマーアイテムの描画（全要素をDrawITHamrで描画する）
		hamr[i].DrawITHamr();
	}
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// [目的] ボムアイテムの描画（全要素をDrawITbombで描画する）
		bomb[i].DrawITbomb();
	}

	// 2Dの物を描画する処理
	// Z比較なし
	SetDepthEnable(FALSE);

	// ライティングを無効
	SetLightEnable(FALSE);


	// ライティングを有効に
	SetLightEnable(TRUE);

	// Z比較あり
	SetDepthEnable(TRUE);
}


//=============================================================================
// 当たり判定処理（安全化＆軽量最適化版）
//=============================================================================
void CheckHitItem(void)
{
	// ===== 巨大化 =====
	for (int i = 0; i < ITEM_MAX; ++i)
	{
		if (!giant[i].IsUsedITgiant() || giant[i].IsPickedITgiant())
			continue;

		const XMFLOAT3 gi_pos = giant[i].GetPositionITgiant();

		for (int j = 0; j < MAX_PLAYER; ++j)
		{
			PLAYER* pj = GetPlayer(j);
			if (!pj || !pj->use) continue;

			if (CollisionBC(pj->pos, gi_pos, pj->size, GIANT_SIZE))
			{
				giant[i].PickITgiant(j);
			}
		}
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ===== 回復アイテム =====
		if (!heal[i].IsUsedITheal())
		continue;

		XMFLOAT3 heal_pos = heal[i].GetPositionITheal();
		for (int j = 0; j < MAX_PLAYER; j++)
		{
			PLAYER* pj = GetPlayer(j);
			if (!pj || !pj->use) continue;

			if (CollisionBC(pj->pos, heal_pos, pj->size, HEAL_SIZE))
			{
				heal[i].PickITheal(j);
			}
		}
	}


	// ===== 透明化 =====
	for (int i = 0; i < 1; ++i)
	{
		if (!invisible[i].IsUsedITinvisible() || invisible[i].IsPickedITinvisible())
			continue;

		const XMFLOAT3 invi_pos = invisible[i].GetPositionITinvisible();

		for (int j = 0; j < MAX_PLAYER; ++j)
		{
			PLAYER* pj = GetPlayer(j);
			if (!pj || !pj->use) continue;

			if (CollisionBC(pj->pos, invi_pos, pj->size, INVISIBLE_SIZE))
			{
				invisible[i].PickITinvisible(j);
			}
		}
	}

	// ===== ボール =====
	for (int i = 0; i < ITEM_MAX; ++i)
	{
		if (!ball[i].IsUsedITball())
			continue;

		const XMFLOAT3 ball_pos = ball[i].GetPositionITball();
		const BOOL picked = ball[i].IsPickedITball();
		const BOOL threw = ball[i].IsThrewITball();

		// フロアに落ちている → 取得判定
		if (!picked && !threw)
		{
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;

				if (CollisionBC(pj->pos, ball_pos, pj->size, BALL_SIZE) && !pj->haveWeapon)
				{
					ball[i].PickITball(j);
				}
			}
		}
		// 投擲中 → ヒット判定
		else if (threw && !picked)
		{
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;
				if (pj->invisible) continue;
				if (CollisionBC(pj->pos, ball_pos, pj->size, BALL_SIZE))
				{
					ball[i].HitITball(j); // 自己ヒットは側で弾いている実装です
				}
			}
		}
	}

	// ===== ボム =====
	for (int i = 0; i < ITEM_MAX; ++i)
	{
		if (!bomb[i].IsUsedITbomb())
			continue;

		const XMFLOAT3 bomb_pos = bomb[i].GetPositionITbomb();
		const BOOL picked = bomb[i].IsPickedITbomb();
		const BOOL expuse = bomb[i].IsExpUseITbomb();
		const BOOL threw = bomb[i].IsThrewITbomb();

		// 落ちている → 取得判定
		if (!picked && !expuse && !threw)
		{
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;

				if (CollisionBC(pj->pos, bomb_pos, pj->size, BOMB_SIZE))
				{
					bomb[i].PickITbomb(j);
				}
			}
		}
		// 爆発中 → ダメージ判定
		else if (expuse)
		{
			const float expsize = bomb[i].GetExpSizeITbomb();
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;
				if (pj->invisible) continue;
				if (CollisionBC(pj->pos, bomb_pos, pj->size, expsize))
				{
					bomb[i].HitITbomb(j);
				}
			}
		}
	}

	// ===== ブーメラン =====
	for (int i = 0; i < ITEM_MAX; ++i)
	{
		if (!boom[i].IsUsedITboom())
			continue;

		const XMFLOAT3 boom_pos = boom[i].GetPositionITboom();
		const BOOL picked = boom[i].IsPickedITboom();
		const BOOL threw = boom[i].IsThrewITboom();

		// 落ちている → 取得判定
		if (!picked && !threw)
		{
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;

				if (CollisionBC(pj->pos, boom_pos, pj->size, BOOM_SIZE))
				{
					boom[i].PickITboom(j);
				}
			}
		}
		// 飛行中 → ヒット判定
		else if (threw)
		{
			for (int j = 0; j < MAX_PLAYER; ++j)
			{
				PLAYER* pj = GetPlayer(j);
				if (!pj || !pj->use) continue;
				if (pj->invisible) continue;
				if (CollisionBC(pj->pos, boom_pos, pj->size, BOOM_SIZE))
				{
					boom[i].HitITboom(j);
				}
			}
		}
	}

	// ===== ハンマー =====
	for (int i = 0; i < ITEM_MAX; ++i)
	{
		if (!hamr[i].IsUsedITHamr())
			continue;

		const BOOL picked = hamr[i].IsPickedITHamr();
		// 装備中はハンマーの「頭」のワールド位置で攻撃判定。未取得時は本体位置でPick。
		const XMFLOAT3 test_pos = picked ? hamr[i].GetHeadWorldPosition()
			: hamr[i].GetPositionITHamr();

		for (int j = 0; j < MAX_PLAYER; ++j)
		{
			PLAYER* pj = GetPlayer(j);
			if (!pj || !pj->use) continue;
			if (pj->invisible) continue;
			if (CollisionBC(pj->pos, test_pos, pj->size, HAMR_SIZE))
			{
				if (!picked) hamr[i].PickITHamr(j);
				else         hamr[i].HitITHamr(j);
			}
		}

#if defined(_DEBUG)
		// デバッグ可視化（不要なら削除OK）
		DrawDebugSphereOutline(test_pos, HAMR_SIZE, XMFLOAT4(1, 0.5f, 0, 1));
		DebugLine_Render(GetCameraViewProjMatrix());
#endif
	}
}

//=============================================================================
// アイテムのランダム生成処理
//=============================================================================
void ItemSpown(void)
{
	//int itemNo = GetRand(0, ITEM_TOTAL_MAX - 1);//本来
	int itemNo = GetRand(0, 5);//仮置き
	XMFLOAT3 SpawnPos = XMFLOAT3(GetRandf(-500.0f, 500.0f), 0.0f, GetRandf(-500.0f, 500.0f));

	switch (itemNo)
	{
	case (-1):
		break;

	case (0):
		for (int i = 0; i < 10; i++)
		{
			if (!giant[i].IsUsedITgiant())
			{
				giant[i].SetITgiant(SpawnPos);
				break;
			}
		}
		break;

	case (1):
		for (int i = 0; i < 1; i++)
		{
			if (!invisible[i].IsUsedITinvisible())
			{
				invisible[i].SetITinvisible(SpawnPos);
				break;
			}
		}
		break;

	case (2):
		for (int i = 0; i < 10; i++)
		{
			if (!ball[i].IsUsedITball())
			{
				ball[i].SetITballObject(SpawnPos);
				break;
			}
		}
		break;

	case (3):
		for (int i = 0; i < 10; i++)
		{
			if (!bomb[i].IsUsedITbomb())
			{
				bomb[i].SetITbombObject(SpawnPos);
				break;
			}
		}
		break;

	case (4): // ブーメラン
		for (int i = 0; i < ITEM_MAX; i++)
		{
			if (!boom[i].IsUsedITboom())
			{
				boom[i].SetITboomObject(SpawnPos, -1); // -1 = 無所属でスポーン
				break;
			}
		}
		break;
	case (5): // ブーメラン
		for (int i = 0; i < ITEM_MAX; i++)
		{
			if (!hamr[i].IsUsedITHamr())
			{
				hamr[i].SetITHamrObject(SpawnPos); 
				break;
			}
		}
		break;
	}
}

//// 球のワイヤーフレームを描画する関数
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices)
//{
//	// XY平面の円を描画
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y + sinf(theta1) * radius, center.z);
//		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y + sinf(theta2) * radius, center.z);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//
//	// XZ平面の円を描画
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x + cosf(theta1) * radius, center.y, center.z + sinf(theta1) * radius);
//		XMFLOAT3 p2(center.x + cosf(theta2) * radius, center.y, center.z + sinf(theta2) * radius);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//
//	// YZ平面の円を描画
//	for (int i = 0; i < slices; ++i) {
//		float theta1 = XM_2PI * i / slices;
//		float theta2 = XM_2PI * (i + 1) / slices;
//		XMFLOAT3 p1(center.x, center.y + cosf(theta1) * radius, center.z + sinf(theta1) * radius);
//		XMFLOAT3 p2(center.x, center.y + cosf(theta2) * radius, center.z + sinf(theta2) * radius);
//		DebugLine_DrawLine(p1, p2, color);
//	}
//}

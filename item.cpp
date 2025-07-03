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

//デバッグ表示
#include "debugline.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ITEM_MAX	(10)


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices = 20);



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF


//アイテム用
//アイテム用
GIANT giant[ITEM_MAX];

INVISIBLE invisible[1];

BALL ball[ITEM_MAX];
BALL* GetBall()  // アクセス用の関数を作成
{
	return &ball[0];
}

BOOM boom;  // ブーメランのインスタンスを作成
BOOM* GetBoomerang()  // アクセス用の関数を作成
{
	return &boom;  // ボールの中にあるブーメランを返す
}

HAMR hamr;
HAMR* GetHammer()  // アクセス用の関数を作成
{
	return &hamr;  // ハンマーのインスタンスを返す
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitItem(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//巨大化アイテムの初期化
		giant[i].InitITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		//透明化アイテムの初期化
		invisible[i].InitITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//ボールアイテムの初期化
		ball[i].InitITball();
	}

	// ブーメランの初期化
	boom.InitITboom();
	boom.SetITboomObject(XMFLOAT3(-150.0f, 0.0f, -150.0f), 0);

	// ハンマーの初期化
	hamr.InitITHamr();
	hamr.SetITHamrObject(XMFLOAT3(100.0f, 0.0f, -100.0f));
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

	// ブーメランの終了処理
	boom.UninitITboom();

	// ハンマーの終了処理
	hamr.UninitITHamr();

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

	if (GetKeyboardTrigger(DIK_6))
	{
		for (int i = 0; i < 10; i++)
		{
			if (!giant[i].IsUsedITgiant())
			{
				giant[i].SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));
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
				invisible[i].SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));
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
				ball[i].SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));
				break;
			}
		}
	}

#endif

	if (g_bPause == FALSE)
		return;

	// 当たり判定処理
	CheckHitItem();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 巨大化アイテムの更新処理
		giant[i].UpdateITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		// 透明化アイテムの更新処理
		invisible[i].UpdateITinvisible();
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ボールアイテムの更新処理
		ball[i].UpdateITball();
	}

	// ブーメランの更新処理
	boom.UpdateITboom();

	// ハンマーの更新処理
	hamr.UpdateITHamr();




}

//=============================================================================
// 描画処理
//=============================================================================
void DrawItem(void)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		// ボールアイテムの描画処理
		ball[i].DrawITball();
	}

	// ブーメランの描画処理
	boom.DrawITboom();

	// ハンマーの描画処理
	hamr.DrawITHamr();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 巨大化アイテムの描画処理
		giant[i].DrawITgiant();
	}

	for (int i = 0; i < 1; i++)
	{
		// 透明化アイテムの描画処理
		invisible[i].DrawITinvisible();
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
// 当たり判定処理
//=============================================================================
void CheckHitItem(void)
{
	PLAYER* player = GetPlayer(); // プレイヤー1 (index=0)

	for (int i = 0; i < ITEM_MAX; i++)
	{
		// 巨大化アイテム
		if (giant[i].IsUsedITgiant() && !giant[i].IsPickedITgiant())
		{
			XMFLOAT3 gi_pos = giant[i].GetPositionITgiant();
			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, gi_pos, player[j].size, GIANT_SIZE))
				{
					giant[i].PickITgiant(j);
				}
			}
		}
	}

	for (int i = 0; i < 1
		; i++)
	{
		//透明化
		if (invisible[i].IsUsedITinvisible() && !invisible[i].IsPickedITinvisible())
		{
			XMFLOAT3 invi_pos = invisible[i].GetPositionITinvisible();

			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, invi_pos, player[j].size, INVISIBLE_SIZE))
				{
					invisible[i].PickITinvisible(j);
				}
			}
		}
	}

	for (int i = 0; i < ITEM_MAX; i++)
	{
		//ボール
		if (bool use = ball[i].IsUsedITball())
		{
			XMFLOAT3 ball_pos = ball[i].GetPositionITball();
			BOOL pick = ball[i].IsPickedITball();
			BOOL to_throw = ball[i].IsThrewITball();

			if (!pick && !to_throw)
			{
				for (int j = 0; j < MAX_PLAYER; j++)
				{
					if (CollisionBC(player[j].pos, ball_pos, player[j].size, BALL_SIZE) && !player[j].haveWeapon)
					{
						ball[i].PickITball(j);
					}
				}
			}
			else if (to_throw && !pick)
			{
				for (int j = 0; j < MAX_PLAYER; j++)
				{
					if (CollisionBC(player[j].pos, ball_pos, player[j].size, BALL_SIZE))
					{
						ball[i].HitITball(j);
					}
				}
			}
		}
	}

	// ブーメラン
	if (bool use = boom.IsUsedITboom())
	{
		XMFLOAT3 boom_pos = boom.GetPositionITboom();
		BOOL pick = boom.IsPickedITboom();
		BOOL to_throw = boom.IsThrewITboom();
		if (!pick && !to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, boom_pos, player[i].size, BOOM_SIZE))
				{
					boom.PickITboom(i);
				}
			}
		}
		else if (to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, boom_pos, player[i].size, BOOM_SIZE))
				{
					boom.HitITboom(i);
				}
			}
		}
	}

	// ハンマー
	if (bool use = hamr.IsUsedITHamr())
	{
		BOOL pick = hamr.IsPickedITHamr();
		XMFLOAT3 test_pos;

		if (pick)
		{
			test_pos = hamr.GetHeadWorldPosition();  // <-- This MUST be here
		}
		else
		{
			test_pos = hamr.GetPositionITHamr();
		}


		for (int i = 0; i < MAX_PLAYER; i++)
		{

			if (CollisionBC(player[i].pos, test_pos, player[i].size, HAMR_SIZE))
			{
				if (!hamr.IsPickedITHamr())
				{
					hamr.PickITHamr(i);
				}
				else
				{
					// Hammer is being held: hit, but do NOT give it to the player hit
					hamr.HitITHamr(i);
				}
			}
		}



		// This will draw the sphere at the swinging head position
		DrawDebugSphereOutline(test_pos, HAMR_SIZE, XMFLOAT4(1, 0.5f, 0, 1));

		DebugLine_Render(GetCameraViewProjMatrix());

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

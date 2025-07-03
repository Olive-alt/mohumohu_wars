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
//ステージギミック用
//アイテム用


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
GIANT* giant[10] = { nullptr };
int giantMax = 10;
int giantCnt = 0;

INVISIBLE invisible;
BALL ball;
BALL* GetBall()  // アクセス用の関数を作成
{
	return &ball;
}
BOOM boom;  // ブーメランのインスタンスを作成
BOOM* GetBoomerang()  // アクセス用の関数を作成
{
	return &boom;  // ボールの中にあるブーメランを返す
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitItem(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		//巨大化アイテムの初期化
		giant[i]->InitITgiant();
	}



	//透明化アイテムの初期化
	invisible.InitITinvisible();
	invisible.SetITinvisible(XMFLOAT3(200.0f, 0.0f, 100.0f));

	//ボールアイテムの初期化
	ball.InitITball();
	ball.SetITballObject(XMFLOAT3(-100.0f, 0.0f, -100.0f));

	boom.InitITboom();
	boom.SetITboomObject(XMFLOAT3(-150.0f, 0.0f, -150.0f), 0);

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitItem(void)
{
	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;
		// 巨大化アイテムの終了処理
		giant[i]->UninitITgiant();
		giant[i] = nullptr;
	}

	// 透明化アイテムの終了処理
	invisible.UninitITinvisible();

	//ボールアイテムの終了処理
	ball.UninitITball();

	// ブーメランの終了処理
	boom.UninitITboom();

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

	if (GetKeyboardTrigger(DIK_K))
	{
		for (int i = 0; i < 10; i++)
		{
			if (giant[i]) continue;

			giant[i] = new GIANT;
			giant[i]->InitITgiant();
			giant[i]->SetITgiant(XMFLOAT3(100.0f, 0.0f, 100.0f));

			if (giant[i])break;
		}

	}

#endif

	if (g_bPause == FALSE)
		return;

	// 当たり判定処理
	CheckHitItem();

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// 巨大化アイテムの更新処理
		giant[i]->UpdateITgiant();
	}


	// 透明化アイテムの更新処理
	invisible.UpdateITinvisible();

	// ボールアイテムの更新処理
	ball.UpdateITball();

	// ブーメランの更新処理
	boom.UpdateITboom();


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawItem(void)
{
	// ボールアイテムの描画処理
	ball.DrawITball();

	// ブーメランの描画処理
	boom.DrawITboom();

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// 巨大化アイテムの描画処理
		giant[i]->DrawITgiant();
	}
	// 透明化アイテムの描画処理
	invisible.DrawITinvisible();

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

	for (int i = 0; i < 10; i++)
	{
		if (!giant[i]) continue;

		// 巨大化アイテム
		if (bool use = giant[i]->IsUsedITgiant())
		{
			XMFLOAT3 gi_pos = giant[i]->GetPositionITgiant();
			for (int j = 0; j < MAX_PLAYER; j++)
			{
				if (CollisionBC(player[j].pos, gi_pos, player[j].size, GIANT_SIZE))
				{
					giant[i]->PickITgiant(j);
				}
			}
		}
	}


	//透明化
	if (bool use = invisible.IsUsedITinvisible())
	{
		XMFLOAT3 invi_pos = invisible.GetPositionITinvisible();

		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (CollisionBC(player[i].pos, invi_pos, player[i].size, INVISIBLE_SIZE))
			{
				invisible.PickITinvisible(i);
			}
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
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, ball_pos, player[i].size, BALL_SIZE))
				{
					ball.PickITball(i);
				}
			}
		}
		else if (to_throw)
		{
			for (int i = 0; i < MAX_PLAYER; i++)
			{
				if (CollisionBC(player[i].pos, ball_pos, player[i].size, BALL_SIZE))
				{
					ball.HitITball(i);
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

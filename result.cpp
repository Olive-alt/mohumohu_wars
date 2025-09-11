//=============================================================================
//
// ゲーム画面処理 [result.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
#include "fade.h"
#include "game.h"

#include "player.h"
#include "meshfield.h"
#include "meshwall.h"
#include "shadow.h"
#include "score.h"
#include "time.h"

//#include "particle.h"
#include "collision.h"
#include "debugproc.h"

#include "result.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************



//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int	g_ViewPortType_Game = TYPE_FULL_SCREEN;

static BOOL	g_bPause = TRUE;	// ポーズON/OFF


//=============================================================================
// 初期化処理
//=============================================================================
// プレイヤーをスコアの降順で並べ替えて、表彰台の位置に配置する関数
// 同点の場合はプレイヤー番号（インデックス）が小さい方を上位とする（0 > 1 > 2 > 3）
static void PlacePlayersOnPodium(const XMFLOAT3& posRank1,
	const XMFLOAT3& posRank2,
	const XMFLOAT3& posRank3,
	const XMFLOAT3& posRank4)
{
	// 対象プレイヤーの取得（最大4人想定）
	PLAYER* players[4] = { GetPlayer(0), GetPlayer(1), GetPlayer(2), GetPlayer(3) };
	// 現在のスコアを取得
	int scores[4] = { GetScore(0),  GetScore(1),  GetScore(2),  GetScore(3) };

	// 並べ替え用の一時配列
	struct Entry { PLAYER* p; int score; int idx; };
	Entry list[4];
	int count = 0;

	// 有効なプレイヤーのみをリストに詰める
	for (int i = 0; i < 4; ++i)
	{
		if (players[i]) {
			list[count].p = players[i];
			list[count].score = scores[i];
			list[count].idx = i;           // タイブレーク用の元インデックス
			++count;
		}
	}

	// 選択ソート（スコア降順、同点はインデックス昇順）で整列
	for (int i = 0; i < count - 1; ++i)
	{
		int best = i;
		for (int j = i + 1; j < count; ++j)
		{
			// より高スコア、または同点でインデックスが小さい方を優先
			if (list[j].score > list[best].score ||
				(list[j].score == list[best].score && list[j].idx < list[best].idx))
			{
				best = j;
			}
		}
		// 見つかった最良要素を先頭側にスワップ
		if (best != i) {
			Entry tmp = list[i];
			list[i] = list[best];
			list[best] = tmp;
		}
	}

	// 表彰台の配置（1位→2位→3位→4位）
	const XMFLOAT3 podiumPos[4] = { posRank1, posRank2, posRank3, posRank4 };

	// 有効人数分だけ位置を適用（最大4人）
	for (int i = 0; i < count && i < 4; ++i)
	{
		list[i].p->pos = podiumPos[i];
	}
}


HRESULT InitResult(void)
{
	g_ViewPortType_Game = TYPE_FULL_SCREEN;

	// 順位に応じた座標
	XMFLOAT3 posRank1 = XMFLOAT3(20.0f, 75.0f, 0.0f);  // 1位の位置
	XMFLOAT3 posRank2 = XMFLOAT3(-15.0f, 60.0f, 0.0f);  // 2位の位置
	XMFLOAT3 posRank3 = XMFLOAT3(55.0f, 45.0f, 0.0f);  // 3位の位置
	XMFLOAT3 posRank4 = XMFLOAT3(-50.0f, 30.0f, 0.0f);  // 4位の位置

	// プレイヤー取得
	PLAYER* p1 = GetPlayer(0);
	PLAYER* p2 = GetPlayer(1);
	PLAYER* p3 = GetPlayer(2);
	PLAYER* p4 = GetPlayer(3);

	// デバッグ用
#ifdef DEBUG
	AddScore(1, 10);
#endif

	// スコア取得
	int score1 = GetScore(0);
	int score2 = GetScore(1);
	int score3 = GetScore(2);
	int score4 = GetScore(3);

	// フィールドの初期化
	InitMeshField(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 100, 100, 13.0f, 13.0f);

	// ライトを有効化	// 影の初期化処理
	//InitShadow();


	// プレイヤーの初期化
	InitPlayer();

	// スコアに基づき順位付けして配置
	PlacePlayersOnPodium(posRank1, posRank2, posRank3, posRank4);


	if (p1 && p2 && p3 && p4)
	{
		if (score1 >= score2)
		{
			// 1Pが1位、2Pが2位、3Pが3位、4Pが4位
			p1->pos = posRank1;
			p2->pos = posRank2;
			p3->pos = posRank3;
			p4->pos = posRank4;
		}
		else
		{
			// 2Pが1位
			p1->pos = posRank2;
			p2->pos = posRank1;
		}
	}


	// 壁の初期化
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 16, 2, 80.0f, 80.0f);

	// 壁(裏側用の半透明)
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_TOP), XMFLOAT3(0.0f, XM_PI, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_LEFT, 0.0f, 0.0f), XMFLOAT3(0.0f, XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(MAP_RIGHT, 0.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI * 0.50f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);
	InitMeshWall(XMFLOAT3(0.0f, 0.0f, MAP_DOWN), XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f), 16, 2, 80.0f, 80.0f);

	// 木を生やす
	//InitTree();

	// スコアの初期化（UI等のために必要なら呼ぶ）しつつ値は元に戻す
	InitScore();
	AddScore(0, score1);
	AddScore(1, score2);
	AddScore(2, score3);
	AddScore(3, score4);

	// パーティクルの初期化
	//InitParticle();

	// BGM再生
	//PlaySound(SOUND_LABEL_BGM_sample001);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void UninitResult(void)
{
	// パーティクルの終了処理
	//UninitParticle();

	// スコアの終了処理
	UninitScore();

	// 壁の終了処理
	UninitMeshWall();

	// 地面の終了処理
	UninitMeshField();

	// プレイヤーの終了処理
	UninitPlayer();

	// 影の終了処理
	UninitShadow();

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateResult(void)
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

	if (g_bPause == FALSE)
		return;

#endif

	if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A))
	{// Enter押したら、ステージを切り替える
		SetFade(FADE_OUT, MODE_TITLE);
	}

	// 地面処理の更新
	UpdateMeshField();

	// プレイヤーの更新処理
	UpdatePlayer();

	// エネミーの更新処理
	//UpdateEnemy();

	// 壁処理の更新
	UpdateMeshWall();

	// パーティクルの更新処理
	//UpdateParticle();

	// 影の更新処理
	//UpdateShadow();

	// 当たり判定処理
	//CheckHit();

	// スコアの更新処理
	UpdateScore();

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawResult0(void)
{
	// 3Dの物を描画する処理
	// 地面の描画処理
	DrawMeshField();

	// 影の描画処理
	DrawShadow();

	// プレイヤーの描画処理
	DrawPlayer();

	// 弾の描画処理

	// 壁の描画処理
	DrawMeshWall();

	// 木の描画処理

	// パーティクルの描画処理
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


void DrawResult(void)
{
	XMFLOAT3 pos;


#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("ViewPortType:%d\n", g_ViewPortType_Game);

#endif

	// プレイヤー視点
	pos = GetPlayer()->pos;
	pos.y = 0.0f;			// カメラ酔いを防ぐためにクリアしている
	SetCameraAT(pos);
	SetCamera();

	switch(g_ViewPortType_Game)
	{
	case TYPE_FULL_SCREEN:
		SetViewPort(TYPE_FULL_SCREEN);
		DrawResult0();
		break;

	case TYPE_LEFT_HALF_SCREEN:
	case TYPE_RIGHT_HALF_SCREEN:
		SetViewPort(TYPE_LEFT_HALF_SCREEN);
		DrawResult0();

		// エネミー視点
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_RIGHT_HALF_SCREEN);
		DrawResult();
		break;

	case TYPE_UP_HALF_SCREEN:
	case TYPE_DOWN_HALF_SCREEN:
		SetViewPort(TYPE_UP_HALF_SCREEN);
		DrawResult();

		// エネミー視点
		pos.y = 0.0f;
		SetCameraAT(pos);
		SetCamera();
		SetViewPort(TYPE_DOWN_HALF_SCREEN);
		DrawResult();
		break;

	}
}
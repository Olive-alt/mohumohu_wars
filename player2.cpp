//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "player2.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER2		"data/MODEL/cone.obj"			// 読み込むモデル名
#define	MODEL_PLAYER2_LEFT	"data/MODEL/torus.obj"			// 読み込むモデル名
#define	MODEL_PLAYER2_RIGHT	"data/MODEL/torus.obj"			// 読み込むモデル名

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER2_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER2_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define PLAYER2_PARTS_MAX	(2)								// プレイヤーのパーツの数

//test
//test2
// 
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER2		g_Player2;						// プレイヤー

static PLAYER2		g_Parts[PLAYER2_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;





// プレイヤーの階層アニメーションデータ


// プレイヤーの頭を左右に動かしているアニメデータ
static INTERPOLATION_DATA move_tbl_left[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};


static INTERPOLATION_DATA move_tbl_right[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, 0.0f),   XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },

};


static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	move_tbl_left,
	move_tbl_right,

};






//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer2(void)
{
	g_Player2.load = TRUE;
	LoadModel(MODEL_PLAYER2, &g_Player2.model);

	g_Player2.pos = XMFLOAT3(-10.0f, PLAYER2_OFFSET_Y + 50.0f, -50.0f);
	g_Player2.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player2.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player2.spd = 0.0f;			// 移動スピードクリア

	g_Player2.use = TRUE;			// TRUE:生きてる
	g_Player2.size = PLAYER2_SIZE;	// 当たり判定の大きさ

	g_Player2.gateUse = FALSE;
	g_Player2.gateCoolTime = 0;

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Player2.pos;
	pos.y -= (PLAYER2_OFFSET_Y - 0.1f);
	g_Player2.shadowIdx = CreateShadow(pos, PLAYER2_SHADOW_SIZE, PLAYER2_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号

	// キーを押した時のプレイヤーの向き
	roty = 0.0f;

	g_Player2.parent = NULL;			// 本体（親）なのでNULLを入れる


	// 階層アニメーションの初期化
	for (int i = 0; i < PLAYER2_PARTS_MAX; i++)
	{
		g_Parts[i].use = FALSE;

		// 位置・回転・スケールの初期設定
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// 親子関係
		g_Parts[i].parent = &g_Player2;		// ← ここに親のアドレスを入れる
		//	g_Parts[腕].parent= &g_Player2;		// 腕だったら親は本体（プレイヤー）
		//	g_Parts[手].parent= &g_Paerts[腕];	// 指が腕の子供だった場合の例

			// 階層アニメーション用のメンバー変数の初期化
		g_Parts[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Parts[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Parts[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット

		// パーツの読み込みはまだしていない
		g_Parts[i].load = FALSE;
	}

	g_Parts[0].use = TRUE;
	g_Parts[0].parent = &g_Player2;	// 親をセット
	g_Parts[0].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[0].tblMax = sizeof(move_tbl_left) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[0].load = TRUE;
	LoadModel(MODEL_PLAYER2_LEFT, &g_Parts[0].model);

	g_Parts[1].use = TRUE;
	g_Parts[1].parent = &g_Player2;	// 親をセット
	g_Parts[1].tblNo = 1;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[1].tblMax = sizeof(move_tbl_right) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[1].load = TRUE;
	LoadModel(MODEL_PLAYER2_RIGHT, &g_Parts[1].model);



	// クォータニオンの初期化
	XMStoreFloat4(&g_Player2.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer2(void)
{
	// モデルの解放処理
	if (g_Player2.load == TRUE)
	{
		UnloadModel(&g_Player2.model);
		g_Player2.load = FALSE;
	}

	// パーツの解放処理
	for (int i = 0; i < PLAYER2_PARTS_MAX; i++)
	{
		if (g_Parts[i].load == TRUE)
		{
			// パーツの解放処理
			UnloadModel(&g_Parts[i].model);
			g_Parts[i].load = FALSE;
		}
	}



}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer2(void)
{
	CAMERA* cam = GetCamera();

	g_Player2.spd *= 0.7f;

	// 移動処理
	if (GetKeyboardPress(DIK_LEFT))
	{
		g_Player2.spd = VALUE_MOVE;
		//g_Player2.pos.x -= g_Player2.spd;
		roty = XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{
		g_Player2.spd = VALUE_MOVE;
		//g_Player2.pos.x += g_Player2.spd;
		roty = -XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_UP))
	{
		g_Player2.spd = VALUE_MOVE;
		//g_Player2.pos.z += g_Player2.spd;
		roty = XM_PI;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{
		g_Player2.spd = VALUE_MOVE;
		//g_Player2.pos.z -= g_Player2.spd;
		roty = 0.0f;
	}

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player2.pos.z = g_Player2.pos.x = 0.0f;
		g_Player2.spd = 0.0f;
		roty = 0.0f;
	}
#endif

	//ワープゲート用処理
	if (g_Player2.gateUse)
	{
		g_Player2.gateCoolTime += 1;

		if (g_Player2.gateCoolTime >= 300)
		{
			g_Player2.gateCoolTime = 0;
			g_Player2.gateUse = FALSE;
		}
	}

	{	// 押した方向にプレイヤーを移動させる
		// 押した方向にプレイヤーを向かせている所
		g_Player2.rot.y = roty + cam->rot.y;

		g_Player2.pos.x -= sinf(g_Player2.rot.y) * g_Player2.spd;
		g_Player2.pos.z -= cosf(g_Player2.rot.y) * g_Player2.spd;
	}


	// レイキャストして足元の高さを求める
	XMFLOAT3 HitPosition;		// 交点
	XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
	BOOL ans = RayHitField(g_Player2.pos, &HitPosition, &Normal);
	if (ans)
	{	// 当たっていたから波の上に乗せている
		g_Player2.pos.y = HitPosition.y + PLAYER2_OFFSET_Y;
	}
	else
	{
		g_Player2.pos.y = PLAYER2_OFFSET_Y;
		Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}


	// 弾発射処理
	if (GetKeyboardTrigger(DIK_RETURN))
	{
		SetBullet(g_Player2.pos, g_Player2.rot);
	}


	// 影もプレイヤーの位置に合わせる
	XMFLOAT3 pos = g_Player2.pos;
	pos.y -= (PLAYER2_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player2.shadowIdx, pos);


	// 階層アニメーション
	for (int i = 0; i < PLAYER2_PARTS_MAX; i++)
	{
		// 使われているなら処理する
		if ((g_Parts[i].use == TRUE) && (g_Parts[i].tblMax > 0))
		{	// 線形補間の処理
			int nowNo = (int)g_Parts[i].time;			// 整数分であるテーブル番号を取り出している
			int maxNo = g_Parts[i].tblMax;				// 登録テーブル数を数えている
			int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
			INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i].tblNo];	// 行動テーブルのアドレスを取得

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

			float nowTime = g_Parts[i].time - nowNo;	// 時間部分である少数を取り出している

			Pos *= nowTime;								// 現在の移動量を計算している
			Rot *= nowTime;								// 現在の回転量を計算している
			Scl *= nowTime;								// 現在の拡大率を計算している

			// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
			XMStoreFloat3(&g_Parts[i].pos, nowPos + Pos);

			// 計算して求めた回転量を現在の移動テーブルに足している
			XMStoreFloat3(&g_Parts[i].rot, nowRot + Rot);

			// 計算して求めた拡大率を現在の移動テーブルに足している
			XMStoreFloat3(&g_Parts[i].scl, nowScl + Scl);

			// frameを使て時間経過処理をする
			g_Parts[i].time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
			if ((int)g_Parts[i].time >= maxNo)			// 登録テーブル最後まで移動したか？
			{
				g_Parts[i].time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
			}

		}

	}


	// ポイントライトのテスト
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Player2.pos;
		pos.y += 20.0f;

		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(1, light);
	}



	//////////////////////////////////////////////////////////////////////
	// 姿勢制御
	//////////////////////////////////////////////////////////////////////

	XMVECTOR vx, nvx, up;
	XMVECTOR quat;
	float len, angle;


	g_Player2.UpVector = Normal;		// プレイヤーを傾ける法線ベクトル
	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	vx = XMVector3Cross(up, XMLoadFloat3(&g_Player2.UpVector));

	nvx = XMVector3Length(vx);
	XMStoreFloat(&len, nvx);
	nvx = XMVector3Normalize(vx);
	//nvx = vx / len;
	angle = asinf(len);

	//quat = XMQuaternionIdentity();

//	quat = XMQuaternionRotationAxis(nvx, angle);
	quat = XMQuaternionRotationNormal(nvx, angle);


	quat = XMQuaternionSlerp(XMLoadFloat4(&g_Player2.Quaternion), quat, 0.05f);
	XMStoreFloat4(&g_Player2.Quaternion, quat);




#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("Player2 X:%f Y:%f Z:% N:%f\n", g_Player2.pos.x, g_Player2.pos.y, g_Player2.pos.z, Normal.y);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer2(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player2.scl.x, g_Player2.scl.y, g_Player2.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player2.rot.x, g_Player2.rot.y + XM_PI, g_Player2.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player2.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player2.pos.x, g_Player2.pos.y, g_Player2.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player2.mtxWorld, mtxWorld);


	// 縁取りの設定
	SetEdge(1);

	// モデル描画
	DrawModel(&g_Player2.model);



	// 階層アニメーション
	for (int i = 0; i < PLAYER2_PARTS_MAX; i++)
	{
		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		if (g_Parts[i].parent != NULL)	// 子供だったら親と結合する
		{
			mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			// ↑
			// g_Player2.mtxWorldを指している
		}

		XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

		// 使われているなら処理する
		if (g_Parts[i].use == FALSE) continue;

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);


		// モデル描画
		DrawModel(&g_Parts[i].model);

	}

	SetEdge(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER2* GetPlayer2(void)
{
	return &g_Player2;
}


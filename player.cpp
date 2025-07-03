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
#include "player.h"
#include "shadow.h"
#include "debugproc.h"
#include "meshfield.h"
#include "item.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/cone.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_LEFT	"data/MODEL/torus.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT	"data/MODEL/torus.obj"			// 読み込むモデル名

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(2)								// プレイヤーのパーツの数



// 
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player[MAX_PLAYER];						// プレイヤー

static PLAYER		g_Parts[MAX_PLAYER][PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;

//HP初期化
static ID3D11Buffer* g_PlayerHpBarVertexBuffer = NULL; // Bar vertex buffer

//HP初期化
void InitPlayerHpBarVertexBuffer() {
	struct VERTEX_3D {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT4 Diffuse;
		XMFLOAT2 TexCoord;
	};
	VERTEX_3D vtx[4];
	// Positions (x, y, z), centered at origin
	vtx[0].Position = XMFLOAT3(-0.5f, +0.5f, 0.0f); vtx[0].TexCoord = XMFLOAT2(0, 0);
	vtx[1].Position = XMFLOAT3(+0.5f, +0.5f, 0.0f); vtx[1].TexCoord = XMFLOAT2(1, 0);
	vtx[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f); vtx[2].TexCoord = XMFLOAT2(0, 1);
	vtx[3].Position = XMFLOAT3(+0.5f, -0.5f, 0.0f); vtx[3].TexCoord = XMFLOAT2(1, 1);
	// Normal and diffuse not used for 2D bar, but fill with dummy values
	for (int i = 0; i < 4; i++) {
		vtx[i].Normal = XMFLOAT3(0, 0, -1);
		vtx[i].Diffuse = XMFLOAT4(1, 1, 1, 1);
	}
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vtx);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA init = { vtx, 0, 0 };
	GetDevice()->CreateBuffer(&bd, &init, &g_PlayerHpBarVertexBuffer);
}


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
HRESULT InitPlayer(void)
{
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		g_Player[i].load = TRUE;
		LoadModel(MODEL_PLAYER, &g_Player[i].model);

		g_Player[i].pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y + 50.0f, -50.0f);
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Player[i].spd = 0.0f;			// 移動スピードクリア
		g_Player[i].hp = 3.0f;
		g_Player[i].use = TRUE;			// TRUE:生きてる
		g_Player[i].size = PLAYER_SIZE;	// 当たり判定の大きさ


		// カプセルの端点を初期化
		g_Player[i].capsuleA.x = g_Player[i].pos.x;
		g_Player[i].capsuleA.y = g_Player[i].pos.y - PLAYER_HEIGHT / 2.0f;
		g_Player[i].capsuleA.z = g_Player[i].pos.z;

		g_Player[i].capsuleB.x = g_Player[i].pos.x;
		g_Player[i].capsuleB.y = g_Player[i].pos.y + PLAYER_HEIGHT / 2.0f;
		g_Player[i].capsuleB.z = g_Player[i].pos.z;


		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Player[i].model, &g_Player[i].diffuse[0]);

		g_Player[i].gateUse = FALSE;
		g_Player[i].gateCoolTime = 0;

		// ここでプレイヤー用の影を作成している
		XMFLOAT3 pos = g_Player[i].pos;
		pos.y -= (PLAYER_OFFSET_Y - 0.1f);
		g_Player[i].shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
		//          ↑
		//        このメンバー変数が生成した影のIndex番号

		// キーを押した時のプレイヤーの向き
		roty = 0.0f;

		g_Player[i].parent = NULL;			// 本体（親）なのでNULLを入れる
	}


	// 階層アニメーションの初期化
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		for (int j = 0; j < PLAYER_PARTS_MAX; j++)
		{
			g_Parts[i][j].use = FALSE;

			// 位置・回転・スケールの初期設定
			g_Parts[i][j].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][j].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][j].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

			// 親子関係
			g_Parts[i][j].parent = &g_Player[i];		// ← ここに親のアドレスを入れる
			//	g_Parts[腕].parent= &g_Player[i];		// 腕だったら親は本体（プレイヤー）
			//	g_Parts[手].parent= &g_Paerts[腕];	// 指が腕の子供だった場合の例

				// 階層アニメーション用のメンバー変数の初期化
			g_Parts[i][j].time = 0.0f;			// 線形補間用のタイマーをクリア
			g_Parts[i][j].tblNo = 0;			// 再生する行動データテーブルNoをセット
			g_Parts[i][j].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット

			// パーツの読み込みはまだしていない
			g_Parts[i][j].load = FALSE;
		}
	}

	for (int i = 0; i < MAX_PLAYER; i++)

	{
		g_Parts[i][0].use = TRUE;
		g_Parts[i][0].parent = &g_Player[i];	// 親をセット
		g_Parts[i][0].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][0].tblMax = sizeof(move_tbl_left) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
		g_Parts[i][0].load = TRUE;
		LoadModel(MODEL_PLAYER_LEFT, &g_Parts[i][0].model);

		g_Parts[i][1].use = TRUE;
		g_Parts[i][1].parent = &g_Player[i];	// 親をセット
		g_Parts[i][1].tblNo = 1;			// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i][1].tblMax = sizeof(move_tbl_right) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
		g_Parts[i][1].load = TRUE;
		LoadModel(MODEL_PLAYER_RIGHT, &g_Parts[i][1].model);




		// クォータニオンの初期化
		XMStoreFloat4(&g_Player[i].Quaternion, XMQuaternionIdentity());
	}

	// HPバー初期化
	InitPlayerHpBarVertexBuffer();

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		// モデルの解放処理
		if (g_Player[i].load == TRUE)
		{
			UnloadModel(&g_Player[i].model);
			g_Player[i].load = FALSE;
		}

		// パーツの解放処理
		for (int j = 0; j < PLAYER_PARTS_MAX; j++)
		{
			if (g_Parts[i][j].load == TRUE)
			{
				// パーツの解放処理
				UnloadModel(&g_Parts[i][j].model);
				g_Parts[i][j].load = FALSE;
			}
		}
	}



}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA* cam = GetCamera();

	// 1) キー入力による移動＆向き設定
	MovePlayers();

	// 2) 各プレイヤーの状態更新
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		// ▼ 速度減衰 ＋ HP テスト
		g_Player[i].spd *= 0.7f;
		if (GetKeyboardTrigger(DIK_L))
		{
			g_Player[i].hp = max(g_Player[i].hp - 1.0f, 0.0f);
		}
		if (GetKeyboardTrigger(DIK_O))
		{
			g_Player[i].hp = min(g_Player[i].hp + 1.0f, 3.0f);
		}

		// 弾発射
		if (GetKeyboardTrigger(DIK_SPACE))
		{
			BALL* ball = GetBall();
			if (ball && ball->IsUsedITball())  // すでに使われてるボールは撃たないようにする
			{
				ball->SetITball(g_Player[i].pos, g_Player[i].rot);  // プレイヤーの位置と向きでボールを発射
			}
		}


		// ▼ ワープゲート用クールタイム
		if (g_Player[i].gateUse)
		{
			if (++g_Player[i].gateCoolTime >= 300)
			{
				g_Player[i].gateCoolTime = 0;
				g_Player[i].gateUse = FALSE;
			}
		}

		// ▼ 地形との当たり判定で Y 座標調整
		XMFLOAT3 HitPos, Normal;
		if (RayHitField(g_Player[i].pos, &HitPos, &Normal))
		{
			g_Player[i].pos.y = HitPos.y + PLAYER_OFFSET_Y;
		}
		else
		{
			g_Player[i].pos.y = PLAYER_OFFSET_Y;
			Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		}

		// ▼ 影の位置も更新
		{
			XMFLOAT3 sh = g_Player[i].pos;
			sh.y -= (PLAYER_OFFSET_Y - 0.1f);
			SetPositionShadow(g_Player[i].shadowIdx, sh);
		}

		// ▼ 階層アニメーション（子パーツ）更新
		for (int j = 0; j < PLAYER_PARTS_MAX; j++)
		{
			if (g_Parts[i][j].use && g_Parts[i][j].tblMax > 0)
			{
				int nowNo = (int)g_Parts[i][j].time;
				int maxNo = g_Parts[i][j].tblMax;
				int nextNo = (nowNo + 1) % maxNo;
				INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i][j].tblNo];

				// キーフレーム読み込み
				XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
				XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
				XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);
				XMVECTOR nextPos = XMLoadFloat3(&tbl[nextNo].pos);
				XMVECTOR nextRot = XMLoadFloat3(&tbl[nextNo].rot);
				XMVECTOR nextScl = XMLoadFloat3(&tbl[nextNo].scl);

				// 差分と補間
				XMVECTOR deltaPos = nextPos - nowPos;
				XMVECTOR deltaRot = nextRot - nowRot;
				XMVECTOR deltaScl = nextScl - nowScl;
				float alpha = g_Parts[i][j].time - nowNo;

				XMStoreFloat3(&g_Parts[i][j].pos, nowPos + deltaPos * alpha);
				XMStoreFloat3(&g_Parts[i][j].rot, nowRot + deltaRot * alpha);
				XMStoreFloat3(&g_Parts[i][j].scl, nowScl + deltaScl * alpha);

				// 時間進行
				g_Parts[i][j].time += 1.0f / tbl[nowNo].frame;
				if ((int)g_Parts[i][j].time >= maxNo)
				{
					g_Parts[i][j].time -= maxNo;
				}
			}
		}

		// ▼ 地面法線による姿勢制御（クォータニオン補間）
		{
			XMVECTOR up = XMVectorSet(0, 1, 0, 0);
			XMVECTOR normV = XMLoadFloat3(&Normal);
			XMVECTOR axis = XMVector3Normalize(XMVector3Cross(up, normV));
			float   length = XMVectorGetX(XMVector3Length(XMVector3Cross(up, normV)));
			float   angle = asinf(length);
			XMVECTOR targetQuat = XMQuaternionRotationNormal(axis, angle);
			XMVECTOR currentQuat = XMLoadFloat4(&g_Player[i].Quaternion);
			XMVECTOR newQuat = XMQuaternionSlerp(currentQuat, targetQuat, 0.05f);
			XMStoreFloat4(&g_Player[i].Quaternion, newQuat);
		}

		g_Player[i].capsuleA.x = g_Player[i].pos.x;
		g_Player[i].capsuleA.y = g_Player[i].pos.y - PLAYER_HEIGHT / 2.0f;
		g_Player[i].capsuleA.z = g_Player[i].pos.z;

		g_Player[i].capsuleB.x = g_Player[i].pos.x;
		g_Player[i].capsuleB.y = g_Player[i].pos.y + PLAYER_HEIGHT / 2.0f;
		g_Player[i].capsuleB.z = g_Player[i].pos.z;


#ifdef _DEBUG
		PrintDebugProc(
			"Player[%d] Pos:(%f,%f,%f) HP:%f\n",
			i,
			g_Player[i].pos.x,
			g_Player[i].pos.y,
			g_Player[i].pos.z,
			g_Player[i].hp
		);
#endif
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (g_Player[i].use == FALSE) {
			continue;
		}

		// カリング無効
		SetCullingMode(CULL_MODE_NONE);

		// 各種行列を定義
		XMMATRIX mtxScl, mtxQuat, mtxRotYaw, mtxTranslate, mtxWorld;

		// 1) スケーリング
		mtxScl = XMMatrixScaling(
			g_Player[i].scl.x,
			g_Player[i].scl.y,
			g_Player[i].scl.z
		);

		// 2) 地形法線による傾き姿勢（クォータニオン）
		mtxQuat = XMMatrixRotationQuaternion(
			XMLoadFloat4(&g_Player[i].Quaternion)
		);

		// 3) キー入力で設定したヨー回転
		//    rot.y に設定した角度をそのまま反映
		mtxRotYaw = XMMatrixRotationRollPitchYaw(
			g_Player[i].rot.x,      // ピッチ（必要な場合）
			g_Player[i].rot.y + XM_PI,  // ヨー（+PI はモデル初期向き補正）
			g_Player[i].rot.z       // ロール（必要な場合）
		);

		// 4) 平行移動
		mtxTranslate = XMMatrixTranslation(
			g_Player[i].pos.x,
			g_Player[i].pos.y,
			g_Player[i].pos.z
		);

		// 合成順：Scale → Quaternion姿勢 → Yaw回転 → Translate
		mtxWorld = mtxScl;
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxQuat);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRotYaw);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		// 保存（デバッグや子パーツ用）
		XMStoreFloat4x4(&g_Player[i].mtxWorld, mtxWorld);

		// 縁取り
		SetEdge(1);

		// 本体モデルを描画
		DrawModel(&g_Player[i].model);

		// ───────────────────────────────
		// 階層アニメーション（子パーツ）描画
		// ───────────────────────────────
		for (int j = 0; j < PLAYER_PARTS_MAX; j++)
		{
			if (!g_Parts[i][j].use) continue;

			// 子パーツ用行列
			XMMATRIX mtxPart = XMMatrixIdentity();
			mtxPart = XMMatrixMultiply(mtxPart,
				XMMatrixScaling(
					g_Parts[i][j].scl.x,
					g_Parts[i][j].scl.y,
					g_Parts[i][j].scl.z
				));
			mtxPart = XMMatrixMultiply(mtxPart,
				XMMatrixRotationRollPitchYaw(
					g_Parts[i][j].rot.x,
					g_Parts[i][j].rot.y,
					g_Parts[i][j].rot.z
				));
			mtxPart = XMMatrixMultiply(mtxPart,
				XMMatrixTranslation(
					g_Parts[i][j].pos.x,
					g_Parts[i][j].pos.y,
					g_Parts[i][j].pos.z
				));

			// 親パーツ（または本体）のワールド行列と結合
			if (g_Parts[i][j].parent)
			{
				mtxPart = XMMatrixMultiply(
					mtxPart,
					XMLoadFloat4x4(&g_Parts[i][j].parent->mtxWorld)
				);
			}

			// 描画
			SetWorldMatrix(&mtxPart);
			DrawModel(&g_Parts[i][j].model);
		}
	}
	SetDepthEnable(FALSE);
	//HP BAR 参加
	SetLightEnable(FALSE);

	DrawPlayerHpBar(); // This stays the same

	SetLightEnable(TRUE);
	SetDepthEnable(TRUE);


	SetEdge(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER* GetPlayer(int index)
{
	if (index < 0 || index >= MAX_PLAYER) {
		return nullptr;
	}
	return &g_Player[index];
}

// 既存コード互換用：引数なしで呼ぶと自動的に index=0 を返す
PLAYER* GetPlayer(void)
{
	return GetPlayer(0);
}

void DrawPlayerHpBar() {
	if (GetMode() != MODE_RESULT)
	{


		// 頂点構造体の定義
		struct VERTEX_3D {
			XMFLOAT3 Position;
			XMFLOAT3 Normal;
			XMFLOAT4 Diffuse;
			XMFLOAT2 TexCoord;
		};

		CAMERA* cam = GetCamera();
		XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);


		// HPバーの幅と高さ、最大HP
		const float HP_WIDTH = 20.0f;
		const float HP_HEIGHT = 6.0f;
		const float maxHp = 3.0f;
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			float percent = g_Player[i].hp / maxHp;              // 現在HPの割合を計算

			percent = fmaxf(0.0f, fminf(1.0f, percent));      // 0～1にクランプ

			XMFLOAT3 hpBarPos = g_Player[i].pos;
			hpBarPos.y += 20.0f;  // プレイヤーの上に表示

			// ビルボード行列（カメラの方向を向くようにする）
			XMMATRIX mtxBillboard = XMMatrixIdentity();
			mtxBillboard.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0.0f);
			mtxBillboard.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0.0f);
			mtxBillboard.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0.0f);

			// マテリアル設定（テクスチャなしで頂点カラーのみ使用）
			MATERIAL mat = {};
			mat.Ambient = mat.Diffuse = XMFLOAT4(1, 1, 1, 1);
			mat.noTexSampling = 1;
			SetMaterial(mat);
			// テクスチャを解除（念のため）
			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

			// ワールド行列作成（位置・回転の適用）
			XMMATRIX mtxScale = XMMatrixIdentity();
			XMMATRIX mtxTrans = XMMatrixTranslation(hpBarPos.x, hpBarPos.y, hpBarPos.z);
			XMMATRIX mtxWorld = mtxScale * mtxBillboard * mtxTrans;
			SetWorldMatrix(&mtxWorld);

			UINT stride = sizeof(VERTEX_3D);
			UINT offset = 0;

			// 1. --- 赤色の背景バー（常に全幅） ---
			VERTEX_3D redVtx[4];
			float left = -HP_WIDTH / 2.0f;
			float right = +HP_WIDTH / 2.0f;

			redVtx[0].Position = XMFLOAT3(left, HP_HEIGHT / 2, 0);  redVtx[0].Diffuse = XMFLOAT4(1, 0, 0, 1); // 左上
			redVtx[1].Position = XMFLOAT3(right, HP_HEIGHT / 2, 0);  redVtx[1].Diffuse = XMFLOAT4(1, 0, 0, 1); // 右上
			redVtx[2].Position = XMFLOAT3(left, -HP_HEIGHT / 2, 0);  redVtx[2].Diffuse = XMFLOAT4(1, 0, 0, 1); // 左下
			redVtx[3].Position = XMFLOAT3(right, -HP_HEIGHT / 2, 0); redVtx[3].Diffuse = XMFLOAT4(1, 0, 0, 1); // 右下
			for (int i = 0; i < 4; i++) {
				redVtx[i].Normal = XMFLOAT3(0, 0, -1);
				redVtx[i].TexCoord = XMFLOAT2(0, 0);
			}
			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(redVtx);
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			D3D11_SUBRESOURCE_DATA init = { redVtx, 0, 0 };
			ID3D11Buffer* barVB = NULL;
			GetDevice()->CreateBuffer(&bd, &init, &barVB);

			GetDeviceContext()->IASetVertexBuffers(0, 1, &barVB, &stride, &offset);
			GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			GetDeviceContext()->Draw(4, 0);
			if (barVB) barVB->Release();

			// 2. --- 緑色のHP本体バー（現在HPの割合で幅を決定） ---
			if (percent > 0.0f) {
				VERTEX_3D greenVtx[4];
				float greenRight = left + HP_WIDTH * percent;  // 緑バーの右端位置
				greenVtx[0].Position = XMFLOAT3(left, HP_HEIGHT / 2, 0);       greenVtx[0].Diffuse = XMFLOAT4(0, 1, 0, 1);
				greenVtx[1].Position = XMFLOAT3(greenRight, HP_HEIGHT / 2, 0); greenVtx[1].Diffuse = XMFLOAT4(0, 1, 0, 1);
				greenVtx[2].Position = XMFLOAT3(left, -HP_HEIGHT / 2, 0);      greenVtx[2].Diffuse = XMFLOAT4(0, 1, 0, 1);
				greenVtx[3].Position = XMFLOAT3(greenRight, -HP_HEIGHT / 2, 0); greenVtx[3].Diffuse = XMFLOAT4(0, 1, 0, 1);
				for (int i = 0; i < 4; i++) {
					greenVtx[i].Normal = XMFLOAT3(0, 0, -1);
					greenVtx[i].TexCoord = XMFLOAT2(0, 0);
				}
				D3D11_SUBRESOURCE_DATA greenInit = { greenVtx, 0, 0 };
				ID3D11Buffer* fgVB = NULL;
				GetDevice()->CreateBuffer(&bd, &greenInit, &fgVB);
				GetDeviceContext()->IASetVertexBuffers(0, 1, &fgVB, &stride, &offset);
				GetDeviceContext()->Draw(4, 0);
				if (fgVB) fgVB->Release();
			}
		}

	}
}


//=============================================================================
// プレイヤー移動処理
//=============================================================================
void MovePlayers(void)
{
	// プレイヤー1 とプレイヤー2 のポインターを取得
	PLAYER* p0 = GetPlayer(0);  // プレイヤー1
	PLAYER* p1 = GetPlayer(1);  // プレイヤー2

	//------------------------------------------------------------
	// プレイヤー1 の移動（WASD）
	//------------------------------------------------------------
	if (p0 && p0->use)
	{
		// 今回はキーごとに “どちらか一方向” だけ動くように else if で列挙
		if (GetKeyboardPress(DIK_A))
		{
			p0->pos.x -= VALUE_MOVE;
			p0->rot.y = XM_PI / 2;      // 左向き
		}
		else if (GetKeyboardPress(DIK_D))
		{
			p0->pos.x += VALUE_MOVE;
			p0->rot.y = -XM_PI / 2;     // 右向き
		}
		else if (GetKeyboardPress(DIK_W))
		{
			p0->pos.z += VALUE_MOVE;
			p0->rot.y = XM_PI;          // 奥向き
		}
		else if (GetKeyboardPress(DIK_S))
		{
			p0->pos.z -= VALUE_MOVE;
			p0->rot.y = 0.0f;           // 手前向き
		}
	}

	//------------------------------------------------------------
	// プレイヤー2 の移動（矢印キー）
	//------------------------------------------------------------
	if (p1 && p1->use)
	{
		if (GetKeyboardPress(DIK_LEFT))
		{
			p1->pos.x -= VALUE_MOVE;
			p1->rot.y = XM_PI / 2;      // 左向き
		}
		else if (GetKeyboardPress(DIK_RIGHT))
		{
			p1->pos.x += VALUE_MOVE;
			p1->rot.y = -XM_PI / 2;     // 右向き
		}
		else if (GetKeyboardPress(DIK_UP))
		{
			p1->pos.z += VALUE_MOVE;
			p1->rot.y = XM_PI;          // 奥向き
		}
		else if (GetKeyboardPress(DIK_DOWN))
		{
			p1->pos.z -= VALUE_MOVE;
			p1->rot.y = 0.0f;           // 手前向き
		}
	}
}
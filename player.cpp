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
#include "player_select.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

#define	VALUE_MOVE			(1.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(14.0f)							// プレイヤーの足元をあわせる


static const char* CHARACTER_MODEL_PATHS[][6] = {
	// 0: 鳥
	{
		"data/MODEL/character/model_bird/cha_bird_head.obj",
		"data/MODEL/character/model_bird/cha_bird_body.obj",
		"data/MODEL/character/model_bird/cha_bird_leftarm.obj",
		"data/MODEL/character/model_bird/cha_bird_rightarm.obj",
		"data/MODEL/character/model_bird/cha_bird_leftleg.obj",
		"data/MODEL/character/model_bird/cha_bird_rightleg.obj"
	},
	// 1: 犬
	{
		"data/MODEL/character/model_fox/cha_fox_head.obj",
		"data/MODEL/character/model_fox/cha_fox_body.obj",
		"data/MODEL/character/model_fox/cha_fox_leftarm.obj",
		"data/MODEL/character/model_fox/cha_fox_rightarm.obj",
		"data/MODEL/character/model_fox/cha_fox_leftleg.obj",
		"data/MODEL/character/model_fox/cha_fox_rightleg.obj"
	},
	// ...他キャラ分も同様に追加
};

#define B_WALK_L_ARM_ANIMATION_INDEX		1  // 左腕用歩きモーション
#define B_WALK_R_ARM_ANIMATION_INDEX		2  // 右腕用歩きモーション

#define B_RESULT_WIN_L_ARM_ANIMATION_INDEX	   5  // 左腕用攻撃モーション
#define B_RESULT_WIN_R_ARM_ANIMATION_INDEX	   6  // 右腕用攻撃モーション
#define B_RESULT_LOSE_L_ARM_ANIMATION_INDEX	   7  // 左腕用攻撃モーション
#define B_RESULT_LOSE_R_ARM_ANIMATION_INDEX	   8  // 右腕用攻撃モーション


// 
//*****************************************************************************
// プロトタイプ宣言
//***********************************************************************
// ******


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player[MAX_PLAYER];						// プレイヤー

static PLAYER		g_Parts[MAX_PLAYER][PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;
static float noiseTable[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 };

bool g_IsCPU[MAX_PLAYER] = { false, false }; // 初期状態は両方人間

extern unsigned int dwFrameCount;

//HP初期化
static ID3D11Buffer* g_PlayerHpBarVertexBuffer = NULL; // Bar vertex buffer

// 状態管理の実体（初期化もしておく）
PLAYER_STATE g_PlayerState[MAX_PLAYER] = { PLAYER_NORMAL };
int g_PlayerAnimTimer[MAX_PLAYER] = { 0 };
XMFLOAT3 g_PlayerKnockback[MAX_PLAYER] = { XMFLOAT3(0,0,0) };

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

static DX11_MODEL g_SharedModels[PLAYER_PARTS_MAX];

static INTERPOLATION_DATA walk_move_tbl_head[] =
{	// pos,							rot,							 scl,						 frame
	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },

};


static INTERPOLATION_DATA walk_move_tbl_l_arm[] =
{	// pos,							rot,										 scl,						 frame
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0.0f,-XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, -XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },

};


static INTERPOLATION_DATA walk_move_tbl_r_arm[] =
{	// pos,							rot,							 scl,						 frame
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0.0f, XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },

};


static INTERPOLATION_DATA walk_move_tbl_l_leg[] =
{	// pos,							rot,							 scl,						 frame
	{ XMFLOAT3(2.0f, -5.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(2.0f, -5.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },

};


static INTERPOLATION_DATA walk_move_tbl_r_leg[] =
{	// pos,							rot,							 scl,						 frame
	{ XMFLOAT3(-2.0f, -5.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-2.0f, -5.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },

};


// リザルト用

//勝ち
static INTERPOLATION_DATA move_tbl_wave_l_arm[] =
{
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 1.25f , 0.0f, -XM_PI / 2), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 1.6f, 0.0f, -2.8f), XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
};
static INTERPOLATION_DATA move_tbl_wave_r_arm[] =
{
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f,XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f,XM_PI / 2),	XMFLOAT3(1.0f, 1.0f, 1.0f), 20 },
};


//負け
static INTERPOLATION_DATA move_tbl_clap_l_arm[] =
{
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, XM_PI / 8), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
	{ XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 2, 0.0f, XM_PI / 2), XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
};

static INTERPOLATION_DATA move_tbl_clap_r_arm[] =
{
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, XM_PI / 2 - 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f),10 },
	{ XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3, 0.0f, XM_PI / 2),         XMFLOAT3(1.0f, 1.0f, 1.0f), 10 },
};



static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	walk_move_tbl_head,
	walk_move_tbl_l_arm,
	walk_move_tbl_r_arm,
	walk_move_tbl_l_leg,
	walk_move_tbl_r_leg,

	move_tbl_wave_l_arm,
	move_tbl_wave_r_arm,
	move_tbl_clap_l_arm,
	move_tbl_clap_r_arm,

};






//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		int selectedChar = GetSelectedCharIndex(i);

		// 本体モデル
		g_Player[i].load = TRUE;
		LoadModel(CHARACTER_MODEL_PATHS[selectedChar][1], &g_Player[i].model);

		// パーツごと
		for (int j = 0; j < PLAYER_PARTS_MAX; j++)
		{
			g_Parts[i][j].use = TRUE;
			g_Parts[i][j].parent = &g_Player[i];   // 全部親は本体でOK
			g_Parts[i][j].time = 0.0f;
			g_Parts[i][j].tblNo = j;              // アニメテーブルの番号はjと一致（0:頭, 1:左腕, 2:右腕, ...）
			// パーツ数とテーブル数が一致している前提
			switch (j)
			{
			case 0:
				LoadModel(CHARACTER_MODEL_PATHS[selectedChar][0], &g_Parts[i][j].model); // 頭
				g_Parts[i][j].tblMax = sizeof(walk_move_tbl_head) / sizeof(INTERPOLATION_DATA);
				break;
			case 1:
				LoadModel(CHARACTER_MODEL_PATHS[selectedChar][2], &g_Parts[i][j].model); // 左腕
				g_Parts[i][j].tblMax = sizeof(walk_move_tbl_l_arm) / sizeof(INTERPOLATION_DATA);
				break;
			case 2:
				LoadModel(CHARACTER_MODEL_PATHS[selectedChar][3], &g_Parts[i][j].model); // 右腕
				g_Parts[i][j].tblMax = sizeof(walk_move_tbl_r_arm) / sizeof(INTERPOLATION_DATA);
				break;
			case 3:
				LoadModel(CHARACTER_MODEL_PATHS[selectedChar][4], &g_Parts[i][j].model); // 左足
				g_Parts[i][j].tblMax = sizeof(walk_move_tbl_l_leg) / sizeof(INTERPOLATION_DATA);
				break;
			case 4:
				LoadModel(CHARACTER_MODEL_PATHS[selectedChar][5], &g_Parts[i][j].model); // 右足
				g_Parts[i][j].tblMax = sizeof(walk_move_tbl_r_leg) / sizeof(INTERPOLATION_DATA);
				break;
			}
			g_Parts[i][j].scl = XMFLOAT3(2.0f, 2.0f, 2.0f);
			g_Parts[i][j].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][j].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[i][j].load = TRUE;
		}

		g_Player[i].pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y + 100.0f, -50.0f);
		g_Player[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Player[i].scl = XMFLOAT3(2.0f, 2.0f, 2.0f);

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

	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	if (GetMode() == MODE_GAME)
	{
		// 1) キー入力による移動＆向き設定
		MovePlayers();

		// 2) 各プレイヤーの状態更新
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			PLAYER& pl = g_Player[i];
			if (!pl.use) continue;

			if (pl.stunTimer > 0.0f)
			{
				// スタン中はノックバックさせる
				pl.pos.x += pl.knockbackVel.x;
				pl.pos.z += pl.knockbackVel.z;
				pl.stunTimer -= 1.0f / 60.0f;
				if (pl.stunTimer <= 0.0f) {
					pl.stunTimer = 0.0f;
					pl.knockbackVel = { 0,0,0 };
				}
				continue; // スタン中は通常移動処理スキップ
			}


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

			PlayerAttack(i);

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

			float blendDuration = 12.0f; // ブレンド時間

			for (int j = 0; j < PLAYER_PARTS_MAX; j++)
			{
				if (g_Parts[i][j].use && g_Parts[i][j].tblMax > 0)
				{
					if (!g_PlayerIsMoving[i])
						continue; // 歩き中のみ進行

					int nowNo = (int)g_Parts[i][j].time;
					int maxNo = g_Parts[i][j].tblMax;
					int nextNo = (nowNo + 1) % maxNo;
					INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i][j].tblNo];

					// キーフレーム補間
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

					// 補間した値を一時変数に格納
					XMFLOAT3 pos, rot, scl;
					XMStoreFloat3(&pos, nowPos + deltaPos * alpha);
					XMStoreFloat3(&rot, nowRot + deltaRot * alpha);
					XMStoreFloat3(&scl, nowScl + deltaScl * alpha);

					// --- モーションブレンド ---
					if (g_PlayerAnimBlendMode[i] == 1 && g_PlayerAnimBlendTimer[i] < blendDuration) {
						float t = g_PlayerAnimBlendTimer[i] / blendDuration;
						rot.x *= t;
					}
					else if (g_PlayerAnimBlendMode[i] == 2 && g_PlayerAnimBlendTimer[i] < blendDuration) {
						float t = g_PlayerAnimBlendTimer[i] / blendDuration;
						rot.x = g_PlayerAnimBlendFrom[i][j] * (1.0f - t);
					}


					// 結果を格納
					g_Parts[i][j].pos = pos;
					g_Parts[i][j].rot = rot;
					g_Parts[i][j].scl = scl;

					// 時間進行
					g_Parts[i][j].time += 1.0f / tbl[nowNo].frame;
					if ((int)g_Parts[i][j].time >= maxNo)
					{
						g_Parts[i][j].time -= maxNo;
					}
				}
			}

			if (g_PlayerState[i] == PLAYER_HIT) {
				UpdatePlayerKnockback(i);
				return; // やられ中は他の入力やアニメ処理を飛ばす（好みで）
			}
			UpdatePlayerPartsAnimation(i);

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
	else if (GetMode() == MODE_RESULT)
	{
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (i == 0)
			{
				PSetAnimation(i, PLAYER_RESULT_WIN);
			}
			else if (i == 1)
			{
				PSetAnimation(i, PLAYER_RESULT_LOSE);
			}

			for (int j = 0; j < PLAYER_PARTS_MAX; j++)
			{
				if (g_Parts[i][j].use && g_Parts[i][j].tblMax > 0)
				{

					int nowNo = (int)g_Parts[i][j].time;
					int maxNo = g_Parts[i][j].tblMax;
					int nextNo = (nowNo + 1) % maxNo;
					INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i][j].tblNo];

					// キーフレーム補間
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

					// 補間した値を一時変数に格納
					XMFLOAT3 pos, rot, scl;
					XMStoreFloat3(&pos, nowPos + deltaPos * alpha);
					XMStoreFloat3(&rot, nowRot + deltaRot * alpha);
					XMStoreFloat3(&scl, nowScl + deltaScl * alpha);


					// 結果を格納
					g_Parts[i][j].pos = pos;
					g_Parts[i][j].rot = rot;
					g_Parts[i][j].scl = scl;

					// 時間進行
					g_Parts[i][j].time += 1.0f / tbl[nowNo].frame;
					if ((int)g_Parts[i][j].time >= maxNo)
					{
						g_Parts[i][j].time -= maxNo;
					}
				}
			}

		}
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
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		PLAYER* p = GetPlayer(i);

		if (!g_Player[i].use) continue;

		// ★ここで前回位置を保存
		p->prevPos = p->pos;


		// ----- CPU -----
		if (g_IsCPU[i])
		{
			// ターゲット（人間プレイヤー）を探す
			int target = -1;
			for (int j = 0; j < MAX_PLAYER; j++) {
				if (j != i && !g_IsCPU[j] && g_Player[j].use) {
					target = j;
					break;
				}
			}
			if (target == -1) {
				g_PlayerIsMoving[i] = false;
				continue;
			}
			PLAYER* tgt = &g_Player[target];

			// 差分ベクトル（人間プレイヤー→CPU）を計算
			float dx = p->pos.x - tgt->pos.x;
			float dz = p->pos.z - tgt->pos.z;
			float dist = sqrtf(dx * dx + dz * dz);

			if (dist < 60.0f && dist > 0.01f) // 0.01fでゼロ除算防止
			{
				dx /= dist;
				dz /= dist;
				p->pos.x += dx * VALUE_MOVE;
				p->pos.z += dz * VALUE_MOVE;

				float targetAngle = atan2f(-dx, -dz);
				p->rot.y = SmoothAngle(p->rot.y, targetAngle, 0.15f);

				g_PlayerIsMoving[i] = true;
			}
			else
			{
				g_PlayerIsMoving[i] = false;
			}
		}
		// ----- 人の操作 -----
		else
		{
			float dx = 0.0f, dz = 0.0f;
			if (i == 0) {
				if (GetKeyboardPress(DIK_A)) dx -= 1.0f;
				if (GetKeyboardPress(DIK_D)) dx += 1.0f;
				if (GetKeyboardPress(DIK_W)) dz += 1.0f;
				if (GetKeyboardPress(DIK_S)) dz -= 1.0f;
				if (IsButtonPressed(0, BUTTON_UP))    dz += 1.0f;
				if (IsButtonPressed(0, BUTTON_DOWN))  dz -= 1.0f;
				if (IsButtonPressed(0, BUTTON_LEFT))  dx -= 1.0f;
				if (IsButtonPressed(0, BUTTON_RIGHT)) dx += 1.0f;
			}
			else if (i == 1) {
				if (GetKeyboardPress(DIK_LEFT)) dx -= 1.0f;
				if (GetKeyboardPress(DIK_RIGHT)) dx += 1.0f;
				if (GetKeyboardPress(DIK_UP)) dz += 1.0f;
				if (GetKeyboardPress(DIK_DOWN)) dz -= 1.0f;
				if (IsButtonPressed(1, BUTTON_UP))    dz += 1.0f;
				if (IsButtonPressed(1, BUTTON_DOWN))  dz -= 1.0f;
				if (IsButtonPressed(1, BUTTON_LEFT))  dx -= 1.0f;
				if (IsButtonPressed(1, BUTTON_RIGHT)) dx += 1.0f;
			}

			if (dx != 0.0f || dz != 0.0f)
			{
				float len = sqrtf(dx * dx + dz * dz);
				dx /= len;
				dz /= len;
				g_Player[i].pos.x += dx * VALUE_MOVE;
				g_Player[i].pos.z += dz * VALUE_MOVE;
				// プレイヤーの向きを更新
				float targetAngle = atan2f(-dx, -dz);
				p->rot.y = SmoothAngle(p->rot.y, targetAngle, 0.15f);

				g_PlayerIsMoving[i] = true;
			}
			else
			{
				g_PlayerIsMoving[i] = false;
			}
		}
	}
}


void UpdatePlayerPartsAnimation(int playerIndex)
{
	// 状態管理（ファイル先頭やplayer.hで宣言しておく）
	static bool g_PlayerPrevMoving[MAX_PLAYER] = { false };
	static int  g_PlayerAnimBlendTimer[MAX_PLAYER] = { 0 }; // blend進行度
	static int  g_PlayerAnimBlendMode[MAX_PLAYER] = { 0 };  // 0:通常, 1:ブレンドIN, 2:ブレンドOUT
	static float g_PlayerAnimBlendFrom[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 }; // ブレンドOUT用

	float blendDuration = 12.0f;
	bool isMoving = g_PlayerIsMoving[playerIndex];

	// --- 状態遷移チェック ---
	if (isMoving && !g_PlayerPrevMoving[playerIndex]) {
		// ブレンドIN開始
		g_PlayerAnimBlendTimer[playerIndex] = 0;
		g_PlayerAnimBlendMode[playerIndex] = 1;
	}
	else if (!isMoving && g_PlayerPrevMoving[playerIndex]) {
		// ブレンドOUT開始
		g_PlayerAnimBlendTimer[playerIndex] = 0;
		g_PlayerAnimBlendMode[playerIndex] = 2;
		// 現在のrot.x値を保存
		for (int j = 0; j < PLAYER_PARTS_MAX; j++) {
			g_PlayerAnimBlendFrom[playerIndex][j] = g_Parts[playerIndex][j].rot.x;
		}
	}
	g_PlayerPrevMoving[playerIndex] = isMoving;

	for (int j = 0; j < PLAYER_PARTS_MAX; j++)
	{
		if (g_Parts[playerIndex][j].use && g_Parts[playerIndex][j].tblMax > 0)
		{
			int nowNo = (int)g_Parts[playerIndex][j].time;
			int maxNo = g_Parts[playerIndex][j].tblMax;
			int nextNo = (nowNo + 1) % maxNo;
			INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[playerIndex][j].tblNo];

			// キーフレーム読み込み
			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);
			XMVECTOR nextPos = XMLoadFloat3(&tbl[nextNo].pos);
			XMVECTOR nextRot = XMLoadFloat3(&tbl[nextNo].rot);
			XMVECTOR nextScl = XMLoadFloat3(&tbl[nextNo].scl);

			// 差分と補間
			float alpha = g_Parts[playerIndex][j].time - nowNo;
			XMFLOAT3 pos, rot, scl;
			XMStoreFloat3(&pos, nowPos + (nextPos - nowPos) * alpha);
			XMStoreFloat3(&rot, nowRot + (nextRot - nowRot) * alpha);
			XMStoreFloat3(&scl, nowScl + (nextScl - nowScl) * alpha);

			// --- ブレンド処理 ---
			if (g_PlayerAnimBlendMode[playerIndex] == 1 && g_PlayerAnimBlendTimer[playerIndex] < blendDuration) {
				// ブレンドIN
				float t = g_PlayerAnimBlendTimer[playerIndex] / blendDuration;
				rot.x *= t; // 0→目標値
			}
			else if (g_PlayerAnimBlendMode[playerIndex] == 2 && g_PlayerAnimBlendTimer[playerIndex] < blendDuration) {
				// ブレンドOUT
				float t = g_PlayerAnimBlendTimer[playerIndex] / blendDuration;
				rot.x = g_PlayerAnimBlendFrom[playerIndex][j] * (1.0f - t); // 今の値→0へ補間
			}
			else if (!isMoving) {
				rot.x = 0.0f; // 静止時は0
			}
			if (isMoving) {
				float tNoise = (float)dwFrameCount * 0.08f + (playerIndex * 50 + j * 23);
				rot.x += GetPeriodicNoise(playerIndex * 50 + j * 23, tNoise, 0.7f); // 強めでまずテスト
				rot.y += GetSmoothNoise(noiseTable[playerIndex][j], 1.0f);
			}
			// elseは普通にキーフレーム値

			g_Parts[playerIndex][j].pos = pos;
			g_Parts[playerIndex][j].rot = rot;
			g_Parts[playerIndex][j].scl = scl;

			// アニメ時間進行（歩き中だけ進める）
			if (isMoving)
			{
				g_Parts[playerIndex][j].time += 1.0f / tbl[nowNo].frame;
				if ((int)g_Parts[playerIndex][j].time >= maxNo)
				{
					g_Parts[playerIndex][j].time -= maxNo;
				}
			}
		}
	}

	// --- ループ後、タイマー進行と状態管理 ---
	if ((g_PlayerAnimBlendMode[playerIndex] == 1 || g_PlayerAnimBlendMode[playerIndex] == 2) && g_PlayerAnimBlendTimer[playerIndex] < blendDuration)
	{
		g_PlayerAnimBlendTimer[playerIndex]++;
	}
	else if (g_PlayerAnimBlendMode[playerIndex] != 0 && g_PlayerAnimBlendTimer[playerIndex] >= blendDuration)
	{
		g_PlayerAnimBlendMode[playerIndex] = 0; // 通常状態に戻す
	}
}

void UpdatePlayerKnockback(int playerIndex)
{
	if (g_PlayerState[playerIndex] == PLAYER_HIT) {
		// 吹っ飛び
		g_Player[playerIndex].pos.x += g_PlayerKnockback[playerIndex].x;
		g_Player[playerIndex].pos.y += g_PlayerKnockback[playerIndex].y;
		g_Player[playerIndex].pos.z += g_PlayerKnockback[playerIndex].z;

		// 徐々に減速（摩擦/空気抵抗）
		g_PlayerKnockback[playerIndex].x *= 0.92f;
		g_PlayerKnockback[playerIndex].y *= 0.80f; // 重力も入れたいなら -0.03f ずつ引くなど
		g_PlayerKnockback[playerIndex].z *= 0.92f;

		// パーツ崩れ
		for (int j = 0; j < PLAYER_PARTS_MAX; j++) {
			// 派手にブルブル揺らすor完全にガクッと崩す（例）
			g_Parts[playerIndex][j].rot.x = sinf(g_PlayerAnimTimer[playerIndex] * 0.6f + j) * 0.7f;
			g_Parts[playerIndex][j].rot.y = sinf(g_PlayerAnimTimer[playerIndex] * 0.7f + j * 2) * 0.6f;
			g_Parts[playerIndex][j].rot.z = sinf(g_PlayerAnimTimer[playerIndex] * 0.5f + j * 3) * 0.5f;
		}

		g_PlayerAnimTimer[playerIndex]++;
		// 一定時間で通常に戻す
		if (g_PlayerAnimTimer[playerIndex] > 36) {
			g_PlayerState[playerIndex] = PLAYER_NORMAL;
			g_PlayerAnimTimer[playerIndex] = 0;
			g_PlayerKnockback[playerIndex] = { 0,0,0 };
		}
	}
	else {
		// 通常処理（階層アニメーション＋モーションブレンドなど）
	}
}

void OnPlayerHit(int i, const XMFLOAT3& hitDirection)
{
	g_PlayerState[i] = PLAYER_HIT;
	g_PlayerAnimTimer[i] = 0;
	// hitDirectionは正規化された方向ベクトル
	float knockbackPower = 1.2f; // 吹っ飛びの強さ
	g_PlayerKnockback[i].x = hitDirection.x * knockbackPower;
	g_PlayerKnockback[i].y = 0.2f; // ちょっとジャンプ気味もOK
	g_PlayerKnockback[i].z = hitDirection.z * knockbackPower;
}

float GetNoise(float strength)
{
	return GetRand(-1000, 1000) / 10000.0f * strength;
}

float GetPeriodicNoise(int seed, float time, float strength)
{
	return sinf(time + seed) * strength;
}

float GetSmoothNoise(float& last, float strength)
{
	float target = GetRand(-1000, 1000) / 10000.0f * strength;
	float smooth = 0.07f;
	last += (target - last) * smooth;
	return last;
}

void PSetAnimation(int playerIndex, PLAYER_STATE animation)
{
	// プレイヤーの現在のアニメーションを設定
	g_Player[playerIndex].currentAnimation = animation;

	// アニメーションタイプに基づいて処理
	switch (animation)
	{
	case PLAYER_WALK:
		// 歩行アニメーション
		g_Parts[playerIndex][1].tblNo = B_WALK_L_ARM_ANIMATION_INDEX;  // 左腕
		g_Parts[playerIndex][2].tblNo = B_WALK_R_ARM_ANIMATION_INDEX;  // 右腕
		break;

	case PLAYER_RESULT_WIN:
		// 攻撃アニメーション
		g_Parts[playerIndex][1].tblNo = B_RESULT_WIN_L_ARM_ANIMATION_INDEX;  // 左腕
		g_Parts[playerIndex][2].tblNo = B_RESULT_WIN_R_ARM_ANIMATION_INDEX;  // 右腕
		break;
	case PLAYER_RESULT_LOSE:
		// 攻撃アニメーション
		g_Parts[playerIndex][1].tblNo = B_RESULT_LOSE_L_ARM_ANIMATION_INDEX;  // 左腕
		g_Parts[playerIndex][2].tblNo = B_RESULT_LOSE_R_ARM_ANIMATION_INDEX;  // 右腕
		break;
	}

}
float NormalizeAngle(float angle)
{
	while (angle > XM_PI) angle -= XM_2PI;
	while (angle < -XM_PI) angle += XM_2PI;
	return angle;
}

float SmoothAngle(float current, float target, float smoothFactor)
{
	current = NormalizeAngle(current);
	target = NormalizeAngle(target);
	float diff = NormalizeAngle(target - current);
	return current + diff * smoothFactor;
}

float turning(float target, float current)
{
	// 角度の差分を求める（-π～πの範囲に収める）
	float diff = target - current;
	while (diff > XM_PI) diff -= XM_2PI;
	while (diff < -XM_PI) diff += XM_2PI;

	// 補間速度（0.1～0.3くらいが自然）
	const float rotateSpeed = 0.15f;

	// 補間
	return current + diff * rotateSpeed;
}

void PlayerAttack(int playerIndex)
{
	// プレイヤー1: DIK_SPACE or パッドAボタン
	// プレイヤー2: 例としてDIK_RETURN or パッドAボタン（padNo=1）
	bool attackTrigger = false;

	if (playerIndex == 0) {
		attackTrigger = GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_A);
	}
	else if (playerIndex == 1) {
		attackTrigger = GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(1, BUTTON_A);
	}

	// 弾発射
	if (attackTrigger)
	{
		// ボール発射
		BALL* ball = GetBall();
		for (int ballCnt = 0; ballCnt < 10; ballCnt++)
		{
			if (ball[ballCnt].IsUsedITball() && ball[ballCnt].IsPickedITball())  // すでに使われてるボールは撃たないようにする
			{
				ball[ballCnt].SetITball(g_Player[playerIndex].pos, g_Player[playerIndex].rot);  // プレイヤーの位置と向きでボールを発射
				break;
			}
		}

		// ボム発射
		BOMB* bomb = GetBomb();
		for (int bombCnt = 0; bombCnt < 10; bombCnt++)
		{
			if (bomb[bombCnt].IsUsedITbomb() && bomb[bombCnt].IsPickedITbomb())  // すでに使われてるボールは撃たないようにする
			{
				bomb[bombCnt].SetITbomb(g_Player[playerIndex].pos, g_Player[playerIndex].rot);  // プレイヤーの位置と向きでボールを発射
				break;
			}
		}
		// ブーメラン
		BOOM* boom = GetBoomerang();
		boom->SetThrowFlag(true);

		// ハンマー
		HAMR* hamr = GetHammer();
		hamr->SetSwingFlag(true);
	}
}
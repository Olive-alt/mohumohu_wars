#include "main.h"
#include "input.h"
#include "IT_giant.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_GIANT		"data/MODEL/sphere.obj"			// 読み込むモデル名

#define GIANT_SCL_RATE		(3.0f)							// 巨大化の倍率
#define GIANT_TIME			(600.0f)							// 巨大化の倍率

#define GIANT_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define GIANT_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
BOOL		load = FALSE;
DX11_MODEL	model;				// モデル情報


//=============================================================================
// 初期化処理
//=============================================================================

HRESULT GIANT::InitITgiant(void)
{
	if (!load)
	{
		load = TRUE;
		LoadModel(MODEL_GIANT, &model);
	}

	use = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	old_scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	old_size = 0.0f;
	giantUse = FALSE;
	giantTimer = 0;
	PlayerIndex = -1;

	return S_OK;
}


void GIANT::UninitITgiant(void)
{
	use = FALSE;
	// モデルの解放処理
	if (load == TRUE)
	{
		UnloadModel(&model);
		load = FALSE;
	}
}

void GIANT::UpdateITgiant(void)
{
	if (use)
	{
		if (giantUse)
		{
			giantTimer += 1;
			PrintDebugProc("giantTimer %d\n", giantTimer);
			if (giantTimer > GIANT_TIME)
			{
				FinishITgiant();
			}
		}
	}
}

void GIANT::DrawITgiant(void)
{
	if (!use || giantUse)return;

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(pos.x, pos.y, pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&m_mtxWorld, mtxWorld);

	// モデル描画
	DrawModel(&model);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void GIANT::SetITgiant(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}

void GIANT::FinishITgiant(void)
{
	giantUse = FALSE;
	use = FALSE;
	pick = FALSE;
	giantTimer = 0;
	PLAYER* player = GetPlayer(PlayerIndex);
	player->scl = old_scl;
	player->size = old_size;
	PlayerIndex = -1;
	player->big = FALSE;
}

void GIANT::PickITgiant(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);

	if (!player->big)
	{
		old_scl = player->scl;
		old_size = player->size;
	}

	giantUse = TRUE;
	pick = TRUE;
	player->big = TRUE;
	PlayerIndex = p_Index;
	player->scl.x *= GIANT_SCL_RATE;
	player->scl.y *= GIANT_SCL_RATE;
	player->scl.z *= GIANT_SCL_RATE;
	//player->size *= GIANT_SCL_RATE;
}

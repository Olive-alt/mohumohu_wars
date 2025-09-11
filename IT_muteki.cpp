#include "main.h"
#include "input.h"
#include "IT_muteki.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_MUTEKI		"data/MODEL/item/item_heal.obj"			// 読み込むモデル名

#define MUTEKI_SCL_RATE		(3.0f)							// 巨大化の倍率
#define MUTEKI_TIME			(600.0f)							// 巨大化の倍率

#define MUTEKI_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define MUTEKI_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define MUTEKI_VALUE   		(180.0f)							// 無敵時間(60フレーム　＊　秒)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
BOOL		muteki_load = FALSE;
DX11_MODEL	muteki_model;				// モデル情報


//=============================================================================
// 初期化処理
//=============================================================================

HRESULT MUTEKI::InitITmuteki(void)
{
	if (!muteki_load)
	{
		muteki_load = TRUE;
		LoadModel(MODEL_MUTEKI, &muteki_model);
	}

	use = FALSE;
	mutekiUse = FALSE;
	pick = FALSE;
	mutekiCount = 0;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	PlayerIndex = -1;

	return S_OK;
}


void MUTEKI::UninitITmuteki(void)
{
	use = FALSE;
	// モデルの解放処理
	if (muteki_load == TRUE)
	{
		UnloadModel(&muteki_model);
		muteki_load = FALSE;
	}
}

void MUTEKI::UpdateITmuteki(void)
{
	if (use)
	{
		if (mutekiUse)
		{
			if (mutekiCount > MUTEKI_VALUE)
			{
				FinishITmuteki();
			}
			mutekiCount++;
		}
	}
}

void MUTEKI::DrawITmuteki(void)
{
	if (!use || pick)return;

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

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
	DrawModel(&muteki_model);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void MUTEKI::SetITmuteki(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}

void MUTEKI::FinishITmuteki(void)
{
	use = FALSE;
	mutekiUse = FALSE;
	pick = FALSE;
	mutekiCount = 0;

	PLAYER* player = GetPlayer(PlayerIndex);
	player->muteki = FALSE;
	PlayerIndex = -1;
}

void MUTEKI::PickITmuteki(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);

	player->muteki = TRUE;
	mutekiUse = TRUE;
	pick = TRUE;
	PlayerIndex = p_Index;
}

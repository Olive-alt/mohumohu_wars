#include "main.h"
#include "input.h"
#include "IT_energydrink.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENADORI		"data/MODEL/item/item_energydrink.obj"			// 読み込むモデル名

#define ENADORI_SCL_RATE		(3.0f)							// 巨大化の倍率
#define ENADORI_TIME			(600.0f)							// 巨大化の倍率

#define ENADORI_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define ENADORI_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define ENADORI_VALUE   		(300.0f)							// 無敵時間(60フレーム　＊　秒)
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
BOOL		enadori_load = FALSE;
DX11_MODEL	enadori_mofel;				// モデル情報


//=============================================================================
// 初期化処理
//=============================================================================

HRESULT ENADORI::InitITenadori(void)
{
	if (!enadori_load)
	{
		enadori_load = TRUE;
		LoadModel(MODEL_ENADORI, &enadori_mofel);
	}

	use = FALSE;
	enadoriUse = FALSE;
	pick = FALSE;
	enadoriCount = 0;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	PlayerIndex = -1;

	return S_OK;
}


void ENADORI::UninitITenadori(void)
{
	use = FALSE;
	// モデルの解放処理
	if (enadori_load == TRUE)
	{
		UnloadModel(&enadori_mofel);
		enadori_load = FALSE;
	}
}

void ENADORI::UpdateITenadori(void)
{
	if (use)
	{
		if (enadoriUse)
		{
			if (enadoriCount > ENADORI_VALUE)
			{
				FinishITenadori();
			}
			enadoriCount++;
		}
	}
}

void ENADORI::DrawITenadori(void)
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
	DrawModel(&enadori_mofel);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void ENADORI::SetITenadori(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}

void ENADORI::FinishITenadori(void)
{
	use = FALSE;
	enadoriUse = FALSE;
	pick = FALSE;
	enadoriCount = 0;

	PLAYER* player = GetPlayer(PlayerIndex);
	player->enadori = FALSE;
	PlayerIndex = -1;
}

void ENADORI::PickITenadori(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);

	player->muteki = TRUE;
	enadoriUse = TRUE;
	pick = TRUE;
	PlayerIndex = p_Index;
}

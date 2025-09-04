#include "main.h"
#include "input.h"
#include "IT_heal.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_HEAL		"data/MODEL/item/item_big2.obj"			// 読み込むモデル名

#define HEAL_SCL_RATE		(3.0f)							// 巨大化の倍率
#define HEAL_TIME			(600.0f)							// 巨大化の倍率

#define HEAL_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define HEAL_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる

#define HEAL_VALUE   		(7.0f)							// プレイヤーの足元をあわせる
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

//*****************************************************************************
// グローバル変数
//*****************************************************************************
BOOL		heal_load = FALSE;
DX11_MODEL	heal_model;				// モデル情報


//=============================================================================
// 初期化処理
//=============================================================================

HRESULT HEAL::InitITheal(void)
{
	if (!heal_load)
	{
		heal_load = TRUE;
		LoadModel(MODEL_HEAL, &heal_model);
	}

	use = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	PlayerIndex = -1;

	return S_OK;
}


void HEAL::UninitITheal(void)
{
	use = FALSE;
	// モデルの解放処理
	if (heal_load == TRUE)
	{
		UnloadModel(&heal_model);
		heal_load = FALSE;
	}
}

void HEAL::UpdateITheal(void)
{
	if (use)
	{

	}
}

void HEAL::DrawITheal(void)
{
	if (!use)return;

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
	DrawModel(&heal_model);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void HEAL::SetITheal(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}

void HEAL::FinishITheal(void)
{
	use = FALSE;
	PlayerIndex = -1;
}

void HEAL::PickITheal(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);

	if (player->hp + HEAL_VALUE > PLAYER_HP_MAX)
	{
		player->hp = PLAYER_HP_MAX;
	}
	else
	{
		player->hp += HEAL_VALUE;
	}

	PlayerIndex = p_Index;
	FinishITheal();
}

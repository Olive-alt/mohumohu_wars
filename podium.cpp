#include "main.h"
#include "renderer.h"
#include "podium.h"
#include "camera.h"


// モデルパス
#define MODEL_PODIUM "data/MODEL/podium.obj"

static PODIUM		g_Podium[MAX_PODIUM];						// プレイヤー


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		g_Podium[i].load = TRUE;
		LoadModel(MODEL_PODIUM, &g_Podium[i].model);

		g_Podium[i].pos = XMFLOAT3(20.0f, 0.0f, 0.0f);
		g_Podium[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Podium[i].scl = XMFLOAT3(4.0f, 4.0f, 4.0f);


		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Podium[i].model, &g_Podium[i].diffuse[0]);
		g_Podium[i].use = TRUE;

	}


	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		// モデルの解放処理
		if (g_Podium[i].load == TRUE)
		{
			UnloadModel(&g_Podium[i].model);
			g_Podium[i].load = FALSE;
		}
	}



}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePodium(void)
{

}
//=============================================================================
// 描画処理
//=============================================================================
void DrawPodium(void)
{
	for (int i = 0; i < MAX_PODIUM; i++)
	{
		// カリング無効
		SetCullingMode(CULL_MODE_NONE);

		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Podium[i].scl.x - 0.7f, g_Podium[i].scl.y - 0.7f, g_Podium[i].scl.z - 0.7f);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Podium[i].rot.x, g_Podium[i].rot.y + XM_PI, g_Podium[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Podium[i].pos.x, g_Podium[i].pos.y, g_Podium[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Podium[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Podium[i].model);

		// カリング設定を戻す
		SetCullingMode(CULL_MODE_BACK);
	}
}

// 既存コード互換用：引数なしで呼ぶと自動的に index=0 を返す
PODIUM* GetPodium(void)
{
	return &(g_Podium[0]);
}

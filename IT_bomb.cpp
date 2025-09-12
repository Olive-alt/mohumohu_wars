#include "main.h"
#include "input.h"
#include "IT_bomb.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"
#include "score.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_BOMB  	"data/MODEL/item/item_bomb.obj"			// 読み込むモデル名
#define	MODEL_BAKUHA  	"data/MODEL/item/item_bakuha.obj"			// 読み込むモデル名


#define	VALUE_MOVE		(5.0f)					// 移動量
#define	VALUE_ROTATE	(XM_PI * 0.02f)			// 回転量

#define GRAVITY			(-9.8f)

#define BOMB_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define	BOMB_DAMAGE			(5.0f)					// ダメージ量
#define	EXP_SIZE			(10.0f)					// 移動量

//アイコン用
#define TEXTURE_MAX			(1)				// テクスチャの数

#define	ICON_WIDTH			(10.0f)			// 頂点サイズ
#define	ICON_HEIGHT			(10.0f)			// 頂点サイズ
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexITbombIcon(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static BOOL					g_bAlpaTest;		// アルファテストON/OFF

BOOL			bomb_load;
DX11_MODEL		bomb_model;				// モデル情報

BOOL			bakuha_load;
DX11_MODEL		bakuha_model;				// モデル情報


static char* g_TextureName[] =
{
	"data/TEXTURE/tree001.png",
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT BOMB::InitITbomb(void)
{
	if (!bomb_load)
	{
		bomb_load = TRUE;
		LoadModel(MODEL_BOMB, &bomb_model);
	}

	if (!bakuha_load)
	{
		bakuha_load = TRUE;
		LoadModel(MODEL_BAKUHA, &bakuha_model);
	}


	use = FALSE;
	to_throw = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	move = XMFLOAT3(0.0f, 0.0f, 0.0f);
	size = BOMB_SIZE;
	expSize = 0.0f;
	expUse = FALSE;
	count = 0.0f;
	PlayerIndex = -1;

	MakeVertexITbombIcon();

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	// 木ワークの初期化
	ZeroMemory(&icon_material, sizeof(icon_material));
	icon_material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	icon_pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	icon_fWidth = ICON_WIDTH;
	icon_fHeight = ICON_HEIGHT;
	icon_scl = XMFLOAT3(1.0f / ICON_WIDTH, 1.0f / ICON_HEIGHT, 1.0f / ICON_HEIGHT);
	icon_use = FALSE;
	g_bAlpaTest = FALSE;

	return S_OK;
}


void BOMB::UninitITbomb(void)
{
	use = FALSE;
	// モデルの解放処理
	if (bomb_load == TRUE)
	{
		UnloadModel(&bomb_model);
		bomb_load = FALSE;
	}

	if (bakuha_load == TRUE)
	{
		UnloadModel(&bakuha_model);
		bakuha_load = FALSE;
	}


	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{// テクスチャの解放
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// 頂点バッファの解放
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}

void BOMB::UpdateITbomb(void)
{
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE : TRUE;
	}

	if (use)
	{
		if (!bomb_load) return;

		if (pick)
		{
			PLAYER* player = GetPlayer(PlayerIndex);
			icon_pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			icon_pos.y = player->pos.y * 2;
			icon_pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			pos.x = player->pos.x;
			pos.y = player->pos.y;
			pos.z = player->pos.z;

		}
		else if (to_throw)
		{
			move.y += GRAVITY * count;

			count += 0.001f;    // 時間を進めている

			pos.x -= sinf(rot.y) * move.x;
			pos.y += move.y;
			pos.z -= cosf(rot.y) * move.z;


			// フィールドの外に出たら弾を消す処理
			if (pos.x < MAP_LEFT
				|| pos.x > MAP_RIGHT
				|| pos.z < MAP_DOWN
				|| pos.z > MAP_TOP)
			{
				FinishITbomb();
			}

			if (pos.y < 0.0f)
			{
				expUse = TRUE;
				to_throw = FALSE;
			}
		}

		if (expUse)
		{
			pos.y = 0.0f;
			expSize += 0.1f;
			scl = XMFLOAT3(expSize, expSize, expSize);
			if (expSize >= EXP_SIZE)
			{
				FinishITbomb();
			}
		}

	}
#ifdef _DEBUG
	//PrintDebugProc(
	//	"bombPos:(%f,%f,%f):bombExpUse(%d):count(%f)\n",
	//	pos.x,
	//	pos.y,
	//	pos.z,
	//	expUse,
	//	count
	//);
#endif

}

void BOMB::DrawITbomb(void)
{
	// αテスト設定
	if (g_bAlpaTest == TRUE)
	{
		// αテストを有効に
		SetAlphaTestEnable(TRUE);
	}

	if (!use)return;

	if (expUse)
	{
		PrintDebugProc("\nO\n");

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
		DrawModel(&bakuha_model);

		// カリング設定を戻す
		SetCullingMode(CULL_MODE_BACK);
	}
	else if (!pick || to_throw)
	{
		PrintDebugProc("\nO\n");

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
		DrawModel(&bomb_model);

		// カリング設定を戻す
		SetCullingMode(CULL_MODE_BACK);
	}
	else if (pick)
	{
		// ライティングを無効
		SetLightEnable(FALSE);

		XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
		CAMERA* cam = GetCamera();

		// 頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// プリミティブトポロジ設定
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// ビューマトリックスを取得
		mtxView = XMLoadFloat4x4(&cam->mtxView);


		// 関数使った版
		mtxWorld = XMMatrixInverse(nullptr, mtxView);
		mtxWorld.r[3].m128_f32[0] = 0.0f;
		mtxWorld.r[3].m128_f32[1] = 0.0f;
		mtxWorld.r[3].m128_f32[2] = 0.0f;

		// スケールを反映
		mtxScl = XMMatrixScaling(icon_scl.x, icon_scl.y, icon_scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(icon_pos.x, icon_pos.y, icon_pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);


		// マテリアル設定
		SetMaterial(icon_material);

		//// テクスチャ設定
		//int texNo = i % TEXTURE_MAX;
		//if (i == 4)	// ５番の木だけテクスチャアニメさせてみる
		//{
		//	// テクスチャアニメはこんな感じで良いよ
		//	g_TexAnim++;
		//	if ((g_TexAnim % 16) == 0)
		//	{
		//		g_TexNo = (g_TexNo + 1) % TEXTURE_MAX;
		//	}
		//	texNo = g_TexNo;
		//}
		//GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[texNo]);

		// ポリゴンの描画
		GetDeviceContext()->Draw(4, 0);


		// ライティングを有効に
		SetLightEnable(TRUE);

		// αテストを無効に
		SetAlphaTestEnable(FALSE);

	}
}

void BOMB::SetITbombObject(XMFLOAT3 set_pos)
{
	use = TRUE;
	expUse = FALSE;
	expSize = 0.0f;
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pos = set_pos;
}

void BOMB::SetITbomb(XMFLOAT3 set_pos, XMFLOAT3 p_rot)
{

	pick = FALSE;
	to_throw = TRUE;
	PLAYER* player = GetPlayer(PlayerIndex);
	pos = player->pos;
	rot = player->rot;
	move = {
		VALUE_MOVE,
		VALUE_MOVE,
		VALUE_MOVE,
	};
	player->haveWeapon = FALSE;

}

void BOMB::HitITbomb(int p_Index)
{
	if (p_Index == PlayerIndex) return;   // 自分自身には当たり判定しない

	PLAYER* player = GetPlayer(p_Index);
	if (!player || !player->use) return;  // 無効なプレイヤーは無視

	// ---- スコア加算 ----
	if (PlayerIndex >= 0) {
		AddScore(PlayerIndex, 150);  // 爆弾は150点とする
	}

	// ---- 爆弾の効果（既存処理）----
	player->use = FALSE;  // プレイヤーを無効化（やられ判定など）

	// ---- 爆弾のリセット ----
	//PlayerIndex = -1;     // 所有者をリセット（多重加点防止）
	//use = FALSE;          // 爆弾を無効化
	//to_throw = FALSE;     // 投げ状態解除
	//expUse = FALSE;       // 爆発状態解除
	FinishITbomb();
}


void BOMB::PickITbomb(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);

	if (player->haveWeapon)return;

	pick = TRUE;
	player->haveWeapon = TRUE;
	PlayerIndex = p_Index;
	//to_throw = FALSE;
	expUse = FALSE;
	expSize = 0.0f;
	pos.x = player->pos.x - sin(XM_PI) * 1.0f;
	pos.y = player->pos.y;
	pos.z = player->pos.z - cos(XM_PI) * 1.0f;
}

void BOMB::FinishITbomb(void)
{
	use = FALSE;
	to_throw = FALSE;
	pick = FALSE;
	expUse = FALSE;
	icon_use = FALSE;
	expSize = 0.0f;
	PlayerIndex = -1;
	move = XMFLOAT3(0.0f, 0.0f, 0.0f);
	count = 0.0f;

}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexITbombIcon(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = 60.0f;
	float fHeight = 90.0f;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}


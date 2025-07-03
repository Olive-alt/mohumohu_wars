#include "main.h"
#include "input.h"
#include "IT_hammer.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_HAMR  	"data/MODEL/HAMMER.obj"			// 読み込むモデル名

#define	VALUE_MOVE		(10.0f)					// 移動量
#define	VALUE_ROTATE	(XM_PI * 0.02f)			// 回転量

#define HAMR_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define HAMR_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる
#define	HAMR_DAMAGE			(5.0f)					// ダメージ量


//アイコン用
#define TEXTURE_MAX			(1)				// テクスチャの数

#define	ICON_WIDTH			(10.0f)			// 頂点サイズ
#define	ICON_HEIGHT			(10.0f)			// 頂点サイズ
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexITHamrIcon(void);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報
static BOOL					g_bAlpaTest;		// アルファテストON/OFF

static char* g_TextureName[] =
{
	"data/TEXTURE/tree001.png",
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT HAMR::InitITHamr(void)
{
	load = TRUE;
	LoadModel(MODEL_HAMR, &model);

	use = FALSE;
	to_swing = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	move = XMFLOAT3(VALUE_MOVE, 1.0f, VALUE_MOVE);
	size = HAMR_SIZE;
	count = 0.0f;
	PlayerIndex = -1;

	MakeVertexITHamrIcon();

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


void HAMR::UninitITHamr(void)
{
	use = FALSE;
	// モデルの解放処理
	if (load == TRUE)
	{
		UnloadModel(&model);
		load = FALSE;
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

void HAMR::UpdateITHamr(void)
{
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE : TRUE;
	}

	if (use)
	{
		if (!load) return;


		if (pick)
		{
			PLAYER* player = GetPlayer(PlayerIndex);
			icon_pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			icon_pos.y = player->pos.y * 2;
			icon_pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			pos.y = player->pos.y;
			pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			rot = player->rot;


			// Swing trigger
			if (GetKeyboardTrigger(DIK_NUMPAD1)) // replace with your attack key
			{
				to_swing = true;
				// Start swing timer, animation, etc.
			}
		}

		if (to_swing)
		{
			// 1フレームで進む角度
			float swingSpeed = 0.2f; // 速くしたければ値を大きく
			swingAngle += swingSpeed;
			if (swingAngle >= swingMax)
			{
				swingAngle = swingMax;
				to_swing = false; // アニメーション終了
			}
		}
		else
		{
			// ゆっくり戻す
			float returnSpeed = 0.1f; // 戻る速さ(小さくするほど遅く戻る)
			if (swingAngle > 0.0f)
			{
				swingAngle -= returnSpeed;
				if (swingAngle < 0.0f)
					swingAngle = 0.0f;
			}
		}




	}
#ifdef _DEBUG
	PrintDebugProc(
		"HamrPos:(%f,%f,%f)\n",
		pos.x,
		pos.y,
		pos.z
	);
#endif

}

void HAMR::DrawITHamr(void)
{
	// αテスト設定
	if (g_bAlpaTest == TRUE)
	{
		// αテストを有効に
		SetAlphaTestEnable(TRUE);
	}

	if (!use)return;

	if (!pick)
	{
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
	else if (pick)
	{
		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

		// カリング無効
		SetCullingMode(CULL_MODE_NONE);

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(scl.x, scl.y, scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// モデルの高さの半分
		float halfHeight = scl.y * 0.5f;

		// ピボットを下端にして回転
		XMMATRIX mtxPivotUp = XMMatrixTranslation(0.0f, halfHeight, 0.0f);
		XMMATRIX mtxPivotDown = XMMatrixTranslation(0.0f, -halfHeight, 0.0f);
		XMMATRIX mtxSwing = XMMatrixRotationX(swingAngle); // Z軸回転なら手前に倒れる

		// 1. Scale
		mtxWorld = XMMatrixIdentity();
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixScaling(scl.x, scl.y, scl.z));

		// 2. Move pivot up to the bottom of hammer
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(0.0f, halfHeight, 0.0f));

		// 3. Rotate forward around X axis (local right)
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixRotationX(-swingAngle));

		// 4. Move pivot back down
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(0.0f, -halfHeight, 0.0f));

		// 5. Player rotation (rot.y is yaw - facing direction)
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z));

		// 6. World position
		mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(pos.x, pos.y, pos.z));

		// Set and draw
		SetWorldMatrix(&mtxWorld);
		XMStoreFloat4x4(&m_mtxWorld, mtxWorld);
		DrawModel(&model);


		// カリング設定を戻す
		SetCullingMode(CULL_MODE_BACK);

	}
}

void HAMR::SetITHamrObject(XMFLOAT3 set_pos)
{
	use = TRUE;
	pos = set_pos;
}


void HAMR::HitITHamr(int p_Index)
{
	if (p_Index == PlayerIndex)
		return;

	PLAYER* player = GetPlayer(p_Index);

	// Only squish if not already squished
	if (!player->squished)
		player->originalScl = player->scl;

	player->scl.y = 0.3f;
	player->scl.x = 1.4f;
	player->scl.z = 1.4f;
	player->squished = true;
	player->squishTimer = 5.0f; // 5 seconds

}




void HAMR::PickITHamr(int p_Index)
{
	PLAYER* player = GetPlayer(p_Index);
	if (player->haveWeapon)return;

	pick = TRUE;

	player->haveWeapon = TRUE;
	PlayerIndex = p_Index;
	pos.x = player->pos.x - sin(XM_PI) * 1.0f;
	pos.y = player->pos.y;
	pos.z = player->pos.z - cos(XM_PI) * 1.0f;
}


//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexITHamrIcon(void)
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

XMFLOAT3 HAMR::GetHeadWorldPosition() const
{
	float halfHeight = scl.y * 0.5f;
	float yOffset = 40.0f / scl.y; // Optional: to raise the head by 30 units in world space

	// The local space point for the hammer head (add yOffset if you want to simulate the +30)
	XMVECTOR localHeadPos = XMVectorSet(0.0f, scl.y + yOffset, 0.0f, 1.0f);

	XMMATRIX mtx = XMMatrixIdentity();
	mtx = XMMatrixMultiply(mtx, XMMatrixScaling(scl.x, scl.y, scl.z));
	mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(0.0f, halfHeight, 0.0f));
	mtx = XMMatrixMultiply(mtx, XMMatrixRotationX(-swingAngle));
	mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(0.0f, -halfHeight, 0.0f));
	mtx = XMMatrixMultiply(mtx, XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z));
	mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(pos.x, pos.y, pos.z));

	XMVECTOR worldHeadPos = XMVector3Transform(localHeadPos, mtx);

	XMFLOAT3 out;
	XMStoreFloat3(&out, worldHeadPos);
	return out;
}

#include "main.h"
#include "input.h"
#include "IT_boomerang.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_BOOM  	"data/MODEL/sphere.obj"			// 読み込むモデル名

#define	VALUE_MOVE		(10.0f)					// 移動量
#define	VALUE_ROTATE	(XM_PI * 0.02f)			// 回転量

#define BOOM_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define BOOM_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる
#define	BOOM_DAMAGE			(5.0f)					// ダメージ量


//アイコン用
#define TEXTURE_MAX			(1)				// テクスチャの数

#define	ICON_WIDTH			(10.0f)			// 頂点サイズ
#define	ICON_HEIGHT			(10.0f)			// 頂点サイズ
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexITboomIcon(void);

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
HRESULT BOOM::InitITboom(void)
{
	load = TRUE;
	LoadModel(MODEL_BOOM, &model);

	use = FALSE;
	to_throw = FALSE;
	pick = FALSE;
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
	move = XMFLOAT3(VALUE_MOVE, 1.0f, VALUE_MOVE);
	size = BOOM_SIZE;
	count = 0.0f;
	PlayerIndex = -1;

	boomerangArcRadius = 50.0f;   
	boomerangSpeed = 1.5f;        
	boomerangTime = 0.0f;

	MakeVertexITboomIcon();

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


void BOOM::UninitITboom(void)
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

void BOOM::UpdateITboom(void)
{
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? FALSE : TRUE;
	}

	if (use)
	{
		if (!load) return;

		if (to_throw)
		{
			boomerangTime += boomerangSpeed * 0.032f;

			PLAYER* player = GetPlayer(PlayerIndex);

			if (!returning)
			{
				// 外向きの軌道: startPos から endPos へ
				float t = boomerangTime / 1.8f; // 0 から 1 まで
				if (t >= 1.0f)
				{
					t = 1.0f;
					returning = true;
					boomerangTime = 0.0f; // 戻りのタイマーリセット
				}

				// 外向きの楕円軌道
				XMFLOAT3 center;
				center.x = (startPos.x + endPos.x) * 0.5f;
				center.y = (startPos.y + endPos.y) * 0.5f;
				center.z = (startPos.z + endPos.z) * 0.5f;

				XMFLOAT3 major;
				major.x = (startPos.x - endPos.x) * 0.5f;
				major.y = 0.0f;
				major.z = (startPos.z - endPos.z) * 0.5f;

				XMFLOAT3 minor;
				minor.x = -(major.z);
				minor.y = 0.0f;
				minor.z = major.x;

				float majorLen = sqrtf(major.x * major.x + major.z * major.z);
				float arcHeight = 0.2f * majorLen;
				float minorLen = sqrtf(minor.x * minor.x + minor.z * minor.z);
				if (minorLen > 0.01f)
				{
					minor.x = (minor.x / minorLen) * arcHeight;
					minor.z = (minor.z / minorLen) * arcHeight;
				}

				float angle = XM_PI * t; // 0 から PI まで

				pos.x = center.x + major.x * cosf(angle) + minor.x * sinf(angle);
				pos.y = center.y;
				pos.z = center.z + major.z * cosf(angle) + minor.z * sinf(angle);
			}
			else
			{
				// 内向きの軌道: endPos からプレイヤーの現在位置へ（毎フレーム再計算）
				PLAYER* player = GetPlayer(PlayerIndex);

				// 毎フレーム、ターゲットをプレイヤーの現在位置にする
				XMFLOAT3 dynamicTarget;
				dynamicTarget.x = player->pos.x - sin(XM_PI) * 1.0f;
				dynamicTarget.y = player->pos.y;
				dynamicTarget.z = player->pos.z - cos(XM_PI) * 1.0f;

				float t = boomerangTime / 1.8f; // 0 から 1 まで
				if (t >= 1.0f) t = 1.0f;

				// 内向きの楕円軌道
				XMFLOAT3 center;
				center.x = (endPos.x + dynamicTarget.x) * 0.5f;
				center.y = (endPos.y + dynamicTarget.y) * 0.5f;
				center.z = (endPos.z + dynamicTarget.z) * 0.5f;

				XMFLOAT3 major;
				major.x = (endPos.x - dynamicTarget.x) * 0.5f;
				major.y = 0.0f;
				major.z = (endPos.z - dynamicTarget.z) * 0.5f;

				XMFLOAT3 minor;
				minor.x = -(major.z);
				minor.y = 0.0f;
				minor.z = major.x;

				float majorLen = sqrtf(major.x * major.x + major.z * major.z);
				float arcHeight = 0.2f * majorLen;
				float minorLen = sqrtf(minor.x * minor.x + minor.z * minor.z);
				if (minorLen > 0.01f)
				{
					minor.x = (minor.x / minorLen) * arcHeight;
					minor.z = (minor.z / minorLen) * arcHeight;
				}

				float angle = XM_PI * t; // 0 から PI まで

				pos.x = center.x + major.x * cosf(angle) + minor.x * sinf(angle);
				pos.y = center.y;
				pos.z = center.z + major.z * cosf(angle) + minor.z * sinf(angle);

				// 目標に十分近ければアーク終了
				float dx = pos.x - dynamicTarget.x;
				float dy = pos.y - dynamicTarget.y;
				float dz = pos.z - dynamicTarget.z;
				float dist = sqrtf(dx * dx + dy * dy + dz * dz);
				if (t >= 1.0f || dist < 2.0f)
				{
					pos = dynamicTarget;
					to_throw = FALSE;
					returning = FALSE;
					pick = TRUE;
					count = 0.0f;
					player->haveWeapon = TRUE;
				}
			}
		}
		if (pick)
		{
			PLAYER* player = GetPlayer(PlayerIndex);

			icon_pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			icon_pos.y = player->pos.y * 2;
			icon_pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			pos.x = player->pos.x - sin(XM_PI) * 1.0f;
			pos.y = player->pos.y;
			pos.z = player->pos.z - cos(XM_PI) * 1.0f;

			if (GetKeyboardTrigger(DIK_NUMPAD0))
			{
				rot = player->rot;
				pick = FALSE;
				to_throw = TRUE;
				count = 0.0f;
				returning = false;
				boomerangTime = 0.0f;

				// スタート位置 = プレイヤーの手
				startPos.x = player->pos.x - sinf(XM_PI) * 1.0f;
				startPos.y = player->pos.y;
				startPos.z = player->pos.z - cosf(XM_PI) * 1.0f;

				// 終点位置 = プレイヤー前方
				float throwDistance = 160.0f;
				endPos.x = player->pos.x + -sinf(rot.y) * throwDistance;
				endPos.y = player->pos.y;
				endPos.z = player->pos.z + -cosf(rot.y) * throwDistance;

				pos = startPos;
			}

		}
	}
#ifdef _DEBUG
	PrintDebugProc(
		"boomPos:(%f,%f,%f)\n",
		pos.x,
		pos.y,
		pos.z
	);
#endif
}

void BOOM::DrawITboom(void)
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

void BOOM::SetITboomObject(XMFLOAT3 set_pos, int playerIndex)
{
	use = TRUE;
	pos = set_pos;
	pick = FALSE;
	to_throw = FALSE; 
	returning = false;
	PlayerIndex = playerIndex;
	PLAYER* player = GetPlayer(PlayerIndex);
	pos = set_pos;
	startPos = player->pos;
	rot = player->rot;
	player->haveWeapon = FALSE;
}


void BOOM::HitITboom(int p_Index)
{
	// 自分自身に当たるのを防ぐ
	if (p_Index == PlayerIndex) return;

	// 当たったプレイヤーから武器を外す
	GetPlayer(p_Index)->haveWeapon = FALSE;

	// 当たったプレイヤーを消す（無効化）
	GetPlayer(p_Index)->use = FALSE;

}




void BOOM::PickITboom(int p_Index)
{
	pick = TRUE;
	PLAYER* player = GetPlayer(p_Index);

	if (player->haveWeapon) return; 

	player->haveWeapon = TRUE;
	PlayerIndex = p_Index;
	pos.x = player->pos.x - sin(XM_PI) * 1.0f;
	pos.y = player->pos.y;
	pos.z = player->pos.z - cos(XM_PI) * 1.0f;
}



//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexITboomIcon(void)
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


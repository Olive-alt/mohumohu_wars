//=============================================================================
//
// タイトル画面処理 [title.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"
#include "meshfield.h"
#include "score.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(SCREEN_WIDTH)	// 背景サイズ
#define TEXTURE_HEIGHT				(SCREEN_HEIGHT)	// 
#define TEXTURE_MAX					(9)				// テクスチャの数

#define TEXTURE_WIDTH_LOGO			(480)			// ロゴサイズ
#define TEXTURE_HEIGHT_LOGO			(80)			// 

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;		// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/Title/ui_title_bg1280x720.png",
	"data/TEXTURE/Title/ui_title_bt_start_off.png",
	"data/TEXTURE/Title/ui_title_bt_start_on.png",
	"data/TEXTURE/Title/ui_title_bt_set_off.png",
	"data/TEXTURE/Title/ui_title_bt_set_on.png",
	"data/TEXTURE/Title/ui_title_bt_end_off.png",
	"data/TEXTURE/Title/ui_title_bt_end_on.png",
	"data/TEXTURE/Title/002.png",

	"data/TEXTURE/effect000.jpg",
};


static BOOL						g_Use;						// TRUE:使っている  FALSE:未使用
static float					g_w, g_h;					// 幅と高さ
static XMFLOAT3					g_Pos;						// ポリゴンの座標
static XMFLOAT3					g_RogPos;					// タイトルロゴの座標
static XMFLOAT3					g_StartPos;					// タイトルロゴの座標
static XMFLOAT3					g_ExitPos;					// タイトルロゴの座標
static int						g_TexNo;					// テクスチャ番号

static int						selectedMenuItem;			// タイトルでのモード選択

float	alpha;
BOOL	flag_alpha;

BOOL	flag_settings;

static BOOL						g_Load = FALSE;

static bool isExitDialogShown = false;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
{
	ID3D11Device* pDevice = GetDevice();

	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// 変数の初期化
	g_Use = TRUE;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
	g_RogPos = XMFLOAT3(g_w / 2, g_h / 5, 0.0f);
	//g_StartPos = XMFLOAT3(g_w/4, g_h/1.5, 0.0f);
	//g_ExitPos = XMFLOAT3(g_w/4 * 3, g_h / 1.5, 0.0f);

	selectedMenuItem = 0;

	g_TexNo = 0;

	alpha = 1.0f;
	flag_alpha = TRUE;

	flag_settings = FALSE;

	//スコアをリセットする
	ResetScore();

	// BGM再生
	PlaySound(SOUND_LABEL_BGM_title);

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTitle(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTitle(void)
{
	if (GetKeyboardTrigger(DIK_DOWN))
	{
		selectedMenuItem += 1;
	}
	else if (selectedMenuItem >= 3)
	{
		selectedMenuItem = 0;
	}

	if (GetKeyboardTrigger(DIK_UP))
	{
		selectedMenuItem -= 1;
	}
	else if (selectedMenuItem <= -1)
	{
		selectedMenuItem = 2;
	}

	if (selectedMenuItem == 0 && GetKeyboardTrigger(DIK_RETURN))
	{// ゲーム開始アイコンでEnter押したら、ゲームスタート
		SetFade(FADE_OUT, MODE_PLAYER_SELECT);
		//SetFade(FADE_OUT, MODE_RESULT);
	}
	if (selectedMenuItem == 1 && GetKeyboardTrigger(DIK_RETURN))
	{// 設定アイコンでEnter押したら、設定ウィンドウを出す
		flag_settings == TRUE;

		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[8]);

		SetSprite(g_VertexBuffer, g_Pos.x / 2, g_Pos.y / 2, g_w / 2, g_h / 2, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);




	}
	//if (selectedMenuItem == 2 && GetKeyboardTrigger(DIK_RETURN))
	//{
	//	int id = MessageBox(NULL, "ゲームを終了しますか？", "起動モード", MB_YESNOCANCEL | MB_ICONQUESTION);
	//	switch (id)
	//	{
	//	case IDYES:        // Yesなら終了
	//	std::exit(-1);
	//	case IDNO:        // Noならタイトルに戻る
	//		return;
	//	case IDCANCEL:    // CANCELでもタイトルへ
	//	default:
	//		break;
	//	}
	//}

	if (selectedMenuItem == 2 && GetKeyboardTrigger(DIK_RETURN) && !isExitDialogShown)
	{
		//isExitDialogShown = true; // 一度だけ表示

		//int id = MessageBox(NULL, "ゲームを終了しますか？", "起動モード", MB_YESNOCANCEL | MB_ICONQUESTION);
		//switch (id)
		//{
		//case IDYES:        // Yesなら終了
		std::exit(-1);
		//	break;

		//case IDNO:	// Noならタイトルに戻る
		//	SetMode(MODE_TITLE);
		//	selectedMenuItem = 2;
		//default:
		//	SetMode(MODE_TITLE);
		//	selectedMenuItem = 2;

		//	isExitDialogShown = false; // ダイアログ閉じたら再表示OKに
		//	break;
		//}
	}

	// ゲームパッドで入力処理
	else if (selectedMenuItem == 0 && IsButtonTriggered(0, BUTTON_START))
	{
		SetFade(FADE_OUT, MODE_GAME);
	}
	else if (selectedMenuItem == 0 && IsButtonTriggered(0, BUTTON_B))
	{
		SetFade(FADE_OUT, MODE_GAME);
	}

	if (flag_alpha == TRUE)
	{
		alpha -= 0.02f;
		if (alpha <= 0.0f)
		{
			alpha = 0.0f;
			flag_alpha = FALSE;
		}
	}
	else
	{
		alpha += 0.02f;
		if (alpha >= 1.0f)
		{
			alpha = 1.0f;
			flag_alpha = TRUE;
		}
	}






#ifdef _DEBUG	// デバッグ情報を表示する
	//char *str = GetDebugStr();
	//sprintf(&str[strlen(str)], " PX:%.2f PY:%.2f", g_Pos.x, g_Pos.y);

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTitle(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// タイトルの背景を描画
	{
		// テクスチャ設定
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

		// １枚のポリゴンの頂点とテクスチャ座標を設定
		SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0.0f, 0.0f, 1.0f, 1.0f);

		// ポリゴン描画
		GetDeviceContext()->Draw(4, 0);
	}

	// タイトルのロゴを描画
	//{
	//	// テクスチャ設定
	//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[ ]);

	//	// １枚のポリゴンの頂点とテクスチャ座標を設定
	//	SetSprite(g_VertexBuffer, g_RogPos.x, g_RogPos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f);
	//	//SetSpriteColor(g_VertexBuffer, g_Pos.x, g_Pos.y, TEXTURE_WIDTH_LOGO, TEXTURE_HEIGHT_LOGO, 0.0f, 0.0f, 1.0f, 1.0f,
	//	//				XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

	//	// ポリゴン描画
	//	GetDeviceContext()->Draw(4, 0);
	//}

	// スタートロゴを描画
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[1]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 12.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	// 設定ロゴを描画
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[3]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 15, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	// ゲーム終了ロゴを描画
	{
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[5]);

		SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 17.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		GetDeviceContext()->Draw(4, 0);
	}

	if (selectedMenuItem == 0)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 12.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	if (selectedMenuItem == 1)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[4]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 15, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	if (selectedMenuItem == 2)
	{
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[6]);

			SetSprite(g_VertexBuffer, g_Pos.x / 3, g_Pos.y / 10 * 17.5, 250.0f, 100.0f, 0.0f, 0.0f, 1.0f, 1.0f);

			GetDeviceContext()->Draw(4, 0);
		}
	}

	// 設定フラグが立ったら、設定ウィンドウを表示
	if (flag_settings == TRUE)
	{

	}

	//	// 加減算のテスト
	//	SetBlendState(BLEND_MODE_ADD);		// 加算合成
	////	SetBlendState(BLEND_MODE_SUBTRACT);	// 減算合成
	//	for(int i=0; i<30; i++)
	//	{
	//		// テクスチャ設定
	//		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[2]);
	//
	//		// １枚のポリゴンの頂点とテクスチャ座標を設定
	//		float dx = 100.0f;
	//		float dy = 100.0f;
	//		float sx = (float)(rand() % 100);
	//		float sy = (float)(rand() % 100);
	//
	//
	//		SetSpriteColor(g_VertexBuffer, dx+sx, dy+sy, 50, 50, 0.0f, 0.0f, 1.0f, 1.0f,
	//			XMFLOAT4(0.3f, 0.3f, 1.0f, 0.5f));
	//
	//		// ポリゴン描画
	//		GetDeviceContext()->Draw(4, 0);
	//	}
	//	SetBlendState(BLEND_MODE_ALPHABLEND);	// 半透明処理を元に戻す

}


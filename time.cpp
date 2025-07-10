//=============================================================================
//
// タイム処理 [time.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "time.h"
#include "sprite.h"
#include <windows.h>

//*****************************************************************************
// 定数定義
//*****************************************************************************
static char* g_TexturName[] = {
"data/TEXTURE/Score/ui_num.png",
};

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };

static BOOL g_Use;
static float g_w, g_h;
static XMFLOAT3 g_Pos;
static int g_TexNo;

static float g_timeLimit = TIME_LIMIT_DEFAULT;
static float g_timeRemaining = 0.0f;
static bool g_timeOver = false;

// Δ時間（秒）計測用
static LARGE_INTEGER g_Frequency;
static LARGE_INTEGER g_PrevTime;
static float g_DeltaTime = 0.0f;

//=============================================================================
// 経過時間（Δ）初期化
//=============================================================================
void InitDeltaTime(void)
{
	QueryPerformanceFrequency(&g_Frequency);
	QueryPerformanceCounter(&g_PrevTime);
	g_DeltaTime = 0.0f;
}

//=============================================================================
// 経過時間（Δ）更新処理（毎フレーム）
//=============================================================================
void UpdateDeltaTime(void)
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	g_DeltaTime = (float)(currentTime.QuadPart - g_PrevTime.QuadPart) / (float)g_Frequency.QuadPart;
	g_PrevTime = currentTime;

}

//=============================================================================
// 経過時間（Δ）取得
//=============================================================================
float GetDeltaTime(void)
{
	return g_DeltaTime;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTime(void)
{
	ID3D11Device* pDevice = GetDevice();

	for (int i = 0; i < TEXTURE_MAX; i++) {
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(pDevice, g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
	}

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_Use = TRUE;
	g_w = TEXTURE_WIDTH;
	g_h = TEXTURE_HEIGHT;
	g_Pos = { 900.0f, 20.0f, 0.0f };
	g_TexNo = 0;

	g_timeLimit = TIME_LIMIT_DEFAULT;
	g_timeRemaining = g_timeLimit;
	g_timeOver = false;

	InitDeltaTime(); // ← 追加

	return S_OK;

}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTime(void)
{
	if (g_VertexBuffer) {
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	for (int i = 0; i < TEXTURE_MAX; i++) {
		if (g_Texture[i]) {
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTime(void)
{
	if (!g_timeOver) {
		g_timeRemaining -= GetDeltaTime(); // フレーム時間で減算
		if (g_timeRemaining <= 0.0f) {
			g_timeRemaining = 0.0f;
			g_timeOver = true;
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTime(void)
{
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	SetWorldViewProjection2D();
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	MATERIAL material = {};
	material.Diffuse = XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_TexNo]);

	int totalSeconds = (int)g_timeRemaining;

	for (int i = 0; i < TIME_DIGIT; i++) {
		float x = totalSeconds % 10;
		float px = g_Pos.x - g_w * i;
		float py = g_Pos.y;
		float pw = g_w;
		float ph = g_h;
		float tw = 0.1f;
		float th = 1.0f;
		float tx = x * tw;
		float ty = 0.0f;

		SetSpriteColor(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
		GetDeviceContext()->Draw(4, 0);

		// 次の桁へ
		totalSeconds /= 10;

	}

}

//=============================================================================
// 外部アクセス関数
//=============================================================================
void SetTimeLimit(float sec)
{
	g_timeLimit = sec;
	g_timeRemaining = sec;
	g_timeOver = false;
}

float GetRemainingTime()
{
	return g_timeRemaining;
}

bool IsTimeOver()
{
	return g_timeOver;
}
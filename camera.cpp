//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// カメラの初期位置(X座標)
#define	POS_Y_CAM			(180.0f)			// カメラの初期位置(Y座標)
#define	POS_Z_CAM			(-160.0f)		// カメラの初期位置(Z座標)
#define ORIGINAL_CAMERA_Y   (180.0f)

//#define	POS_X_CAM		(0.0f)			// カメラの初期位置(X座標)
//#define	POS_Y_CAM		(150.0f)		// カメラの初期位置(Y座標)
//#define	POS_Z_CAM		(-400.0f)		// カメラの初期位置(Z座標)


#define	VIEW_ANGLE		(XMConvertToRadians(60.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

#define	VALUE_MOVE_CAMERA	(2.0f)										// カメラの移動量
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// カメラの回転量

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static CAMERA			g_Camera;		// カメラデータ

static int				g_ViewPortType = TYPE_FULL_SCREEN;

//=============================================================================
// 初期化処理
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };

	// 視点と注視点の距離を計算
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);
	
	// ビューポートタイプの初期化
	SetViewPort(g_ViewPortType);
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void UpdateCamera(void)
{

#ifdef _DEBUG

	if (GetKeyboardPress(DIK_Z))
	{// 視点旋回「左」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_C))
	{// 視点旋回「右」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_Y))
	{// 視点移動「上」
		g_Camera.pos.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_N))
	{// 視点移動「下」
		g_Camera.pos.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_Q))
	{// 注視点旋回「左」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_E))
	{// 注視点旋回「右」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_T))
	{// 注視点移動「上」
		g_Camera.at.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_B))
	{// 注視点移動「下」
		g_Camera.at.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_U))
	{// 近づく
		g_Camera.len -= VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_M))
	{// 離れる
		g_Camera.len += VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	// カメラを初期に戻す
	if (GetKeyboardPress(DIK_R))
	{
		UninitCamera();
		InitCamera();
	}

#endif



#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");
#endif
}


//=============================================================================
// カメラの更新
//=============================================================================
void SetCamera(void) 
{
	// ビューマトリックス設定
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&g_Camera.at), XMLoadFloat3(&g_Camera.up));
	SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// カメラの取得
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// ビューポート設定
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;


	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int GetViewPortType(void)
{
	return g_ViewPortType;
}



// カメラの視点と注視点をセット
void SetCameraAT(XMFLOAT3 pos)
{
	// カメラの注視点を引数の座標にしてみる
	g_Camera.at = pos;

	// カメラの視点をカメラのY軸回転に対応させている
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;

}

//=============================================================================
//==============================================================================
// ターゲットが画面外に出る前にカメラを上昇させる関数
//==============================================================================
// ==== Camera tuning (変更可能) ====
struct CameraTuning {
	float pitchDeg = 35.0f;     // ピッチ角(度)
	float panLerp = 0.15f;     // 平行移動の追従
	float zoomLerpOut = 0.20f;     // ズームアウト応答
	float zoomLerpIn = 0.12f;     // ズームイン応答
	float minRadius = 1.0f;      // 半径の下限
	float minDist = 8.0f;      // 距離の下限
	float maxY = 400.0f;    // 高さ上限
	float minHeightY = ORIGINAL_CAMERA_Y - 2.0f; // 高さ下限

	// 画面内判定(NDC)
	float softInNDC = 0.72f;
	float softOutNDC = 0.88f;
	int   dwellFrames = 8;

	// 近距離バイアス
	float nudgeInNDC = 0.65f;
	float nudgeHyst = 0.05f;
	float nudgeFactor = 0.85f;
	float holdZoomInLerp = 0.15f;

	// 1フレームでのズームアウト量上限(ワールド単位)
	float maxZoomOutStep = 0.75f;
};

static CameraTuning g_CamTune{};
inline void SetCameraTuning(const CameraTuning& t) { g_CamTune = t; }
inline const CameraTuning& GetCameraTuning() { return g_CamTune; }


void EnsureCameraFramesTargets(XMFLOAT3 target1, XMFLOAT3 target2, float viewEdgeBuffer = 0.15f)
{
	// ==== チューニング値 ====
	const CameraTuning& K = GetCameraTuning();

	// ==== ヘルパ ====
	auto MinF = [](float a, float b) { return a < b ? a : b; };
	auto MaxF = [](float a, float b) { return a > b ? a : b; };
	auto ClampF = [&](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };
	auto LerpF = [&](float a, float b, float t) { t = ClampF(t, 0.0f, 1.0f); return a + (b - a) * t; };
	auto Lerp3 = [&](const XMFLOAT3& a, const XMFLOAT3& b, float t) {
		XMFLOAT3 r{ LerpF(a.x,b.x,t), LerpF(a.y,b.y,t), LerpF(a.z,b.z,t) }; return r;
		};

	SetCamera();

	// ==== ターゲット中心 ====
	XMVECTOR v1 = XMLoadFloat3(&target1);
	XMVECTOR v2 = XMLoadFloat3(&target2);
	XMVECTOR centerV = XMVectorScale(XMVectorAdd(v1, v2), 0.5f);
	XMFLOAT3 center; XMStoreFloat3(&center, centerV);

	// 分離半径
	float distTargets = XMVectorGetX(XMVector3Length(XMVectorSubtract(v1, v2)));
	float radius = MaxF(0.5f * distTargets, K.minRadius);

	// ==== 射影パラメータ ====
	XMFLOAT4X4 P = g_Camera.mtxProjection;
	const float tanHalfFovX = 1.0f / P._11;
	const float tanHalfFovY = 1.0f / P._22;
	const float safety = 1.0f + viewEdgeBuffer;

	// 収まるための距離
	const float dVert = (radius * safety) / tanHalfFovY;
	const float dHoriz = (radius * safety) / tanHalfFovX;
	float baseFitDist = MaxF(MaxF(dVert, dHoriz), K.minDist);

	// ==== カメラ基底 ====
	XMVECTOR camPosV = XMLoadFloat3(&g_Camera.pos);
	XMVECTOR camAtV = XMLoadFloat3(&g_Camera.at);
	XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(camAtV, camPosV));
	XMVECTOR forwardXZ = XMVectorSet(XMVectorGetX(forward), 0.0f, XMVectorGetZ(forward), 0.0f);
	float lenXZ = XMVectorGetX(XMVector3Length(forwardXZ));
	forwardXZ = (lenXZ < 1e-4f) ? XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) : XMVectorScale(forwardXZ, 1.0f / lenXZ);

	const float currentDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(camAtV, camPosV)));

	// ==== 画面上の広がり(NDC) ====
	auto WorldToNDC = [&]() {
		XMMATRIX view = XMLoadFloat4x4(&g_Camera.mtxView);
		XMMATRIX proj = XMLoadFloat4x4(&g_Camera.mtxProjection);
		XMMATRIX vp = XMMatrixMultiply(view, proj);
		XMVECTOR ndc1 = XMVector3TransformCoord(v1, vp);
		XMVECTOR ndc2 = XMVector3TransformCoord(v2, vp);
		float ex = MaxF(fabsf(XMVectorGetX(ndc1)), fabsf(XMVectorGetX(ndc2)));
		float ey = MaxF(fabsf(XMVectorGetY(ndc1)), fabsf(XMVectorGetY(ndc2)));
		return MaxF(ex, ey);
		};
	float ndcExtent = WorldToNDC();

	// ==== 状態機械 ====
	enum ZoomState { Hold = 0, ZoomOut = 1, ZoomIn = 2 };
	static ZoomState sState = Hold;
	static int       sDwell = 0;
	auto AdvanceState = [&]() {
		switch (sState) {
		case Hold:
			if (ndcExtent > K.softOutNDC) { sState = ZoomOut; sDwell = 0; }
			else if (ndcExtent < K.softInNDC) { sState = ZoomIn; sDwell = 0; }
			break;
		case ZoomOut:
			if (ndcExtent < K.softInNDC) { if (++sDwell >= K.dwellFrames) { sState = Hold; sDwell = 0; } }
			else { sDwell = 0; }
			break;
		case ZoomIn:
			if (ndcExtent > K.softOutNDC) { if (++sDwell >= K.dwellFrames) { sState = Hold; sDwell = 0; } }
			else { sDwell = 0; }
			break;
		}
		};
	AdvanceState();

	// ==== 距離の制約 ====
	const float pitchRad = XMConvertToRadians(K.pitchDeg);
	const float sinP = sinf(pitchRad), cosP = cosf(pitchRad);

	float allowedMinD = K.minDist;
	if (K.minHeightY > center.y && sinP > 1e-4f)
		allowedMinD = MaxF(allowedMinD, (K.minHeightY - center.y) / sinP);
	baseFitDist = MaxF(baseFitDist, allowedMinD);

	float allowedMaxD = (K.maxY > center.y && sinP > 1e-4f) ? ((K.maxY - center.y) / sinP) : baseFitDist;

	// ==== 近距離バイアス ====
	static bool sCloseBias = false;
	if (!sCloseBias && ndcExtent < (K.nudgeInNDC - K.nudgeHyst)) sCloseBias = true;
	if (sCloseBias && ndcExtent > (K.nudgeInNDC + K.nudgeHyst)) sCloseBias = false;

	float desiredFitDist = baseFitDist;
	if (sCloseBias) desiredFitDist = ClampF(baseFitDist * K.nudgeFactor, allowedMinD, allowedMaxD);

	// ==== 目標距離決定 ====
	float targetDistance = currentDistance;
	if (sState == ZoomOut) {
		targetDistance = MaxF(currentDistance, MinF(baseFitDist, allowedMaxD));
	}
	else if (sState == ZoomIn) {
		targetDistance = MinF(currentDistance, MaxF(allowedMinD, desiredFitDist));
	}
	else { // Hold
		if (sCloseBias) targetDistance = MinF(targetDistance, desiredFitDist);
	}

	// ==== 平滑化状態 ====
	static bool     sInit = false;
	static XMFLOAT3 sCenter = {};
	static float    sDistance = 0.0f;
	if (!sInit) {
		sCenter = center;
		sDistance = ClampF(currentDistance, allowedMinD, MaxF(allowedMinD, baseFitDist));
		sInit = true;
	}

	// ==== 平行移動(パン) ====
	sCenter = Lerp3(sCenter, center, K.panLerp);

	// ==== ズーム(片側制限あり) ====
	float zoomT = (sState == ZoomOut) ? K.zoomLerpOut : (sState == ZoomIn ? K.zoomLerpIn : 0.0f);
	if ((sState == Hold || sState == ZoomIn) && targetDistance < sDistance)
		zoomT = MaxF(zoomT, K.holdZoomInLerp);

	// ズームアウト時のみ1フレーム上限を適用
	{
		float lerped = LerpF(sDistance, targetDistance, zoomT);
		if (sState == ZoomOut) {
			float step = lerped - sDistance;
			step = ClampF(step, 0.0f, K.maxZoomOutStep);
			sDistance += step;
		}
		else {
			sDistance = lerped;
		}
	}
	sDistance = MaxF(sDistance, allowedMinD);

	// ==== 位置構築 ====
	XMVECTOR centerSV = XMLoadFloat3(&sCenter);
	XMVECTOR offset = XMVectorAdd(
		XMVectorScale(forwardXZ, -cosP * sDistance),
		XMVectorSet(0.0f, sinP * sDistance, 0.0f, 0.0f)
	);
	XMVECTOR desiredPosV = XMVectorAdd(centerSV, offset);

	// 高さクランプ
	XMFLOAT3 desiredPos; XMStoreFloat3(&desiredPos, desiredPosV);
	desiredPos.y = ClampF(desiredPos.y, K.minHeightY, K.maxY);

	// ==== 適用 ====
	g_Camera.at = sCenter;
	g_Camera.pos = desiredPos;

	SetCamera();
}


// 追加: ビュー×プロジェクション行列を取得する
XMMATRIX GetCameraViewProjMatrix(void)
{
	// 必要なら#include <DirectXMath.h>も！
	XMMATRIX view = XMLoadFloat4x4(&g_Camera.mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&g_Camera.mtxProjection);
	return view * proj;
}
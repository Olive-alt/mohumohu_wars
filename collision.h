//=============================================================================
//
// 当たり判定処理 [collision.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
BOOL CollisionBB(XMFLOAT3 mpos, XMFLOAT3 ypos, XMFLOAT3 msize, XMFLOAT3 ysize);
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2);

float dotProduct(XMVECTOR *v1, XMVECTOR *v2);
void crossProduct(XMVECTOR *ret, XMVECTOR *v1, XMVECTOR *v2);
BOOL RayCast(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 pos0, XMFLOAT3 pos1, XMFLOAT3 *hit, XMFLOAT3 *normal);

BOOL CollisionCapsule(const XMFLOAT3& p1a, const XMFLOAT3& p1b, float r1, const XMFLOAT3& p2a, const XMFLOAT3& p2b, float r2);
BOOL CollisionCapsuleSphere(const XMFLOAT3& capA, const XMFLOAT3& capB, float capRadius, const XMFLOAT3& spherePos, float sphereRadius);



//=============================================================================
//
// 当たり判定処理 [collision.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "collision.h"
#include "debugline.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// 構造体定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************


//=============================================================================
// BBによる当たり判定処理
// 回転は考慮しない
// 戻り値：当たってたらTRUE
//=============================================================================
//               ポジション１　　ポジション2　　サイズ1　　　　　サイズ2
BOOL CollisionBB(XMFLOAT3 mpos, XMFLOAT3 ypos, XMFLOAT3 msize, XMFLOAT3 ysize)
{
	BOOL ans = FALSE;	// 外れをセットしておく

	// 座標が中心点なので計算しやすく半分にしている
	msize.x /= 2;
	msize.y /= 2;
	msize.z /= 2;
	ysize.x /= 2;
	ysize.y /= 2;
	ysize.z /= 2;

	// バウンディングボックス(BB)の処理
	if ((mpos.x + msize.x > ypos.x - ysize.x) &&
		(mpos.x - msize.x < ypos.x + ysize.x) &&
		(mpos.y + msize.y > ypos.y - ysize.y) &&
		(mpos.y - msize.y < ypos.y + ysize.y) &&
		(mpos.z + msize.z > ypos.z - ysize.z) &&
		(mpos.z - msize.z < ypos.z + ysize.z))
	{
		// 当たった時の処理
		ans = TRUE;
	}

	return ans;
}

//=============================================================================
// BCによる当たり判定処理
// サイズは半径
// 戻り値：当たってたらTRUE
//=============================================================================
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2)
{
	BOOL ans = FALSE;						// 外れをセットしておく

	float len = (r1 + r2) * (r1 + r2);		// 半径を2乗した物
	XMVECTOR temp = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos2);
	temp = XMVector3LengthSq(temp);			// 2点間の距離（2乗した物）
	float lenSq = 0.0f;
	XMStoreFloat(&lenSq, temp);

	// 半径を2乗した物より距離が短い？
	if (len > lenSq)
	{
		ans = TRUE;	// 当たっている
	}

	return ans;
}


//=============================================================================
// 内積(dot)
//=============================================================================
float dotProduct(XMVECTOR *v1, XMVECTOR *v2)
{
#if 0
	float ans = v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
#else
	// ダイレクトＸでは、、、
	XMVECTOR temp = XMVector3Dot(*v1, *v2);
	float ans = 0.0f;
	XMStoreFloat(&ans, temp);
#endif

	return(ans);
}


//=============================================================================
// 外積(cross)
//=============================================================================
void crossProduct(XMVECTOR *ret, XMVECTOR *v1, XMVECTOR *v2)
{
#if 0
	ret->x = v1->y * v2->z - v1->z * v2->y;
	ret->y = v1->z * v2->x - v1->x * v2->z;
	ret->z = v1->x * v2->y - v1->y * v2->x;
#else
	// ダイレクトＸでは、、、
	*ret = XMVector3Cross(*v1, *v2);
#endif

}


//=============================================================================
// レイキャスト
// p0, p1, p2　ポリゴンの３頂点
// pos0 始点
// pos1 終点
// hit　交点の返却用
// normal 法線ベクトルの返却用
// 当たっている場合、TRUEを返す
//=============================================================================
BOOL RayCast(XMFLOAT3 xp0, XMFLOAT3 xp1, XMFLOAT3 xp2, XMFLOAT3 xpos0, XMFLOAT3 xpos1, XMFLOAT3 *hit, XMFLOAT3 *normal)
{
	XMVECTOR	p0   = XMLoadFloat3(&xp0);
	XMVECTOR	p1   = XMLoadFloat3(&xp1);
	XMVECTOR	p2   = XMLoadFloat3(&xp2);
	XMVECTOR	pos0 = XMLoadFloat3(&xpos0);
	XMVECTOR	pos1 = XMLoadFloat3(&xpos1);

	XMVECTOR	nor;	// ポリゴンの法線
	XMVECTOR	vec1;
	XMVECTOR	vec2;
	float		d1, d2;

	{	// ポリゴンの外積をとって法線を求める(この処理は固定物なら予めInit()で行っておくと良い)
		vec1 = p1 - p0;
		vec2 = p2 - p0;
		crossProduct(&nor, &vec2, &vec1);
		nor = XMVector3Normalize(nor);		// 計算しやすいように法線をノーマライズしておく(このベクトルの長さを１にしている)
		XMStoreFloat3(normal, nor);			// 求めた法線を入れておく
	}

	// ポリゴン平面と線分の内積とって衝突している可能性を調べる（鋭角なら＋、鈍角ならー、直角なら０）
	vec1 = pos0 - p0;
	vec2 = pos1 - p0;
	{	// 求めたポリゴンの法線と２つのベクトル（線分の両端とポリゴン上の任意の点）の内積とって衝突している可能性を調べる
		d1 = dotProduct(&vec1, &nor);
		d2 = dotProduct(&vec2, &nor);
		if (((d1 * d2) > 0.0f) || (d1 == 0 && d2 == 0))
		{
			// 当たっている可能性は無い
			return(FALSE);
		}
	}


	{	// ポリゴンと線分の交点を求める
		d1 = (float)fabs(d1);	// 絶対値を求めている
		d2 = (float)fabs(d2);	// 絶対値を求めている
		float a = d1 / (d1 + d2);							// 内分比

		XMVECTOR	vec3 = (1 - a) * vec1 + a * vec2;		// p0から交点へのベクトル
		XMVECTOR	p3 = p0 + vec3;							// 交点
		XMStoreFloat3(hit, p3);								// 求めた交点を入れておく

		{	// 求めた交点がポリゴンの中にあるか調べる

			// ポリゴンの各辺のベクトル
			XMVECTOR	v1 = p1 - p0;
			XMVECTOR	v2 = p2 - p1;
			XMVECTOR	v3 = p0 - p2;

			// 各頂点と交点とのベクトル
			XMVECTOR	v4 = p3 - p1;
			XMVECTOR	v5 = p3 - p2;
			XMVECTOR	v6 = p3 - p0;

			// 外積で各辺の法線を求めて、ポリゴンの法線との内積をとって符号をチェックする
			XMVECTOR	n1, n2, n3;

			crossProduct(&n1, &v4, &v1);
			if (dotProduct(&n1, &nor) < 0.0f) return(FALSE);	// 当たっていない

			crossProduct(&n2, &v5, &v2);
			if (dotProduct(&n2, &nor) < 0.0f) return(FALSE);	// 当たっていない
			
			crossProduct(&n3, &v6, &v3);
			if (dotProduct(&n3, &nor) < 0.0f) return(FALSE);	// 当たっていない
		}
	}

	return(TRUE);	// 当たっている！(hitには当たっている交点が入っている。normalには法線が入っている)
}


//=============================================================================
// カプセル同士の当たり判定処理
// p1a, p1b: カプセル1の端点
// r1: カプセル1の半径
// p2a, p2b: カプセル2の端点
// r2: カプセル2の半径
// 戻り値：当たってたらTRUE
//=============================================================================
BOOL CollisionCapsule(
	const XMFLOAT3& p1a, const XMFLOAT3& p1b, float r1,
	const XMFLOAT3& p2a, const XMFLOAT3& p2b, float r2)
{
	// 線分間の最近接点を求める
	XMVECTOR a0 = XMLoadFloat3(&p1a);
	XMVECTOR a1 = XMLoadFloat3(&p1b);
	XMVECTOR b0 = XMLoadFloat3(&p2a);
	XMVECTOR b1 = XMLoadFloat3(&p2b);

	// 公式より: 線分a(s), 線分b(t), s,t∈[0,1]
	XMVECTOR d1 = a1 - a0; // カプセル1の線分
	XMVECTOR d2 = b1 - b0; // カプセル2の線分
	XMVECTOR r = a0 - b0;

	float a = XMVectorGetX(XMVector3Dot(d1, d1)); // |d1|^2
	float e = XMVectorGetX(XMVector3Dot(d2, d2)); // |d2|^2
	float f = XMVectorGetX(XMVector3Dot(d2, r));

	float s, t;
	if (a <= 1e-6f && e <= 1e-6f) {
		// 両方とも点
		s = t = 0.0f;
	}
	else if (a <= 1e-6f) {
		// カプセル1が点
		s = 0.0f;
		t = f / e;
		t = max(0.0f, min(1.0f, t));
	}
	else {
		float c = XMVectorGetX(XMVector3Dot(d1, r));
		if (e <= 1e-6f) {
			// カプセル2が点
			t = 0.0f;
			s = -c / a;
			s = max(0.0f, min(1.0f, s));
		}
		else {
			float b = XMVectorGetX(XMVector3Dot(d1, d2));
			float denom = a * e - b * b;
			if (denom != 0.0f) {
				s = (b * f - c * e) / denom;
				s = max(0.0f, min(1.0f, s));
			}
			else {
				s = 0.0f;
			}
			t = (b * s + f) / e;
			t = max(0.0f, min(1.0f, t));
		}
	}
	// 最近接点
	XMVECTOR c1 = a0 + d1 * s;
	XMVECTOR c2 = b0 + d2 * t;
	XMVECTOR diff = c1 - c2;
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
	float rSum = r1 + r2;

	return distSq <= rSum * rSum;
}

//=============================================================================
// カプセルと球の当たり判定処理
// カプセル線分 capA-capB, 半径 capRadius
// 球中心 spherePos, 半径 sphereRadius
//=============================================================================
BOOL CollisionCapsuleSphere(
	const XMFLOAT3& capA,
	const XMFLOAT3& capB,
	float capRadius,
	const XMFLOAT3& spherePos,
	float sphereRadius
)
{
	// カプセルの線分ab
	XMVECTOR a = XMLoadFloat3(&capA);
	XMVECTOR b = XMLoadFloat3(&capB);
	XMVECTOR p = XMLoadFloat3(&spherePos);
	XMVECTOR ab = b - a;
	XMVECTOR ap = p - a;

	float abLenSq = XMVectorGetX(XMVector3LengthSq(ab));
	float t = 0.0f;

	if (abLenSq > 0.0f) {
		// 球の中心pからカプセル線分abへの最近接点を求める
		t = XMVectorGetX(XMVector3Dot(ap, ab)) / abLenSq;
		t = max(0.0f, min(1.0f, t)); // [0,1]の範囲にクランプ
	}

	XMVECTOR closest = a + ab * t; // 線分上の最近接点
	XMVECTOR diff = p - closest;
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
	float sumR = capRadius + sphereRadius;

	return (distSq <= sumR * sumR);
}


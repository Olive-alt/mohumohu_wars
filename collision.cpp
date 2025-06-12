//=============================================================================
//
// �����蔻�菈�� [collision.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "collision.h"
#include "debugline.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************


//*****************************************************************************
// �\���̒�`
//*****************************************************************************


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************


//=============================================================================
// BB�ɂ�铖���蔻�菈��
// ��]�͍l�����Ȃ�
// �߂�l�F�������Ă���TRUE
//=============================================================================
//               �|�W�V�����P�@�@�|�W�V����2�@�@�T�C�Y1�@�@�@�@�@�T�C�Y2
BOOL CollisionBB(XMFLOAT3 mpos, XMFLOAT3 ypos, XMFLOAT3 msize, XMFLOAT3 ysize)
{
	BOOL ans = FALSE;	// �O����Z�b�g���Ă���

	// ���W�����S�_�Ȃ̂Ōv�Z���₷�������ɂ��Ă���
	msize.x /= 2;
	msize.y /= 2;
	msize.z /= 2;
	ysize.x /= 2;
	ysize.y /= 2;
	ysize.z /= 2;

	// �o�E���f�B���O�{�b�N�X(BB)�̏���
	if ((mpos.x + msize.x > ypos.x - ysize.x) &&
		(mpos.x - msize.x < ypos.x + ysize.x) &&
		(mpos.y + msize.y > ypos.y - ysize.y) &&
		(mpos.y - msize.y < ypos.y + ysize.y) &&
		(mpos.z + msize.z > ypos.z - ysize.z) &&
		(mpos.z - msize.z < ypos.z + ysize.z))
	{
		// �����������̏���
		ans = TRUE;
	}

	return ans;
}

//=============================================================================
// BC�ɂ�铖���蔻�菈��
// �T�C�Y�͔��a
// �߂�l�F�������Ă���TRUE
//=============================================================================
BOOL CollisionBC(XMFLOAT3 pos1, XMFLOAT3 pos2, float r1, float r2)
{
	BOOL ans = FALSE;						// �O����Z�b�g���Ă���

	float len = (r1 + r2) * (r1 + r2);		// ���a��2�悵����
	XMVECTOR temp = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos2);
	temp = XMVector3LengthSq(temp);			// 2�_�Ԃ̋����i2�悵�����j
	float lenSq = 0.0f;
	XMStoreFloat(&lenSq, temp);

	// ���a��2�悵������苗�����Z���H
	if (len > lenSq)
	{
		ans = TRUE;	// �������Ă���
	}

	return ans;
}


//=============================================================================
// ����(dot)
//=============================================================================
float dotProduct(XMVECTOR *v1, XMVECTOR *v2)
{
#if 0
	float ans = v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
#else
	// �_�C���N�g�w�ł́A�A�A
	XMVECTOR temp = XMVector3Dot(*v1, *v2);
	float ans = 0.0f;
	XMStoreFloat(&ans, temp);
#endif

	return(ans);
}


//=============================================================================
// �O��(cross)
//=============================================================================
void crossProduct(XMVECTOR *ret, XMVECTOR *v1, XMVECTOR *v2)
{
#if 0
	ret->x = v1->y * v2->z - v1->z * v2->y;
	ret->y = v1->z * v2->x - v1->x * v2->z;
	ret->z = v1->x * v2->y - v1->y * v2->x;
#else
	// �_�C���N�g�w�ł́A�A�A
	*ret = XMVector3Cross(*v1, *v2);
#endif

}


//=============================================================================
// ���C�L���X�g
// p0, p1, p2�@�|���S���̂R���_
// pos0 �n�_
// pos1 �I�_
// hit�@��_�̕ԋp�p
// normal �@���x�N�g���̕ԋp�p
// �������Ă���ꍇ�ATRUE��Ԃ�
//=============================================================================
BOOL RayCast(XMFLOAT3 xp0, XMFLOAT3 xp1, XMFLOAT3 xp2, XMFLOAT3 xpos0, XMFLOAT3 xpos1, XMFLOAT3 *hit, XMFLOAT3 *normal)
{
	XMVECTOR	p0   = XMLoadFloat3(&xp0);
	XMVECTOR	p1   = XMLoadFloat3(&xp1);
	XMVECTOR	p2   = XMLoadFloat3(&xp2);
	XMVECTOR	pos0 = XMLoadFloat3(&xpos0);
	XMVECTOR	pos1 = XMLoadFloat3(&xpos1);

	XMVECTOR	nor;	// �|���S���̖@��
	XMVECTOR	vec1;
	XMVECTOR	vec2;
	float		d1, d2;

	{	// �|���S���̊O�ς��Ƃ��Ė@�������߂�(���̏����͌Œ蕨�Ȃ�\��Init()�ōs���Ă����Ɨǂ�)
		vec1 = p1 - p0;
		vec2 = p2 - p0;
		crossProduct(&nor, &vec2, &vec1);
		nor = XMVector3Normalize(nor);		// �v�Z���₷���悤�ɖ@�����m�[�}���C�Y���Ă���(���̃x�N�g���̒������P�ɂ��Ă���)
		XMStoreFloat3(normal, nor);			// ���߂��@�������Ă���
	}

	// �|���S�����ʂƐ����̓��ςƂ��ďՓ˂��Ă���\���𒲂ׂ�i�s�p�Ȃ�{�A�݊p�Ȃ�[�A���p�Ȃ�O�j
	vec1 = pos0 - p0;
	vec2 = pos1 - p0;
	{	// ���߂��|���S���̖@���ƂQ�̃x�N�g���i�����̗��[�ƃ|���S����̔C�ӂ̓_�j�̓��ςƂ��ďՓ˂��Ă���\���𒲂ׂ�
		d1 = dotProduct(&vec1, &nor);
		d2 = dotProduct(&vec2, &nor);
		if (((d1 * d2) > 0.0f) || (d1 == 0 && d2 == 0))
		{
			// �������Ă���\���͖���
			return(FALSE);
		}
	}


	{	// �|���S���Ɛ����̌�_�����߂�
		d1 = (float)fabs(d1);	// ��Βl�����߂Ă���
		d2 = (float)fabs(d2);	// ��Βl�����߂Ă���
		float a = d1 / (d1 + d2);							// ������

		XMVECTOR	vec3 = (1 - a) * vec1 + a * vec2;		// p0�����_�ւ̃x�N�g��
		XMVECTOR	p3 = p0 + vec3;							// ��_
		XMStoreFloat3(hit, p3);								// ���߂���_�����Ă���

		{	// ���߂���_���|���S���̒��ɂ��邩���ׂ�

			// �|���S���̊e�ӂ̃x�N�g��
			XMVECTOR	v1 = p1 - p0;
			XMVECTOR	v2 = p2 - p1;
			XMVECTOR	v3 = p0 - p2;

			// �e���_�ƌ�_�Ƃ̃x�N�g��
			XMVECTOR	v4 = p3 - p1;
			XMVECTOR	v5 = p3 - p2;
			XMVECTOR	v6 = p3 - p0;

			// �O�ςŊe�ӂ̖@�������߂āA�|���S���̖@���Ƃ̓��ς��Ƃ��ĕ������`�F�b�N����
			XMVECTOR	n1, n2, n3;

			crossProduct(&n1, &v4, &v1);
			if (dotProduct(&n1, &nor) < 0.0f) return(FALSE);	// �������Ă��Ȃ�

			crossProduct(&n2, &v5, &v2);
			if (dotProduct(&n2, &nor) < 0.0f) return(FALSE);	// �������Ă��Ȃ�
			
			crossProduct(&n3, &v6, &v3);
			if (dotProduct(&n3, &nor) < 0.0f) return(FALSE);	// �������Ă��Ȃ�
		}
	}

	return(TRUE);	// �������Ă���I(hit�ɂ͓������Ă����_�������Ă���Bnormal�ɂ͖@���������Ă���)
}


//=============================================================================
// �J�v�Z�����m�̓����蔻�菈��
// p1a, p1b: �J�v�Z��1�̒[�_
// r1: �J�v�Z��1�̔��a
// p2a, p2b: �J�v�Z��2�̒[�_
// r2: �J�v�Z��2�̔��a
// �߂�l�F�������Ă���TRUE
//=============================================================================
BOOL CollisionCapsule(
	const XMFLOAT3& p1a, const XMFLOAT3& p1b, float r1,
	const XMFLOAT3& p2a, const XMFLOAT3& p2b, float r2)
{
	// �����Ԃ̍ŋߐړ_�����߂�
	XMVECTOR a0 = XMLoadFloat3(&p1a);
	XMVECTOR a1 = XMLoadFloat3(&p1b);
	XMVECTOR b0 = XMLoadFloat3(&p2a);
	XMVECTOR b1 = XMLoadFloat3(&p2b);

	// �������: ����a(s), ����b(t), s,t��[0,1]
	XMVECTOR d1 = a1 - a0; // �J�v�Z��1�̐���
	XMVECTOR d2 = b1 - b0; // �J�v�Z��2�̐���
	XMVECTOR r = a0 - b0;

	float a = XMVectorGetX(XMVector3Dot(d1, d1)); // |d1|^2
	float e = XMVectorGetX(XMVector3Dot(d2, d2)); // |d2|^2
	float f = XMVectorGetX(XMVector3Dot(d2, r));

	float s, t;
	if (a <= 1e-6f && e <= 1e-6f) {
		// �����Ƃ��_
		s = t = 0.0f;
	}
	else if (a <= 1e-6f) {
		// �J�v�Z��1���_
		s = 0.0f;
		t = f / e;
		t = max(0.0f, min(1.0f, t));
	}
	else {
		float c = XMVectorGetX(XMVector3Dot(d1, r));
		if (e <= 1e-6f) {
			// �J�v�Z��2���_
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
	// �ŋߐړ_
	XMVECTOR c1 = a0 + d1 * s;
	XMVECTOR c2 = b0 + d2 * t;
	XMVECTOR diff = c1 - c2;
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
	float rSum = r1 + r2;

	return distSq <= rSum * rSum;
}

//=============================================================================
// �J�v�Z���Ƌ��̓����蔻�菈��
// �J�v�Z������ capA-capB, ���a capRadius
// �����S spherePos, ���a sphereRadius
//=============================================================================
BOOL CollisionCapsuleSphere(
	const XMFLOAT3& capA,
	const XMFLOAT3& capB,
	float capRadius,
	const XMFLOAT3& spherePos,
	float sphereRadius
)
{
	// �J�v�Z���̐���ab
	XMVECTOR a = XMLoadFloat3(&capA);
	XMVECTOR b = XMLoadFloat3(&capB);
	XMVECTOR p = XMLoadFloat3(&spherePos);
	XMVECTOR ab = b - a;
	XMVECTOR ap = p - a;

	float abLenSq = XMVectorGetX(XMVector3LengthSq(ab));
	float t = 0.0f;

	if (abLenSq > 0.0f) {
		// ���̒��Sp����J�v�Z������ab�ւ̍ŋߐړ_�����߂�
		t = XMVectorGetX(XMVector3Dot(ap, ab)) / abLenSq;
		t = max(0.0f, min(1.0f, t)); // [0,1]�͈̔͂ɃN�����v
	}

	XMVECTOR closest = a + ab * t; // ������̍ŋߐړ_
	XMVECTOR diff = p - closest;
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
	float sumR = capRadius + sphereRadius;

	return (distSq <= sumR * sumR);
}


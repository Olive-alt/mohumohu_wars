//=============================================================================
//
// �Q�[����ʏ��� [game.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IT_ball.h"
#include "IT_boomerang.h"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitItem(void);
void UninitItem(void);
void UpdateItem(void);
void DrawItem(void);

void CheckHitItem(void);
BALL* GetBall();  // �A�N�Z�X�p�̊֐����쐬
BOOM* GetBoomerang();  // �A�N�Z�X�p�̊֐����쐬
//void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices); //debug�p�̊֐�

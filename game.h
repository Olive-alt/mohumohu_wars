//=============================================================================
//
// �Q�[����ʏ��� [game.h]
// Author : 
//
//=============================================================================
#pragma once
#include "IT_ball.h"


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitGame(void);
void UninitGame(void);
void UpdateGame(void);
void DrawGame(void);

BALL* GetBall();  // �A�N�Z�X�p�̊֐����쐬

void DrawDebugSphereOutline(const XMFLOAT3& center, float radius, const XMFLOAT4& color, int slices); //debug�p�̊֐�

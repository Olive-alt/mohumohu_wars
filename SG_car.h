//=============================================================================
//
// 地面処理 [field.h]
// Author : 
//
//=============================================================================
#pragma once

#include"main.h"
#include "model.h"

// マップ範囲・レーン
#define SG_CAR_LANE_LEFT   10.0f    // 左→右 車線Z座標
#define SG_CAR_LANE_RIGHT -10.0f    // 右→左 車線Z座標
#define SG_CAR_MAP_LEFT   -50.0f
#define SG_CAR_MAP_RIGHT   50.0f
#define SG_CAR_MAX    3
#define SG_CAR_SPEED  1.5f
#define SG_CAR_RADIUS 10.0f

class CarSystem {
public:
    struct Car {
        bool use;
        XMFLOAT3 pos, rot, scl;
        int lane; // 0:左→右 1:右→左
        bool enteredScreen = false;  // 一度でも画面内に入ったか

        void Init(const XMFLOAT3& startPos, int lane_);
        void Update();
        void Draw(const DX11_MODEL* model) const;
        bool CheckHit(const XMFLOAT3& targetPos, float radius) const;
    };

    Car cars[SG_CAR_MAX];
    DX11_MODEL carModel;

    void Init();
    void Uninit();
    void Update();
    void Draw();
    void SpawnCar();
    void CheckCarHitPlayers();
};

extern CarSystem g_CarSystem;

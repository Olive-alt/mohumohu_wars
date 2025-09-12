#pragma once

#include "main.h"
#include "model.h"

// マップ範囲
#define SG_CAR_MAP_LEFT   -50.0f
#define SG_CAR_MAP_RIGHT   50.0f

// 同時台数・挙動
#define SG_CAR_MAX            24
#define SG_CAR_SPEED          1.5f
#define SG_CAR_RADIUS         30.0f
#define SG_CAR_WALL_W         32.0f
#define SG_CAR_WALL_H         18.0f
#define SG_CAR_WALL_D          6.0f
#define SG_CAR_FRONT_W        20.0f
#define SG_CAR_FRONT_D        20.0f
#define SG_CAR_KIND_MAX        5

// スポーン設定
#define SG_CAR_SPAWN_TRIES     1
#define SG_CAR_SPAWN_CHANCE   100   // 1/50

// ── 下側道路の2レーン（従来値）
#define SG_CAR_LANE_LEFT     90.0f     // 左→右のZ
#define SG_CAR_LANE_RIGHT   -10.0f     // 右→左のZ

// ── 上側道路の2レーン（下側からのオフセット）
#define SG_CAR_LANE2_OFFSET  60.0f     // 位置がズレるならここだけ調整
#define SG_CAR_LANE2_LEFT    360.0f     // 左→右のZ（上側）
#define SG_CAR_LANE2_RIGHT    460.0f     // 右→左のZ（上側）
class CarSystem {
public:
    struct Car {
        bool     use;
        XMFLOAT3 pos, rot, scl;
        int      lane;     // 0:左→右  1:右→左
        int      kind;     // 見た目の種類
        bool     enteredScreen = false;

        void Init(const XMFLOAT3& startPos, int lane_);
        void Update();
        void Draw(const DX11_MODEL* model) const;
        bool CheckHit(const XMFLOAT3& targetPos, float radius) const;
        bool CheckHitFront(const XMFLOAT3& targetPos, float radius) const;
    };

    Car        cars[SG_CAR_MAX];
    DX11_MODEL carModel[SG_CAR_KIND_MAX];

    void Init();
    void Uninit();
    void Update();
    void Draw();
    void SpawnCar();
    void CheckCarHitPlayers();
};

extern CarSystem g_CarSystem;

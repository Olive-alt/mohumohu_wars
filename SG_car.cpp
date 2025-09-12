#include "main.h"
#include "SG_car.h"
#include "renderer.h"
#include "player.h"
#include "debugproc.h"
#include "collision.h"
#include "sound.h"
#include "stage_select.h"
#include <string.h>

CarSystem g_CarSystem;
PLAYER* car_player = GetPlayer();

// 見た目モデル
static const char* CAR_MODEL[SG_CAR_KIND_MAX] = {
    "data/MODEL/car/car.obj",
    "data/MODEL/car/car2.obj",
    "data/MODEL/car/car3.obj",
    "data/MODEL/car/car4.obj",
    "data/MODEL/car/car5.obj",
};

// roadステージのみ稼働
static inline bool IsRoadStage() {
    return (strcmp(g_SelectedStageFile, "stage_road.txt") == 0);
}

// ───────────── Car ─────────────
void CarSystem::Car::Init(const XMFLOAT3& startPos, int lane_) {
    use = true;
    pos = startPos;
    lane = lane_;
    rot = XMFLOAT3(0, (lane == 0) ? +XM_PIDIV2 : -XM_PIDIV2, 0);
    scl = XMFLOAT3(1, 1, 1);
    enteredScreen = false;
    kind = GetRand(0, SG_CAR_KIND_MAX - 1);
}

void CarSystem::Car::Update() {
    if (!use) return;

    pos.x += (lane == 0 ? +SG_CAR_SPEED : -SG_CAR_SPEED);

    if (!enteredScreen) {
        if (pos.x >= MAP_LEFT && pos.x <= MAP_RIGHT &&
            pos.z >= MAP_DOWN && pos.z <= MAP_TOP) {
            enteredScreen = true;
        }
    }
    else {
        if (pos.x < MAP_LEFT || pos.x > MAP_RIGHT ||
            pos.z < MAP_DOWN || pos.z > MAP_TOP) {
            use = false;
        }
    }
}

void CarSystem::Car::Draw(const DX11_MODEL* model) const {
    if (!use) return;
    extern unsigned int dwFrameCount;
    float t = dwFrameCount * (1.0f / 60.0f) + pos.x * 0.03f;

    float shakeY = sinf(t * 6.0f) * 0.8f;
    float rollZ = sinf(t * 4.2f) * 0.09f;

    XMMATRIX mtxWorld =
        XMMatrixScaling(scl.x, scl.y, scl.z) *
        XMMatrixRotationRollPitchYaw(rot.x + shakeY * 0.03f, rot.y, rot.z + rollZ) *
        XMMatrixTranslation(pos.x, pos.y + shakeY, pos.z);

    SetWorldMatrix(&mtxWorld);
    DrawModel((DX11_MODEL*)model);
}

bool CarSystem::Car::CheckHit(const XMFLOAT3& targetPos, float radius) const {
    if (!use) return false;
    float dx = pos.x - targetPos.x;
    float dz = pos.z - targetPos.z;
    float distSq = dx * dx + dz * dz;
    float minDist = SG_CAR_RADIUS + radius;
    return distSq <= (minDist * minDist);
}

bool CarSystem::Car::CheckHitFront(const XMFLOAT3& targetPos, float radius) const {
    if (!use) return false;
    float dx = targetPos.x - pos.x;
    float dz = targetPos.z - pos.z;
    float dirX = (lane == 0) ? +1.0f : -1.0f;
    float dot = dx * dirX + dz * 0.0f;
    if (dot <= 0.0f) return false;

    float distSq = dx * dx + dz * dz;
    float minDist = SG_CAR_RADIUS + radius;
    return distSq <= (minDist * minDist);
}

// ───────────── CarSystem ─────────────
void CarSystem::Init() {
    for (int k = 0; k < SG_CAR_KIND_MAX; ++k) {
        LoadModel(CAR_MODEL[k], &carModel[k]);
    }
    for (int i = 0; i < SG_CAR_MAX; ++i) {
        cars[i].use = false;
    }
}

void CarSystem::Uninit() {
    for (int i = 0; i < SG_CAR_MAX; ++i) cars[i].use = false;
    for (int k = 0; k < SG_CAR_KIND_MAX; ++k) {
        UnloadModel(&carModel[k]);
        ZeroMemory(&carModel[k], sizeof(carModel[k]));
    }
}

// 指定レーンに1台スポーン（上下2本の道路対応）
void CarSystem::SpawnCar() {
    if (!IsRoadStage()) return;

    for (int i = 0; i < SG_CAR_MAX; ++i) {
        if (!cars[i].use) {
            // 進行方向（逆走あり）と、上下どちらの道路か
            int dir = GetRand(0, 1);   // 0:左→右 / 1:右→左
            int road = GetRand(0, 1);   // 0:下の道路 / 1:上の道路

            float spawnX = (dir == 0) ? (SG_CAR_MAP_LEFT - 250.0f)
                : (SG_CAR_MAP_RIGHT + 250.0f);

            float spawnZ;
            if (road == 0) {
                // 下側
                spawnZ = (dir == 0) ? SG_CAR_LANE_LEFT : SG_CAR_LANE_RIGHT;
            }
            else {
                // 上側
                spawnZ = (dir == 0) ? SG_CAR_LANE2_LEFT : SG_CAR_LANE2_RIGHT;
            }

            // 安全策：万一はみ出していてもMAP内へ収める
            if (spawnZ < MAP_DOWN + 1.0f) spawnZ = MAP_DOWN + 1.0f;
            if (spawnZ > MAP_TOP - 1.0f) spawnZ = MAP_TOP - 1.0f;

            cars[i].Init(XMFLOAT3(spawnX, 0.0f, spawnZ), dir);
            break;
        }
    }
}

void CarSystem::Update() {
    if (!IsRoadStage()) {
        for (int i = 0; i < SG_CAR_MAX; ++i) cars[i].use = false;
        return;
    }

    for (int i = 0; i < SG_CAR_MAX; ++i) cars[i].Update();

    // 密度アップ：1フレームに複数回抽選
    for (int t = 0; t < SG_CAR_SPAWN_TRIES; ++t) {
        if (GetRand(0, SG_CAR_SPAWN_CHANCE) == 0) {
            int active = 0;
            for (int i = 0; i < SG_CAR_MAX; ++i) if (cars[i].use) active++;
            if (active < SG_CAR_MAX) SpawnCar();
        }
    }
}

void CarSystem::Draw() {
    if (!IsRoadStage()) return;
    for (int i = 0; i < SG_CAR_MAX; ++i) {
        if (cars[i].use) cars[i].Draw(&carModel[cars[i].kind]);
    }
}

void CarSystem::CheckCarHitPlayers() {
    if (!IsRoadStage()) return;

    for (int i = 0; i < SG_CAR_MAX; ++i) {
        Car& car = cars[i];
        if (!car.use) continue;

        for (int p = 0; p < MAX_PLAYER; ++p) {
            if (!car_player[p].use || car_player[p].stunTimer > 0.0f) continue;

            // 前方：吹っ飛ばし
            float offset = (SG_CAR_WALL_D / 2.0f) + (SG_CAR_FRONT_D / 2.0f);
            float frontX = car.pos.x + ((car.lane == 0) ? offset : -offset);
            XMFLOAT3 frontBoxPos = { frontX, car.pos.y, car.pos.z };

            bool isFront = (car.lane == 0)
                ? (car_player[p].pos.x > car.pos.x)
                : (car_player[p].pos.x < car.pos.x);

            if (isFront && CollisionRect2D_XZ(frontBoxPos, car_player[p].pos,
                SG_CAR_FRONT_W, SG_CAR_FRONT_D))
            {
                float baseDir = (car.lane == 0) ? 0.0f : XM_PI;
                float ang = baseDir + GetRandf(-0.5f, 0.5f);
                float v = 12.0f;
                car_player[p].knockbackVel.x = v * cosf(ang);
                car_player[p].knockbackVel.z = v * sinf(ang);
                car_player[p].stunTimer = 1.0f;
                PrintDebugProc("車前方ヒット：吹っ飛ばし\n");
                PlaySound(SOUND_LABEL_SE_shot004);
            }
            else {
                // 側面：押し戻し
                float wallBoxX = car.pos.x + ((car.lane == 0)
                    ? -SG_CAR_WALL_D / 4.0f : SG_CAR_WALL_D / 4.0f);
                XMFLOAT3 wallBoxPos = { wallBoxX, car.pos.y, car.pos.z };

                if (CollisionBB(wallBoxPos, car_player[p].pos,
                    XMFLOAT3(SG_CAR_WALL_W, SG_CAR_WALL_H, SG_CAR_WALL_D),
                    XMFLOAT3(car_player[p].size, SG_CAR_WALL_H, car_player[p].size)))
                {
                    float dx = car_player[p].pos.x - car.pos.x;
                    float dz = car_player[p].pos.z - car.pos.z;
                    float len = sqrtf(dx * dx + dz * dz);
                    if (len > 0.0001f) {
                        float push = (SG_CAR_WALL_W / 2.0f) + car_player[p].size + 0.5f;
                        dx /= len; dz /= len;
                        car_player[p].pos.x = car.pos.x + dx * push;
                        car_player[p].pos.z = car.pos.z + dz * push;
                    }
                    car_player[p].stunTimer = 0.0f;
                    car_player[p].knockbackVel = { 0,0,0 };
                    PrintDebugProc("車横ヒット：壁判定\n");
                    PlaySound(SOUND_LABEL_SE_shot004);
                }
            }
        }
    }
}

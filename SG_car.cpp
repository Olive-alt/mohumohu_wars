#include "SG_car.h"
#include "main.h"
#include "renderer.h"
#include "player.h"

// グローバルインスタンス
CarSystem g_CarSystem;
PLAYER* car_player = GetPlayer();
// モデルファイル
#define SG_CAR_MODEL_PATH "data/MODEL/car.obj"

// -------------------------
// Carメンバ関数
// -------------------------

// 生成＆初期化
void CarSystem::Car::Init(const XMFLOAT3& startPos, int lane_)
{
    use = true;
    pos = startPos;
    lane = lane_;
    rot = XMFLOAT3(0, (lane == 0) ? +XM_PIDIV2 : -XM_PIDIV2, 0);
    scl = XMFLOAT3(1, 1, 1);
    enteredScreen = false; // フラグ初期化
}

// 位置更新・削除判定
void CarSystem::Car::Update()
{
    if (!use) return;

    pos.x += (lane == 0 ? +SG_CAR_SPEED : -SG_CAR_SPEED);

    // 一度でも画面内に入ったか判定
    if (!enteredScreen) {
        if (pos.x >= MAP_LEFT && pos.x <= MAP_RIGHT &&
            pos.z >= MAP_DOWN && pos.z <= MAP_TOP)
        {
            enteredScreen = true;
        }
    }
    // 画面内に入った後、再び画面外に出たら消す
    else {
        if (pos.x < MAP_LEFT || pos.x > MAP_RIGHT ||
            pos.z < MAP_DOWN || pos.z > MAP_TOP)
        {
            use = false;
        }
    }
}

// 描画
void CarSystem::Car::Draw(const DX11_MODEL* model) const
{
    if (!use) return;
    XMMATRIX mtxWorld =
        XMMatrixScaling(scl.x, scl.y, scl.z) *
        XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) *
        XMMatrixTranslation(pos.x, pos.y, pos.z);

    SetWorldMatrix(&mtxWorld);
    DrawModel((DX11_MODEL*)model);
}

// 当たり判定
bool CarSystem::Car::CheckHit(const XMFLOAT3& targetPos, float radius) const
{
    if (!use) return false;
    float dx = pos.x - targetPos.x;
    float dz = pos.z - targetPos.z;
    float distSq = dx * dx + dz * dz;
    float minDist = SG_CAR_RADIUS + radius;
    return distSq <= (minDist * minDist);
}

// -------------------------
// CarSystem本体
// -------------------------

// 初期化
void CarSystem::Init()
{
    // モデルの読み込み
    LoadModel(SG_CAR_MODEL_PATH, &carModel);

    // 全車リセット
    for (int i = 0; i < SG_CAR_MAX; ++i)
        cars[i].use = false;
}

// 終了・モデル解放
void CarSystem::Uninit()
{
    UnloadModel(&carModel);
}

// 車生成（空いてるスロットにランダムで左右いずれか生成）
void CarSystem::SpawnCar()
{
    for (int i = 0; i < SG_CAR_MAX; ++i)
    {
        if (!cars[i].use)
        {
            int lane = GetRand(0, 1); // 0:左→右 1:右→左
            float spawnX = (lane == 0) ? SG_CAR_MAP_LEFT - 250.0f : SG_CAR_MAP_RIGHT + 250.0f;
            float spawnZ = (lane == 0) ? SG_CAR_LANE_LEFT : SG_CAR_LANE_RIGHT;
            cars[i].Init(XMFLOAT3(spawnX, 0.0f, spawnZ), lane);
            break;
        }
    }
}

// 更新
void CarSystem::Update()
{
    // 車の個別更新
    for (int i = 0; i < SG_CAR_MAX; ++i)
        cars[i].Update();

    // ランダムで車を出現させる（1/60フレームで約3秒に1回出現想定）
    if (GetRand(0, 300) == 0)
    {
        // 台数制限
        int active = 0;
        for (int i = 0; i < SG_CAR_MAX; ++i)
            if (cars[i].use) active++;
        if (active < SG_CAR_MAX)
            SpawnCar();
    }
}

// 描画
void CarSystem::Draw()
{
    for (int i = 0; i < SG_CAR_MAX; ++i)
        cars[i].Draw(&carModel);
}

void CarSystem::CheckCarHitPlayers()
{
    for (int i = 0; i < SG_CAR_MAX; ++i)
    {
        Car& car = cars[i];
        if (!car.use) continue;

        for (int p = 0; p < MAX_PLAYER; ++p)
        {
            if (!car_player[p].use || car_player[p].stunTimer > 0.0f)
                continue;

            if (car.CheckHit(car_player[p].pos, car_player[p].radius))
            {
                // 進行方向（lane=0:+X, lane=1:-X）
                float baseDir = (car.lane == 0) ? 0.0f : 180.0f; // degree

                // ±30度ランダム
                float angleOffset = GetRandf(-30.0f, 30.0f); // -30～+30度
                float angle = (baseDir + angleOffset) * XM_PI / 180.0f; // ラジアン変換

                float knockbackSpeed = 10.0f;
                car_player[p].knockbackVel.x = knockbackSpeed * cosf(angle);
                car_player[p].knockbackVel.y = 0.0f;
                car_player[p].knockbackVel.z = knockbackSpeed * sinf(angle);

                car_player[p].stunTimer = 1.0f; // スタン時間（例）
            }
        }
    }
}
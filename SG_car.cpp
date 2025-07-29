#include "SG_car.h"
#include "main.h"
#include "renderer.h"
#include "player.h"
#include "debugproc.h"
#include "collision.h"

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

    // --- 走行演出：バウンド＆ローリング ---
    extern unsigned int dwFrameCount;
    float t = dwFrameCount * (1.0f / 60.0f) + pos.x * 0.03f; // 個体ごとに揺れをズラす

    // 上下バウンド（走行感アップ）
    float shakeY = sinf(t * 6.0f) * 0.8f;
    // 左右ローリング
    float rollZ = sinf(t * 4.2f) * 0.09f;

    // モデルワールド行列
    XMMATRIX mtxWorld =
        XMMatrixScaling(scl.x, scl.y, scl.z) *
        XMMatrixRotationRollPitchYaw(rot.x + shakeY * 0.03f, rot.y, rot.z + rollZ) *
        XMMatrixTranslation(pos.x, pos.y + shakeY, pos.z);

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
    // 全車非アクティブ（万一描画などでアクセスされないよう）
    for (int i = 0; i < SG_CAR_MAX; ++i)
        cars[i].use = false;

    // モデル解放
    UnloadModel(&carModel);

    // モデルデータの初期化（ダングリング防止）
    ZeroMemory(&carModel, sizeof(carModel));
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

            // --------- 前方2D矩形（XZ）による吹っ飛ばし ---------
            float offset = (SG_CAR_WALL_D / 2.0f) + (SG_CAR_FRONT_D / 2.0f);
            float frontX = car.pos.x + ((car.lane == 0) ? offset : -offset);
            XMFLOAT3 frontBoxPos = { frontX, car.pos.y, car.pos.z };

            bool isFront = false;
            if (car.lane == 0 && car_player[p].pos.x > car.pos.x) isFront = true;
            if (car.lane == 1 && car_player[p].pos.x < car.pos.x) isFront = true;

            if (isFront &&
                CollisionRect2D_XZ(frontBoxPos, car_player[p].pos, SG_CAR_FRONT_W, SG_CAR_FRONT_D))
            {
                // 吹っ飛ばし処理（今のままでOK）
                float baseDir = (car.lane == 0) ? 0.0f : XM_PI;
                float angleOffset = GetRandf(-0.5f, 0.5f);
                float angle = baseDir + angleOffset;
                float knockbackSpeed = 12.0f;
                car_player[p].knockbackVel.x = knockbackSpeed * cosf(angle);
                car_player[p].knockbackVel.z = knockbackSpeed * sinf(angle);
                car_player[p].stunTimer = 1.0f;
                PrintDebugProc("車前方ヒット：吹っ飛ばし\n");
            }
            // --------- 壁判定は「後ろ寄り＆短いボックス」 ---------
            else
            {
                // 「車の後ろ寄り」に壁ボックスを配置
                float wallBoxX = car.pos.x + ((car.lane == 0) ? -SG_CAR_WALL_D / 4.0f : SG_CAR_WALL_D / 4.0f);
                XMFLOAT3 wallBoxPos = { wallBoxX, car.pos.y, car.pos.z };

                if (CollisionBB(
                    wallBoxPos, car_player[p].pos,
                    XMFLOAT3(SG_CAR_WALL_W, SG_CAR_WALL_H, SG_CAR_WALL_D),
                    XMFLOAT3(car_player[p].size, SG_CAR_WALL_H, car_player[p].size)))
                {
                    // 壁処理（押し戻し）
                    float dx = car_player[p].pos.x - car.pos.x;
                    float dz = car_player[p].pos.z - car.pos.z;
                    float len = sqrtf(dx * dx + dz * dz);

                    if (len > 0.0001f) {
                        float pushBack = (SG_CAR_WALL_W / 2.0f) + car_player[p].size + 0.5f;
                        dx /= len;
                        dz /= len;
                        car_player[p].pos.x = car.pos.x + dx * pushBack;
                        car_player[p].pos.z = car.pos.z + dz * pushBack;
                    }
                    car_player[p].stunTimer = 0.0f;
                    car_player[p].knockbackVel = { 0,0,0 };
                    PrintDebugProc("車横ヒット：壁判定\n");
                }
            }
        }
    }
}

bool CarSystem::Car::CheckHitFront(const XMFLOAT3& targetPos, float radius) const
{
    if (!use) return false;
    float dx = targetPos.x - pos.x;
    float dz = targetPos.z - pos.z;

    // 車の進行方向ベクトル（X正かX負）
    float dirX = (lane == 0) ? +1.0f : -1.0f;
    float dirZ = 0.0f;
    float dot = dx * dirX + dz * dirZ;
    if (dot <= 0.0f) return false; // 後ろ・真横は判定しない

    float distSq = dx * dx + dz * dz;
    float minDist = SG_CAR_RADIUS + radius;
    return distSq <= (minDist * minDist);
}

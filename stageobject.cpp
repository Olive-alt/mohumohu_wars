#include "stageobject.h"
#include "debugproc.h"
#include "main.h"
#include <fstream>
#include <algorithm>
#include <cfloat>
#include <cmath>

using namespace DirectX;

std::vector<STAGE_OBJECT> g_StageObjects;
std::vector<DX11_MODEL>   g_EditorModels;

static std::vector<float> g_ModelRadius;
static std::vector<int>   g_FloorModelIndices = { 3, 9, 10, 12, 13, 15 }; // bg_1, road, land_01, lotus_leaf, bridge, lake_road
static float               g_WaterLevel = -1.0e9f;

#undef max
#undef min

static bool IsFloorModelIndex(int idx)
{
    return std::find(g_FloorModelIndices.begin(), g_FloorModelIndices.end(), idx) != g_FloorModelIndices.end();
}
void SetFloorModelIndices(const std::vector<int>& indices) { g_FloorModelIndices = indices; }
void SetWaterLevel(float y) { g_WaterLevel = y; }
float GetWaterLevel() { return g_WaterLevel; }

void LoadStageObjects(const char* filename)
{
    PrintDebugProc("LoadStageObjects: %s\n", filename);
    std::ifstream ifs(filename);
    if (!ifs.is_open()) { PrintDebugProc("ファイルオープン失敗\n"); return; }
    int n; ifs >> n;
    g_StageObjects.clear();
    g_StageObjects.reserve(std::max(n, 0));
    for (int i = 0; i < n; ++i) {
        STAGE_OBJECT obj{};
        ifs >> obj.pos.x >> obj.pos.y >> obj.pos.z
            >> obj.rot.x >> obj.rot.y >> obj.rot.z
            >> obj.scl.x >> obj.scl.y >> obj.scl.z
            >> obj.modelIndex;
        if (obj.modelIndex < 0) obj.modelIndex = 0;
        g_StageObjects.push_back(obj);
    }
}

void InitStageModels()
{
    g_EditorModels.clear();
    g_ModelRadius.clear();

    DX11_MODEL m;
    LoadModel("data/MODEL/stage/stage_object/cone.obj", &m); g_EditorModels.push_back(m); // 0
    LoadModel("data/MODEL/stage/stage_object/sphere.obj", &m); g_EditorModels.push_back(m); // 1
    LoadModel("data/MODEL/stage/stage_object/grass_00.obj", &m); g_EditorModels.push_back(m); // 2
    LoadModel("data/MODEL/stage/stage_object/bg_1.obj", &m); g_EditorModels.push_back(m); // 3 (床)
    LoadModel("data/MODEL/stage/stage_object/tree_01.obj", &m); g_EditorModels.push_back(m); // 4
    LoadModel("data/MODEL/stage/stage_object/tree_03.obj", &m); g_EditorModels.push_back(m); // 5
    LoadModel("data/MODEL/stage/stage_object/tree_06.obj", &m); g_EditorModels.push_back(m); // 6
    LoadModel("data/MODEL/stage/stage_object/tree_07.obj", &m); /* pushしない（Editorも未push） */
    LoadModel("data/MODEL/stage/stage_object/sky.obj", &m); g_EditorModels.push_back(m); // 7
    LoadModel("data/MODEL/stage/stage_object/stone_01.obj", &m); g_EditorModels.push_back(m); // 8
    LoadModel("data/MODEL/stage/stage_object/road.obj", &m); g_EditorModels.push_back(m); // 9 (床)
    LoadModel("data/MODEL/stage/stage_object/land_01.obj", &m); g_EditorModels.push_back(m); // 10(床)
    LoadModel("data/MODEL/stage/stage_object/gate.obj", &m); g_EditorModels.push_back(m); // 11
    LoadModel("data/MODEL/stage/stage_object/lotus_leaf.obj", &m); g_EditorModels.push_back(m); // 12(足場)
    LoadModel("data/MODEL/stage/stage_object/bridge.obj", &m); g_EditorModels.push_back(m); // 13(床)
    LoadModel("data/MODEL/stage/stage_object/Pillar.obj", &m); g_EditorModels.push_back(m); // 14
    LoadModel("data/MODEL/stage/stage_object/lake_road.obj", &m); g_EditorModels.push_back(m); // 15(床)

    g_ModelRadius.assign(g_EditorModels.size(), 1.0f);
    if (g_ModelRadius.size() > 0)  g_ModelRadius[0] = 1.0f;
    if (g_ModelRadius.size() > 1)  g_ModelRadius[1] = 0.5f;
    if (g_ModelRadius.size() > 2)  g_ModelRadius[2] = 0.5f;
    if (g_ModelRadius.size() > 3)  g_ModelRadius[3] = 30.0f;
    if (g_ModelRadius.size() > 4)  g_ModelRadius[4] = 2.0f;
    if (g_ModelRadius.size() > 5)  g_ModelRadius[5] = 2.5f;
    if (g_ModelRadius.size() > 6)  g_ModelRadius[6] = 3.0f;
    if (g_ModelRadius.size() > 7)  g_ModelRadius[7] = 1000.0f; // sky
    if (g_ModelRadius.size() > 8)  g_ModelRadius[8] = 1.5f;
    if (g_ModelRadius.size() > 9)  g_ModelRadius[9] = 20.0f;   // road
    if (g_ModelRadius.size() > 10) g_ModelRadius[10] = 15.0f;   // land_01
    if (g_ModelRadius.size() > 11) g_ModelRadius[11] = 5.0f;    // gate
    if (g_ModelRadius.size() > 12) g_ModelRadius[12] = 1.0f;    // lotus_leaf
    if (g_ModelRadius.size() > 13) g_ModelRadius[13] = 8.0f;    // bridge
    if (g_ModelRadius.size() > 14) g_ModelRadius[14] = 1.5f;    // Pillar
    if (g_ModelRadius.size() > 15) g_ModelRadius[15] = 20.0f;   // lake_road

    g_FloorModelIndices = { 3, 9, 10, 12, 13, 15 };
}

void UninitStageObjects()
{
    for (auto& m : g_EditorModels) UnloadModel(&m);
    g_EditorModels.clear();
    g_StageObjects.clear();
    g_ModelRadius.clear();
    g_WaterLevel = -1.0e9f;
}

void DrawStageObjects()
{
    for (const auto& obj : g_StageObjects) {
        XMMATRIX w =
            XMMatrixScaling(obj.scl.x, obj.scl.y, obj.scl.z) *
            XMMatrixRotationRollPitchYaw(obj.rot.x, obj.rot.y, obj.rot.z) *
            XMMatrixTranslation(obj.pos.x, obj.pos.y, obj.pos.z);
        SetWorldMatrix(&w);
        if ((size_t)obj.modelIndex < g_EditorModels.size()) {
            DrawModel(&g_EditorModels[obj.modelIndex]);
        }
    }
}

bool StageCollideSphere(const XMFLOAT3& center, float radius, int* outHitIndex)
{
    if (outHitIndex) *outHitIndex = -1;
    const XMVECTOR C = XMLoadFloat3(&center);

    for (int i = 0; i < (int)g_StageObjects.size(); ++i) {
        const auto& obj = g_StageObjects[i];
        if ((size_t)obj.modelIndex >= g_ModelRadius.size()) continue;

        if (IsFloorModelIndex(obj.modelIndex) || obj.modelIndex == 7) continue; // 床/空は除外

        float baseR = g_ModelRadius[obj.modelIndex];
        float sMax = std::max(std::max(obj.scl.x, obj.scl.y), obj.scl.z);
        float R = baseR * sMax;

        XMVECTOR O = XMLoadFloat3(&obj.pos);
        float dist = XMVectorGetX(XMVector3Length(O - C));
        if (dist <= (R + radius)) {
            if (outHitIndex) *outHitIndex = i;
            return true;
        }
    }
    return false;
}

bool StageRaycastDown(const XMFLOAT3& from, float maxDrop, float* outY)
{
    const float fromY = from.y;
    float bestY = -FLT_MAX;
    bool  hit = false;

    for (const auto& obj : g_StageObjects) {
        if (!IsFloorModelIndex(obj.modelIndex)) continue;
        const float floorY = obj.pos.y;
        if (floorY <= fromY && floorY >= (fromY - maxDrop)) {
            if (floorY > bestY) { bestY = floorY; hit = true; }
        }
    }
    if (hit && outY) *outY = bestY;
    return hit;
}

bool ResolveFloorPenetration(XMFLOAT3* inOutCenter, float radius, float margin)
{
    if (!inOutCenter) return false;

    const float kMaxDrop = radius * 2.5f + 1.0f;
    float floorY = 0.0f;
    if (!StageRaycastDown(*inOutCenter, kMaxDrop, &floorY))
        return false;

    const float minCenterY = floorY + radius + margin;
    if (inOutCenter->y < minCenterY) {
        inOutCenter->y = minCenterY;
        return true;
    }
    return false;
}

bool StageIsOnFloor(const XMFLOAT3& from, float maxDrop)
{
    float y;
    return StageRaycastDown(from, maxDrop, &y);
}

bool StageGetNearestFloorPoint(const XMFLOAT3& from, XMFLOAT3* outPos, float upOffset)
{
    if (!outPos) return false;

    float bestDist2 = FLT_MAX;
    const STAGE_OBJECT* best = nullptr;

    for (const auto& obj : g_StageObjects) {
        if (!IsFloorModelIndex(obj.modelIndex)) continue;
        float dx = obj.pos.x - from.x;
        float dz = obj.pos.z - from.z;
        float d2 = dx * dx + dz * dz;
        if (d2 < bestDist2) { bestDist2 = d2; best = &obj; }
    }
    if (!best) return false;

    *outPos = XMFLOAT3(best->pos.x, best->pos.y + upOffset, best->pos.z);
    return true;
}

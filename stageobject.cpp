#include "stageobject.h"
#include "debugproc.h"
#include "main.h"

std::vector<STAGE_OBJECT> g_StageObjects;
std::vector<DX11_MODEL> g_EditorModels;

void LoadStageObjects(const char* filename)
{
    PrintDebugProc("LoadStageObjects: %s\n", filename); // ←追加
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        PrintDebugProc("ファイルオープン失敗: %s\n", filename); // ←追加
        return;
    }
    int n; ifs >> n;
    PrintDebugProc("オブジェクト数: %d\n", n); // ←追加
    g_StageObjects.clear();
    for (int i = 0; i < n; ++i) {
        STAGE_OBJECT obj;
        ifs >> obj.pos.x >> obj.pos.y >> obj.pos.z
            >> obj.rot.x >> obj.rot.y >> obj.rot.z
            >> obj.scl.x >> obj.scl.y >> obj.scl.z
            >> obj.modelIndex;
        g_StageObjects.push_back(obj);
    }
}

void InitStageModels()
{
    g_EditorModels.clear();
    DX11_MODEL m;
    LoadModel("data/MODEL/Stage/stage_object/cone.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/sphere.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/grass_00.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/bg_1.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/tree_01.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/tree_03.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/tree_06.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/sky.obj", &m); g_EditorModels.push_back(m);
    LoadModel("data/MODEL/Stage/stage_object/stone_01.obj", &m); g_EditorModels.push_back(m);
}

void UninitStageObjects()
{
    for (auto& m : g_EditorModels) UnloadModel(&m);
    g_EditorModels.clear();
    g_StageObjects.clear();
}

void DrawStageObjects()
{
    for (const auto& obj : g_StageObjects) {
        XMMATRIX w = XMMatrixScaling(obj.scl.x, obj.scl.y, obj.scl.z)
            * XMMatrixRotationRollPitchYaw(obj.rot.x, obj.rot.y, obj.rot.z)
            * XMMatrixTranslation(obj.pos.x, obj.pos.y, obj.pos.z);
        SetWorldMatrix(&w);
        DrawModel(&g_EditorModels[obj.modelIndex]);
    }
}


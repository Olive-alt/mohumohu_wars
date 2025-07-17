
#pragma once
#include "model.h"

struct STAGE_OBJECT {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 rot;
    DirectX::XMFLOAT3 scl;
    int modelIndex;
};

void LoadStageObjects(const char* filename);
void DrawStageObjects();
void InitStageModels();
void UninitStageObjects();
extern std::vector<STAGE_OBJECT> g_StageObjects;

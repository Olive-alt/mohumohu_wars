#pragma once
#include <DirectXMath.h>

HRESULT InitStageSelect(void);
void    UninitStageSelect(void);
void    UpdateStageSelect(void);
void    DrawStageSelect(void);

// ŽÀ‘Ì‚Í stage_select.cpp ‚É‚ ‚é
extern char g_SelectedStageFile[64];

struct STAGE_SELECT_3D {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation;
    float             scale;
    int               stageId;
    bool              isSelected;
};

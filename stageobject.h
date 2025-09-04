#pragma once
#include <vector>
#include <DirectXMath.h>
#include "model.h"

// エディター保存1インスタンス
struct STAGE_OBJECT {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 rot;
    DirectX::XMFLOAT3 scl;
    int               modelIndex;
};

// ロード／描画／破棄
void LoadStageObjects(const char* filename);
void DrawStageObjects();
void InitStageModels();
void UninitStageObjects();

// ===== 実行時の軽量当たり & 床レイ & 水面 =====

// （球近似）center/radius がステージに触れたら true
bool StageCollideSphere(const DirectX::XMFLOAT3& center, float radius, int* outHitIndex = nullptr);

// 上→下の床レイ：from から maxDrop の範囲で最も近い床Yを返す
bool StageRaycastDown(const DirectX::XMFLOAT3& from, float maxDrop, float* outY);

// 床押し戻し：床Y + 半径 + 余白 までYを持ち上げる
bool ResolveFloorPenetration(DirectX::XMFLOAT3* inOutCenter, float radius, float margin);

// “水面Y” の管理（未設定は -1e9 で無効相当）
void  SetWaterLevel(float y);
float GetWaterLevel();

// 補助：近傍に床があれば true
bool StageIsOnFloor(const DirectX::XMFLOAT3& from, float maxDrop);

// 補助：最寄り床の真上 + upOffset を返す
bool StageGetNearestFloorPoint(const DirectX::XMFLOAT3& from, DirectX::XMFLOAT3* outPos, float upOffset);

// “床”扱いのモデルインデックスを登録（省略時は既定セット）
void SetFloorModelIndices(const std::vector<int>& indices);

// 共有データ
extern std::vector<STAGE_OBJECT> g_StageObjects;
extern std::vector<DX11_MODEL>   g_EditorModels;

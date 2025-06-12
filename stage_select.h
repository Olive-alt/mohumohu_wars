#pragma once


// ステージ選択画面の関数宣言
HRESULT InitStageSelect(void);    // 初期化
void UninitStageSelect(void);     // 終了処理
void UpdateStageSelect(void);     // 更新処理
void DrawStageSelect(void);       // 描画処理

// 3Dステージ選択用の構造体
struct STAGE_SELECT_3D {
    XMFLOAT3 position;   // ステージの位置
    XMFLOAT3 rotation;   // ステージの回転
    float scale;         // ステージのスケール
    int stageId;         // ステージID
    bool isSelected;     // 選択中かどうか
};
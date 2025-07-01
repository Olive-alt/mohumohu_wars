#pragma once

// モード選択画面の関数宣言
HRESULT InitModeSelect(void);    // 初期化
void UninitModeSelect(void);     // 終了処理
void UpdateModeSelect(void);     // 更新処理
void DrawModeSelect(void);       // 描画処理

// モード選択用の構造体（将来的に拡張できるように用意）
struct MODE_SELECT_INFO {
    int modeId;                 // モードID
    const char* modeName;       // モード名
    bool isSelected;            // 現在選択中か
};

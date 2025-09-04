#pragma once
//=============================================================================
//
// ハンマー [IT_hammer.h]
//
//=============================================================================

#include "renderer.h"
#include <random>
#include "model.h"

#define HAMR_SIZE (10.0f)
#define HAMR_MAX  (1)

class HAMR
{
private:
    // 変換・状態
    XMFLOAT3 pos;    // 位置
    XMFLOAT3 rot;    // 向き
    XMFLOAT3 scl;    // スケール
    XMFLOAT3 move;
    int      PlayerIndex;

    BOOL     use;
    BOOL     to_swing;
    BOOL     pick;
    XMFLOAT4 diffuse[MODEL_MAX_MATERIAL];

    float    spd;
    float    size;
    int      shadowIdx;
    float    count;

    // アイコン描画用
    XMFLOAT3  icon_pos;
    XMFLOAT3  icon_scl;
    MATERIAL  icon_material;
    float     icon_fWidth;
    float     icon_fHeight;
    BOOL      icon_use;

    // 行列
    XMFLOAT4X4 m_mtxWorld;

    // スイング
    float       swingAngle = 0.0f;
    const float swingMax = XM_PIDIV2; // 90°

    // 入力フラグ（プレイヤー→ハンマーへ）
    bool wantToSwing = false;

    // ★追加：耐久（3回で壊れる）
    int  swingCount = 0;

public:
    HRESULT InitITHamr(void);
    void    UninitITHamr(void);
    void    UpdateITHamr(void);
    void    DrawITHamr(void);

    void    SetITHamrObject(XMFLOAT3 set_pos);
    void    HitITHamr(int p_Index);
    void    PickITHamr(int p_Index);

    // 衝突用（装備中はハンマー頭ワールド座標を使う）
    XMFLOAT3 GetHeadWorldPosition() const;

    // 状態参照
    bool     IsUsedITHamr()   const { return use; }
    bool     IsPickedITHamr() const { return pick; }
    XMFLOAT3 GetPositionITHamr() const { return pos; }

    // プレイヤーから振り指示を受け取る
    void SetSwingFlag(bool flag) { wantToSwing = flag; }
    bool GetSwingFlag()   const { return wantToSwing; }

    // ★追加：耐久カウンタ制御（外からリセットしたい時用）
    void ResetSwingCount() { swingCount = 0; }
};

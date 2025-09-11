//=============================================================================
//
// モデル処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "model.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER          (4)       // プレイヤーの数
#define PLAYER_PARTS_MAX    (5)       // パーツ数
#define PLAYER_SIZE         (15.0f)   // 当たり半径（=size と同値で扱う）
#define PLAYER_HEIGHT       (20.0f)   // カプセル高さ
#define PLAYER_HP_MAX       (10.0f)   // 最大HP
//*****************************************************************************
// 列挙
//*****************************************************************************
typedef enum
{
    PLAYER_NORMAL,
    PLAYER_WALK,
    PLAYER_HIT,
    PLAYER_DEAD,
    PLAYER_RESULT_WIN,
    PLAYER_RESULT_LOSE,
} PLAYER_STATE;

//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
    XMFLOAT4X4 mtxWorld;   // ワールド行列
    XMFLOAT3   pos;        // 位置
    XMFLOAT3   rot;        // 向き
    XMFLOAT3   scl;        // スケール

    float      spd;        // 速度（任意）
    BOOL       load;
    DX11_MODEL model;
    XMFLOAT4   diffuse[MODEL_MAX_MATERIAL];

    int        shadowIdx;
    float      hp;

    // ワープゲート
    BOOL       gateUse;
    int        gateCoolTime;

    // バフ
    BOOL       invisible;

    // 武器重複防止
    BOOL       haveWeapon;

    // 無敵判定用
    BOOL       muteki;

    // エナドリ判定用
    BOOL       enadori;

    BOOL       use;

    float      size;       // 当たりサイズ（半径と同値で運用）
    float      radius;     // 当たり半径（=PLAYER_SIZEに同期）

    // 階層アニメ
    float      time;
    int        tblNo;
    int        tblMax;

    PLAYER* parent;

    // 姿勢
    XMFLOAT4   Quaternion;
    XMFLOAT3   UpVector;

    // カプセル当たり
    XMFLOAT3   capsuleA;   // 下端
    XMFLOAT3   capsuleB;   // 上端

    // ノックバック/スタン
    float      stunTimer;
    XMFLOAT3   knockbackVel;

    // 補助
    XMFLOAT3   prevPos;

    // アニメ状態
    PLAYER_STATE currentAnimation;

    // 演出（未使用でも保持）
    bool       squished;
    float      squishTimer;
    XMFLOAT3   originalScl;
};

//*****************************************************************************
// 外部変数
//*****************************************************************************
extern bool        g_IsCPU[MAX_PLAYER];

extern PLAYER_STATE g_PlayerState[MAX_PLAYER];
extern int          g_PlayerAnimTimer[MAX_PLAYER];
extern XMFLOAT3     g_PlayerKnockback[MAX_PLAYER];

// 歩行ブレンド制御（ヘッダで宣言→cppで定義）
extern bool   g_PlayerIsMoving[MAX_PLAYER];
extern bool   g_PlayerPrevMoving[MAX_PLAYER];
extern int    g_PlayerAnimBlendTimer[MAX_PLAYER];
extern int    g_PlayerAnimBlendMode[MAX_PLAYER];               // 0:通常,1:IN,2:OUT
extern float  g_PlayerAnimBlendFrom[MAX_PLAYER][PLAYER_PARTS_MAX];

//*****************************************************************************
// プロトタイプ宣言（すべてここに集約）
//*****************************************************************************
HRESULT InitPlayer(void);
void    UninitPlayer(void);
void    UpdatePlayer(void);
void    DrawPlayer(void);

// プレイヤー取得（互換＋インデックス）
PLAYER* GetPlayer(void);
PLAYER* GetPlayer(int index);

// Bar
void    DrawPlayerHpBar(void);

// 入力/移動/攻撃
void    MovePlayers(void);
void    PlayerAttack(int playerIndex);

// HP増減
void    AddPlayerHP(int index, float add);

// アニメ/被弾
void    UpdatePlayerPartsAnimation(int playerIndex);
void    UpdatePlayerKnockback(int playerIndex);
void    OnPlayerHit(int i, const XMFLOAT3& hitDirection);

// 演出ユーティリティ
float   GetNoise(float strength = 0.12f);
float   GetPeriodicNoise(int seed, float time, float strength = 0.12f);
float   GetSmoothNoise(float& last, float strength = 0.12f);
void    PSetAnimation(int playerIndex, PLAYER_STATE animation);

// 角度ユーティリティ
float   NormalizeAngle(float angle);
float   SmoothAngle(float current, float target, float smoothFactor = 0.1f);
float   turning(float target, float current);

// ===== 当たり/地形追従の共通ヘルパ =====
void    AdjustYByTerrainAndUpdateShadow(int index, DirectX::XMFLOAT3& outNormal);
void    UpdatePoseByGroundNormal(int index, const DirectX::XMFLOAT3& groundNormal);
void    UpdateCollisionCapsule(int index);
void    UpdateWarpGateCooldown(int index);
bool    HandleStunAndKnockback(int index);
void    UpdateHpDebugKeys(int index);
void    DebugPrintPlayer(int index);


//*****************************************************************************
// CPUパーティAI
// *****************************************************************************
// ==== CPUパーティAI用の状態 ====
struct PartyAIState {
    int   targetIdx;                // 追う相手
    DirectX::XMFLOAT3 wanderTarget; // 徘徊の目的地
    int   thinkCooldown;            // 思考クールダウン
    int   fireCooldown;             // 発射クールダウン
    float wanderSpeedScale;         // 徘徊時の速度スケール
    float aimNoisePhase;            // ノイズ用の位相

    int   aimLockFrames;            // 目標角に収まっている連続フレーム数
    int   postPickDelay;            // アイテム拾った直後の待ちフレーム
    bool  hadWeapon;                // 前フレームで武器を持っていたか
};
extern PartyAIState g_PartyAI[MAX_PLAYER];

// ==== CPUユーティリティ（外部リンケージに統一） ====
bool  AI_FindNearestWeaponItem(const DirectX::XMFLOAT3& selfPos, DirectX::XMFLOAT3& outPos);
int   AI_FindNearestOpponent(int selfIndex);
void  AI_PickNewWanderTarget(int selfIndex, const DirectX::XMFLOAT3& origin);
float AI_AngleDiff(float a, float b);
float AI_Frand01(void);
void  AI_ForceAttack(int playerIndex);
float YawFromDir(float vx, float vz);
void AI_PickNewWanderTarget(int selfIndex, const XMFLOAT3& origin);
void UpdateFallAndRespawn(int idx);
bool IsBeachStage();

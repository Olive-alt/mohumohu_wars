//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "shadow.h"
#include "debugproc.h"
#include "meshfield.h"
#include "item.h"
#include "player_select.h"
#include "stageobject.h"
#include "time.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define VALUE_MOVE         (1.0f)
#define VALUE_ROTATE       (D3DX_PI * 0.02f)
#define PLAYER_SHADOW_SIZE (0.4f)
#define PLAYER_OFFSET_Y    (14.0f)

// 共有モデルパス（一部抜粋・必要に応じて拡張）
static const char* CHARACTER_MODEL_PATHS[][6] = {
    // 0: 鳥
    {
        "data/MODEL/character/model_bird/cha_bird_head.obj",
        "data/MODEL/character/model_bird/cha_bird_body.obj",
        "data/MODEL/character/model_bird/cha_bird_leftarm.obj",
        "data/MODEL/character/model_bird/cha_bird_rightarm.obj",
        "data/MODEL/character/model_bird/cha_bird_leftleg.obj",
        "data/MODEL/character/model_bird/cha_bird_rightleg.obj"
    },
    // 1: 犬
    {
        "data/MODEL/character/model_fox/cha_fox_head.obj",
        "data/MODEL/character/model_fox/cha_fox_body.obj",
        "data/MODEL/character/model_fox/cha_fox_leftarm.obj",
        "data/MODEL/character/model_fox/cha_fox_rightarm.obj",
        "data/MODEL/character/model_fox/cha_fox_leftleg.obj",
        "data/MODEL/character/model_fox/cha_fox_rightleg.obj"
    },
    // 2: 猫
    {
        "data/MODEL/character/model_cat/cha_cat_head.obj",
        "data/MODEL/character/model_cat/cha_cat_body.obj",
        "data/MODEL/character/model_cat/cha_cat_leftarm.obj",
        "data/MODEL/character/model_cat/cha_cat_rightarm.obj",
        "data/MODEL/character/model_cat/cha_cat_leftleg.obj",
        "data/MODEL/character/model_cat/cha_cat_rightleg.obj"
	},
    // 3:犬
    {
        "data/MODEL/character/model_dog/cha_dog_head.obj",
        "data/MODEL/character/model_dog/cha_dog_body.obj",
        "data/MODEL/character/model_dog/cha_dog_leftarm.obj",
        "data/MODEL/character/model_dog/cha_dog_rightarm.obj",
        "data/MODEL/character/model_dog/cha_dog_leftleg.obj",
        "data/MODEL/character/model_dog/cha_dog_rightleg.obj"
	},

};

// Walk/Result 用のテーブル index
#define B_WALK_L_ARM_ANIMATION_INDEX         1
#define B_WALK_R_ARM_ANIMATION_INDEX         2
#define B_RESULT_WIN_L_ARM_ANIMATION_INDEX   5
#define B_RESULT_WIN_R_ARM_ANIMATION_INDEX   6
#define B_RESULT_LOSE_L_ARM_ANIMATION_INDEX  7
#define B_RESULT_LOSE_R_ARM_ANIMATION_INDEX  8

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER g_Player[MAX_PLAYER];                      // 本体
static PLAYER g_Parts[MAX_PLAYER][PLAYER_PARTS_MAX];     // 子パーツ

static float  roty = 0.0f;
static LIGHT  g_Light;
static float  noiseTable[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 };

bool    g_IsCPU[MAX_PLAYER] = { false, false }; // 初期は人間操作

// 状態管理の実体
PLAYER_STATE g_PlayerState[MAX_PLAYER] = { PLAYER_NORMAL };
int          g_PlayerAnimTimer[MAX_PLAYER] = { 0 };
XMFLOAT3     g_PlayerKnockback[MAX_PLAYER] = { XMFLOAT3(0,0,0) };

// 歩行ブレンド用の実体（ヘッダは extern）
bool   g_PlayerIsMoving[MAX_PLAYER] = { false, false };
bool   g_PlayerPrevMoving[MAX_PLAYER] = { false, false };
int    g_PlayerAnimBlendTimer[MAX_PLAYER] = { 0 };
int    g_PlayerAnimBlendMode[MAX_PLAYER] = { 0 };
float  g_PlayerAnimBlendFrom[MAX_PLAYER][PLAYER_PARTS_MAX] = { 0 };

// HPバーVB（簡易）
static ID3D11Buffer* g_PlayerHpBarVertexBuffer = nullptr;
static float g_NoiseClock = 0.0f;

static DirectX::XMFLOAT3 g_LastSafePos[MAX_PLAYER] = { DirectX::XMFLOAT3(0,0,0) };
// CPU用の簡易ステアリング状態
struct SimpleAI {
    int   target;        // 追う相手
    float orbitPhase;    // 角位相（ストレーフ用）
    int   orbitDir;      // -1 or +1（左右どちら回りで絡むか）
    float orbitRadius;   // からみ半径（8〜14）
    int   thinkCooldown; // 次の意思決定まで
    float speedScale;    // 速度微調整（0.9〜1.2）
};
static SimpleAI g_SAI[MAX_PLAYER] = {};

PartyAIState g_PartyAI[MAX_PLAYER] = {};

// ==== チューニング定数（好みに合わせて調整OK） ====
static const float AI_SEEK_SPEED = 1.00f;    // アイテム/敵へ向かう速度倍率
static const float AI_WANDER_SPEED = 0.75f;    // 徘徊時の速度倍率
static const float AI_WANDER_RADIUS = 48.0f;    // 徘徊目的地のランダム半径
static const int   AI_THINK_MIN = 36;       // 思考再設定 最小フレーム
static const int   AI_THINK_MAX = 60;       // 思考再設定 最大フレーム
static const int   AI_FIRE_CD_MIN = 28;       // 発射クールダウン最小
static const int   AI_FIRE_CD_MAX = 40;       // 発射クールダウン最大
static const float AI_ALIGN_DEG = 20.0f;    // “向けた”とみなす角度(度)
static const float AI_NOISE_PROB = 0.5f;     // ノイズ発射する確率
static const float AI_NOISE_DEG_MAX = 8.0f;    // 付与する角度ノイズ(±度)
static const float STAGE_HALF = 85.0f;    // 端回避のための境界
static const float EDGE_MARGIN = 5.0f;
static const float AI_AIM_LOCK_DEG = 10.0f;  // この角度以内を"狙えてる"とみなす
static const int   AI_AIM_LOCK_FRAMES = 8;      // 連続で何フレーム収まればOKか
static const int   AI_POST_PICK_DELAY = 12;     // 拾ってから撃つまで最低待機F

//*****************************************************************************
// プロトタイプ（このcpp内部のみで使うユーティリティ）
//*****************************************************************************
static float GetRand01() { return (float)(rand() % 1000) / 1000.0f; }

//*****************************************************************************
// アニメテーブル（省略せず現状のまま）
//*****************************************************************************
static INTERPOLATION_DATA walk_move_tbl_head[] = {
    { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0,0,0), XMFLOAT3(1,1,1), 30 },
    { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0,0,0), XMFLOAT3(1,1,1), 30 },
};
static INTERPOLATION_DATA walk_move_tbl_l_arm[] = {
    { XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0,-XM_PI / 2), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(-4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3,0,-XM_PI / 2), XMFLOAT3(1,1,1), 20 },
};
static INTERPOLATION_DATA walk_move_tbl_r_arm[] = {
    { XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(-XM_PI / 3,0,XM_PI / 2), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(4.0f, 1.0f, 0.0f), XMFLOAT3(XM_PI / 3, 0,XM_PI / 2), XMFLOAT3(1,1,1), 20 },
};
static INTERPOLATION_DATA walk_move_tbl_l_leg[] = {
    { XMFLOAT3(2.0f,-5.0f,0.0f), XMFLOAT3(-XM_PI / 3,0,0), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(2.0f,-5.0f,0.0f), XMFLOAT3(XM_PI / 3, 0,0), XMFLOAT3(1,1,1), 20 },
};
static INTERPOLATION_DATA walk_move_tbl_r_leg[] = {
    { XMFLOAT3(-2.0f,-5.0f,0.0f), XMFLOAT3(XM_PI / 3,0,0), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(-2.0f,-5.0f,0.0f), XMFLOAT3(-XM_PI / 3,0,0), XMFLOAT3(1,1,1), 20 },
};
// result (win/lose)
static INTERPOLATION_DATA move_tbl_wave_l_arm[] = {
    { XMFLOAT3(-4,1,0), XMFLOAT3(XM_PI / 1.25f,0,-XM_PI / 2), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(-4,1,0), XMFLOAT3(XM_PI / 1.6f,0,-2.8f),     XMFLOAT3(1,1,1), 20 },
};
static INTERPOLATION_DATA move_tbl_wave_r_arm[] = {
    { XMFLOAT3(4,1,0), XMFLOAT3(0,0,XM_PI / 2), XMFLOAT3(1,1,1), 20 },
    { XMFLOAT3(4,1,0), XMFLOAT3(0,0,XM_PI / 2), XMFLOAT3(1,1,1), 20 },
};
static INTERPOLATION_DATA move_tbl_clap_l_arm[] = {
    { XMFLOAT3(-4,1,0), XMFLOAT3(XM_PI / 2,0,XM_PI / 8), XMFLOAT3(1,1,1), 10 },
    { XMFLOAT3(-4,1,0), XMFLOAT3(XM_PI / 2,0,XM_PI / 2), XMFLOAT3(1,1,1), 10 },
};
static INTERPOLATION_DATA move_tbl_clap_r_arm[] = {
    { XMFLOAT3(4,1,0), XMFLOAT3(-XM_PI / 3,0,XM_PI / 2 - 1.0f), XMFLOAT3(1,1,1),10 },
    { XMFLOAT3(4,1,0), XMFLOAT3(-XM_PI / 3,0,XM_PI / 2),      XMFLOAT3(1,1,1),10 },
};
static INTERPOLATION_DATA* g_MoveTblAdr[] = {
    walk_move_tbl_head,
    walk_move_tbl_l_arm,
    walk_move_tbl_r_arm,
    walk_move_tbl_l_leg,
    walk_move_tbl_r_leg,
    move_tbl_wave_l_arm,
    move_tbl_wave_r_arm,
    move_tbl_clap_l_arm,
    move_tbl_clap_r_arm,
};

//*****************************************************************************
// HPバーVB初期化
//*****************************************************************************
static void InitPlayerHpBarVertexBuffer()
{
    struct VERTEX_3D {
        XMFLOAT3 Position; XMFLOAT3 Normal; XMFLOAT4 Diffuse; XMFLOAT2 TexCoord;
    } vtx[4];

    vtx[0].Position = XMFLOAT3(-0.5f, +0.5f, 0.0f); vtx[0].TexCoord = XMFLOAT2(0, 0);
    vtx[1].Position = XMFLOAT3(+0.5f, +0.5f, 0.0f); vtx[1].TexCoord = XMFLOAT2(1, 0);
    vtx[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f); vtx[2].TexCoord = XMFLOAT2(0, 1);
    vtx[3].Position = XMFLOAT3(+0.5f, -0.5f, 0.0f); vtx[3].TexCoord = XMFLOAT2(1, 1);

    for (int i = 0; i < 4; i++) { vtx[i].Normal = XMFLOAT3(0, 0, -1); vtx[i].Diffuse = XMFLOAT4(1, 1, 1, 1); }

    D3D11_BUFFER_DESC bd = {}; bd.Usage = D3D11_USAGE_DEFAULT; bd.ByteWidth = sizeof(vtx); bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA init = { vtx,0,0 };
    GetDevice()->CreateBuffer(&bd, &init, &g_PlayerHpBarVertexBuffer);
}

//*****************************************************************************
// 初期化
//*****************************************************************************
HRESULT InitPlayer(void)
{
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        int selectedChar = GetSelectedCharIndex(i);

        // 本体
        g_Player[i].load = TRUE;
        LoadModel(CHARACTER_MODEL_PATHS[selectedChar][1], &g_Player[i].model);

        // 子パーツ
        for (int j = 0; j < PLAYER_PARTS_MAX; j++)
        {
            g_Parts[i][j].use = TRUE;
            g_Parts[i][j].parent = &g_Player[i];
            g_Parts[i][j].time = 0.0f;
            g_Parts[i][j].tblNo = j;

            switch (j)
            {
            case 0: LoadModel(CHARACTER_MODEL_PATHS[selectedChar][0], &g_Parts[i][j].model); g_Parts[i][j].tblMax = sizeof(walk_move_tbl_head) / sizeof(INTERPOLATION_DATA); break;
            case 1: LoadModel(CHARACTER_MODEL_PATHS[selectedChar][2], &g_Parts[i][j].model); g_Parts[i][j].tblMax = sizeof(walk_move_tbl_l_arm) / sizeof(INTERPOLATION_DATA); break;
            case 2: LoadModel(CHARACTER_MODEL_PATHS[selectedChar][3], &g_Parts[i][j].model); g_Parts[i][j].tblMax = sizeof(walk_move_tbl_r_arm) / sizeof(INTERPOLATION_DATA); break;
            case 3: LoadModel(CHARACTER_MODEL_PATHS[selectedChar][4], &g_Parts[i][j].model); g_Parts[i][j].tblMax = sizeof(walk_move_tbl_l_leg) / sizeof(INTERPOLATION_DATA); break;
            case 4: LoadModel(CHARACTER_MODEL_PATHS[selectedChar][5], &g_Parts[i][j].model); g_Parts[i][j].tblMax = sizeof(walk_move_tbl_r_leg) / sizeof(INTERPOLATION_DATA); break;
            }
            g_Parts[i][j].scl = XMFLOAT3(2, 2, 2);
            g_Parts[i][j].rot = XMFLOAT3(0, 0, 0);
            g_Parts[i][j].pos = XMFLOAT3(0, 0, 0);
            g_Parts[i][j].load = TRUE;
        }

        // 位置/姿勢初期化
        g_Player[i].pos = XMFLOAT3(-10.0f, PLAYER_OFFSET_Y + 100.0f, -50.0f);
        g_Player[i].rot = XMFLOAT3(0, 0, 0);
        g_Player[i].scl = XMFLOAT3(2, 2, 2);
        g_Player[i].Quaternion = XMFLOAT4(0, 0, 0, 1);
        g_Player[i].UpVector = XMFLOAT3(0, 1, 0);

        g_Player[i].spd = 0.0f;
        g_Player[i].hp = PLAYER_HP_MAX;
        g_Player[i].muteki = FALSE;
        g_Player[i].enadori = FALSE;
        g_Player[i].use = TRUE;

        g_Player[i].size = PLAYER_SIZE;
        g_Player[i].radius = PLAYER_SIZE; 
        g_Player[i].prevPos = g_Player[i].pos;

        // カプセル初期化（仮。毎フレーム更新で上書き）
        g_Player[i].capsuleA = XMFLOAT3(g_Player[i].pos.x, g_Player[i].pos.y - PLAYER_HEIGHT / 2.0f, g_Player[i].pos.z);
        g_Player[i].capsuleB = XMFLOAT3(g_Player[i].pos.x, g_Player[i].pos.y + PLAYER_HEIGHT / 2.0f, g_Player[i].pos.z);

        // Diffuse退避
        GetModelDiffuse(&g_Player[i].model, &g_Player[i].diffuse[0]);

        // ワープ初期化
        g_Player[i].gateUse = FALSE;
        g_Player[i].gateCoolTime = 0;

        // 影生成
        XMFLOAT3 pos = g_Player[i].pos; pos.y -= (PLAYER_OFFSET_Y - 0.1f);
        g_Player[i].shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);

        roty = 0.0f;
        g_Player[i].parent = NULL;

		g_Player[i].squished = false; // Initialize squished state
		g_Player[i].squishTimer = 0.0f;
		g_Player[i].originalScl = g_Player[i].scl; // Save original scale

        //安全地点の初期化（開始時の位置）
        g_LastSafePos[i] = g_Player[i].pos;
    }
    g_Player[0].pos = XMFLOAT3(-150.0f, PLAYER_OFFSET_Y + 100.0f, 150.0f);
    g_Player[1].pos = XMFLOAT3(150.0f, PLAYER_OFFSET_Y + 100.0f, 150.0f);
    g_Player[2].pos = XMFLOAT3(-150.0f, PLAYER_OFFSET_Y + 100.0f, -150.0f);
    g_Player[3].pos = XMFLOAT3(10.0f, PLAYER_OFFSET_Y + 100.0f, -150.0f);

    InitPlayerHpBarVertexBuffer();
    return S_OK;
}

//*****************************************************************************
// 終了
//*****************************************************************************
void UninitPlayer(void)
{
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        if (g_Player[i].load == TRUE)
        {
            UnloadModel(&g_Player[i].model);
            g_Player[i].load = FALSE;
        }
    }
    if (g_PlayerHpBarVertexBuffer) { g_PlayerHpBarVertexBuffer->Release(); g_PlayerHpBarVertexBuffer = nullptr; }
}

//*****************************************************************************
// 更新
//*****************************************************************************
void UpdatePlayer(void)
{
    if (GetMode() == MODE_GAME)
    {
        g_NoiseClock += 0.08f;
        if (g_NoiseClock > 10000.0f) g_NoiseClock -= 10000.0f;

        // 1) 入力/AI
        MovePlayers();

        // 2) 各プレイヤー更新
        for (int i = 0; i < MAX_PLAYER; i++)
        {
            PLAYER& pl = g_Player[i];
            if (!pl.use) continue;

            // --- スタン/ノックバック処理（動いたら続く） ---
            if (HandleStunAndKnockback(i))
            {
                UpdateCollisionCapsule(i);
                continue; // スタン中は移動のみして次へ
            }

            // --- 攻撃入力（またはAIからの発射） ---
            PlayerAttack(i);

            // --- ワープゲートCD ---
            UpdateWarpGateCooldown(i);

            // --- 地形吸着 & 影更新 ---
            XMFLOAT3 groundN;
            AdjustYByTerrainAndUpdateShadow(i, groundN);

            // --- 法線に沿った姿勢補正（Slerp） ---
            UpdatePoseByGroundNormal(i, groundN);

            // --- ★ 落下検知＆復帰（水上/キルゾーン） ---
            UpdateFallAndRespawn(i);
            // --- 階層アニメ更新 ---
            UpdatePlayerPartsAnimation(i);

            // --- カプセル更新（最後に必ず） ---
            UpdateCollisionCapsule(i);


            for (int i = 0; i < MAX_PLAYER; i++)
            {
                // プレイヤーごとのつぶれ効果を更新する
                PLAYER& pl = g_Player[i];
                bool& squished = pl.squished;          
                float& squishTimer = pl.squishTimer;   
                XMFLOAT3& originalScl = pl.originalScl;
                XMFLOAT3& scl = pl.scl;                

                if (squished) // プレイヤーがつぶれている状態なら
                {
                    
                    squishTimer -= GetDeltaTime();

                    // タイマーが0以下になったら → 元のサイズに戻す
                    if (squishTimer <= 0.0f)
                    {
                        scl = originalScl;    
                        squished = false;     
                        squishTimer = 0.0f;   
                    }
                }
            }




#ifdef _DEBUG
            DebugPrintPlayer(i);
#endif
        }
    }
    else if (GetMode() == MODE_RESULT)
    {
        // 結果演出（必要最小限・歩行アニメ停止）
        for (int i = 0; i < MAX_PLAYER; i++)
        {
            if (i == 0) PSetAnimation(i, PLAYER_RESULT_WIN);
            else        PSetAnimation(i, PLAYER_RESULT_LOSE);

            // パーツの時間だけ進める（簡易）
            for (int j = 0; j < PLAYER_PARTS_MAX; j++)
            {
                if (g_Parts[i][j].use && g_Parts[i][j].tblMax > 0)
                {
                    int nowNo = (int)g_Parts[i][j].time;
                    int maxNo = g_Parts[i][j].tblMax;
                    int nextNo = (nowNo + 1) % maxNo;
                    INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i][j].tblNo];

                    XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
                    XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
                    XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);
                    XMVECTOR nextPos = XMLoadFloat3(&tbl[nextNo].pos);
                    XMVECTOR nextRot = XMLoadFloat3(&tbl[nextNo].rot);
                    XMVECTOR nextScl = XMLoadFloat3(&tbl[nextNo].scl);

                    float alpha = g_Parts[i][j].time - nowNo;

                    XMFLOAT3 pos, rot, scl;
                    XMStoreFloat3(&pos, nowPos + (nextPos - nowPos) * alpha);
                    XMStoreFloat3(&rot, nowRot + (nextRot - nowRot) * alpha);
                    XMStoreFloat3(&scl, nowScl + (nextScl - nowScl) * alpha);

                    g_Parts[i][j].pos = pos;
                    g_Parts[i][j].rot = rot;
                    g_Parts[i][j].scl = scl;

                    g_Parts[i][j].time += 1.0f / tbl[nowNo].frame;
                    if ((int)g_Parts[i][j].time >= maxNo) g_Parts[i][j].time -= maxNo;
                }
            }
        }
    }
}

//*****************************************************************************
// 描画
//*****************************************************************************
void DrawPlayer(void)
{
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        if (g_Player[i].use == FALSE) continue;

        SetCullingMode(CULL_MODE_NONE);

        XMMATRIX mtxScl = XMMatrixScaling(g_Player[i].scl.x, g_Player[i].scl.y, g_Player[i].scl.z);
        XMMATRIX mtxQuat = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player[i].Quaternion));
        XMMATRIX mtxYaw = XMMatrixRotationRollPitchYaw(g_Player[i].rot.x, g_Player[i].rot.y + XM_PI, g_Player[i].rot.z);
        XMMATRIX mtxTrs = XMMatrixTranslation(g_Player[i].pos.x, g_Player[i].pos.y, g_Player[i].pos.z);
        XMMATRIX mtxWorld = mtxScl * mtxQuat * mtxYaw * mtxTrs;

        SetWorldMatrix(&mtxWorld);
        XMStoreFloat4x4(&g_Player[i].mtxWorld, mtxWorld);

        if (g_Player[i].muteki)SetEdge(1);
        DrawModel(&g_Player[i].model);

        // パーツ
        for (int j = 0; j < PLAYER_PARTS_MAX; j++)
        {
            if (!g_Parts[i][j].use) continue;

            XMMATRIX m = XMMatrixScaling(g_Parts[i][j].scl.x, g_Parts[i][j].scl.y, g_Parts[i][j].scl.z);
            m = m * XMMatrixRotationRollPitchYaw(g_Parts[i][j].rot.x, g_Parts[i][j].rot.y, g_Parts[i][j].rot.z);
            m = m * XMMatrixTranslation(g_Parts[i][j].pos.x, g_Parts[i][j].pos.y, g_Parts[i][j].pos.z);
            if (g_Parts[i][j].parent)
                m = m * XMLoadFloat4x4(&g_Parts[i][j].parent->mtxWorld);

            SetWorldMatrix(&m);
            DrawModel(&g_Parts[i][j].model);
            SetEdge(0);
        }
    }

    SetDepthEnable(FALSE);
    SetLightEnable(FALSE);
    DrawPlayerHpBar();
    SetLightEnable(TRUE);
    SetDepthEnable(TRUE);
    SetEdge(0);
    SetCullingMode(CULL_MODE_BACK);
}

//*****************************************************************************
// 取得
//*****************************************************************************
PLAYER* GetPlayer(int index) { if (index < 0 || index >= MAX_PLAYER) return nullptr; return &g_Player[index]; }
PLAYER* GetPlayer(void) { return GetPlayer(0); }

//*****************************************************************************
// HPバー（簡易）
//*****************************************************************************
void DrawPlayerHpBar(void)
{
    if (GetMode() == MODE_RESULT) return;

    // 共有VBが無ければ初期化（保険）
    if (!g_PlayerHpBarVertexBuffer) {
        InitPlayerHpBarVertexBuffer();
    }

    struct VERTEX_3D { XMFLOAT3 Position; XMFLOAT3 Normal; XMFLOAT4 Diffuse; XMFLOAT2 TexCoord; };

    CAMERA* cam = GetCamera();
    XMMATRIX mtxView = XMLoadFloat4x4(&cam->mtxView);

    const float HP_WIDTH = 20.0f;
    const float HP_HEIGHT = 6.0f;
    const float maxHp = 3.0f;

    // マテリアルとテクスチャ無効化は最初に1回
    MATERIAL mat = {}; mat.Ambient = mat.Diffuse = XMFLOAT4(1, 1, 1, 1); mat.noTexSampling = 1;
    SetMaterial(mat);
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

    // ビルボードの基底（View の回転だけを取り出す）
    XMMATRIX mtxBB = XMMatrixIdentity();
    mtxBB.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0);
    mtxBB.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0);
    mtxBB.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0);

    // 共有VBを一度だけバインド
    UINT stride = sizeof(VERTEX_3D), offset = 0;
    ID3D11Buffer* vb = g_PlayerHpBarVertexBuffer;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    for (int i = 0; i < MAX_PLAYER; i++)
    {
        const PLAYER& pl = g_Player[i];
        if (!pl.use) continue;

        float percent = pl.hp / maxHp;
        percent = fmaxf(0.0f, fminf(1.0f, percent));

        // バーの表示位置
        XMFLOAT3 hpBarPos = pl.pos;
        hpBarPos.y += 20.0f;

        // 背景（赤）— 幅 HP_WIDTH, 高さ HP_HEIGHT のスケール
        XMMATRIX mRed =
            XMMatrixScaling(HP_WIDTH, HP_HEIGHT, 1.0f) *
            mtxBB *
            XMMatrixTranslation(hpBarPos.x, hpBarPos.y, hpBarPos.z);
        SetWorldMatrix(&mRed);
        MATERIAL m = {};
        m.Diffuse = XMFLOAT4(1, 0, 0, 1);  // 赤など
        m.noTexSampling = 1;
        SetMaterial(m);
        GetDeviceContext()->Draw(4, 0);

        // 緑（HP本体）— 左端を固定したまま X 方向だけ割合スケール
        if (percent > 0.0f)
        {
            // 中心原点の板を左基準にしたいので、縮小後に +((percent - 1) * 0.5 * 幅) を足して位置補正
            float anchorShift = (percent - 1.0f) * 0.5f * HP_WIDTH;

            XMMATRIX mGreen =
                XMMatrixScaling(HP_WIDTH * percent, HP_HEIGHT, 1.0f) *
                XMMatrixTranslation(anchorShift, 0.0f, 0.0f) *
                mtxBB *
                XMMatrixTranslation(hpBarPos.x, hpBarPos.y, hpBarPos.z);
            SetWorldMatrix(&mGreen);
            MATERIAL m = {};
            m.Diffuse = XMFLOAT4(0, 1, 0, 1);  // 赤など
            m.noTexSampling = 1;
            SetMaterial(m);
            GetDeviceContext()->Draw(4, 0);
        }
    }
}

//*****************************************************************************
// 入力/移動
//*****************************************************************************
void MovePlayers(void)
{

    for (int i = 0; i < MAX_PLAYER; i++)
    {
        PLAYER* p = GetPlayer(i);
        if (!p || !p->use) continue;

        p->prevPos = p->pos; // 先読みや被弾演出用

        if (g_IsCPU[i])
        {
            // === 拾った直後の待ち（"持った瞬間に発射"を防止） ===
            if (!g_PartyAI[i].hadWeapon && p->haveWeapon) {
                g_PartyAI[i].postPickDelay = AI_POST_PICK_DELAY;      // 待機をセット
                g_PartyAI[i].aimLockFrames = 0;                       // ロックやり直し
                // 連射抑制：拾った直後はCDも少し入れておく
                if (g_PartyAI[i].fireCooldown < AI_POST_PICK_DELAY / 2)
                    g_PartyAI[i].fireCooldown = AI_POST_PICK_DELAY / 2;
            }
            // 現在の所持状態を記録（各分岐の continue に備えて先に更新）
            g_PartyAI[i].hadWeapon = p->haveWeapon;
            
            // クールダウン更新
            if (g_PartyAI[i].fireCooldown > 0) g_PartyAI[i].fireCooldown--;

            // 1) “武器未所持”なら：アイテム優先
            if (!p->haveWeapon)
            {
                XMFLOAT3 itemPos;
                if (AI_FindNearestWeaponItem(p->pos, itemPos))
                {
                    // 目標アイテムへ接近
                    float vx = itemPos.x - p->pos.x;
                    float vz = itemPos.z - p->pos.z;
                    float v2 = vx * vx + vz * vz;
                    if (v2 > 1e-6f) {
                        float inv = 1.0f / sqrtf(v2);
                        vx *= inv; vz *= inv;

                        // 端回避（軽く内側へ）
                        if (fabsf(p->pos.x) > (STAGE_HALF - EDGE_MARGIN) || fabsf(p->pos.z) > (STAGE_HALF - EDGE_MARGIN)) {
                            vx += (p->pos.x > 0 ? -0.3f : 0.3f);
                            vz += (p->pos.z > 0 ? -0.3f : 0.3f);
                        }

                        // 正規化
                        float s2 = vx * vx + vz * vz;
                        if (s2 > 1e-6f) { float invs = 1.0f / sqrtf(s2); vx *= invs; vz *= invs; }

                        p->pos.x += vx * VALUE_MOVE * AI_SEEK_SPEED;
                        p->pos.z += vz * VALUE_MOVE * AI_SEEK_SPEED;

                        float yaw = YawFromDir(vx, vz);
                        p->rot.y = SmoothAngle(p->rot.y, yaw, 0.20f);
                        g_PlayerIsMoving[i] = true;
                    }
                    else {
                        g_PlayerIsMoving[i] = false;
                    }
                    continue; // ← アイテム優先
                }
                else
                {
                    // 2) フィールドにアイテムが無い：ランダム徘徊
                    if (g_PartyAI[i].thinkCooldown <= 0) {
                        AI_PickNewWanderTarget(i, p->pos);
                    }
                    else {
                        g_PartyAI[i].thinkCooldown--;
                    }

                    float vx = g_PartyAI[i].wanderTarget.x - p->pos.x;
                    float vz = g_PartyAI[i].wanderTarget.z - p->pos.z;
                    float v2 = vx * vx + vz * vz;

                    if (v2 < 4.0f) { // 近づいたら次の点
                        AI_PickNewWanderTarget(i, p->pos);
                    }
                    else {
                        float inv = 1.0f / sqrtf(v2);
                        vx *= inv; vz *= inv;

                        // ふらつき（左右にわずかに）
                        float wob = sinf(g_NoiseClock + i * 0.6f) * 0.12f;
                        float sx = -vz * wob, sz = vx * wob;
                        vx = vx * 0.92f + sx * 0.08f;
                        vz = vz * 0.92f + sz * 0.08f;

                        // 端回避
                        if (fabsf(p->pos.x) > (STAGE_HALF - EDGE_MARGIN) || fabsf(p->pos.z) > (STAGE_HALF - EDGE_MARGIN)) {
                            vx += (p->pos.x > 0 ? -0.3f : 0.3f);
                            vz += (p->pos.z > 0 ? -0.3f : 0.3f);
                        }

                        // 正規化
                        float s2 = vx * vx + vz * vz;
                        if (s2 > 1e-6f) { float invs = 1.0f / sqrtf(s2); vx *= invs; vz *= invs; }

                        p->pos.x += vx * VALUE_MOVE * AI_WANDER_SPEED * g_PartyAI[i].wanderSpeedScale;
                        p->pos.z += vz * VALUE_MOVE * AI_WANDER_SPEED * g_PartyAI[i].wanderSpeedScale;

                        float yaw = YawFromDir(vx, vz);
                        p->rot.y = SmoothAngle(p->rot.y, yaw, 0.16f);
                        g_PlayerIsMoving[i] = true;
                    }
                    continue;
                }
            }

            // 3) “武器を持っている”：最寄りの相手へ向けて発射（ロック制）
            int target = AI_FindNearestOpponent(i);
            if (target >= 0)
            {
                PLAYER* t = GetPlayer(target);

                // 目標角（最も近い相手）
                float tx = t->pos.x - p->pos.x;
                float tz = t->pos.z - p->pos.z;
                float yawTarget = YawFromDir(tx, tz);

                // まずは滑らかに向く
                p->rot.y = SmoothAngle(p->rot.y, yawTarget, 0.22f);

                // --- ロック条件評価 ---
                // 角度誤差（度）を測って、一定フレーム連続でしきい値以内なら"ロック完了"
                float degErr = fabsf(AI_AngleDiff(p->rot.y, yawTarget)) * 180.0f / XM_PI;
                if (degErr <= AI_AIM_LOCK_DEG) g_PartyAI[i].aimLockFrames++;
                else                           g_PartyAI[i].aimLockFrames = 0;

                // 拾った直後は一定フレーム待機（向き直しのラグを待つ）
                if (g_PartyAI[i].postPickDelay > 0) {
                    g_PartyAI[i].postPickDelay--;
                }

                // --- 発射判定 ---
                if (g_PartyAI[i].fireCooldown == 0 &&
                    g_PartyAI[i].postPickDelay == 0 &&
                    g_PartyAI[i].aimLockFrames >= AI_AIM_LOCK_FRAMES)
                {
                    // 発射直前にランダムノイズを角度へ付与（±AI_NOISE_DEG_MAX）
                    float yawShoot = yawTarget;
                    if (AI_Frand01() < AI_NOISE_PROB) {
                        float noiseDeg = (AI_Frand01() * 2.0f - 1.0f) * AI_NOISE_DEG_MAX;
                        yawShoot += noiseDeg * (XM_PI / 180.0f);
                    }
                    // その瞬間だけ発射角に合わせる（見た目はほぼ揃った状態で撃つ）
                    p->rot.y = yawShoot;

                    AI_ForceAttack(i);  // 入力不要の発射

                    // 連射防止＆ロックをリセット
                    g_PartyAI[i].fireCooldown =
                        AI_FIRE_CD_MIN + (rand() % (AI_FIRE_CD_MAX - AI_FIRE_CD_MIN + 1));
                    g_PartyAI[i].aimLockFrames = 0;
                }
                // フィールドを回る動き（CPUごとに変化）
                float nd = sqrtf(tx * tx + tz * tz);
                if (nd > 1e-6f) { tx /= nd; tz /= nd; }

                float rpick = fmodf(AI_Frand01() + i * 0.23f, 1.0f);

                if (rpick < 0.60f) {
                    // 徘徊：ターゲットがいても一定確率で wander を優先
                    if (g_PartyAI[i].thinkCooldown <= 0) { AI_PickNewWanderTarget(i, p->pos); }
                    float vx = g_PartyAI[i].wanderTarget.x - p->pos.x;
                    float vz = g_PartyAI[i].wanderTarget.z - p->pos.z;
                    float v2 = vx * vx + vz * vz;
                    if (v2 < 9.0f) { AI_PickNewWanderTarget(i, p->pos); }
                    else {
                        float inv = 1.0f / sqrtf(v2); vx *= inv; vz *= inv;
                        float wob = sinf(g_NoiseClock + i * 0.6f) * 0.12f;
                        float sx = -vz * wob, sz = vx * wob;
                        vx = vx * 0.90f + sx * 0.10f;
                        vz = vz * 0.90f + sz * 0.10f;
                        if (fabsf(p->pos.x) > (STAGE_HALF - EDGE_MARGIN) || fabsf(p->pos.z) > (STAGE_HALF - EDGE_MARGIN)) {
                            vx += (p->pos.x > 0 ? -0.3f : 0.3f);
                            vz += (p->pos.z > 0 ? -0.3f : 0.3f);
                        }
                        float s2 = vx * vx + vz * vz; if (s2 > 1e-6f) { float invs = 1.0f / sqrtf(s2); vx *= invs; vz *= invs; }
                        p->pos.x += vx * VALUE_MOVE * AI_WANDER_SPEED * g_PartyAI[i].wanderSpeedScale;
                        p->pos.z += vz * VALUE_MOVE * AI_WANDER_SPEED * g_PartyAI[i].wanderSpeedScale;
                        float yaw = YawFromDir(vx, vz);
                        p->rot.y = SmoothAngle(p->rot.y, yaw, 0.16f);
                    }
                }
                else if (nd > 18.0f) {
                    // 遠い：ゆっくり接近（端回避を混ぜる）
                    float vx = tx, vz = tz;
                    if (fabsf(p->pos.x) > (STAGE_HALF - EDGE_MARGIN) || fabsf(p->pos.z) > (STAGE_HALF - EDGE_MARGIN)) {
                        vx += (p->pos.x > 0 ? -0.3f : 0.3f);
                        vz += (p->pos.z > 0 ? -0.3f : 0.3f);
                    }
                    float s2 = vx * vx + vz * vz; if (s2 > 1e-6f) { float invs = 1.0f / sqrtf(s2); vx *= invs; vz *= invs; }
                    p->pos.x += vx * VALUE_MOVE * 0.40f;
                    p->pos.z += vz * VALUE_MOVE * 0.40f;
                }
                else {
                    // 近い：接線方向へ流れて“張り付き”を避ける（少し離脱も）
                    float vx = -tz, vz = tx;
                    float s2 = vx * vx + vz * vz; if (s2 > 1e-6f) { float invs = 1.0f / sqrtf(s2); vx *= invs; vz *= invs; }
                    float back = (nd < 12.0f) ? 0.35f : 0.15f;
                    vx = vx * 0.85f + (-tx) * back * 0.15f;
                    vz = vz * 0.85f + (-tz) * back * 0.15f;
                    vx += (AI_Frand01() - 0.5f) * 0.15f;
                    vz += (AI_Frand01() - 0.5f) * 0.15f;
                    float s3 = vx * vx + vz * vz; if (s3 > 1e-6f) { float invs = 1.0f / sqrtf(s3); vx *= invs; vz *= invs; }
                    p->pos.x += vx * VALUE_MOVE * 0.55f;
                    p->pos.z += vz * VALUE_MOVE * 0.55f;
                }

                g_PlayerIsMoving[i] = true;
}
            else
            {
                // 相手が居ないレアケース：徘徊へ
                if (g_PartyAI[i].thinkCooldown <= 0) AI_PickNewWanderTarget(i, p->pos);
                g_PlayerIsMoving[i] = false;
            }

            continue; // CPUブロック終わり
        }
        else
        {
            float dx = 0.0f, dz = 0.0f;
            if (i == 0) {
                if (GetKeyboardPress(DIK_A)) dx -= 1.0f;
                if (GetKeyboardPress(DIK_D)) dx += 1.0f;
                if (GetKeyboardPress(DIK_W)) dz += 1.0f;
                if (GetKeyboardPress(DIK_S)) dz -= 1.0f;
                if (IsButtonPressed(0, BUTTON_UP)) dz += 1.0f;
                if (IsButtonPressed(0, BUTTON_DOWN)) dz -= 1.0f;
                if (IsButtonPressed(0, BUTTON_LEFT)) dx -= 1.0f;
                if (IsButtonPressed(0, BUTTON_RIGHT)) dx += 1.0f;
            }
            else if (i == 1) {
                if (GetKeyboardPress(DIK_LEFT))  dx -= 1.0f;
                if (GetKeyboardPress(DIK_RIGHT)) dx += 1.0f;
                if (GetKeyboardPress(DIK_UP))    dz += 1.0f;
                if (GetKeyboardPress(DIK_DOWN))  dz -= 1.0f;
                if (IsButtonPressed(1, BUTTON_UP)) dz += 1.0f;
                if (IsButtonPressed(1, BUTTON_DOWN)) dz -= 1.0f;
                if (IsButtonPressed(1, BUTTON_LEFT)) dx -= 1.0f;
                if (IsButtonPressed(1, BUTTON_RIGHT)) dx += 1.0f;
            }

            if (dx != 0.0f || dz != 0.0f) {
                float inv = 1.0f / sqrtf(dx * dx + dz * dz);
                dx *= inv; dz *= inv;
                p->pos.x += dx * VALUE_MOVE;
                p->pos.z += dz * VALUE_MOVE;

                float targetAngle = atan2f(-dx, -dz);
                p->rot.y = SmoothAngle(p->rot.y, targetAngle, 0.15f);
                g_PlayerIsMoving[i] = true;
            }
            else {
                g_PlayerIsMoving[i] = false;
            }
        }
    }
}

//*****************************************************************************
// HP増減
//*****************************************************************************
void AddPlayerHP(int index, float add)
{
    g_Player[index].hp += add;
    if (g_Player[index].hp > PLAYER_HP_MAX)
    {
        g_Player[index].hp = PLAYER_HP_MAX;
    }
}

//*****************************************************************************
// 階層アニメ
//*****************************************************************************
void UpdatePlayerPartsAnimation(int playerIndex)
{
    float blendDuration = 12.0f;
    bool isMoving = g_PlayerIsMoving[playerIndex];

    // 状態遷移
    if (isMoving && !g_PlayerPrevMoving[playerIndex]) {
        g_PlayerAnimBlendTimer[playerIndex] = 0;
        g_PlayerAnimBlendMode[playerIndex] = 1; // IN
    }
    else if (!isMoving && g_PlayerPrevMoving[playerIndex]) {
        g_PlayerAnimBlendTimer[playerIndex] = 0;
        g_PlayerAnimBlendMode[playerIndex] = 2; // OUT
        for (int j = 0; j < PLAYER_PARTS_MAX; j++)
            g_PlayerAnimBlendFrom[playerIndex][j] = g_Parts[playerIndex][j].rot.x;
    }
    g_PlayerPrevMoving[playerIndex] = isMoving;

    for (int j = 0; j < PLAYER_PARTS_MAX; j++)
    {
        if (!g_Parts[playerIndex][j].use || g_Parts[playerIndex][j].tblMax <= 0) continue;

        int nowNo = (int)g_Parts[playerIndex][j].time;
        int maxNo = g_Parts[playerIndex][j].tblMax;
        int nextNo = (nowNo + 1) % maxNo;
        INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[playerIndex][j].tblNo];

        XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);
        XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);
        XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);
        XMVECTOR nxtPos = XMLoadFloat3(&tbl[nextNo].pos);
        XMVECTOR nxtRot = XMLoadFloat3(&tbl[nextNo].rot);
        XMVECTOR nxtScl = XMLoadFloat3(&tbl[nextNo].scl);

        float alpha = g_Parts[playerIndex][j].time - nowNo;
        XMFLOAT3 pos, rot, scl;
        XMStoreFloat3(&pos, nowPos + (nxtPos - nowPos) * alpha);
        XMStoreFloat3(&rot, nowRot + (nxtRot - nowRot) * alpha);
        XMStoreFloat3(&scl, nowScl + (nxtScl - nowScl) * alpha);

        // ブレンド
        if (g_PlayerAnimBlendMode[playerIndex] == 1 && g_PlayerAnimBlendTimer[playerIndex] < blendDuration) {
            float t = g_PlayerAnimBlendTimer[playerIndex] / blendDuration;
            rot.x *= t;
        }
        else if (g_PlayerAnimBlendMode[playerIndex] == 2 && g_PlayerAnimBlendTimer[playerIndex] < blendDuration) {
            float t = g_PlayerAnimBlendTimer[playerIndex] / blendDuration;
            rot.x = g_PlayerAnimBlendFrom[playerIndex][j] * (1.0f - t);
        }
        else if (!isMoving) {
            rot.x = 0.0f;
        }

        if (isMoving) {
            float tNoise = g_NoiseClock;
            rot.x += GetPeriodicNoise(playerIndex * 50 + j * 23, tNoise, 0.7f);
            rot.y += GetSmoothNoise(noiseTable[playerIndex][j], 1.0f);
        }

        g_Parts[playerIndex][j].pos = pos;
        g_Parts[playerIndex][j].rot = rot;
        g_Parts[playerIndex][j].scl = scl;

        if (isMoving) {
            g_Parts[playerIndex][j].time += 1.0f / tbl[nowNo].frame;
            if ((int)g_Parts[playerIndex][j].time >= maxNo) g_Parts[playerIndex][j].time -= maxNo;
        }
    }

    if ((g_PlayerAnimBlendMode[playerIndex] == 1 || g_PlayerAnimBlendMode[playerIndex] == 2) &&
        g_PlayerAnimBlendTimer[playerIndex] < blendDuration)
    {
        g_PlayerAnimBlendTimer[playerIndex]++;
    }
    else if (g_PlayerAnimBlendMode[playerIndex] != 0 &&
        g_PlayerAnimBlendTimer[playerIndex] >= blendDuration)
    {
        g_PlayerAnimBlendMode[playerIndex] = 0;
    }
}

//*****************************************************************************
// 被弾ノックバック
//*****************************************************************************
void UpdatePlayerKnockback(int playerIndex)
{
    if (g_PlayerState[playerIndex] != PLAYER_HIT) return;

    g_Player[playerIndex].pos.x += g_PlayerKnockback[playerIndex].x;
    g_Player[playerIndex].pos.y += g_PlayerKnockback[playerIndex].y;
    g_Player[playerIndex].pos.z += g_PlayerKnockback[playerIndex].z;

    g_PlayerKnockback[playerIndex].x *= 0.92f;
    g_PlayerKnockback[playerIndex].y *= 0.80f;
    g_PlayerKnockback[playerIndex].z *= 0.92f;

    for (int j = 0; j < PLAYER_PARTS_MAX; j++) {
        g_Parts[playerIndex][j].rot.x = sinf(g_PlayerAnimTimer[playerIndex] * 0.6f + j) * 0.7f;
        g_Parts[playerIndex][j].rot.y = sinf(g_PlayerAnimTimer[playerIndex] * 0.7f + j * 2) * 0.6f;
        g_Parts[playerIndex][j].rot.z = sinf(g_PlayerAnimTimer[playerIndex] * 0.5f + j * 3) * 0.5f;
    }

    g_PlayerAnimTimer[playerIndex]++;
    if (g_PlayerAnimTimer[playerIndex] > 36) {
        g_PlayerState[playerIndex] = PLAYER_NORMAL;
        g_PlayerAnimTimer[playerIndex] = 0;
        g_PlayerKnockback[playerIndex] = XMFLOAT3(0, 0, 0);
    }
}

void OnPlayerHit(int i, const XMFLOAT3& hitDirection)
{
    g_PlayerState[i] = PLAYER_HIT;
    g_PlayerAnimTimer[i] = 0;
    float power = 1.2f;
    g_PlayerKnockback[i].x = hitDirection.x * power;
    g_PlayerKnockback[i].y = 0.2f;
    g_PlayerKnockback[i].z = hitDirection.z * power;
}

//*****************************************************************************
// ノイズ/演出
//*****************************************************************************
float GetNoise(float s) { return GetRand(-1000, 1000) / 500.0f * s; }
float GetPeriodicNoise(int seed, float t, float s) { return sinf(t + seed) * s; }
float GetSmoothNoise(float& last, float s) { float tgt = GetRand(-1000, 1000) / 500.0f * s; float sm = 0.07f; last += (tgt - last) * sm; return last; }

//*****************************************************************************
// アニメ切替
//*****************************************************************************
void PSetAnimation(int playerIndex, PLAYER_STATE animation)
{
    g_Player[playerIndex].currentAnimation = animation;
    switch (animation)
    {
    case PLAYER_WALK:
        g_Parts[playerIndex][1].tblNo = B_WALK_L_ARM_ANIMATION_INDEX;
        g_Parts[playerIndex][2].tblNo = B_WALK_R_ARM_ANIMATION_INDEX;
        break;
    case PLAYER_RESULT_WIN:
        g_Parts[playerIndex][1].tblNo = B_RESULT_WIN_L_ARM_ANIMATION_INDEX;
        g_Parts[playerIndex][2].tblNo = B_RESULT_WIN_R_ARM_ANIMATION_INDEX;
        break;
    case PLAYER_RESULT_LOSE:
        g_Parts[playerIndex][1].tblNo = B_RESULT_LOSE_L_ARM_ANIMATION_INDEX;
        g_Parts[playerIndex][2].tblNo = B_RESULT_LOSE_R_ARM_ANIMATION_INDEX;
        break;
    default: break;
    }
}

//*****************************************************************************
// 角度ユーティリティ
//*****************************************************************************
float NormalizeAngle(float a) { while (a > XM_PI) a -= XM_2PI; while (a < -XM_PI) a += XM_2PI; return a; }
float SmoothAngle(float cur, float tgt, float s) { cur = NormalizeAngle(cur); tgt = NormalizeAngle(tgt); float d = NormalizeAngle(tgt - cur); return cur + d * s; }
float turning(float target, float current)
{
    float diff = target - current;
    while (diff > XM_PI) diff -= XM_2PI;
    while (diff < -XM_PI) diff += XM_2PI;
    const float rotateSpeed = 0.15f;
    return current + diff * rotateSpeed;
}

//*****************************************************************************
// 攻撃（入力）
//*****************************************************************************
void PlayerAttack(int playerIndex)
{
    bool attackTrigger = false;
    if (playerIndex == 0) attackTrigger = GetKeyboardTrigger(DIK_SPACE) || IsButtonTriggered(0, BUTTON_A);
    else if (playerIndex == 1) attackTrigger = GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(1, BUTTON_A);

    if (!attackTrigger) return;

    // ボール
    BALL* ball = GetBall();
    for (int k = 0; k < 10; k++) {
        if (ball[k].IsUsedITball() && ball[k].IsPickedITball()) {
            ball[k].SetITball(g_Player[playerIndex].pos, g_Player[playerIndex].rot);
            break;
        }
    }
    // ボム
    BOMB* bomb = GetBomb();
    for (int k = 0; k < 10; k++) {
        if (bomb[k].IsUsedITbomb() && bomb[k].IsPickedITbomb()) {
            bomb[k].SetITbomb(g_Player[playerIndex].pos, g_Player[playerIndex].rot);
            break;
        }
    }
    // ブーメラン
    BOOM* boom = GetBoomerang();
    boom->SetThrowFlag(true);
    // ハンマー
    HAMR* hamr = GetHammer();
    hamr->SetSwingFlag(true);
}

//*****************************************************************************
// === ここから共通ヘルパ ===
//*****************************************************************************
void AdjustYByTerrainAndUpdateShadow(int i, DirectX::XMFLOAT3& outNormal)
{
    PLAYER* p = GetPlayer(i); if (!p) return;

    XMFLOAT3 hitPos, normal;
    if (RayHitField(p->pos, &hitPos, &normal)) {
        p->pos.y = hitPos.y + PLAYER_OFFSET_Y;
        outNormal = normal;
    }
    else {
        p->pos.y = PLAYER_OFFSET_Y;
        outNormal = XMFLOAT3(0, 1, 0);
    }

    XMFLOAT3 sh = p->pos; sh.y -= (PLAYER_OFFSET_Y - 0.1f);
    SetPositionShadow(p->shadowIdx, sh);
}

void UpdatePoseByGroundNormal(int i, const DirectX::XMFLOAT3& n)
{
    PLAYER* p = GetPlayer(i); if (!p) return;

    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR nv = XMLoadFloat3(&n);
    XMVECTOR axis = XMVector3Normalize(XMVector3Cross(up, nv));
    float   len = XMVectorGetX(XMVector3Length(XMVector3Cross(up, nv)));
    float   ang = asinf(len);
    XMVECTOR tgtQ = XMQuaternionRotationNormal(axis, ang);
    XMVECTOR curQ = XMLoadFloat4(&p->Quaternion);
    XMVECTOR newQ = XMQuaternionSlerp(curQ, tgtQ, 0.05f);
    XMStoreFloat4(&p->Quaternion, newQ);
}

void UpdateCollisionCapsule(int i)
{
    PLAYER* p = GetPlayer(i); if (!p) return;
    p->capsuleA = XMFLOAT3(p->pos.x, p->pos.y - PLAYER_HEIGHT / 2.0f, p->pos.z);
    p->capsuleB = XMFLOAT3(p->pos.x, p->pos.y + PLAYER_HEIGHT / 2.0f, p->pos.z);
    p->radius = PLAYER_SIZE; // ★ 常に同期（半径参照系の当たりを保証）
}

void UpdateWarpGateCooldown(int i)
{
    if (g_Player[i].gateUse) {
        if (++g_Player[i].gateCoolTime >= 300) {
            g_Player[i].gateCoolTime = 0;
            g_Player[i].gateUse = FALSE;
        }
    }
}

bool HandleStunAndKnockback(int i)
{
    PLAYER* p = GetPlayer(i); if (!p || !p->use) return false;

    if (g_PlayerState[i] == PLAYER_HIT) {
        UpdatePlayerKnockback(i);
        return true; // 被弾中はここで止める
    }
    if (p->stunTimer > 0.0f) {
        p->pos.x += p->knockbackVel.x;
        p->pos.z += p->knockbackVel.z;
        p->stunTimer -= 1.0f / 60.0f;
        if (p->stunTimer <= 0) { p->stunTimer = 0; p->knockbackVel = XMFLOAT3(0, 0, 0); }
        return true;
    }
    return false;
}

void UpdateHpDebugKeys(int i)
{
#ifdef _DEBUG
    if (GetKeyboardTrigger(DIK_L)) g_Player[i].hp = max(g_Player[i].hp - 1.0f, 0.0f);
    if (GetKeyboardTrigger(DIK_O)) g_Player[i].hp = min(g_Player[i].hp + 1.0f, 3.0f);
#endif
}

void DebugPrintPlayer(int i)
{
#ifdef _DEBUG
    PrintDebugProc("P%d pos:(%.2f,%.2f,%.2f) rotY:%.2f HP:%.1f state:%d gateCT:%d\n",
        i, g_Player[i].pos.x, g_Player[i].pos.y, g_Player[i].pos.z,
        g_Player[i].rot.y, g_Player[i].hp, (int)g_PlayerState[i], g_Player[i].gateCoolTime);
#endif
}

// ==== AI ユーティリティ実装 ====
float AI_Frand01(void) { return (float)(rand() & 1023) / 1023.0f; }

float AI_AngleDiff(float a, float b) {
    float d = a - b;
    while (d > XM_PI) d -= XM_2PI;
    while (d < -XM_PI) d += XM_2PI;
    return d;
}

int AI_FindNearestOpponent(int selfIndex) {
    PLAYER* me = GetPlayer(selfIndex);
    if (!me || !me->use) return -1;
    int best = -1; float bestD2 = FLT_MAX;
    for (int j = 0; j < MAX_PLAYER; j++) {
        if (j == selfIndex) continue;
        PLAYER* pj = GetPlayer(j);
        if (!pj || !pj->use) continue;
        float dx = pj->pos.x - me->pos.x;
        float dz = pj->pos.z - me->pos.z;
        float d2 = dx * dx + dz * dz;
        if (d2 < bestD2) { bestD2 = d2; best = j; }
    }
    return best;
}

// 拾える“武器系アイテム”の最近傍を探索（見つかれば true）
bool AI_FindNearestWeaponItem(const DirectX::XMFLOAT3& selfPos, DirectX::XMFLOAT3& outPos)
{
    float bestD2 = FLT_MAX; bool found = false;

    // Ball（配列10想定）
    if (BALL* ball = GetBall()) {
        for (int i = 0; i < 10; i++) {
            if (!ball[i].IsUsedITball()) continue;
            if (ball[i].IsPickedITball()) continue;
            if (ball[i].IsThrewITball())  continue;
            XMFLOAT3 p = ball[i].GetPositionITball();
            float dx = p.x - selfPos.x, dz = p.z - selfPos.z, d2 = dx * dx + dz * dz;
            if (d2 < bestD2) { bestD2 = d2; outPos = p; found = true; }
        }
    }
    // Bomb（配列10想定）
    if (BOMB* bomb = GetBomb()) {
        for (int i = 0; i < 10; i++) {
            if (!bomb[i].IsUsedITbomb()) continue;
            if (bomb[i].IsPickedITbomb()) continue;
            if (bomb[i].IsExpUseITbomb()) continue;
            if (bomb[i].IsThrewITbomb())  continue;
            XMFLOAT3 p = bomb[i].GetPositionITbomb();
            float dx = p.x - selfPos.x, dz = p.z - selfPos.z, d2 = dx * dx + dz * dz;
            if (d2 < bestD2) { bestD2 = d2; outPos = p; found = true; }
        }
    }
    // Boomerang（単体想定）
    if (BOOM* boom = GetBoomerang()) {
        if (boom->IsUsedITboom() && !boom->IsPickedITboom() && !boom->IsThrewITboom()) {
            XMFLOAT3 p = boom->GetPositionITboom();
            float dx = p.x - selfPos.x, dz = p.z - selfPos.z, d2 = dx * dx + dz * dz;
            if (d2 < bestD2) { bestD2 = d2; outPos = p; found = true; }
        }
    }
    // Hammer（単体想定）
    if (HAMR* hamr = GetHammer()) {
        if (hamr->IsUsedITHamr() && !hamr->IsPickedITHamr()) {
            XMFLOAT3 p = hamr->GetPositionITHamr();
            float dx = p.x - selfPos.x, dz = p.z - selfPos.z, d2 = dx * dx + dz * dz;
            if (d2 < bestD2) { bestD2 = d2; outPos = p; found = true; }
        }
    }
    return found;
}


void AI_ForceAttack(int playerIndex)
{
    // ボール
    if (BALL* ball = GetBall()) {
        for (int k = 0; k < 10; k++) {
            if (ball[k].IsUsedITball() && ball[k].IsPickedITball()) {
                ball[k].SetITball(g_Player[playerIndex].pos, g_Player[playerIndex].rot);
                return;
            }
        }
    }
    // ボム
    if (BOMB* bomb = GetBomb()) {
        for (int k = 0; k < 10; k++) {
            if (bomb[k].IsUsedITbomb() && bomb[k].IsPickedITbomb()) {
                bomb[k].SetITbomb(g_Player[playerIndex].pos, g_Player[playerIndex].rot);
                return;
            }
        }
    }
    // ブーメラン（単体想定）
    if (BOOM* boom = GetBoomerang()) { boom->SetThrowFlag(true); }
    // ハンマー（単体想定）
    if (HAMR* hamr = GetHammer()) { hamr->SetSwingFlag(true); }
}

// 人間操作と同じ角度系：世界方向ベクトル(vx,vz) → yaw
static inline float YawFromDir(float vx, float vz)
{
    // 人間側は targetAngle = atan2f(-dx, -dz) を使っている
    return atan2f(-vx, -vz);
}

void AI_PickNewWanderTarget(int selfIndex, const XMFLOAT3& origin)
{
    int nearIdx = AI_FindNearestOpponent(selfIndex);
    XMFLOAT3 focus = (nearIdx >= 0) ? GetPlayer(nearIdx)->pos
        : XMFLOAT3{ origin.x + 1.0f, origin.y, origin.z + 1.0f };

    float vx = focus.x - origin.x;
    float vz = focus.z - origin.z;
    float len = sqrtf(vx * vx + vz * vz);
    if (len > 1e-4f) { vx /= len; vz /= len; }
    else { vx = 0.0f; vz = 1.0f; }

    // 半径 AI_WANDER_RADIUS の範囲で「前方 ±θ」にランダム配置
    float dist = AI_WANDER_RADIUS * (0.6f + AI_Frand01() * 0.6f);   // 0.6〜1.2倍
    float ang = (AI_Frand01() - 0.5f) * XM_PI * 0.65f;             // ±約117°
    float ca = cosf(ang), sa = sinf(ang);
    float rx = vx * ca - vz * sa;
    float rz = vx * sa + vz * ca;

    XMFLOAT3 tgt{ origin.x + rx * dist, origin.y, origin.z + rz * dist };

    const float STAGE_HALF = 85.0f, EDGE_MARGIN = 5.0f;
    tgt.x = fmaxf(-STAGE_HALF + EDGE_MARGIN, fminf(STAGE_HALF - EDGE_MARGIN, tgt.x));
    tgt.z = fmaxf(-STAGE_HALF + EDGE_MARGIN, fminf(STAGE_HALF - EDGE_MARGIN, tgt.z));

    g_PartyAI[selfIndex].wanderTarget = tgt;
    g_PartyAI[selfIndex].wanderSpeedScale = 0.85f + 0.3f * AI_Frand01();
    g_PartyAI[selfIndex].thinkCooldown = 36 + (rand() % 25); // 36〜60F
}

bool IsBeachStage()
{
    // g_SelectedStageFile に "beach" が含まれたら水上アスレ扱い
    const char* s = g_SelectedStageFile;
    if (!s) return false;
    for (const char* p = s; *p; ++p) {
        char c = *p;
        if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a'); // 小文字化
        if (c == 'b' && p[1] == 'e' && p[2] == 'a' && p[3] == 'c' && p[4] == 'h') return true;
    }
    return false;
}

void UpdateFallAndRespawn(int idx)
{
    PLAYER& pl = GetPlayer(idx)[0]; // GetPlayer(idx) がポインタ返しでも安全に

    // 1) 床めり込みの押し戻し（坂・段差で“地面に押し戻す”）
    ResolveFloorPenetration(&pl.pos, pl.radius, 0.02f);  // 上下分離

    // 2) “安全地点”の更新（床上で安定しているとみなせるフレーム）
    static DirectX::XMFLOAT3 s_LastSafePos[MAX_PLAYER];
    const float dy = pl.pos.y - pl.prevPos.y;
    if (fabsf(dy) < 0.2f) s_LastSafePos[idx] = pl.pos;

    // 3) 水上アスレ（beach）での落水復帰
    auto isBeach = []() -> bool {
        extern char g_SelectedStageFile[];
        for (const char* p = g_SelectedStageFile; *p; ++p) {
            char c = (*p | 32);
            if (c == 'b' && (p[1] | 32) == 'e' && (p[2] | 32) == 'a' && (p[3] | 32) == 'c' && (p[4] | 32) == 'h') return true;
        }
        return false;
        };
    const float waterY = GetWaterLevel();
    if (isBeach() && waterY > -1.0e8f && pl.pos.y < (waterY - 2.0f)) {
        DirectX::XMFLOAT3 revive;
        if (!StageGetNearestFloorPoint(pl.pos, &revive, pl.radius + 0.1f)) {
            revive = s_LastSafePos[idx]; revive.y += pl.radius + 0.1f;
        }
        pl.pos = revive; pl.prevPos = pl.pos;
        return;
    }

    // 4) キルゾーン（どこまでも落下した時の保険）
    if (pl.pos.y < -50.0f) {
        DirectX::XMFLOAT3 revive;
        if (!StageGetNearestFloorPoint(pl.pos, &revive, pl.radius + 0.1f)) {
            revive = s_LastSafePos[idx]; revive.y += pl.radius + 0.1f;
        }
        pl.pos = revive; pl.prevPos = pl.pos;
        return;
    }
}
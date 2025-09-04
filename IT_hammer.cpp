#include "main.h"        // ★重要：最初に
#include "input.h"
#include "IT_hammer.h"
#include "renderer.h"
#include "player.h"
#include "camera.h"
#include "debugproc.h"
#include "shadow.h"
#include "sound.h"
#include "score.h"
//*****************************************************************************
// マクロ
//*****************************************************************************
#define MODEL_HAMR           "data/MODEL/item/item_hammer.obj"
#define VALUE_MOVE           (10.0f)
#define VALUE_ROTATE         (XM_PI * 0.02f)
#define HAMR_SHADOW_SIZE     (0.4f)
#define HAMR_OFFSET_Y        (7.0f)
#define HAMR_DAMAGE          (5.0f)

// アイコン用
#define TEXTURE_MAX          (1)
#define ICON_WIDTH           (10.0f)
#define ICON_HEIGHT          (10.0f)

//*****************************************************************************
// プロトタイプ
//*****************************************************************************
static HRESULT MakeVertexITHamrIcon(void);

//*****************************************************************************
// グローバル
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };
static BOOL                        g_bAlpaTest = FALSE;

static BOOL         hammer_load;
static DX11_MODEL   hammer_model;

// テクスチャ
static char* g_TextureName[] =
{
    "data/TEXTURE/tree001.png",
};

//=============================================================================
// 初期化
//=============================================================================
HRESULT HAMR::InitITHamr(void)
{
    hammer_load = TRUE;
    LoadModel(MODEL_HAMR, &hammer_model);

    use = FALSE;
    to_swing = FALSE;
    pick = FALSE;

    pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
    scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

    move = XMFLOAT3(VALUE_MOVE, 1.0f, VALUE_MOVE);
    size = HAMR_SIZE;
    count = 0.0f;
    PlayerIndex = -1;

    // 入力・スイング状態
    wantToSwing = false;
    swingAngle = 0.0f;

    // ★耐久
    swingCount = 0;

    // アイコン
    MakeVertexITHamrIcon();
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(
            GetDevice(), g_TextureName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    ZeroMemory(&icon_material, sizeof(icon_material));
    icon_material.Diffuse = XMFLOAT4(1, 1, 1, 1);
    icon_pos = XMFLOAT3(0, 0, 0);
    icon_fWidth = ICON_WIDTH;
    icon_fHeight = ICON_HEIGHT;
    icon_scl = XMFLOAT3(1.0f / ICON_WIDTH, 1.0f / ICON_HEIGHT, 1.0f / ICON_HEIGHT);
    icon_use = FALSE;

    g_bAlpaTest = FALSE;

    return S_OK;
}

//=============================================================================
// 終了
//=============================================================================
void HAMR::UninitITHamr(void)
{
    use = FALSE;

    if (hammer_load == TRUE)
    {
        UnloadModel(&hammer_model);
        hammer_load = FALSE;
    }

    for (int n = 0; n < TEXTURE_MAX; ++n)
    {
        if (g_Texture[n] != NULL)
        {
            g_Texture[n]->Release();
            g_Texture[n] = NULL;
        }
    }
    if (g_VertexBuffer != NULL)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    // ★耐久リセット
    swingCount = 0;
    wantToSwing = false;
    to_swing = FALSE;
    pick = FALSE;
    PlayerIndex = -1;
}

//=============================================================================
// 更新
//=============================================================================
void HAMR::UpdateITHamr(void)
{
    if (!use || !hammer_load) return;

    // 落ちている場合：地面でバウンド等あればここに
    if (!pick)
    {
        // アイテム状態の見せ方があればここに（省略なしで何も変更していません）
    }

    // 装備中：プレイヤーの位置・向きを追従
    if (pick)
    {
        PLAYER* pl = GetPlayer(PlayerIndex);
        if (pl && pl->use)
        {
            pos = pl->pos;
            rot = pl->rot;
        }

        // 入力：プレイヤー側から SetSwingFlag(true) が来る
        if (!to_swing && wantToSwing)
        {
            to_swing = TRUE;
            wantToSwing = false;
            scoredThisSwing = false; // <-- reset at start of swing
            PlaySound(SOUND_LABEL_SE_shot002);
        }

    }

    // スイング
    if (to_swing)
    {
        const float swingSpeed = 0.22f;
        swingAngle += swingSpeed;

        if (swingAngle >= swingMax)
        {
            swingAngle = swingMax;
            to_swing = FALSE;

            // ★ここで1回消費（3回で壊れる）
            swingCount++;

            if (swingCount >= 3)
            {
                // 所持解除
                if (pick && PlayerIndex >= 0)
                {
                    PLAYER* pl = GetPlayer(PlayerIndex);
                    if (pl) pl->haveWeapon = FALSE; // プレイヤーの所持解除 :contentReference[oaicite:2]{index=2}
                }

                // アイテム破壊（非表示化）
                use = FALSE;
                pick = FALSE;
                to_swing = FALSE;
                PlayerIndex = -1;
                swingCount = 0;
                scl = XMFLOAT3(0, 0, 0);
                return;
            }
        }
    }
    else
    {
        const float returnSpeed = 0.12f;
        if (swingAngle > 0.0f)
        {
            swingAngle -= returnSpeed;
            if (swingAngle < 0.0f) swingAngle = 0.0f;
        }
    }

    // アイコンなどの汎用更新があればここに（現状そのまま）
}

//=============================================================================
// 描画
//=============================================================================
void HAMR::DrawITHamr(void)
{
    if (!use) return;

    // カリングを両面にするなどの描画設定が必要ならここで
    // SetCullingMode(CULL_MODE_NONE);

    // プレイヤーに装備されている前提での変換
    XMMATRIX mtxWorld = XMMatrixIdentity();

    const float halfH = scl.y * 0.5f;

    // Scale
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixScaling(scl.x, scl.y, scl.z));
    // ピボット移動（下端に回転中心）
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(0.0f, halfH, 0.0f));
    // スイング回転（前方向へ倒す想定：-X）
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixRotationX(-swingAngle));
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(0.0f, -halfH, 0.0f));
    // プレイヤー回転・位置
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z));
    mtxWorld = XMMatrixMultiply(mtxWorld, XMMatrixTranslation(pos.x, pos.y, pos.z));

    SetWorldMatrix(&mtxWorld);
    XMStoreFloat4x4(&m_mtxWorld, mtxWorld);
    DrawModel(&hammer_model);

    // SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// 生成
//=============================================================================
void HAMR::SetITHamrObject(XMFLOAT3 set_pos)
{
    use = TRUE;
    pick = FALSE;
    to_swing = FALSE;
    wantToSwing = false;

    pos = set_pos;
    rot = XMFLOAT3(0, 0, 0);
    scl = XMFLOAT3(1, 1, 1);
    PlayerIndex = -1;

    swingAngle = 0.0f;
    swingCount = 0;
}

//=============================================================================
// ヒット（装備中の当たり判定から呼ばれる）
//=============================================================================

void HAMR::HitITHamr(int p_Index)
{
    if (p_Index == PlayerIndex) return;   // 自分自身には当たり判定しない

    PLAYER* player = GetPlayer(p_Index);
    if (!player || !player->use) return;  // 無効なプレイヤーは無視

    // ---- スコア加算 ----
    // 1スイングにつき1回だけ加点する
    if (!scoredThisSwing)
    {
        AddScore(PlayerIndex, 100);  // ハンマーの所有者に100点を加算
        scoredThisSwing = true;      // フラグを立てて二重加算を防止
    }

    // ---- つぶれ演出 ----
    if (!player->squished)
        player->originalScl = player->scl; // 元のスケールを保存

    player->scl.y = 0.3f;   // 高さを縮める
    player->scl.x = 1.4f;   // 横方向に広げる
    player->scl.z = 1.4f;   // 奥行きも広げる
    player->squished = true;          // つぶれ状態フラグ
    player->squishTimer = 5.0f;       // 5秒間継続
}


//=============================================================================
// 取得（地面から拾う）
//=============================================================================
void HAMR::PickITHamr(int p_Index)
{
    PLAYER* pl = GetPlayer(p_Index);
    if (!pl || !pl->use) return;
    if (pl->haveWeapon)  return;         // 多重所持防止

    pick = TRUE;
    pl->haveWeapon = TRUE;               // 所持開始 :contentReference[oaicite:3]{index=3}
    PlayerIndex = p_Index;

    // プレイヤー周りの装備位置にスナップ
    pos = pl->pos;
    rot = pl->rot;

    // 耐久リセット
    swingCount = 0;
    swingAngle = 0.0f;
    wantToSwing = false;
}

//=============================================================================
// ハンマー頭のワールド座標（当たり判定用：装備中はこちらを使う）
//=============================================================================
XMFLOAT3 HAMR::GetHeadWorldPosition() const
{
    const float halfH = scl.y * 0.5f;
    const float yOffset = 40.0f / (scl.y != 0.0f ? scl.y : 1.0f);

    XMVECTOR localHead = XMVectorSet(0.0f, scl.y + yOffset, 0.0f, 1.0f);

    XMMATRIX mtx = XMMatrixIdentity();
    mtx = XMMatrixMultiply(mtx, XMMatrixScaling(scl.x, scl.y, scl.z));
    mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(0.0f, halfH, 0.0f));
    mtx = XMMatrixMultiply(mtx, XMMatrixRotationX(-swingAngle));
    mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(0.0f, -halfH, 0.0f));
    mtx = XMMatrixMultiply(mtx, XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z));
    mtx = XMMatrixMultiply(mtx, XMMatrixTranslation(pos.x, pos.y, pos.z));

    XMVECTOR worldPos = XMVector3Transform(localHead, mtx);
    XMFLOAT3 out; XMStoreFloat3(&out, worldPos);
    return out;
}

//=============================================================================
// 頂点情報（アイコン）
//=============================================================================
static HRESULT MakeVertexITHamrIcon(void)
{
    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    // 頂点に値を入れる
    D3D11_MAPPED_SUBRESOURCE msr;
    GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    VERTEX_3D* v = (VERTEX_3D*)msr.pData;

    const float fWidth = 60.0f;
    const float fHeight = 90.0f;

    // 頂点座標
    v[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
    v[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
    v[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
    v[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

    // 法線
    v[0].Normal = v[1].Normal = v[2].Normal = v[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    // 拡散色
    v[0].Diffuse = v[1].Diffuse = v[2].Diffuse = v[3].Diffuse = XMFLOAT4(1, 1, 1, 1);

    // UV
    v[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
    v[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
    v[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
    v[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

    GetDeviceContext()->Unmap(g_VertexBuffer, 0);

    return S_OK;
}

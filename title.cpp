//=============================================================================
//
// タイトル画面処理 [title.cpp]
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "fade.h"
#include "sound.h"
#include "sprite.h"
#include "title.h"
#include "meshfield.h"
#include "score.h"

//*****************************************************************************
// テクスチャ番号定義
//*****************************************************************************
#define TEXTURE_BG         0   // タイトル背景
#define TEXTURE_BTN_START  1   // スタートボタン（非選択）
#define TEXTURE_BTN_START_ON 2 // スタートボタン（選択中）
#define TEXTURE_BTN_SET    3   // 設定ボタン（非選択）
#define TEXTURE_BTN_SET_ON 4   // 設定ボタン（選択中）
#define TEXTURE_BTN_END    5   // 終了ボタン（非選択）
#define TEXTURE_BTN_END_ON 6   // 終了ボタン（選択中）
#define TEXTURE_BLACK      7   // 設定画面用 黒ベール
#define TEXTURE_BAR_BG     8   // 音量バー背景（白画像）
#define TEXTURE_VOL_TITLE  9   // "SOUND"ラベル画像（※未使用可）
#define TEXTURE_PAW        10  // 音量バー用 肉球ノッチ画像
#define TEXTURE_TITLE      11  // タイトルロゴ画像

#define TEXTURE_VOL_SETTING      12  // 
#define TEXTURE_VOL_SETTING_BGM      13  // 
#define TEXTURE_VOL_SETTING_SE      14  // 

#define TEXTURE_MAX        15  // テクスチャ枚数

#define TITLE_MENU_NUM     3   // メニュー項目数

//*****************************************************************************
// ボタン描画用テクスチャインデックスとY位置倍率
//*****************************************************************************
static const int g_MenuBtnTex[TITLE_MENU_NUM][2] = {
    {TEXTURE_BTN_START, TEXTURE_BTN_START_ON},
    {TEXTURE_BTN_SET,   TEXTURE_BTN_SET_ON},
    {TEXTURE_BTN_END,   TEXTURE_BTN_END_ON}
};
// メニュー表示のY座標倍率（画面高さ依存で配置）
static const float g_MenuYRate[TITLE_MENU_NUM] = { 12.5f, 15.0f, 17.5f };

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;            // 2Dスプライト用バッファ
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL }; // テクスチャ配列
static char* g_TexturName[TEXTURE_MAX] = {
    "data/TEXTURE/Title/ui_title_bg1280x720.png",
    "data/TEXTURE/Title/ui_title_bt_start_off.png",
    "data/TEXTURE/Title/ui_title_bt_start_on.png",
    "data/TEXTURE/Title/ui_title_bt_set_off.png",
    "data/TEXTURE/Title/ui_title_bt_set_on.png",
    "data/TEXTURE/Title/ui_title_bt_end_off.png",
    "data/TEXTURE/Title/ui_title_bt_end_on.png",
    "data/TEXTURE/Title/Black.png",
    "data/TEXTURE/Title/white.png",
    "data/TEXTURE/Title/Sound_Volume.png",
    "data/TEXTURE/Title/ani_paw.png",
    "data/TEXTURE/Title/ui_title_logo.png",

    "data/TEXTURE/Title/ui_setting_window.png",
    "data/TEXTURE/Title/ui_setting_slider_bgm.png",
    "data/TEXTURE/Title/ui_setting_slider_se.png",

};

static BOOL g_Load = FALSE;                 // 読み込みフラグ
static XMFLOAT3 g_Pos;                      // 画面中心座標
static float g_w, g_h;                      // 画面の幅・高さ
static int selectedMenuItem = 0;            // 現在選択中のメニュー
static BOOL flag_settings = FALSE;          // 設定ウィンドウ表示フラグ
static int setting_select = 0;              // 設定中の選択バー 0:BGM, 1:SE
static float setting_delay = 0.0f;          // 入力連打防止ディレイ

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTitle(void)
{
    // --- テクスチャ読み込み ---
    for (int i = 0; i < TEXTURE_MAX; i++)
    {
        g_Texture[i] = NULL;
        D3DX11CreateShaderResourceViewFromFile(GetDevice(),
            g_TexturName[i], NULL, NULL, &g_Texture[i], NULL);
    }

    // --- 頂点バッファ生成 ---
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VERTEX_3D) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

    // --- 画面サイズ・変数初期化 ---
    g_w = SCREEN_WIDTH;
    g_h = SCREEN_HEIGHT;
    g_Pos = XMFLOAT3(g_w / 2, g_h / 2, 0.0f);
    selectedMenuItem = 0;
    flag_settings = FALSE;

    ResetScore(); // スコアリセット
    PlaySound(SOUND_LABEL_BGM_mofu_1); // タイトルBGM再生

    g_Load = TRUE;
    return S_OK;
}

//=============================================================================
// 終了処理（リソース解放）
//=============================================================================
void UninitTitle(void)
{
    if (!g_Load) return;
    if (g_VertexBuffer) { g_VertexBuffer->Release(); g_VertexBuffer = NULL; }
    for (int i = 0; i < TEXTURE_MAX; i++)
        if (g_Texture[i]) { g_Texture[i]->Release(); g_Texture[i] = NULL; }
    g_Load = FALSE;
}

//=============================================================================
// 更新処理（入力・メニュー遷移・音量調整）
//=============================================================================
void UpdateTitle(void)
{
    // 設定ウィンドウ処理
    if (flag_settings) {
        // ディレイ（連打防止）
        if (setting_delay > 0.0f) setting_delay -= 1.0f / 60.0f;

        // BGM/SE 選択切替（上下キーorスティック）
        if (setting_delay <= 0.0f) {
            if (GetKeyboardTrigger(DIK_UP) || IsButtonTriggered(0, BUTTON_UP) ||
                GetKeyboardTrigger(DIK_DOWN) || IsButtonTriggered(0, BUTTON_DOWN)) {
                setting_select = (setting_select + 1) % 2;
                setting_delay = 0.15f;
            }
        }

        // 音量調整（左右キー/スティック）
        if (setting_delay <= 0.0f) {
            if (GetKeyboardTrigger(DIK_LEFT) || IsButtonTriggered(0, BUTTON_LEFT)) {
                if (setting_select == 0) DecreaseBGMVolume();
                else DecreaseSEVolume();
                setting_delay = 0.08f;
            }
            if (GetKeyboardTrigger(DIK_RIGHT) || IsButtonTriggered(0, BUTTON_RIGHT)) {
                if (setting_select == 0) IncreaseBGMVolume();
                else IncreaseSEVolume();
                setting_delay = 0.08f;
            }
        }

        // 設定画面を閉じる（Enter or Bボタン）
        if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_B)) {
            flag_settings = FALSE;
        }
        return;
    }

    // タイトルメニュー入力
    // ↓メニュー項目ダウン（ループ）
    if (GetKeyboardTrigger(DIK_DOWN) || IsButtonTriggered(0, BUTTON_DOWN)) {
        selectedMenuItem = (selectedMenuItem + 1) % TITLE_MENU_NUM;
        PlaySound(SOUND_LABEL_SE_switch01);
    }
    // ↑メニュー項目アップ（ループ）
    if (GetKeyboardTrigger(DIK_UP) || IsButtonTriggered(0, BUTTON_UP)) {
        selectedMenuItem = (selectedMenuItem + TITLE_MENU_NUM - 1) % TITLE_MENU_NUM;
        PlaySound(SOUND_LABEL_SE_switch01);
    }

    // 決定（Enter or Aボタン）
    if (GetKeyboardTrigger(DIK_RETURN) || IsButtonTriggered(0, BUTTON_A)) {
        if (selectedMenuItem == 0) {
            SetFade(FADE_OUT, MODE_PLAYER_SELECT); // ゲーム開始
        }
        else if (selectedMenuItem == 1) {
            flag_settings = TRUE;
            setting_select = 0;                   // 設定ウィンドウを開く
        }
        else if (selectedMenuItem == 2) {
            std::exit(-1);                        // 終了
        }
    }
}

//=============================================================================
// 描画処理（タイトル・メニュー・設定ウィンドウ）
//=============================================================================
void DrawTitle(void)
{
    // 2D描画バッファ設定
    UINT stride = sizeof(VERTEX_3D), offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    SetWorldViewProjection2D();
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // タイトル背景描画
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_BG]);
    SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w, g_h, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // タイトルロゴ
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_TITLE]);
    SetSprite(g_VertexBuffer, g_Pos.x * 1.48, g_Pos.y / 2.4, g_w / 2.2, g_h / 3.2, 0, 0, 1, 1);
    GetDeviceContext()->Draw(4, 0);

    // メニュー項目（ボタン）描画
    for (int i = 0; i < TITLE_MENU_NUM; i++) {
        // 選択中はON画像、それ以外はOFF画像を使う
        int texIdx = g_MenuBtnTex[i][selectedMenuItem == i ? 1 : 0];
        float y = g_Pos.y / 10 * g_MenuYRate[i];
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[texIdx]);
        SetSprite(g_VertexBuffer, g_Pos.x / 3, y, 250.0f, 100.0f, 0, 0, 1, 1);
        GetDeviceContext()->Draw(4, 0);
    }

    // 設定ウィンドウ描画
    if (flag_settings) {
        //--- 黒ベール（Black.png＋αで半透明化） ---
        if (g_Texture[TEXTURE_BLACK]) {
            SetSpriteColor(g_VertexBuffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, XMFLOAT4(1, 1, 1, 0.6f)); // α=0.6で透かし
            GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_BLACK]);
            GetDeviceContext()->Draw(4, 0);
        }

        // 音量バーの枠
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_VOL_SETTING]);
        SetSprite(g_VertexBuffer, g_Pos.x, g_Pos.y, g_w / 2, g_h / 2, 0, 0, 1, 1);
        GetDeviceContext()->Draw(4, 0);

        //--- 音量バー（BGM, SE） ---
        // ラベル画像化する場合はここでg_Texture[X]を描画
        DrawVolumeBar(368, 248, 286, 22, GetBGMVolume(), setting_select == 0, "BGM");
        DrawVolumeBar(368, 287, 286, 22, GetSEVolume(), setting_select == 1, "SE");
        // ※操作ガイド表示などもここに追加可
    }
}


//=============================================================================
// 音量バー描画
// x, y  : バー中心座標
// w, h  : バー幅・高さ
// volume: 音量 (0.0～1.0)
// selected: 選択中ならtrue（色やノッチで強調）
// label : "BGM"などのラベル（未使用可）
//=============================================================================
void DrawVolumeBar(float x, float y, float w, float h, float volume, bool selected, const char* label)
{
    //--- バー背景（白画像 or 灰色） ---
    if (g_Texture[TEXTURE_VOL_SETTING_SE]) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_VOL_SETTING_SE]);
        //SetSprite(g_VertexBuffer, x, y, w, h, 0, 0, 1, 1);
        SetSpriteLeftTop(g_VertexBuffer, x, y, w, h, 0, 0, 1, 1);
        GetDeviceContext()->Draw(4, 0);
    }
    else {
        SetSpriteColor(g_VertexBuffer, x, y, w, h, 0, 0, 1, 1, XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f));
        GetDeviceContext()->Draw(4, 0);
    }

    //--- バー本体（緑or黄色） ---
    XMFLOAT4 barColor = selected ? XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f) : XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f);
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_VOL_SETTING_BGM]);
    SetSpriteLeftTop(g_VertexBuffer, x, y, w * volume, h, 0, 0, volume, 1);
    GetDeviceContext()->Draw(4, 0);

    //--- ノッチ（肉球画像 or 矩形） ---
    if (g_Texture[TEXTURE_PAW]) {
        float pawX = x + w * volume - 24;
        GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[TEXTURE_PAW]);
        SetSpriteLeftTop(g_VertexBuffer, pawX, y - 12, 48, 48, 0, 0, 1, 1);
        GetDeviceContext()->Draw(4, 0);
    }
    else if (selected) {
        // 画像がなければ黄色ノッチで代用
        float pawX = x - w / 2 + w * volume;
        SetSpriteColor(g_VertexBuffer, pawX, y, 24, 24, 0, 0, 1, 1, XMFLOAT4(1, 1, 0.2f, 1));
        GetDeviceContext()->Draw(4, 0);
    }

}

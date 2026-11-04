//==================================================================================
//
// ポーズ画面の処理 [pause.cpp]
// Author : TENMA SAITO
//
// 頂点バッファの構成 : [半透明の黒背景] [再開] [リプレイ] [終了]
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "pause.h"
#include "fade.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define PAUSE_ICON_WIDTH		(200.0f)		// ポーズアイコンの幅
#define PAUSE_ICON_HEIGHT		(200.0f)		// ポーズアイコンの高さ
#define PAUSE_ICON_POS_Y		(300.0f)		// ポーズアイコンの上端
#define PAUSE_REPEAT_FRAME		(20)			// 長押しで選択を移動する間隔(フレーム)
#define PAUSE_REPLAY_FADE		(60)			// リプレイ時のフェード時間(フレーム)
#define PAUSE_BG_COLOR			D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.7f)	// 背景の色(半透明の黒)
#define PAUSE_SELECT_COLOR		D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)	// 選択中のアイコンの色
#define PAUSE_UNSELECT_COLOR	D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f)	// 選択していないアイコンの色
#define PAUSE_TEX_MARGIN		(0.01f)			// テクスチャ左端のにじみ防止の余白

//**********************************************************************************
// *** ポーズアイコンの種類 ***
//**********************************************************************************
typedef enum
{
	PAUSE_TYPE_BLACKBG = 0,			// ポーズ画面の黒い半透明の背景
	PAUSE_TYPE_CONTINUE,			// 再開
	PAUSE_TYPE_REPLAY,				// リプレイ
	PAUSE_TYPE_EXIT,				// 終了
	PAUSE_TYPE_MAX
} PAUSE_TYPE;

//**********************************************************************************
// *** ポーズアイコン構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置(左上)
	float fWidth;			// 横幅
	float fHeight;			// 縦幅
	bool bUse;				// 使われているか
} PAUSE_ICON;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void DoPauseSwitch(PAUSE_TYPE type);
static void SetIconVertexPause(VERTEX_2D *pVtx, const PAUSE_ICON *pIcon, D3DXCOLOR col);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_apTexturePause[PAUSE_TYPE_MAX] = {};	// テクスチャへのポインタ
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffPause = NULL;				// 頂点バッファのポインタ
static PAUSE_ICON g_aPauseIcon[PAUSE_TYPE_MAX];						// ポーズアイコンの情報
static PAUSE_TYPE g_currentPauseType;								// 選択中のポーズアイコン
static bool g_bShowPause;											// ポーズ画面を表示するか(デバッグ用に切り替え可)

// --- 各ポーズアイコンの位置 (左上) ---
static const D3DXVECTOR3 g_aIconPos[PAUSE_TYPE_MAX] =
{
	D3DXVECTOR3(0.0f, 0.0f, 0.0f),											// 背景は画面全体
	D3DXVECTOR3(250.0f - (PAUSE_ICON_WIDTH * 0.5f), PAUSE_ICON_POS_Y, 0.0f),
	D3DXVECTOR3(640.0f - (PAUSE_ICON_WIDTH * 0.5f), PAUSE_ICON_POS_Y, 0.0f),
	D3DXVECTOR3(1030.0f - (PAUSE_ICON_WIDTH * 0.5f), PAUSE_ICON_POS_Y, 0.0f),
};

// --- 各ポーズアイコンのテクスチャ (背景はテクスチャ無し) ---
static const char *g_apPauseIconTexture[PAUSE_TYPE_MAX] =
{
	"",
	"data/TEXTURE/PAUSE/CONTINUE.png",
	"data/TEXTURE/PAUSE/REPLAY.png",
	"data/TEXTURE/PAUSE/EXIT.png",
};

//==================================================================================
// --- ポーズ画面の初期化 ---
//==================================================================================
void InitPause(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** ポーズアイコンの初期化・テクスチャの読み込み ***/
	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		g_aPauseIcon[nCntPause].pos = g_aIconPos[nCntPause];
		g_aPauseIcon[nCntPause].fWidth = PAUSE_ICON_WIDTH;
		g_aPauseIcon[nCntPause].fHeight = PAUSE_ICON_HEIGHT;
		g_aPauseIcon[nCntPause].bUse = false;

		if (g_apPauseIconTexture[nCntPause][0] != '\0')
		{ // テクスチャが指定されていれば読み込む
			D3DXCreateTextureFromFile(pDevice, g_apPauseIconTexture[nCntPause], &g_apTexturePause[nCntPause]);
		}
	}

	/*** 最初は「再開」を選択 ***/
	g_currentPauseType = PAUSE_TYPE_CONTINUE;
	g_aPauseIcon[g_currentPauseType].bUse = true;
	g_bShowPause = true;

	/*** 頂点バッファの生成 (背景 + アイコン) ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * PAUSE_TYPE_MAX,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffPause,
		NULL);

	g_pVtxBuffPause->Lock(0, 0, (void**)&pVtx, 0);

	/*** 半透明の黒い背景 (画面全体) ***/
	SetFullScreenPolygon(pVtx);
	SetDefaultPolygon2D(pVtx, PAUSE_BG_COLOR);
	pVtx += 4;

	/*** ポーズアイコン ***/
	for (int nCntPause = PAUSE_TYPE_CONTINUE; nCntPause < PAUSE_TYPE_MAX; nCntPause++, pVtx += 4)
	{
		SetIconVertexPause(pVtx, &g_aPauseIcon[nCntPause], PAUSE_UNSELECT_COLOR);
	}

	g_pVtxBuffPause->Unlock();
}

//==================================================================================
// --- ポーズ画面の終了 ---
//==================================================================================
void UninitPause(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		RELEASE(g_apTexturePause[nCntPause]);
	}

	RELEASE(g_pVtxBuffPause);
}

//==================================================================================
// --- ポーズ画面の更新 ---
//==================================================================================
void UpdatePause(void)
{
	if (GetKeyboardTrigger(DIK_F))
	{ // デバッグ用 : ポーズ画面の表示を切り替え (スクリーンショット撮影など)
		g_bShowPause = !g_bShowPause;
	}

	if (GetEnablePause() == false || g_bShowPause == false)
	{ // ポーズしていない間は、次に開いた時のために「再開」を選択しておく
		g_currentPauseType = PAUSE_TYPE_CONTINUE;
		return;
	}

	if (GetFade() == FADE_NONE)
	{ // フェード中は操作不能
		int nPauseType = g_currentPauseType;		// 選択中のアイコン(計算用)

		if (GetKeyboardRepeat(DIK_D)
			|| GetJoypadRepeat(JOYKEY_RIGHT, PAUSE_REPEAT_FRAME)
			|| GetJoyThumbRepeat(JOYTHUMB_LX_UP, PAUSE_REPEAT_FRAME))
		{ // 右入力 : 次のアイコンへ (最後なら最初へ)
			PlaySound(SOUND_LABEL_SE_SELECT);

			nPauseType++;
			if (FAILED(CheckIndex(PAUSE_TYPE_MAX, nPauseType, PAUSE_TYPE_CONTINUE)))
			{
				nPauseType = PAUSE_TYPE_CONTINUE;
			}
		}
		else if (GetKeyboardRepeat(DIK_A)
			|| GetJoypadRepeat(JOYKEY_LEFT, PAUSE_REPEAT_FRAME)
			|| GetJoyThumbRepeat(JOYTHUMB_LX_DOWN, PAUSE_REPEAT_FRAME))
		{ // 左入力 : 前のアイコンへ (最初なら最後へ)
			PlaySound(SOUND_LABEL_SE_SELECT);

			nPauseType--;
			if (FAILED(CheckIndex(PAUSE_TYPE_MAX, nPauseType, PAUSE_TYPE_CONTINUE)))
			{
				nPauseType = PAUSE_TYPE_EXIT;
			}
		}

		g_currentPauseType = (PAUSE_TYPE)nPauseType;

		if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_A))
		{ // 決定 : 選択中のアイコンの処理を実行
			PlaySound(SOUND_LABEL_SE_ENTER);

			DoPauseSwitch(g_currentPauseType);
		}
	}

	/*** 選択中のアイコンだけ不透明にする ***/
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
	g_pVtxBuffPause->Lock(0, 0, (void**)&pVtx, 0);

	pVtx += 4;		// 背景は更新しない

	for (int nCntPause = PAUSE_TYPE_CONTINUE; nCntPause < PAUSE_TYPE_MAX; nCntPause++, pVtx += 4)
	{
		D3DXCOLOR col = (nCntPause == g_currentPauseType) ? PAUSE_SELECT_COLOR : PAUSE_UNSELECT_COLOR;	// アイコンの色

		SetIconVertexPause(pVtx, &g_aPauseIcon[nCntPause], col);
	}

	g_pVtxBuffPause->Unlock();
}

//==================================================================================
// --- ポーズ画面の描画 ---
//==================================================================================
void DrawPause(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	if (GetEnablePause() == false || g_bShowPause == false)
	{ // ポーズしていない
		return;
	}

	/*** 頂点バッファ・頂点フォーマットの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffPause, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);

	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		/*** テクスチャの設定・ポリゴンの描画 (背景はテクスチャ無し) ***/
		pDevice->SetTexture(0, g_apTexturePause[nCntPause]);
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntPause, 2);
	}
}

//==================================================================================
// --- 選択したポーズアイコンの処理 ---
//==================================================================================
static void DoPauseSwitch(PAUSE_TYPE type)
{
	switch (type)
	{
	case PAUSE_TYPE_CONTINUE:		// 再開 : ポーズを解除
		SetEnablePause(false);
		break;

	case PAUSE_TYPE_REPLAY:			// リプレイ : ゲーム画面を最初から
		SetStageExac(0);
		SetFade(MODE_GAME, FADE_TYPE_NORMAL, PAUSE_REPLAY_FADE);
		SetEnablePause(false);
		break;

	case PAUSE_TYPE_EXIT:			// 終了 : タイトル画面へ戻る
		SetStageExac(0);
		SetFade(MODE_TITLE, FADE_TYPE_NORMAL);
		FadeSound(SOUND_LABEL_BGM_TITLE);
		break;

	default:
		break;
	}
}

//==================================================================================
// --- ポーズアイコン1つ分の頂点情報を設定 ---
//==================================================================================
static void SetIconVertexPause(VERTEX_2D *pVtx, const PAUSE_ICON *pIcon, D3DXCOLOR col)
{
	pVtx[0].pos = D3DXVECTOR3(pIcon->pos.x, pIcon->pos.y, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(pIcon->pos.x + pIcon->fWidth, pIcon->pos.y, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(pIcon->pos.x, pIcon->pos.y + pIcon->fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(pIcon->pos.x + pIcon->fWidth, pIcon->pos.y + pIcon->fHeight, 0.0f);

	SetDefaultPolygon2D(pVtx, col);

	pVtx[0].tex.x = PAUSE_TEX_MARGIN;
	pVtx[2].tex.x = PAUSE_TEX_MARGIN;
}

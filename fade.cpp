//==================================================================================
//
// 画面遷移のフェード処理 [fade.cpp]
// Author : TENMA SAITO
//
// SetFade でフェードアウト → 真っ暗(不透明)になった時点で SetMode → フェードイン。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "fade.h"
#include "input.h"
#include "sound.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TEXTURE_FADE_COLOR	D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f)		// テクスチャフェードの初期色(白・透明)

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_pTextureFade = NULL;		// テクスチャフェード用テクスチャ(現在は未読み込み)
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffFade = NULL;	// 頂点バッファのポインタ
static FADE g_fade;										// フェードの状態
static MODE g_modeNext;									// フェードアウト後に移行する画面(モード)
static D3DXCOLOR g_colorFade;							// フェードポリゴンの色
static float g_fAlphaFade;								// 1フレームあたりのα値の増減量
static FADE_TYPE g_typeFade;							// フェードの種類

//==================================================================================
// --- フェードの初期化 (起動直後はフェードインから始まる) ---
//==================================================================================
void InitFade(MODE modeNext)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	g_fade = FADE_IN;										// フェードイン状態に
	g_modeNext = modeNext;									// 次の画面を設定
	g_colorFade = NORMAL_FADE;								// 黒で
	g_colorFade.a = 1.0f;									// 不透明から開始
	g_fAlphaFade = (1.0f / (float)NORMAL_FADECOUNT);		// フェード速度を初期化
	g_typeFade = FADE_TYPE_NORMAL;							// 通常フェードに設定

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffFade,
		NULL);

	/*** 画面全体を覆うポリゴンを設定 ***/
	g_pVtxBuffFade->Lock(0, 0, (void**)&pVtx, 0);

	SetFullScreenPolygon(pVtx);
	SetDefaultPolygon2D(pVtx, g_colorFade);

	g_pVtxBuffFade->Unlock();
}

//==================================================================================
// --- フェードの終了 ---
//==================================================================================
void UninitFade(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	RELEASE(g_pTextureFade);
	RELEASE(g_pVtxBuffFade);
}

//==================================================================================
// --- フェードの更新 ---
//==================================================================================
void UpdateFade(void)
{
	if (g_fade == FADE_NONE)
	{ // フェードしていない
		return;
	}

	if (g_fade == FADE_IN)
	{ // フェードイン : α値を下げていく
		g_colorFade.a -= g_fAlphaFade;
		if (g_colorFade.a <= 0.0f)
		{ // フェードイン終了
			g_colorFade.a = 0.0f;
			g_fade = FADE_NONE;
		}

		if ((GetKeyboardTrigger(DIK_RETURN) == true
			|| GetJoypadTrigger(JOYKEY_A) == true
			|| GetJoypadTrigger(JOYKEY_START) == true)
			&& g_modeNext != MODE_GAME)
		{ // ゲーム画面以外では、決定入力でフェードインをスキップ
			g_colorFade.a = 0.0f;
			SetVolume(SOUND_STANDARD, SETSOUND_MAX);
		}
	}
	else if (g_fade == FADE_OUT)
	{ // フェードアウト : α値を上げていく
		g_colorFade.a += g_fAlphaFade;
		if (g_colorFade.a >= 1.0f)
		{ // フェードアウト終了 → 画面を切り替えてフェードインへ
			g_colorFade.a = 1.0f;
			g_fade = FADE_IN;

			SetMode(g_modeNext);
		}
	}

	/*** 頂点カラーの更新 ***/
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
	g_pVtxBuffFade->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].col = g_colorFade;
	}

	g_pVtxBuffFade->Unlock();
}

//==================================================================================
// --- フェードの描画 ---
//==================================================================================
void DrawFade(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** 頂点バッファ・頂点フォーマットの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffFade, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);

	/*** テクスチャの設定 (通常フェードはテクスチャ無し) ***/
	pDevice->SetTexture(0, (g_typeFade == FADE_TYPE_TEXTURE) ? g_pTextureFade : NULL);

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

//==================================================================================
// --- フェードの開始 ---
//==================================================================================
void SetFade(MODE modeNext, FADE_TYPE type, int nCounterFade)
{
	g_fade = FADE_OUT;							// フェードアウトから開始
	g_modeNext = modeNext;						// フェードアウト後のモードを保存
	g_fAlphaFade = (1.0f / (float)nCounterFade);	// フェード速度

	/*** 種類別の初期色 ***/
	switch (type)
	{
	case FADE_TYPE_NORMAL:		// 黒(透明)から暗転
		g_colorFade = NORMAL_FADE;
		g_typeFade = FADE_TYPE_NORMAL;
		break;

	case FADE_TYPE_TEXTURE:		// テクスチャ付きの白(透明)から
		g_colorFade = TEXTURE_FADE_COLOR;
		g_typeFade = FADE_TYPE_TEXTURE;
		break;
	}
}

//==================================================================================
// --- フェードの状態の取得 ---
//==================================================================================
FADE GetFade(void)
{
	return g_fade;
}

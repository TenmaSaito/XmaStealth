//==================================================================================
//
// タイマー(数字表示)の処理 [timer.cpp]
// Author : TENMA SAITO
//
// ※ 現在のゲーム画面の制限時間は score.cpp で表示しており、
//    このモジュールは Init / Update / Uninit のみ呼ばれている(描画はしていない)。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "timer.h"
#include "sound.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TIMER_TEXTURE		"data/TEXTURE/number003.png"	// 数字テクスチャのパス
#define TIMER_POS			D3DXVECTOR3(610.0f, 80.0f, 0.0f)	// 表示位置(1桁目の中心)
#define NUM_PLACE			(3)				// タイマーの桁数
#define NUM_WIDTH			(25.0f)			// 数字1つの横幅の半分
#define NUM_HEIGHT			(30.0f)			// 数字1つの縦幅の半分
#define NUM_SPACE			(40.0f)			// 数字の間隔
#define NUM_TEX_WIDTH		(0.1f)			// 数字1つ分のテクスチャ幅 (0～9 の10個並び)

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void UpdateDigitTimer(void);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_pTextureTimer = NULL;		// テクスチャへのポインタ
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffTimer = NULL;	// 頂点バッファのポインタ
static D3DXVECTOR3 g_posTimer;							// タイマーを表示する位置
static D3DXCOLOR g_colTimer;							// タイマーの色
static int g_nTimer;									// タイマーの値

//==================================================================================
// --- タイマーの初期化処理 ---
//==================================================================================
void InitTimer(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice, TIMER_TEXTURE, &g_pTextureTimer);

	g_posTimer = TIMER_POS;
	g_colTimer = D3DXCOLOR_NULL;
	g_nTimer = 0;

	/*** 頂点バッファの生成 (桁数分の四角形) ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * (4 * NUM_PLACE),
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffTimer,
		NULL);

	/*** 各桁の頂点情報の設定 ***/
	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++, pVtx += 4)
	{
		D3DXVECTOR3 posPlace = g_posTimer;			// この桁の中心位置
		posPlace.x += NUM_SPACE * nCntPlace;

		SetCenterPolygon2D(pVtx, posPlace, NUM_WIDTH * 2.0f, NUM_HEIGHT * 2.0f);
		SetDefaultPolygon2D(pVtx, g_colTimer);

		/*** 最初は「0」を表示 ***/
		pVtx[1].tex.x = NUM_TEX_WIDTH;
		pVtx[3].tex.x = NUM_TEX_WIDTH;
	}

	g_pVtxBuffTimer->Unlock();
}

//==================================================================================
// --- タイマーの終了 ---
//==================================================================================
void UninitTimer(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	RELEASE(g_pTextureTimer);
	RELEASE(g_pVtxBuffTimer);
}

//==================================================================================
// --- タイマーの更新 (色の反映) ---
//==================================================================================
void UpdateTimer(void)
{
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ

	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntVtx = 0; nCntVtx < 4 * NUM_PLACE; nCntVtx++)
	{
		pVtx[nCntVtx].col = g_colTimer;
	}

	g_pVtxBuffTimer->Unlock();
}

//==================================================================================
// --- タイマーの描画 ---
//==================================================================================
void DrawTimer(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffTimer, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);
	pDevice->SetTexture(0, g_pTextureTimer);

	/*** 各桁の描画 ***/
	for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++)
	{
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntPlace, 2);
	}
}

//==================================================================================
// --- タイマーの設定 ---
//==================================================================================
void SetTimer(int nTimer)
{
	g_nTimer = nTimer;

	UpdateDigitTimer();
}

//==================================================================================
// --- タイマーの取得 ---
//==================================================================================
int GetTimer(void)
{
	return g_nTimer;
}

//==================================================================================
// --- タイマーの加算 (0未満にはならない) ---
//==================================================================================
void AddTimer(int nValue)
{
	g_nTimer += nValue;

	if (g_nTimer <= 0)
	{
		g_nTimer = 0;
	}

	UpdateDigitTimer();
}

//==================================================================================
// --- 現在の値に合わせて各桁のテクスチャ座標を更新 ---
//==================================================================================
static void UpdateDigitTimer(void)
{
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
	int nDivisor = 1;			// 桁を取り出すための除数 (1の位 → 1, 10の位 → 10 ...)

	for (int nCntPlace = 1; nCntPlace < NUM_PLACE; nCntPlace++)
	{ // 最上位の桁の除数を求める
		nDivisor *= 10;
	}

	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++, pVtx += 4, nDivisor /= 10)
	{ // 上の桁から順に数字を設定
		int nDigit = (g_nTimer / nDivisor) % 10;		// この桁の数字
		float fTexU = NUM_TEX_WIDTH * nDigit;			// この数字のテクスチャ左端

		pVtx[0].tex = D3DXVECTOR2(fTexU, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(fTexU, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 1.0f);
	}

	g_pVtxBuffTimer->Unlock();
}

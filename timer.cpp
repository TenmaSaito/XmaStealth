//================================================================================================================
//
// DirectXのタイマー処理 [timer.cpp]
// Author : TENMA
//
//================================================================================================================
#include "game.h"
#include "timer.h"
#include "sound.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define NUM_PLACE			(3)				// タイマーの桁数
#define NUM_WIDTH			(25.0f)			// タイマーの数字の横幅
#define NUM_HEIGHT			(30.0f)			// タイマーの数字の縦幅
#define NUM_SPACE			(40.0f)			// タイマーの数字の隙間
#define TIMERGAUGE_COLOR	D3DXCOLOR(1.0f,1.0f,1.0f,1.0f)		// タイマーのゲージの色

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureTimer = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffTimer = NULL;	// 頂点バッファのポインタ
D3DXVECTOR3 g_posTimer;							// タイマーを表示する位置
D3DXCOLOR g_colTimer;							// タイマーの色
int g_nTimer;									// タイマーの値
int g_nGaugeTimer;								// タイマーのゲージのNo
int g_nAlphaTimer;								// α値変更

//=================================================================================================
// --- タイマーの初期化処理 ---
//=================================================================================================
void InitTimer(void)
{
	LPDIRECT3DDEVICE9 pDevice;			// デバイスのポインタ
	int nCntTimer;

	// デバイスの取得
	pDevice = GetDevice();

	// テクスチャの読み込み
	D3DXCreateTextureFromFile(pDevice,
		"data\\TEXTURE\\number003.png",
		&g_pTextureTimer);

	g_posTimer = D3DXVECTOR3(610.0f, 48.0f, 0.0f);		// 位置を初期化
	g_colTimer = D3DXCOLOR_NULL;
	g_nTimer = 0;								// 値を初期化
	g_nAlphaTimer = 1;

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * NUM_PLACE,			// NUM_PLACE分の頂点を作成
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffTimer,
		NULL);

	VERTEX_2D* pVtx;					// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		// 頂点座標の設定(座標設定は必ず右回りで！！！)
		pVtx[0].pos.x = g_posTimer.x + (NUM_SPACE * nCntTimer) - NUM_WIDTH;
		pVtx[0].pos.y = g_posTimer.y - NUM_HEIGHT;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_posTimer.x + (NUM_SPACE * nCntTimer) + NUM_WIDTH;
		pVtx[1].pos.y = g_posTimer.y - NUM_HEIGHT;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_posTimer.x + (NUM_SPACE * nCntTimer) - NUM_WIDTH;
		pVtx[2].pos.y = g_posTimer.y + NUM_HEIGHT;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_posTimer.x + (NUM_SPACE * nCntTimer) + NUM_WIDTH;
		pVtx[3].pos.y = g_posTimer.y + NUM_HEIGHT;
		pVtx[3].pos.z = 0.0f;

		// rhwの設定
		pVtx[0].rhw = 1.0f;
		pVtx[1].rhw = 1.0f;
		pVtx[2].rhw = 1.0f;
		pVtx[3].rhw = 1.0f;

		// 頂点カラーの設定
		pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		// テクスチャ座標の設定
		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(0.1f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(0.1f, 1.0f);

		pVtx += 4;
	}

	g_pVtxBuffTimer->Unlock();

}

//=================================================================================================
// --- タイマーの終了 ---
//=================================================================================================
void UninitTimer(void)
{
	// テクスチャの破棄(必ず行うこと！！！)
	if (g_pTextureTimer != NULL)
	{
		g_pTextureTimer->Release();
		g_pTextureTimer = NULL;
	}

	// 頂点バッファの破棄(必ず行うこと！！！)
	if (g_pVtxBuffTimer != NULL)
	{
		g_pVtxBuffTimer->Release();
		g_pVtxBuffTimer = NULL;
	}
}

//=================================================================================================
// --- タイマーの更新 ---
//=================================================================================================
void UpdateTimer(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	// 後で書きたきゃ書け
	for (int nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		// 頂点カラーの設定
		pVtx[0].col = g_colTimer;
		pVtx[1].col = g_colTimer;
		pVtx[2].col = g_colTimer;
		pVtx[3].col = g_colTimer;

		pVtx += 4;
	}

	g_pVtxBuffTimer->Unlock();
}

//=================================================================================================
// --- タイマーの描画 ---
//=================================================================================================
void DrawTimer(void)
{
	LPDIRECT3DDEVICE9 pDevice;			// デバイスのポインタ
	int nCntTimer;

	// デバイスの取得
	pDevice = GetDevice();

	//頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, g_pVtxBuffTimer, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// テクスチャの設定
	pDevice->SetTexture(0, g_pTextureTimer);

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		// ポリゴンの描画
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
			4 * nCntTimer,								// 描画する最初の頂点インデックス
			2);											// 描画するプリミティブの数
	}
}

//=================================================================================================
// --- タイマーの設定 ---
//=================================================================================================
void SetTimer(int nTimer)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int aTexU[NUM_PLACE];				//各桁の数字を収納
	int nDiff = 1;
	int nCntTimer;

	g_nTimer = nTimer;

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		aTexU[nCntTimer] = g_nTimer % (int)powf(10.0f, (float)(NUM_PLACE - nCntTimer)) / (int)powf(10.0f, (float)(NUM_PLACE - nCntTimer) - 1.0f);
	}

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		// テクスチャ座標の設定
		pVtx[0].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]), 0.0f);
		pVtx[1].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]) + 0.1f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]), 1.0f);
		pVtx[3].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]) + 0.1f, 1.0f);

		pVtx += 4;
	}

	g_pVtxBuffTimer->Unlock();
}

//=================================================================================================
// --- タイマーの取得 ---
//=================================================================================================
int GetTimer(void)
{
	return g_nTimer;
}

//=================================================================================================
// --- タイマーの追加 ---
//=================================================================================================
void AddTimer(int nValue)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int aTexU[NUM_PLACE];				// 各桁の数字を収納
	int nCntTimer;

	g_nTimer += nValue;

	if (g_nTimer <= 0)
	{
		g_nTimer = 0;
	}

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		aTexU[nCntTimer] = g_nTimer % (int)powf(10.0f,(float)(NUM_PLACE - nCntTimer)) / (int)powf(10.0f, (float)(NUM_PLACE - nCntTimer) - 1.0f);
	}

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	g_pVtxBuffTimer->Lock(0, 0, (void**)&pVtx, 0);

	for (nCntTimer = 0; nCntTimer < NUM_PLACE; nCntTimer++)
	{
		// テクスチャ座標の設定
		pVtx[0].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]), 0.0f);
		pVtx[1].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]) + 0.1f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]), 1.0f);
		pVtx[3].tex = D3DXVECTOR2((0.1f * aTexU[nCntTimer]) + 0.1f, 1.0f);

		pVtx += 4;
	}

	g_pVtxBuffTimer->Unlock();
}
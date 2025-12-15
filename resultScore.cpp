//================================================================================================================
//
// DirectXのスコア処理 [resultScore.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "resultScore.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define NUM_PLACE			(2)				// スコアの桁数
#define NUM_WIDTH			(45)			// 一桁の横幅
#define NUM_HEIGHT			(55)			// 一桁の縦幅
#define CHAR_WIDTH			(125)			// 文字列のサイズ(幅)
#define CHAR_HEIGHT			(50)			// 文字列のサイズ(高さ)
#define GOAL_SCOREPOS		D3DXVECTOR3(580.0f, 550.0f, 0.0f)		// スコアの最終位置

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureResultScore = {};		// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffResultScore = NULL;	// 頂点バッファのポインタ
D3DXVECTOR3 g_posResultScore;							// スコアを表示する位置
float g_fRotScore;										// スコアの回転角度(テスト)
int g_nResultScore;										// スコアの値

//**********************************************************************************
//*** テクスチャ ***
//**********************************************************************************
const char *g_aResultTex[]
{
	"data\\TEXTURE\\RESULT\\RESULT_CHAR.png",
	"data\\TEXTURE\\UI\\UI_STOCKNUM.png"
};

//================================================================================================================
// --- スコア表示の初期化 ---
//================================================================================================================
void InitResultScore(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int nCntResultScore;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
		"data\\TEXTURE\\UI\\UI_STOCKNUM.png",
		&g_pTextureResultScore);

	g_posResultScore = D3DXVECTOR3(1300.0f, 62.5f, 0.0f);		// 位置を初期化
	g_nResultScore = 0;											// 値を初期化
	g_fRotScore = 0.0f;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * NUM_PLACE,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffResultScore,
		NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffResultScore->Lock(0, 0, (void**)&pVtx, 0);

	for (nCntResultScore = 0; nCntResultScore < NUM_PLACE; nCntResultScore++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) - NUM_WIDTH;
		pVtx[0].pos.y = g_posResultScore.y - NUM_HEIGHT;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) + NUM_WIDTH;
		pVtx[1].pos.y = g_posResultScore.y - NUM_HEIGHT;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) - NUM_WIDTH;
		pVtx[2].pos.y = g_posResultScore.y + NUM_HEIGHT;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) + NUM_WIDTH;
		pVtx[3].pos.y = g_posResultScore.y + NUM_HEIGHT;
		pVtx[3].pos.z = 0.0f;

		/*** rhwの設定 ***/
		pVtx[0].rhw = 1.0f;
		pVtx[1].rhw = 1.0f;
		pVtx[2].rhw = 1.0f;
		pVtx[3].rhw = 1.0f;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(0.1f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(0.1f, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffResultScore->Unlock();
}

//================================================================================================================
// --- スコア表示の終了 ---
//================================================================================================================
void UninitResultScore(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureResultScore != NULL)
	{
		g_pTextureResultScore->Release();
		g_pTextureResultScore = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffResultScore != NULL)
	{
		g_pVtxBuffResultScore->Release();
		g_pVtxBuffResultScore = NULL;
	}
}

//================================================================================================================
// --- スコア表示の更新 ---
//================================================================================================================
void UpdateResultScore(void)
{
	VERTEX_2D* pVtx = NULL;

	if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_A) || GetJoypadTrigger(JOYKEY_START))
	{
		g_posResultScore.x = GOAL_SCOREPOS.x;
		g_fRotScore = 0.0;
	}

	if (g_posResultScore.x <= GOAL_SCOREPOS.x)
	{
		g_posResultScore.x = GOAL_SCOREPOS.x;
		if ((int)(g_fRotScore * 10.0f) % (int)(D3DX_PI * 20.0f) != 0)
		{
			g_fRotScore += 0.04f;
		}
	}
	else
	{
		g_fRotScore += 0.04f;
		g_posResultScore.x -= 5.0f;
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffResultScore->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntResultScore = 0; nCntResultScore < NUM_PLACE; nCntResultScore++)
	{
		/*** 頂点座標の設定の設定 ***/
		/*pVtx[0].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) - NUM_WIDTH;
		pVtx[0].pos.y = g_posResultScore.y - NUM_HEIGHT;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) + NUM_WIDTH;
		pVtx[1].pos.y = g_posResultScore.y - NUM_HEIGHT;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) - NUM_WIDTH;
		pVtx[2].pos.y = g_posResultScore.y + NUM_HEIGHT;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore) + NUM_WIDTH;
		pVtx[3].pos.y = g_posResultScore.y + NUM_HEIGHT;
		pVtx[3].pos.z = 0.0f;*/

		RollPolygon(pVtx, D3DXVECTOR3(g_posResultScore.x + ((NUM_WIDTH * 2) * nCntResultScore), g_posResultScore.y, 0.0f), NUM_WIDTH, NUM_HEIGHT, g_fRotScore, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffResultScore->Unlock();
}

//================================================================================================================
// --- スコア表示の描画 ---
//================================================================================================================
void DrawResultScore(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	int nCntResultScore;

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffResultScore, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	/*** テクスチャの設定 ***/
	pDevice->SetTexture(0, g_pTextureResultScore);

	for (nCntResultScore = 0; nCntResultScore < NUM_PLACE; nCntResultScore++)
	{
		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
			4 * nCntResultScore,						// 描画する最初の頂点インデックス
			2);											// 描画するプリミティブの数
	}
}

//================================================================================================================
// --- スコア表示の設定 ---
//================================================================================================================
void SetResultScore(D3DXVECTOR3 pos, int nResultScore)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int aTexU[NUM_PLACE];				// 各桁の数字を収納
	int nCntResultScore;

	/*** 描画位置を代入 ***/
	g_posResultScore = pos;

	/*** スコアを代入 ***/
	g_nResultScore = nResultScore;

	/*** 代入された値を各桁に分解 ***/
	for (nCntResultScore = 0; nCntResultScore < NUM_PLACE; nCntResultScore++)
	{
		aTexU[nCntResultScore] = g_nResultScore % (int)powf(10.0f, (float)(NUM_PLACE - nCntResultScore)) / (int)powf(10.0f, (float)(NUM_PLACE - nCntResultScore) - 1.0f);
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffResultScore->Lock(0, 0, (void**)&pVtx, 0);

	/*** 分解した値を各桁で描画 ***/
	for (nCntResultScore = 0; nCntResultScore < NUM_PLACE; nCntResultScore++)
	{
		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2((0.1f * aTexU[nCntResultScore]), 0.0f);
		pVtx[1].tex = D3DXVECTOR2((0.1f * aTexU[nCntResultScore]) + 0.1f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2((0.1f * aTexU[nCntResultScore]), 1.0f);
		pVtx[3].tex = D3DXVECTOR2((0.1f * aTexU[nCntResultScore]) + 0.1f, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffResultScore->Unlock();
}
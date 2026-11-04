//==================================================================================
//
// スコア(数字表示)の処理 [score.cpp]
// Author : TENMA SAITO
//
// 任意の桁数の数字を画面に表示する汎用UI。ゲーム画面の制限時間・オーナメント数・
// スコアや、リザルト画面の各スコアの表示に使う。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "score.h"
#include "sound.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_SCORE			(16)			// 設定できるスコアの最大数
#define NUM_WIDTH			(20.0f)			// 数字1つの横幅の半分
#define NUM_HEIGHT			(25.0f)			// 数字1つの縦幅の半分
#define NUM_SPACE			(40.0f)			// 数字の間隔
#define NUM_TEX_WIDTH		(0.1f)			// 数字1つ分のテクスチャ幅 (0～9 の10個並び)

//**********************************************************************************
// *** スコア構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置(最上位の桁の中心)
	int nScore;							// スコアの値
	int nScoreNum;						// 表示する桁数
	bool bUse;							// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ(桁数分の四角形)
} Score, *PSCORE;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static bool IsValidIndexScore(int nIdxScore);
static void UpdateVertexPosScore(PSCORE pScore);
static void UpdateVertexTexScore(PSCORE pScore);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static Score g_aScore[MAX_SCORE];		// スコアの情報
static int g_nIdxTextureScore;			// 数字テクスチャの番号(全スコア共通)

//==================================================================================
// --- スコアの初期化処理 ---
//==================================================================================
void InitScore(void)
{
	ZeroMemory(&g_aScore[0], sizeof g_aScore);

	g_nIdxTextureScore = -1;
}

//==================================================================================
// --- スコアの終了 ---
//==================================================================================
void UninitScore(void)
{
	for (int nCntScore = 0; nCntScore < MAX_SCORE; nCntScore++)
	{
		/*** 頂点バッファの破棄 ***/
		RELEASE(g_aScore[nCntScore].pVtxBuff);
	}
}

//==================================================================================
// --- スコアの更新 ---
//==================================================================================
void UpdateScore(void)
{

}

//==================================================================================
// --- スコアの描画 ---
//==================================================================================
void DrawScore(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	for (int nCntScore = 0; nCntScore < MAX_SCORE; nCntScore++)
	{
		PSCORE pScore = &g_aScore[nCntScore];		// 描画するスコア

		if (pScore->bUse == false)
		{ // 未使用
			continue;
		}

		/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, pScore->pVtxBuff, 0, sizeof(VERTEX_2D));
		pDevice->SetFVF(FVF_VERTEX_2D);
		pDevice->SetTexture(0, GetTexture(g_nIdxTextureScore));

		/*** 各桁の描画 ***/
		for (int nCntPlace = 0; nCntPlace < pScore->nScoreNum; nCntPlace++)
		{
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntPlace, 2);
		}
	}
}

//==================================================================================
// --- スコアの設置 ---
// 戻り値 : 設置したスコアの番号 (空きが無ければ -1)
//==================================================================================
int SetScore(int nScore, D3DXVECTOR3 posCenter, int nScoreNum)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	for (int nCntScore = 0; nCntScore < MAX_SCORE; nCntScore++)
	{
		PSCORE pScore = &g_aScore[nCntScore];		// 設置先のスコア

		if (pScore->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pScore->pos = posCenter;
		pScore->nScore = nScore;
		pScore->nScoreNum = nScoreNum;
		pScore->bUse = true;

		/*** 頂点バッファの生成 (桁数分の四角形) ***/
		pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * (4 * nScoreNum),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_2D,
			D3DPOOL_MANAGED,
			&pScore->pVtxBuff,
			NULL);

		/*** rhw・頂点カラーの設定 ***/
		VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
		pScore->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

		for (int nCntPlace = 0; nCntPlace < nScoreNum; nCntPlace++, pVtx += 4)
		{
			SetDefaultPolygon2D(pVtx);
		}

		pScore->pVtxBuff->Unlock();

		/*** 頂点座標・数字の設定 ***/
		UpdateVertexPosScore(pScore);
		UpdateVertexTexScore(pScore);

		return nCntScore;
	}

	return -1;
}

//==================================================================================
// --- スコアの値の取得 (番号が範囲外なら -1) ---
//==================================================================================
int GetScore(int nIdxScore)
{
	if (nIdxScore < 0 || nIdxScore >= MAX_SCORE)
	{ // 番号が範囲外
		return -1;
	}

	return g_aScore[nIdxScore].nScore;
}

//==================================================================================
// --- スコアの加算 (0未満にはならない) ---
//==================================================================================
void AddScore(int nValue, int nIdxScore)
{
	if (IsValidIndexScore(nIdxScore) == false)
	{
		return;
	}

	PSCORE pScore = &g_aScore[nIdxScore];		// 加算するスコア

	pScore->nScore += nValue;
	if (pScore->nScore <= 0)
	{
		pScore->nScore = 0;
	}

	UpdateVertexTexScore(pScore);
}

//==================================================================================
// --- スコアの値の設定 (0未満にはならない) ---
//==================================================================================
void SetScore(int nScore, int nIdxScore)
{
	if (IsValidIndexScore(nIdxScore) == false)
	{
		return;
	}

	PSCORE pScore = &g_aScore[nIdxScore];		// 設定するスコア

	pScore->nScore = nScore;
	if (pScore->nScore <= 0)
	{
		pScore->nScore = 0;
	}

	UpdateVertexPosScore(pScore);
	UpdateVertexTexScore(pScore);
}

//==================================================================================
// --- スコアの移動 ---
//==================================================================================
void MoveScore(D3DXVECTOR3 pos, int nIdxScore)
{
	if (IsValidIndexScore(nIdxScore) == false)
	{
		return;
	}

	PSCORE pScore = &g_aScore[nIdxScore];		// 移動するスコア

	pScore->pos = pos;

	UpdateVertexPosScore(pScore);
}

//==================================================================================
// --- スコアの消去 ---
// ※ 現在は無効化されている (呼んでも何もしない)
//==================================================================================
void DestroyScore(int nIdxScore)
{

}

//==================================================================================
// --- 数字テクスチャの設定 ---
//==================================================================================
void SetTextureScore(int nIdxTexture)
{
	g_nIdxTextureScore = nIdxTexture;
}

//==================================================================================
// --- スコア番号が範囲内で、頂点バッファが生成済みか ---
//==================================================================================
static bool IsValidIndexScore(int nIdxScore)
{
	return (nIdxScore >= 0 && nIdxScore < MAX_SCORE && g_aScore[nIdxScore].pVtxBuff != NULL);
}

//==================================================================================
// --- 各桁の頂点座標の更新 ---
//==================================================================================
static void UpdateVertexPosScore(PSCORE pScore)
{
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ

	pScore->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntPlace = 0; nCntPlace < pScore->nScoreNum; nCntPlace++, pVtx += 4)
	{
		D3DXVECTOR3 posPlace = pScore->pos;			// この桁の中心位置
		posPlace.x += NUM_SPACE * nCntPlace;

		SetCenterPolygon2D(pVtx, posPlace, NUM_WIDTH * 2.0f, NUM_HEIGHT * 2.0f);
	}

	pScore->pVtxBuff->Unlock();
}

//==================================================================================
// --- 現在の値に合わせて各桁のテクスチャ座標を更新 ---
//==================================================================================
static void UpdateVertexTexScore(PSCORE pScore)
{
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
	int nDivisor = 1;			// 桁を取り出すための除数 (1の位 → 1, 10の位 → 10 ...)

	for (int nCntPlace = 1; nCntPlace < pScore->nScoreNum; nCntPlace++)
	{ // 最上位の桁の除数を求める
		nDivisor *= 10;
	}

	pScore->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntPlace = 0; nCntPlace < pScore->nScoreNum; nCntPlace++, pVtx += 4, nDivisor /= 10)
	{ // 上の桁から順に数字を設定
		int nDigit = (pScore->nScore / nDivisor) % 10;		// この桁の数字
		float fTexU = NUM_TEX_WIDTH * nDigit;				// この数字のテクスチャ左端

		pVtx[0].tex = D3DXVECTOR2(fTexU, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(fTexU, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 1.0f);
	}

	pScore->pVtxBuff->Unlock();
}

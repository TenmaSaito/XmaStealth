//==================================================================================
//
// 2Dポリゴン(UI画像)の表示処理 [2Dpolygon.cpp]
// Author : TENMA SAITO
//
// InitScript で読み込んだテクスチャを、画面上の任意の位置に表示する汎用UI。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "2Dpolygon.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_POLYGON			(256)		// 同時に表示できるポリゴンの最大数

//**********************************************************************************
// *** ポリゴン構造体の定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 中心位置
	D3DXVECTOR3 rot;					// 向き(未使用)
	float fWidth;						// 横幅
	float fHeight;						// 高さ
	int nIdxTexture;					// テクスチャ番号
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	bool bUse;							// 使っているか
} _2DPOLYGON, *LP2DPOLYGON;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static bool IsValidIndex2DPolygon(int nIdx2DPolygon);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static _2DPOLYGON g_a2DPolygon[MAX_POLYGON];		// ポリゴンの情報

//==================================================================================
// --- ポリゴンの初期化処理 ---
//==================================================================================
void Init2DPolygon(void)
{
	ZeroMemory(&g_a2DPolygon[0], sizeof g_a2DPolygon);
}

//==================================================================================
// --- ポリゴンの終了処理 ---
//==================================================================================
void Uninit2DPolygon(void)
{
	for (int nCntPoly = 0; nCntPoly < MAX_POLYGON; nCntPoly++)
	{
		/*** 頂点バッファの破棄 ***/
		RELEASE(g_a2DPolygon[nCntPoly].pVtxBuff);
	}
}

//==================================================================================
// --- ポリゴンの更新処理 ---
//==================================================================================
void Update2DPolygon(void)
{

}

//==================================================================================
// --- ポリゴンの描画処理 ---
//==================================================================================
void Draw2DPolygon(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	for (int nCntPoly = 0; nCntPoly < MAX_POLYGON; nCntPoly++)
	{
		LP2DPOLYGON p2DPoly = &g_a2DPolygon[nCntPoly];		// 描画するポリゴン

		if (p2DPoly->bUse == false)
		{ // 未使用
			continue;
		}

		/*** 頂点バッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, p2DPoly->pVtxBuff, 0, sizeof(VERTEX_2D));
		pDevice->SetTexture(0, GetTexture(p2DPoly->nIdxTexture));

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}
}

//==================================================================================
// --- ポリゴンの設置処理 ---
// 戻り値 : 設置したポリゴンの番号 (空きが無ければ -1)
//==================================================================================
int Set2DPolygon(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR2 size, int nIdxTexture)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	for (int nCntPoly = 0; nCntPoly < MAX_POLYGON; nCntPoly++)
	{
		LP2DPOLYGON p2DPoly = &g_a2DPolygon[nCntPoly];		// 設置先のポリゴン

		if (p2DPoly->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		p2DPoly->pos = pos;
		p2DPoly->rot = rot;
		p2DPoly->fWidth = size.x;
		p2DPoly->fHeight = size.y;
		p2DPoly->nIdxTexture = nIdxTexture;
		p2DPoly->bUse = true;

		if (p2DPoly->pVtxBuff == NULL)
		{ // 頂点バッファが未生成なら生成 (破棄済みの枠は使い回す)
			pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_2D,
				D3DPOOL_MANAGED,
				&p2DPoly->pVtxBuff,
				NULL);
		}

		/*** 頂点情報の設定 ***/
		p2DPoly->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

		SetCenterPolygon2D(pVtx, pos, size.x, size.y);
		SetDefaultPolygon2D(pVtx);

		p2DPoly->pVtxBuff->Unlock();

		return nCntPoly;
	}

	return -1;
}

//==================================================================================
// --- ポリゴンの移動処理 ---
//==================================================================================
void SetPosition2DPolygon(int nIdx2DPolygon, D3DXVECTOR3 pos)
{
	if (IsValidIndex2DPolygon(nIdx2DPolygon) == false)
	{ // 番号が範囲外
		return;
	}

	LP2DPOLYGON p2DPoly = &g_a2DPolygon[nIdx2DPolygon];		// 移動するポリゴン
	VERTEX_2D *pVtx = NULL;									// 頂点情報へのポインタ

	p2DPoly->pos = pos;

	/*** 頂点座標の更新 ***/
	p2DPoly->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	SetCenterPolygon2D(pVtx, pos, p2DPoly->fWidth, p2DPoly->fHeight);

	p2DPoly->pVtxBuff->Unlock();
}

//==================================================================================
// --- ポリゴンの消去処理 ---
// ※ ゲーム画面では UI を消さない仕様のため、何もしない
//==================================================================================
void Destroy2DPolygon(int nIdx2DPolygon)
{
	if (GetMode() == MODE_GAME)
	{
		return;
	}

	if (IsValidIndex2DPolygon(nIdx2DPolygon) == false)
	{ // 番号が範囲外
		return;
	}

	g_a2DPolygon[nIdx2DPolygon].bUse = false;
}

//==================================================================================
// --- ポリゴン番号が範囲内か ---
//==================================================================================
static bool IsValidIndex2DPolygon(int nIdx2DPolygon)
{
	return (nIdx2DPolygon >= 0 && nIdx2DPolygon < MAX_POLYGON);
}

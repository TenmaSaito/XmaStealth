//==================================================================================
//
// クマの状態アイコン(UI)の表示処理 [bearState.cpp]
// Author : TENMA SAITO
//
// 画面下部に表示するクマのアイコン。発見状態などに応じて色を変える。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "bearState.h"

//**********************************************************************************
// *** クマの状態構造体の定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 中心位置
	D3DXCOLOR col;						// 色
	int nIdxTexture;					// テクスチャ番号
	bool bUse;							// 使っているか
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
} BearState, *LPBEARSTATE;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static BearState g_bearState;		// クマの状態の情報

//==================================================================================
// --- クマの状態の初期化処理 ---
//==================================================================================
void InitBearState(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	LPBEARSTATE pBear = &g_bearState;				// クマの状態

	/*** 各変数の初期化 ***/
	ZeroMemory(pBear, sizeof(BearState));

	/*** 頂点バッファの生成 (頂点の中身は SetBearState で設定) ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&pBear->pVtxBuff,
		NULL);
}

//==================================================================================
// --- クマの状態の終了処理 ---
//==================================================================================
void UninitBearState(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_bearState.pVtxBuff);
}

//==================================================================================
// --- クマの状態の更新処理 ---
//==================================================================================
void UpdateBearState(void)
{

}

//==================================================================================
// --- クマの状態の描画処理 ---
//==================================================================================
void DrawBearState(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	LPBEARSTATE pBear = &g_bearState;				// クマの状態

	if (pBear->bUse == false)
	{ // 未設定なら描画しない
		return;
	}

	/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
	pDevice->SetStreamSource(0, pBear->pVtxBuff, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);
	pDevice->SetTexture(0, GetTexture(pBear->nIdxTexture));

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

//==================================================================================
// --- クマの状態の設定処理 ---
//==================================================================================
void SetBearState(D3DXVECTOR3 pos, D3DXVECTOR2 size, int nIdxTexture)
{
	LPBEARSTATE pBear = &g_bearState;		// クマの状態
	VERTEX_2D *pVtx = NULL;					// 頂点情報へのポインタ

	pBear->pos = pos;
	pBear->col = DEF_COL;
	pBear->nIdxTexture = nIdxTexture;
	pBear->bUse = true;

	/*** 頂点情報の設定 ***/
	pBear->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	SetCenterPolygon2D(pVtx, pos, size.x, size.y);
	SetDefaultPolygon2D(pVtx, pBear->col);

	pBear->pVtxBuff->Unlock();
}

//==================================================================================
// --- クマの状態の色の変更処理 ---
//==================================================================================
void SetBearStateColor(D3DXCOLOR col)
{
	LPBEARSTATE pBear = &g_bearState;		// クマの状態
	VERTEX_2D *pVtx = NULL;					// 頂点情報へのポインタ

	pBear->col = col;

	/*** 頂点カラーの設定 ***/
	pBear->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].col = pBear->col;
	}

	pBear->pVtxBuff->Unlock();
}

//==================================================================================
//
// ゲームクリア背景の表示処理 [gameclearBg.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "gameclearBg.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define BG_TEXTURE_PATH		"data/TEXTURE/gameclearBg.jpg"		// 背景テクスチャのパス

//**********************************************************************************
// *** ゲームクリア背景構造体の定義 ***
//**********************************************************************************
typedef struct
{
	bool bUse;							// 使っているか
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DTEXTURE9 pTexture;		// テクスチャのポインタ
} GameClearBg;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static GameClearBg g_clearBg;		// ゲームクリア背景の情報

//==================================================================================
// --- ゲームクリア背景の初期化処理 ---
//==================================================================================
void InitGameClearBg(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	GameClearBg *pBg = &g_clearBg;					// 背景の情報
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	ZeroMemory(pBg, sizeof(GameClearBg));

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice, BG_TEXTURE_PATH, &pBg->pTexture);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&pBg->pVtxBuff,
		NULL);

	/*** 頂点バッファをロックして画面全体を覆うポリゴンを設定 ***/
	pBg->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	SetFullScreenPolygon(pVtx);

	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].rhw = 1.0f;				// 座標変換用係数
		pVtx[nCntVtx].col = D3DXCOLOR_NULL;		// 頂点カラー
	}

	/*** テクスチャ座標の設定 ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	pBg->pVtxBuff->Unlock();
}

//==================================================================================
// --- ゲームクリア背景の終了処理 ---
//==================================================================================
void UninitGameClearBg(void)
{
	GameClearBg *pBg = &g_clearBg;		// 背景の情報

	/*** 頂点バッファ・テクスチャの破棄 ***/
	RELEASE(pBg->pVtxBuff);
	RELEASE(pBg->pTexture);
}

//==================================================================================
// --- ゲームクリア背景の更新処理 ---
//==================================================================================
void UpdateGameClearBg(void)
{

}

//==================================================================================
// --- ゲームクリア背景の描画処理 ---
//==================================================================================
void DrawGameClearBg(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	GameClearBg *pBg = &g_clearBg;					// 背景の情報

	/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
	pDevice->SetStreamSource(0, pBg->pVtxBuff, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);
	pDevice->SetTexture(0, pBg->pTexture);

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

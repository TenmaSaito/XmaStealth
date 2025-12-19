//================================================================================================================
//
// DirectXの二点間の線分描画表示処理 [line.cpp]
// Author : TENMA
//
//================================================================================================================
//*************************************************************************************************
//*** インクルードファイル ***
//*************************************************************************************************
#include "line.h"
#include "debugproc.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_LINE		(32)		// 線分の最大数

//*************************************************************************************************
//*** 線分構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 posStart;			// 線分の開始位置
	D3DXVECTOR3 posEnd;				// 線分の終了位置
	D3DXCOLOR col;					// 色
	D3DXMATRIX mtxWorld;			
	bool bUse;						// 使用状態
}Line;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffLine = NULL;	// 頂点バッファのポインタ
Line g_aLine[MAX_LINE];				// 線分の情報
int g_nCountLineVertex;				// 現在設定済みの線分の数
bool g_bDisp;

//================================================================================================================
// --- 線分描画の初期化 ---
//================================================================================================================
void InitLine(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntLine = 0; nCntLine < MAX_LINE; nCntLine++)
	{
		g_aLine[nCntLine].posStart = D3DXVECTOR3_NULL;
		g_aLine[nCntLine].posEnd = D3DXVECTOR3_NULL;
		g_aLine[nCntLine].bUse = false;
	}

	g_bDisp = false;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 2 * MAX_LINE,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffLine,
		NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffLine->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntLine = 0; nCntLine < MAX_LINE; nCntLine++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos = g_aLine[nCntLine].posStart;
		pVtx[1].pos = g_aLine[nCntLine].posEnd;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = D3DXCOLOR(0.0f, 1.0f, 0.5f, 1.0f);
		pVtx[1].col = D3DXCOLOR(0.0f, 1.0f, 0.5f, 1.0f);

		pVtx += 2;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffLine->Unlock();
}

//================================================================================================================
// --- 線分描画の終了 ---
//================================================================================================================
void UninitLine(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffLine);

	if (g_pVtxBuffLine != NULL)
	{
		g_pVtxBuffLine->Release();
		g_pVtxBuffLine = NULL;
	}
}

//================================================================================================================
// --- 線分描画の更新 ---
//================================================================================================================
void UpdateLine(void)
{
	VERTEX_3D* pVtx;						// 頂点情報へのポインタ

	g_nCountLineVertex = 0;

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffLine->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntLine = 0; nCntLine < MAX_LINE; nCntLine++)
	{
		if (g_aLine[nCntLine].bUse == true)
		{
			pVtx[0].pos = g_aLine[nCntLine].posStart;
			pVtx[1].pos = g_aLine[nCntLine].posEnd;

			if (g_bDisp == true)
			{
				PrintDebugProc("[0] %~2f \n[1] %~2f\n", pVtx[0].pos.x, pVtx[0].pos.z, pVtx[1].pos.x, pVtx[1].pos.z);
			}

			/*** 頂点カラー設定 ***/
			pVtx[0].col = g_aLine[nCntLine].col;
			pVtx[1].col = g_aLine[nCntLine].col;
		}

		pVtx += 2;
	}

	if (GetKeyboardTrigger(DIK_L))
	{
		g_bDisp = g_bDisp ^ true;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffLine->Unlock();
}

//================================================================================================================
// --- 線分描画の描画 ---
//================================================================================================================
void DrawLine(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	// レンダリングステートの設定
	LIGHTING(pDevice, FALSE) // ライティング無効化

	if (g_bDisp == true)
	{
		for (int nCntLine = 0; nCntLine < MAX_LINE; nCntLine++)
		{
			if (g_aLine[nCntLine].bUse == true)
			{
				D3DXMatrixIdentity(&g_aLine[nCntLine].mtxWorld);

				/*** ワールドマトリックスの設定 ***/
				pDevice->SetTransform(D3DTS_WORLD, &g_aLine[nCntLine].mtxWorld);

				/*** 頂点バッファをデータストリームに設定 ***/
				pDevice->SetStreamSource(0, g_pVtxBuffLine, 0, sizeof(VERTEX_3D));

				/*** 頂点フォーマットの設定 ***/
				pDevice->SetFVF(FVF_VERTEX_3D);

				/*** テクスチャの設定 ***/
				pDevice->SetTexture(0, NULL);

				/*** ポリゴンの描画 ***/
				pDevice->DrawPrimitive(D3DPT_LINELIST,			// プリミティブの種類
					2 * nCntLine,								// 描画する最初の頂点インデックス
					1);											// 描画するプリミティブの数
			}

			//g_aLine[nCntLine].bUse = false;
		}
	}

	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE); // ライティング無効化
}

//================================================================================================================
// --- 線分描画の設置 ---
//================================================================================================================
void SetLine(D3DXVECTOR3 posStart, D3DXVECTOR3 posEnd, D3DXCOLOR col)
{
	for (int nCntLine = 0; nCntLine < MAX_LINE; nCntLine++)
	{
		if (g_aLine[nCntLine].bUse == false)
		{
			g_aLine[nCntLine].posStart = posStart;
			g_aLine[nCntLine].posEnd = posEnd;
			g_aLine[nCntLine].col = col;

			g_aLine[nCntLine].bUse = true;

			break;
		}
	}

	g_nCountLineVertex++;
}

//================================================================================================================
// --- 線分描画の設置 ---
//================================================================================================================
void SetLine(D3DXVECTOR3 posStart, D3DXCOLOR col)
{

}
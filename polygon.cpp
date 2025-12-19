//================================================================================================================
//
// DirectXのポリゴン表示処理 [polygon.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "gameoverBg.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_POLYGON			(256)		// 出現最大数

//*************************************************************************************************
//*** ポリゴン構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 rot;		// 向き
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	float fWidth;			// 横幅
	float fSize;			// 縦もしくは横幅
	int nIdTexture;			// テクスチャ番号
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	bool bUse;
}POLYGON, *LPPOLYGON;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
POLYGON g_aPolygon[MAX_POLYGON];				// ポリゴンの情報

//================================================================================================================
// --- ポリゴンの初期化処理 ---
//================================================================================================================
void InitPolygon(void)
{
	ZeroMemory(&g_aPolygon[0], sizeof(POLYGON) * MAX_POLYGON);
}

//================================================================================================================
// --- ポリゴンの終了処理 ---
//================================================================================================================
void UninitPolygon(void)
{
	LPPOLYGON pPoly = &g_aPolygon[0];

	for (int nCntRelease = 0; nCntRelease < MAX_POLYGON; nCntRelease++, pPoly++)
	{
		/*** 頂点バッファの破棄 ***/
		RELEASE(pPoly->pVtxBuff);
	}
}

//================================================================================================================
// --- ポリゴンの更新処理 ---
//================================================================================================================
void UpdatePolygon(void)
{
	
}

//================================================================================================================
// --- ポリゴンの描画処理 ---
//================================================================================================================
void DrawPolygon(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPPOLYGON pPoly = &g_aPolygon[0];
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxRot;			// ビューマトリックスの取得用

	for (int nCntPoly = 0; nCntPoly < MAX_POLYGON; nCntPoly++, pPoly++)
	{
		if (pPoly->bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pPoly->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pPoly->rot.y,			// Y軸回転
				pPoly->rot.x,			// X軸回転
				pPoly->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pPoly->mtxWorld, &pPoly->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pPoly->pos.x,
				pPoly->pos.y,
				pPoly->pos.z);

			D3DXMatrixMultiply(&pPoly->mtxWorld, &pPoly->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pPoly->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pPoly->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pPoly->nIdTexture));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				0,											// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}
}

//================================================================================================================
// --- ポリゴンの設置処理 ---
//================================================================================================================
int SetPolygon(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR2 size, bool bDepth, int nIdTexture)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPPOLYGON pPoly = &g_aPolygon[0];
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	for (int nCntPoly = 0; nCntPoly < MAX_POLYGON; nCntPoly++, pPoly++)
	{
		if (pPoly->bUse != true)
		{
			/*** 頂点バッファの生成 ***/
			pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4,
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pPoly->pVtxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pPoly->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			/*** 頂点カラー設定 ***/
			pVtx[0].col = D3DXCOLOR_NULL;
			pVtx[1].col = D3DXCOLOR_NULL;
			pVtx[2].col = D3DXCOLOR_NULL;
			pVtx[3].col = D3DXCOLOR_NULL;

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
			pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

			/*** 頂点バッファの設定を終了 ***/
			pPoly->pVtxBuff->Unlock();

			return nCntPoly;
		}
	}
	
	return -1;
}

//================================================================================================================
// --- ポリゴンの移動処理 ---
//================================================================================================================
void SetPositionPolygon(int nIdPolygon, D3DXVECTOR3 pos)
{
	if (nIdPolygon == -1) return;
	LPPOLYGON pPoly = &g_aPolygon[nIdPolygon];

	pPoly->pos = pos;
}
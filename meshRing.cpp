//================================================================================================================
//
// DirectXのメッシュリング用表示処理 [meshRing.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "meshRing.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MESH			(128)		// メッシュシリンダーの最大設置数
#define MESHRING_RADIUS		(300.0f)	// シリンダーの半径
#define MESHRING_HEIGHT		(100.0f)	// シリンダーの高さ
#define MESHRING_XNUM		(40)		// 横の分割数
#define MESHRING_ZNUM		(1)			// 縦の分割数
#define VTXMESH(nNumX, nNumY)	((nNumX + 1) * (nNumY + 1))	// VTXBUFF確保
#define INDEX_BUFFER(vertexNum, xBlock, yBlock)	(vertexNum + ((xBlock + 1) * (yBlock - 1)) + ((yBlock - 1) * 2))	// インデックスバッファ確保用
#define DRAWPRIM_NUM(xBlock, yBlock)	(((xBlock * yBlock) * 2) + ((yBlock - 1) * 4))	// 描画プリミティブ用

//*************************************************************************************************
//*** メッシュフィールド構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// ポリゴンの位置
	D3DXVECTOR3 rot;		// ポリゴンの角度
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	float fRadiusInner;		// リングの内半径
	float fRadiusOuter;		// リングの外半径
	float fSpdInner;		// 内半径の拡大速度
	float fSpdOuter;		// 外半径の拡大速度
	int nIdxTexture;		// テクスチャインデックス
	int nCounter;			// カウンター
	int nLife;				// ライフ
	int nMaxLife;			// 最大ライフ
	bool bUse;				// 使用状況
	LPDIRECT3DTEXTURE9		pTexture;	// テクスチャへのポインタ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshRing, * LPMESHRING;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MeshRing g_aRing[MAX_MESH];		// メッシュフィールド
int g_nIdxTextureRing;

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitMeshRing(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	WORD* pIdx;							// インデックス情報へのポインタ
	LPMESHRING pRing = &g_aRing[0];
	HRESULT hr;

	g_nIdxTextureRing = 0;

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pRing++)
	{
		pRing->pos = {};
		pRing->rot = {};
		pRing->fRadiusInner = 0.0f;
		pRing->fRadiusOuter = 0.0f;
		pRing->fSpdInner = 0.0f;
		pRing->fSpdOuter = 0.0f;
		pRing->nCounter = 0;
		pRing->nIdxTexture = -1;
		pRing->nLife = 0;
		pRing->nMaxLife = 0;
		pRing->bUse = false;

		/*** 頂点バッファの生成 ***/
		hr = pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * (VTXMESH(16, 1)),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_3D,
			D3DPOOL_MANAGED,
			&pRing->pVtxBuff,
			NULL);

		/*** 頂点バッファの設定 ***/
		pRing->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

		if (SUCCEEDED(hr))
		{
			for (int nCntHeight = 0; nCntHeight < 2; nCntHeight++)
			{
				/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
				for (int nCntX = 0; nCntX < (16 + 1); nCntX++)
				{
					float fAngle = 0.0f;

					fAngle = (D3DX_2PI / (float)16) * nCntX;

					D3DXVECTOR3 nor = {};
					D3DXVECTOR3 pos = {};
					D3DXVECTOR2 tex = {};
					int nCn = nCntX + (nCntHeight * (16 + 1));

					float fRadius;
					if (nCntHeight == 0)
					{
						fRadius = pRing->fRadiusOuter;
					}
					else
					{
						fRadius = pRing->fRadiusInner;
					}

					pos.x = sinf(fAngle) * (fRadius * 0.5f);
					pos.y = 0.0f;
					pos.z = cosf(fAngle) * (fRadius * 0.5f);

					pVtx[nCn].pos = pos;

					pVtx[nCn].col = D3DXCOLOR_NULL;

					nor.x = pos.x - pRing->pos.x;
					nor.y = 0.0f;
					nor.z = pos.z - pRing->pos.z;

					D3DXVec3Normalize(&nor, &nor);

					pVtx[nCn].nor = nor;

					pVtx[nCn].tex.x = (1.0f / (float)16) * nCntX;
					pVtx[nCn].tex.y = (1.0f * (float)nCntHeight);
				}
			}
		}

		/*** 頂点バッファの設定を終了 ***/
		pRing->pVtxBuff->Unlock();

		/*** インデックスバッファの生成 ***/
		hr = pDevice->CreateIndexBuffer(sizeof(WORD) * INDEX_BUFFER(VTXMESH(16, 1), 16, 1),		// sizeof(WORD) * 必要なインデックス数
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&pRing->pIdxBuff,
			NULL);

		/*** インデックスバッファをロックし、頂点番号データを取得 ***/
		pRing->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

		if (SUCCEEDED(hr))
		{
			/*** 頂点番号データの設定 ***/
			SetIndex(pIdx, 16, 1);
		}

		/*** インデックスバッファをアンロック ***/
		pRing->pIdxBuff->Unlock();
	}
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitMeshRing(void)
{
	LPMESHRING pRing = &g_aRing[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pRing++)
	{
		/*** テクスチャの破棄 ***/
		RELEASE(pRing->pTexture);

		/*** 頂点バッファの破棄 ***/
		RELEASE(pRing->pVtxBuff);

		/*** インデックスバッファの破棄 ***/
		RELEASE(pRing->pIdxBuff);
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateMeshRing(void)
{
	LPMESHRING pRing = &g_aRing[0];
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pRing++)
	{
		if (pRing->bUse == true)
		{
			pRing->fRadiusInner += pRing->fSpdInner;
			pRing->fRadiusOuter += pRing->fSpdOuter;

			/*** 頂点バッファの設定 ***/
			pRing->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			for (int nCntHeight = 0; nCntHeight < 2; nCntHeight++)
			{
				/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
				for (int nCntX = 0; nCntX < (16 + 1); nCntX++)
				{
					float fAngle = 0.0f;

					fAngle = (D3DX_2PI / (float)16) * nCntX;
					RepairRot(&fAngle, &fAngle);

					D3DXVECTOR3 nor = {};
					D3DXVECTOR3 pos = {};
					D3DXVECTOR2 tex = {};
					int nCn = nCntX + (nCntHeight * (16 + 1));

					float fRadius;
					if (nCntHeight == 0)
					{
						fRadius = pRing->fRadiusOuter;
					}
					else
					{
						fRadius = pRing->fRadiusInner;
					}

					pos.x = sinf(fAngle) * (fRadius * 0.5f);
					pos.y = 0.0f;
					pos.z = cosf(fAngle) * (fRadius * 0.5f);

					pVtx[nCn].pos = pos;

					D3DXCOLOR col = D3DXCOLOR_NULL;
					col.a = (float)pRing->nLife / (float)pRing->nMaxLife;
					pVtx[nCn].col = col;

					nor.x = pos.x - pRing->pos.x;
					nor.y = 0.0f;
					nor.z = pos.z - pRing->pos.z;

					D3DXVec3Normalize(&nor, &nor);

					pVtx[nCn].nor = nor;

					pVtx[nCn].tex.x = (1.0f / (float)16) * nCntX;
					pVtx[nCn].tex.y = (1.0f * (float)nCntHeight);
				}
			}
			
			/*** 頂点バッファの設定を終了 ***/
			pRing->pVtxBuff->Unlock();

			pRing->nLife--;
			if (pRing->nLife <= 0)
			{
				pRing->bUse = false;
			}
		}
	}
}

//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawMeshRing(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMESHRING pRing = &g_aRing[0];
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス

	//LIGHTING(pDevice, FALSE);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	// アルファブレンディングを加算合成に設定！
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pRing++)
	{
		if (pRing->bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pRing->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pRing->rot.y,			// Y軸回転
				pRing->rot.x,			// X軸回転
				pRing->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pRing->mtxWorld, &pRing->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pRing->pos.x,
				pRing->pos.y,
				pRing->pos.z);

			D3DXMatrixMultiply(&pRing->mtxWorld, &pRing->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pRing->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pRing->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** インデックスバッファをデータストリームに設定 ***/
			pDevice->SetIndices(pRing->pIdxBuff);

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pRing->nIdxTexture));

			/*** インデックスを利用したポリゴンの描画 ***/
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
				0,
				0,
				VTXMESH(16, 1),		// 頂点数
				0,
				DRAWPRIM_NUM(16, 1));	// 描画するプリミティブ(三角ポリゴン)の数
		}
	}

	// アルファブレンディングを元に戻す！！(重要！)
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//LIGHTING(pDevice, TRUE);
}

//================================================================================================================
// --- メッシュシリンダーの設置処理 ---
//================================================================================================================
void SetMeshRing(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fRadiusInner, float fRadiusOuter, int nIdxTexture, float fSpdInner, float fSpdOuter, int nLife)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	WORD* pIdx;							// インデックス情報へのポインタ
	HRESULT hr;
	LPMESHRING pRing = &g_aRing[0];

	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++, pRing++)
	{
		if (pRing->bUse == false)
		{
			pRing->pos = pos;
			pRing->rot = rot;
			pRing->fRadiusInner = fRadiusInner;
			pRing->fRadiusOuter = fRadiusOuter;
			pRing->nIdxTexture = g_nIdxTextureRing;
			pRing->bUse = true;
			pRing->fSpdInner = fSpdInner;
			pRing->fSpdOuter = fSpdOuter;
			pRing->nCounter = 0;
			pRing->nLife = nLife;
			pRing->nMaxLife = nLife;

			/*** 頂点バッファの設定 ***/
			pRing->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			for (int nCntHeight = 0; nCntHeight < 2; nCntHeight++)
			{
				/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
				for (int nCntX = 0; nCntX < (16 + 1); nCntX++)
				{
					float fAngle = 0.0f;

					fAngle = (D3DX_2PI / (float)16) * nCntX;
					RepairRot(&fAngle, &fAngle);

					D3DXVECTOR3 nor = {};
					D3DXVECTOR3 pos = {};
					D3DXVECTOR2 tex = {};
					int nCn = nCntX + (nCntHeight * (16 + 1));

					float fRadius;
					if (nCntHeight == 0)
					{
						fRadius = pRing->fRadiusOuter;
					}
					else
					{
						fRadius = pRing->fRadiusInner;
					}
					
					pos.x = sinf(fAngle) * (fRadius * 0.5f);
					pos.y = 0.0f;
					pos.z = cosf(fAngle) * (fRadius * 0.5f);

					pVtx[nCn].pos = pos;

					pVtx[nCn].col = D3DXCOLOR_NULL;

					nor.x = pos.x - pRing->pos.x;
					nor.y = 0.0f;
					nor.z = pos.z - pRing->pos.z;

					D3DXVec3Normalize(&nor, &nor);

					pVtx[nCn].nor = nor;

					pVtx[nCn].tex.x = (1.0f / (float)16) * nCntX;
					pVtx[nCn].tex.y = (1.0f * (float)nCntHeight);
				}
			}

			/*** 頂点バッファの設定を終了 ***/
			pRing->pVtxBuff->Unlock();

			break;
		}
	}
}

void SetIndexTextureRing(int nIdxTexture)
{
	g_nIdxTextureRing = nIdxTexture;
}

int GetIndexTextureRing(void)
{
	return g_nIdxTextureRing;
}
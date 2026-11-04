//==================================================================================
//
// メッシュシリンダー(円柱)の処理 [meshCylinder.cpp]
// Author : TENMA SAITO
//
// 主に背景の空(内側にテクスチャを貼った巨大な円柱)として使う。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "meshCylinder.h"
#include "effect.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MESH				(32)		// メッシュシリンダーの最大設置数
#define MESHCYLINDER_XNUM		(40)		// 横の分割数
#define MESHCYLINDER_YNUM		(1)			// 縦の分割数

//**********************************************************************************
// *** メッシュシリンダー構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置(底面の中心)
	D3DXVECTOR3 rot;					// 角度
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fRadius;						// 直径 (※ 頂点は半分の半径で配置される)
	float fHeight;						// 高さ
	int nXBlock;						// 横の分割数
	int nYBlock;						// 縦の分割数
	int nIdxTexture;					// テクスチャ番号
	bool bUse;							// 使用状況
	bool bUseLight;						// ライティングするか
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshCylinder, *LPMESHCYLINDER;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static MeshCylinder g_aCylinder[MAX_MESH];		// メッシュシリンダーの情報

//==================================================================================
// --- メッシュシリンダーの初期化処理 ---
//==================================================================================
void InitMeshCylinder(void)
{
	for (int nCntCylinder = 0; nCntCylinder < MAX_MESH; nCntCylinder++)
	{
		LPMESHCYLINDER pCylinder = &g_aCylinder[nCntCylinder];		// 初期化するシリンダー

		pCylinder->pos = D3DXVECTOR3_NULL;
		pCylinder->rot = D3DXVECTOR3_NULL;
		pCylinder->fRadius = 0.0f;
		pCylinder->fHeight = 0.0f;
		pCylinder->nIdxTexture = -1;
		pCylinder->nXBlock = 0;
		pCylinder->nYBlock = 0;
		pCylinder->bUse = false;
		pCylinder->bUseLight = true;
	}
}

//==================================================================================
// --- メッシュシリンダーの終了処理 ---
//==================================================================================
void UninitMeshCylinder(void)
{
	for (int nCntCylinder = 0; nCntCylinder < MAX_MESH; nCntCylinder++)
	{
		/*** 頂点バッファ・インデックスバッファの破棄 ***/
		RELEASE(g_aCylinder[nCntCylinder].pVtxBuff);
		RELEASE(g_aCylinder[nCntCylinder].pIdxBuff);
	}
}

//==================================================================================
// --- メッシュシリンダーの更新処理 ---
//==================================================================================
void UpdateMeshCylinder(void)
{

}

//==================================================================================
// --- メッシュシリンダーの描画処理 ---
//==================================================================================
void DrawMeshCylinder(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	for (int nCntCylinder = 0; nCntCylinder < MAX_MESH; nCntCylinder++)
	{
		LPMESHCYLINDER pCylinder = &g_aCylinder[nCntCylinder];		// 描画するシリンダー

		if (pCylinder->bUse == false)
		{ // 未使用
			continue;
		}

		LIGHTING(pDevice, pCylinder->bUseLight);

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pCylinder->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pCylinder->rot.y, pCylinder->rot.x, pCylinder->rot.z);
		D3DXMatrixMultiply(&pCylinder->mtxWorld, &pCylinder->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pCylinder->pos.x, pCylinder->pos.y, pCylinder->pos.z);
		D3DXMatrixMultiply(&pCylinder->mtxWorld, &pCylinder->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pCylinder->mtxWorld);

		/*** 頂点バッファ・インデックスバッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, pCylinder->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetIndices(pCylinder->pIdxBuff);
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pCylinder->nIdxTexture));

		/*** インデックスを利用したポリゴンの描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
			0,
			0,
			MESH_VTX_NUM(pCylinder->nXBlock, pCylinder->nYBlock),		// 頂点数
			0,
			MESH_PRIM_NUM(pCylinder->nXBlock, pCylinder->nYBlock));		// 描画する三角形の数

		LIGHTING(pDevice, TRUE);
	}
}

//==================================================================================
// --- メッシュシリンダーの設置処理 ---
// ※ nXBlock / nYBlock / type は現在未使用 (分割数は MESHCYLINDER_XNUM / YNUM で固定)
// 戻り値 : 設置したシリンダーの番号 (空きが無ければ -1)
//==================================================================================
int SetMeshCylinder(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fHeight, float fRadius, int nXBlock, int nYBlock, CULLINGTYPE type, int nIdxTexture, bool bUseLight)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	for (int nCntCylinder = 0; nCntCylinder < MAX_MESH; nCntCylinder++)
	{
		LPMESHCYLINDER pCylinder = &g_aCylinder[nCntCylinder];		// 設置先のシリンダー

		if (pCylinder->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pCylinder->pos = pos;
		pCylinder->rot = rot;
		pCylinder->fRadius = fRadius;
		pCylinder->fHeight = fHeight;
		pCylinder->nXBlock = MESHCYLINDER_XNUM;
		pCylinder->nYBlock = MESHCYLINDER_YNUM;
		pCylinder->nIdxTexture = nIdxTexture;
		pCylinder->bUse = true;
		pCylinder->bUseLight = bUseLight;

		int nXBlockUse = pCylinder->nXBlock;		// 実際に使う横の分割数
		int nYBlockUse = pCylinder->nYBlock;		// 実際に使う縦の分割数

		/*** 頂点バッファの生成・設定 ***/
		if (SUCCEEDED(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * MESH_VTX_NUM(nXBlockUse, nYBlockUse),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_3D,
			D3DPOOL_MANAGED,
			&pCylinder->pVtxBuff,
			NULL)))
		{
			VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ
			pCylinder->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			for (int nCntY = 0; nCntY <= nYBlockUse; nCntY++)
			{ // 上の段から順に
				float fPosY = (pCylinder->fHeight / (float)nYBlockUse) * (nYBlockUse - nCntY);		// この段の高さ

				for (int nCntX = 0; nCntX <= nXBlockUse; nCntX++)
				{ // 円周に沿って
					float fAngle = (D3DX_2PI / (float)nXBlockUse) * nCntX;		// 円周上の角度
					int nIdxVtx = nCntX + (nCntY * (nXBlockUse + 1));			// 頂点番号

					D3DXVECTOR3 posVtx = D3DXVECTOR3(sinf(fAngle) * (pCylinder->fRadius * 0.5f),
						fPosY,
						cosf(fAngle) * (pCylinder->fRadius * 0.5f));			// 頂点座標

					D3DXVECTOR3 nor = D3DXVECTOR3(posVtx.x - pCylinder->pos.x,
						fPosY,
						posVtx.z - pCylinder->pos.z);							// 法線
					D3DXVec3Normalize(&nor, &nor);

					pVtx[nIdxVtx].pos = posVtx;
					pVtx[nIdxVtx].nor = nor;
					pVtx[nIdxVtx].col = D3DXCOLOR_NULL;
					pVtx[nIdxVtx].tex.x = (1.0f / (float)nXBlockUse) * nCntX;
					pVtx[nIdxVtx].tex.y = (float)nCntY;
				}
			}

			pCylinder->pVtxBuff->Unlock();
		}

		/*** インデックスバッファの生成・設定 ***/
		if (SUCCEEDED(pDevice->CreateIndexBuffer(sizeof(WORD) * MESH_IDX_NUM(nXBlockUse, nYBlockUse),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&pCylinder->pIdxBuff,
			NULL)))
		{
			WORD *pIdx = NULL;		// インデックス情報へのポインタ
			pCylinder->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			SetIndex(pIdx, nXBlockUse, nYBlockUse);

			pCylinder->pIdxBuff->Unlock();
		}

		return nCntCylinder;
	}

	return -1;
}

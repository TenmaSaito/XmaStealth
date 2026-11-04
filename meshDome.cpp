//==================================================================================
//
// メッシュドーム(半球)の処理 [meshDome.cpp]
// Author : TENMA SAITO
//
// 頂点バッファの構成 : [てっぺんの扇(nXBlock + 1頂点)] [側面の格子(MESH_VTX_NUM頂点)]
// ※ 現在 SetMeshDome を呼んでいる画面は無い (タイトル画面で Init / Draw のみ呼ばれている)
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "meshDome.h"
#include "effect.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MESH			(32)		// メッシュドームの最大設置数

//**********************************************************************************
// *** メッシュドーム構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置(中心)
	D3DXVECTOR3 rot;					// 角度
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fRadius;						// 直径 (※ 頂点は半分の半径で配置される)
	int nXBlock;						// 横(円周)の分割数
	int nYBlock;						// 縦の分割数
	int nIdxTexture;					// テクスチャ番号
	D3DCULL cullType;					// カリングタイプ
	bool bUse;							// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshDome, *LPMESHDOME;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static MeshDome g_aDome[MAX_MESH];		// メッシュドームの情報

//==================================================================================
// --- メッシュドームの初期化処理 ---
//==================================================================================
void InitMeshDome(void)
{
	for (int nCntDome = 0; nCntDome < MAX_MESH; nCntDome++)
	{
		LPMESHDOME pDome = &g_aDome[nCntDome];		// 初期化するドーム

		pDome->pos = D3DXVECTOR3_NULL;
		pDome->rot = D3DXVECTOR3_NULL;
		pDome->fRadius = 0.0f;
		pDome->nIdxTexture = -1;
		pDome->nXBlock = 0;
		pDome->nYBlock = 0;
		pDome->bUse = false;
	}
}

//==================================================================================
// --- メッシュドームの終了処理 ---
//==================================================================================
void UninitMeshDome(void)
{
	for (int nCntDome = 0; nCntDome < MAX_MESH; nCntDome++)
	{
		/*** 頂点バッファ・インデックスバッファの破棄 ***/
		RELEASE(g_aDome[nCntDome].pVtxBuff);
		RELEASE(g_aDome[nCntDome].pIdxBuff);
	}
}

//==================================================================================
// --- メッシュドームの更新処理 ---
//==================================================================================
void UpdateMeshDome(void)
{

}

//==================================================================================
// --- メッシュドームの描画処理 ---
//==================================================================================
void DrawMeshDome(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	for (int nCntDome = 0; nCntDome < MAX_MESH; nCntDome++)
	{
		LPMESHDOME pDome = &g_aDome[nCntDome];		// 描画するドーム

		if (pDome->bUse == false)
		{ // 未使用
			continue;
		}

		int nNumVtxTop = pDome->nXBlock + 1;		// てっぺんの扇の頂点数

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pDome->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pDome->rot.y, pDome->rot.x, pDome->rot.z);
		D3DXMatrixMultiply(&pDome->mtxWorld, &pDome->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pDome->pos.x, pDome->pos.y, pDome->pos.z);
		D3DXMatrixMultiply(&pDome->mtxWorld, &pDome->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pDome->mtxWorld);

		/*** 頂点バッファ・インデックスバッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, pDome->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetIndices(pDome->pIdxBuff);
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pDome->nIdxTexture));

		/*** てっぺんの扇の描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN,
			0,
			0,
			nNumVtxTop,				// 頂点数
			0,						// 開始インデックス
			pDome->nXBlock);		// 描画する三角形の数

		/*** 側面の描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
			0,
			0,
			nNumVtxTop + MESH_VTX_NUM(pDome->nXBlock, pDome->nYBlock),		// 頂点数
			nNumVtxTop + 1,													// 開始インデックス(扇の分の次)
			MESH_PRIM_NUM(pDome->nXBlock, pDome->nYBlock));					// 描画する三角形の数
	}
}

//==================================================================================
// --- メッシュドームの設置処理 ---
// 戻り値 : 設置したドームの番号 (空きが無ければ -1)
//==================================================================================
int SetMeshDome(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nXBlock, int nYBlock, float fRadius, D3DCULL type, int nIdxTexture)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	for (int nCntDome = 0; nCntDome < MAX_MESH; nCntDome++)
	{
		LPMESHDOME pDome = &g_aDome[nCntDome];		// 設置先のドーム

		if (pDome->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pDome->pos = pos;
		pDome->rot = rot;
		pDome->fRadius = fRadius;
		pDome->nXBlock = nXBlock;
		pDome->nYBlock = nYBlock;
		pDome->nIdxTexture = nIdxTexture;
		pDome->cullType = type;
		pDome->bUse = true;

		int nNumVtxTop = nXBlock + 1;							// てっぺんの扇の頂点数
		float fAngleY = D3DX_HALFPI / (float)nYBlock;			// 縦1段あたりの角度

		/*** 頂点バッファの生成・設定 ***/
		if (SUCCEEDED(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * (nNumVtxTop + MESH_VTX_NUM(nXBlock, nYBlock)),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_3D,
			D3DPOOL_MANAGED,
			&pDome->pVtxBuff,
			NULL)))
		{
			VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ
			pDome->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			/*** てっぺんの扇 : [0] が頂点、[1]～ が最上段の円周 ***/
			for (int nCntVtx = 0; nCntVtx < nNumVtxTop; nCntVtx++)
			{
				int nRing = (nCntVtx == 0) ? 0 : 1;											// 0 : てっぺん / 1 : 最上段
				float fRingRadius = sinf(nRing * fAngleY) * pDome->fRadius;					// この段の直径
				float fHeight = cosf(nRing * fAngleY) * (pDome->fRadius * 0.5f);			// この段の高さ
				float fAngle = (D3DX_2PI / (float)nXBlock) * (nCntVtx - 1);					// 円周上の角度

				pVtx[nCntVtx].pos = D3DXVECTOR3(sinf(fAngle) * (fRingRadius * 0.5f), fHeight, cosf(fAngle) * (fRingRadius * 0.5f));
				if (nCntVtx == 0) pVtx[nCntVtx].pos = D3DXVECTOR3(0.0f, fHeight, 0.0f);

				D3DXVECTOR3 nor = pDome->pos - pVtx[nCntVtx].pos;		// 法線(内向き)
				D3DXVec3Normalize(&nor, &nor);

				pVtx[nCntVtx].nor = nor;
				pVtx[nCntVtx].col = D3DXCOLOR_NULL;
			}

			pVtx += nNumVtxTop;

			/*** 側面の格子 ***/
			for (int nCntY = 0; nCntY <= nYBlock; nCntY++)
			{
				float fRingRadius = sinf((float)(nCntY + 1) * fAngleY) * pDome->fRadius;		// この段の直径
				float fHeight = cosf((float)(nCntY + 1) * fAngleY) * (pDome->fRadius * 0.5f);	// この段の高さ

				for (int nCntX = 0; nCntX <= nXBlock; nCntX++)
				{
					float fAngle = (D3DX_2PI / (float)nXBlock) * nCntX;		// 円周上の角度
					int nIdxVtx = nCntX + (nCntY * (nXBlock + 1));			// 頂点番号

					D3DXVECTOR3 posVtx = D3DXVECTOR3(sinf(fAngle) * (fRingRadius * 0.5f), fHeight, cosf(fAngle) * (fRingRadius * 0.5f));
					D3DXVECTOR3 nor = pDome->pos - posVtx;		// 法線(内向き)
					D3DXVec3Normalize(&nor, &nor);

					pVtx[nIdxVtx].pos = posVtx;
					pVtx[nIdxVtx].nor = nor;
					pVtx[nIdxVtx].col = D3DXCOLOR_NULL;
					pVtx[nIdxVtx].tex.x = (1.0f / (float)nXBlock) * nCntX;
					pVtx[nIdxVtx].tex.y = (float)nCntY;
				}
			}

			pDome->pVtxBuff->Unlock();
		}

		/*** インデックスバッファの生成・設定 ***/
		if (SUCCEEDED(pDevice->CreateIndexBuffer(sizeof(WORD) * (nNumVtxTop + 1 + MESH_IDX_NUM(nXBlock, nYBlock)),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&pDome->pIdxBuff,
			NULL)))
		{
			WORD *pIdx = NULL;		// インデックス情報へのポインタ
			pDome->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			/*** てっぺんの扇 : 0, 1, 2, ... , nXBlock, 1 (最後に最初の円周の点に戻って閉じる) ***/
			for (int nCntIdx = 0; nCntIdx < nNumVtxTop + 1; nCntIdx++)
			{
				pIdx[nCntIdx] = (WORD)((nCntIdx % nNumVtxTop) + (nCntIdx / nNumVtxTop));
			}

			/*** 側面 : 格子の頂点は扇の後ろに並んでいるため、その分ずらす ***/
			SetIndex(&pIdx[nNumVtxTop + 1], nXBlock, nYBlock, nNumVtxTop);

			pDome->pIdxBuff->Unlock();
		}

		return nCntDome;
	}

	return -1;
}

//================================================================================================================
//
// DirectXのメッシュフィールド用表示処理 [meshDome.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "meshDome.h"
#include "effect.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MESH			(32)		// メッシュシリンダーの最大設置数
#define NUM_MESH			(1)			// メッシュシリンダーの数
#define MESHDOME_RADIUS		(300.0f)	// シリンダーの半径
#define MESHDOME_HEIGHT		(100.0f)	// シリンダーの高さ
#define MESHDOME_XNUM		(16)		// 横の分割数
#define MESHDOME_ZNUM		(16)		// 縦の分割数
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
	float fRadius;			// シリンダーの半径
	int nXBlock;			// Xの分割数
	int nYBlock;			// Yの分割数
	int nIdxTexture;		// テクスチャインデックス
	D3DCULL cullType;		// カリングタイプ
	bool bUse;				// 使用状況
	LPDIRECT3DTEXTURE9		pTexture;	// テクスチャへのポインタ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshDome, *LPMESHDOME;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MeshDome g_aDome[MAX_MESH];		// メッシュフィールド

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitMeshDome(void)
{
	LPMESHDOME pDome = &g_aDome[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pDome++)
	{
		pDome->pos = {};
		pDome->rot = {};
		pDome->fRadius = 0.0f;
		pDome->nIdxTexture = -1;
		pDome->nXBlock = 0;
		pDome->nYBlock = 0;
		pDome->bUse = false;
	}
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitMeshDome(void)
{
	LPMESHDOME pDome = &g_aDome[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pDome++)
	{
		/*** テクスチャの破棄 ***/
		RELEASE(pDome->pTexture);

		/*** 頂点バッファの破棄 ***/
		RELEASE(pDome->pVtxBuff);

		/*** インデックスバッファの破棄 ***/
		RELEASE(pDome->pIdxBuff);
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateMeshDome(void)
{
	
}

//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawMeshDome(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMESHDOME pDome = &g_aDome[0];
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	HRESULT hr;
	DWORD cullDef;

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pDome++)
	{
		if (pDome->bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pDome->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pDome->rot.y,			// Y軸回転
				pDome->rot.x,			// X軸回転
				pDome->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pDome->mtxWorld, &pDome->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pDome->pos.x,
				pDome->pos.y,
				pDome->pos.z);

			D3DXMatrixMultiply(&pDome->mtxWorld, &pDome->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pDome->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pDome->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** インデックスバッファをデータストリームに設定 ***/
			pDevice->SetIndices(pDome->pIdxBuff);

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pDome->nIdxTexture));

			// ポリゴンの描画
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN,
				0,
				0,
				pDome->nXBlock + 1,		// 頂点数
				0,
				pDome->nXBlock);		// 描画するプリミティブ(三角ポリゴン)の数

			/*** インデックスを利用したポリゴンの描画 ***/
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
				0,
				0,
				VTXMESH(pDome->nXBlock, pDome->nYBlock),		// 頂点数
				pDome->nXBlock + 2,
				DRAWPRIM_NUM(pDome->nXBlock, pDome->nYBlock));	// 描画するプリミティブ(三角ポリゴン)の数

			// 保存したカリングタイプを設定
			// pDevice->SetRenderState(D3DRS_CULLMODE, cullDef);
		}
	}
}

//================================================================================================================
// --- メッシュシリンダーの設置処理 ---
//================================================================================================================
int SetMeshDome(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nXBlock, int nYBlock, float fRadius, D3DCULL type, int nIdxTexture)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	WORD* pIdx;							// インデックス情報へのポインタ
	HRESULT hr;
	LPMESHDOME pDome = &g_aDome[0];

	for (int nCntDome = 0; nCntDome < MAX_MESH; nCntDome++, pDome++)
	{
		if (pDome->bUse == false)
		{
			int nCntVtx;
			int nCntIdx;

			pDome->pos = pos;
			pDome->rot = rot;
			pDome->fRadius = fRadius;
			pDome->nXBlock = nXBlock;
			pDome->nYBlock = nYBlock;
			pDome->nIdxTexture = nIdxTexture;
			pDome->cullType = type;
			pDome->bUse = true;

			/*** 頂点バッファの生成 ***/
			hr = pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * (VTXMESH(pDome->nXBlock, pDome->nYBlock) + (pDome->nXBlock + 1)),
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pDome->pVtxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pDome->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			int nn = 0;

			int nCircle = 0;
			D3DXVECTOR3 posCircle[300] = {};
			int nCntCircle = 0;

			for (nCntCircle = 0; nCntCircle < (pDome->nXBlock + 1); nCntCircle++)
			{
				float fAngle = 0.0f;
				float fRadius = sinf((float)(nCircle) * (D3DX_HALFPI / (float)(pDome->nYBlock))) * pDome->fRadius;
				float fHeight = cosf((float)(nCircle) * (D3DX_HALFPI / (float)(pDome->nYBlock))) * (pDome->fRadius * 0.5f);

				fAngle = (D3DX_2PI / (float)pDome->nXBlock) * (nCntCircle - 1);

				if (nCntCircle == 0)
				{
					pVtx[nCntCircle].pos.x = 0.0f;
					pVtx[nCntCircle].pos.y = fHeight;
					pVtx[nCntCircle].pos.z = 0.0f;
					nCircle++;
					// SetEffect(pVtx[nCntCircle].pos, D3DXVECTOR3_NULL, 0.0f, 0.5f, 0.5f);
				}
				else
				{
					pVtx[nCntCircle].pos.x = sinf(fAngle) * (fRadius * 0.5f);
					pVtx[nCntCircle].pos.y = fHeight;
					pVtx[nCntCircle].pos.z = cosf(fAngle) * (fRadius * 0.5f);
					// SetEffect(pVtx[nCntCircle].pos, D3DXVECTOR3_NULL, 0.0f, 0.5f, 0.5f);
				}

				D3DXVECTOR3 nor = (pDome->pos - pVtx[nCntCircle].pos);
				pVtx[nCntCircle].col = D3DXCOLOR_NULL;
				D3DXVec3Normalize(&nor, &nor);
				pVtx[nCntCircle].nor = nor;

				posCircle[nCntCircle] = pVtx[nCntCircle].pos + pDome->pos;
			}

			pVtx += nCntCircle;

			if (SUCCEEDED(hr))
			{
				for (int nCntHeight = 0; nCntHeight < (pDome->nYBlock + 1); nCntHeight++)
				{
					/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
					for (int nCntX = 0; nCntX < (pDome->nXBlock + 1); nCntX++)
					{
						float fAngle = 0.0f;
						float fRadius = sinf((float)(nCntHeight + 1) * (D3DX_HALFPI / (float)(pDome->nYBlock))) * pDome->fRadius;
						float fHeight = cosf((float)(nCntHeight + 1) * (D3DX_HALFPI / (float)(pDome->nYBlock))) * (pDome->fRadius * 0.5f);

						fAngle = (D3DX_2PI / (float)pDome->nXBlock) * nCntX;

						D3DXVECTOR3 nor = {};
						D3DXVECTOR3 pos = {};
						D3DXVECTOR2 tex = {};
						int nCn = nCntX + (nCntHeight * (pDome->nXBlock + 1));

						nn = nCn;

						pos.x = sinf(fAngle) * (fRadius * 0.5f);
						pos.y = fHeight;
						pos.z = cosf(fAngle) * (fRadius * 0.5f);

						pVtx[nCn].pos = pos;

						if (nCntHeight == 0)
						{
							//SetEffect(pos, D3DXVECTOR3_NULL, 0.0f, 0.1f, 0.1f);
						}

						pVtx[nCn].col = D3DXCOLOR_NULL;

						nor.x = pDome->pos.x - pos.x;
						nor.y = pDome->pos.y - pos.y;
						nor.z = pDome->pos.z - pos.z;

						D3DXVec3Normalize(&nor, &nor);

						pVtx[nCn].nor = nor;

						pVtx[nCn].tex.x = (1.0f / (float)pDome->nXBlock) * nCntX;
						pVtx[nCn].tex.y = (1.0f * (float)nCntHeight);
					}
				}
			}

			/*** 頂点バッファの設定を終了 ***/
			pDome->pVtxBuff->Unlock();

			int nIdx = (INDEX_BUFFER(VTXMESH(pDome->nXBlock, pDome->nYBlock), pDome->nXBlock, pDome->nYBlock) + (pDome->nXBlock + 1));

			/*** インデックスバッファの生成 ***/
			hr = pDevice->CreateIndexBuffer(sizeof(WORD) * (INDEX_BUFFER(VTXMESH(pDome->nXBlock, pDome->nYBlock), pDome->nXBlock, pDome->nYBlock) + (pDome->nXBlock + 2)),		// sizeof(WORD) * 必要なインデックス数
				D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16,
				D3DPOOL_MANAGED,
				&pDome->pIdxBuff,
				NULL);

			/*** インデックスバッファをロックし、頂点番号データを取得 ***/
			pDome->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);
			
			WORD wIdx[2048] = {};
			int nCntIndex = 0;

			for (nCntCircle = 0; nCntCircle < (pDome->nXBlock + 2); nCntCircle++)
			{
				pIdx[nCntCircle] = (nCntCircle % (pDome->nXBlock + 1)) + (nCntCircle / (pDome->nXBlock + 1));
			}

			pIdx += nCntCircle;

			if (SUCCEEDED(hr))
			{
				/*** 頂点番号データの設定 ***/
				nCntIndex = SetIndex(pIdx, pDome->nXBlock, pDome->nYBlock, 0, &wIdx[0]);
			}

			/*** インデックスバッファをアンロック ***/
			pDome->pIdxBuff->Unlock();

			return nCntDome;
		}
	}

	return -1;
}

//================================================================================================================
// --- メッシュフィールドの当たり判定処理 ---
//================================================================================================================
#if 0
void CollisionMeshFieldA(D3DXVECTOR3* pPos, D3DXVECTOR3* pPosOld, D3DXVECTOR3* pMove)
{
	//VERTEX_3D *pVtx;					// 頂点情報へのポインタ
	//D3DXVECTOR3 pos[3];							// 境界線ベクトルの始点と終点
	//D3DXVECTOR3 vecLine[3] = {};	// 境界線ベクトル
	//D3DXVECTOR3 vecToPos[3] = {};	// 位置と境界線のはじめを結んだベクトル
	//D3DXVECTOR3 vecNor = {};
	//D3DXVECTOR3 vecGai[2] = {};
	//D3DXVECTOR3 vecMeshLine = {};	// 面のベクトル
	//D3DXVECTOR3 vecToPosTriangle = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	//bool bInside[3];
	//float fVecPos[3] = {};
	//float fVecPosTriangle = 0.0f;

	//if (pPos == NULL || pPosOld == NULL || pMove == NULL)
	//{
	//	return;
	//}

	///*** 頂点バッファの読み込み ***/
	//g_pVtxBuffMeshDome->Lock(0, 0, (void**)&pVtx, D3DLOCK_READONLY);

	//int n = (sizeof g_aMField / sizeof(MeshField));

	//for (int nCntMesh = 0; nCntMesh < (sizeof g_aMField / sizeof(MeshField)); nCntMesh++)
	//{
	//	if (g_aMField[nCntMesh].bUse == true
	//		&& pPos->x >= g_aMField[nCntMesh].pos.x - g_aMField[nCntMesh].fWidth * 0.5f
	//		&& pPos->x <= g_aMField[nCntMesh].pos.x + g_aMField[nCntMesh].fWidth * 0.5f
	//		&& pPos->z >= g_aMField[nCntMesh].pos.z - g_aMField[nCntMesh].fDepth * 0.5f
	//		&& pPos->z <= g_aMField[nCntMesh].pos.z + g_aMField[nCntMesh].fDepth * 0.5f)
	//	{ // 使用状況チェック

	//		n = DRAWPRIM_NUM(MESH_X_NUM, MESH_Z_NUM);

	//		for (int nCntPrimitive = 0; nCntPrimitive < DRAWPRIM_NUM(MESH_X_NUM, MESH_Z_NUM); nCntPrimitive++)
	//		{ // プリミティブ描画数分、当たり判定を確認
	//			if (g_wIdx[nCntMesh][nCntPrimitive] == g_wIdx[nCntMesh][nCntPrimitive + 1]
	//				|| g_wIdx[nCntMesh][nCntPrimitive + 1] == g_wIdx[nCntMesh][nCntPrimitive + 2])
	//			{ // インデックスが重なっている三角ポリゴンは無視
	//				continue;
	//			}

	//			/*** 各三角形の頂点を取得 ***/
	//			pos[0] = pVtx[g_wIdx[nCntMesh][nCntPrimitive]].pos;
	//			pos[1] = pVtx[g_wIdx[nCntMesh][nCntPrimitive + 1]].pos;
	//			pos[2] = pVtx[g_wIdx[nCntMesh][nCntPrimitive + 2]].pos;

	//			/*** 境界線ベクトルを計算 ***/
	//			vecLine[0] = pos[1] - pos[0];
	//			vecLine[1] = pos[2] - pos[1];
	//			vecLine[2] = pos[0] - pos[2];

	//			/*** 現在位置との関係を外積を使い求める ***/
	//			vecToPos[0] = *pPos - pos[0];
	//			vecToPos[1] = *pPos - pos[1];
	//			vecToPos[2] = *pPos - pos[2];

	//			fVecPos[0] = (vecLine[0].z * vecToPos[0].x) - (vecLine[0].x * vecToPos[0].z);	// This is 外積
	//			fVecPos[1] = (vecLine[1].z * vecToPos[1].x) - (vecLine[1].x * vecToPos[1].z);	// This is 外積
	//			fVecPos[2] = (vecLine[2].z * vecToPos[2].x) - (vecLine[2].x * vecToPos[2].z);	// This is 外積

	//			/*** 三角形の番号によって外積判定が変わるため、場合分け ***/
	//			if ((fVecPos[0] > 0 && fVecPos[1] > 0 && fVecPos[2] > 0) 
	//				&& (nCntPrimitive % 2) == 0)
	//			{ // もし偶数番目の三角形なら、境界線ベクトルの内側にいれば
	//				PrintDebugProc("On Triangle [%~3d]\n", 
	//					g_wIdx[nCntMesh][nCntPrimitive],
	//					g_wIdx[nCntMesh][nCntPrimitive + 1],
	//					g_wIdx[nCntMesh][nCntPrimitive + 2]);

	//				/** その三角形の内側にいる判定 **/
	//				SetEffect(pos[0], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[0] + (vecLine[0] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				SetEffect(pos[1], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[1] + (vecLine[1] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				SetEffect(pos[2], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[2] + (vecLine[2] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				/*** 外積用ベクトルを用意 ***/
	//				vecGai[0] = pos[1] - pos[0];
	//				vecGai[1] = pos[2] - pos[0];

	//				/** ベクトルを用いて３次元ベクトルの外積を求める **/
	//				vecNor.x = (vecGai[0].y * vecGai[1].z) - (vecGai[0].z * vecGai[1].y);
	//				vecNor.y = (vecGai[0].z * vecGai[1].x) - (vecGai[0].x * vecGai[1].z);
	//				vecNor.z = (vecGai[0].x * vecGai[1].y) - (vecGai[0].y * vecGai[1].x);

	//				/** 法線ベクトルを正規化 **/
	//				D3DXVec3Normalize(&vecNor, &vecNor);

	//				PrintDebugProc("vecNor : %~3f\n",
	//					vecNor.x,
	//					vecNor.y,
	//					vecNor.z);

	//				vecMeshLine = (pos[1] + vecLine[1] * 0.5f) - pos[0];

	//				vecToPosTriangle = *pPos - pos[0];

	//				fVecPosTriangle = (vecMeshLine.z * vecToPosTriangle.x) - (vecMeshLine.x * vecToPosTriangle.z);

	//				PrintDebugProc("fVecPosTriangle : %f\n", fVecPosTriangle);
	//			}

	//			if ((fVecPos[0] < 0 && fVecPos[1] < 0 && fVecPos[2] < 0)
	//				&& (nCntPrimitive % 2) == 1)
	//			{ // もし奇数個の三角形なら、境界線ベクトルの外側にいれば
	//				PrintDebugProc("On Triangle [%~3d]\n",
	//					g_wIdx[nCntMesh][nCntPrimitive],
	//					g_wIdx[nCntMesh][nCntPrimitive + 1],
	//					g_wIdx[nCntMesh][nCntPrimitive + 2]);

	//				/** その三角形の内側にいる判定 **/
	//				SetEffect(pos[0], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[0] + (vecLine[0] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				SetEffect(pos[1], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[1] + (vecLine[1] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				SetEffect(pos[2], D3DXVECTOR3_NULL, 0.0f, 5.0f, 5.0f);

	//				for (int nCntEffect = 0; nCntEffect < 50; nCntEffect++)
	//				{
	//					SetEffect(pos[2] + (vecLine[2] * ((float)nCntEffect / 50.0f)), D3DXVECTOR3_NULL, 0.0f, 2.5f, 2.5f);
	//				}

	//				vecGai[0] = pos[1] - pos[0];
	//				vecGai[1] = pos[2] - pos[0];

	//				vecNor.x = (vecGai[0].y * vecGai[1].z) - (vecGai[0].z * vecGai[1].y);
	//				vecNor.y = (vecGai[0].z * vecGai[1].x) - (vecGai[0].x * vecGai[1].z);
	//				vecNor.z = (vecGai[0].x * vecGai[1].y) - (vecGai[0].y * vecGai[1].x);

	//				vecNor *= -1;

	//				D3DXVec3Normalize(&vecNor, &vecNor);

	//				PrintDebugProc("vecNor : %~3f\n",
	//					vecNor.x,
	//					vecNor.y,
	//					vecNor.z);
	//			}
	//		}
	//	}
	//}

	///*** 頂点バッファの設定を終了 ***/
	//g_pVtxBuffMeshDome->Unlock();
}
#endif
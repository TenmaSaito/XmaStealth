//================================================================================================================
//
// DirectXのメッシュフィールド用表示処理 [meshCylinder.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "meshCylinder.h"
#include "effect.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MESH				(32)		// メッシュシリンダーの最大設置数
#define NUM_MESH				(1)			// メッシュシリンダーの数
#define MESHCYLINDER_RADIUS		(300.0f)	// シリンダーの半径
#define MESHCYLINDER_HEIGHT		(100.0f)	// シリンダーの高さ
#define MESHCYLINDER_XNUM		(40)		// 横の分割数
#define MESHCYLINDER_ZNUM		(1)			// 縦の分割数
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
	float fHeight;			// シリンダーの高さ
	int nXBlock;				// Xの分割数
	int nYBlock;				// Yの分割数
	int nIdxTexture;		// テクスチャインデックス
	bool bUse;				// 使用状況
	bool bUseLight;			// ライティングのオンオフ
	LPDIRECT3DTEXTURE9		pTexture;	// テクスチャへのポインタ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshCylinder, *LPMESHCYLINDER;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MeshCylinder g_aCylinder[MAX_MESH];		// メッシュフィールド

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitMeshCylinder(void)
{
	LPMESHCYLINDER pCylinder = &g_aCylinder[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pCylinder++)
	{
		pCylinder->pos = {};
		pCylinder->rot = {};
		pCylinder->fRadius = 0.0f;
		pCylinder->fHeight = 0.0f;
		pCylinder->nIdxTexture = -1;
		pCylinder->nXBlock = 0;
		pCylinder->nYBlock = 0;
		pCylinder->bUse = false;
		pCylinder->bUseLight = true;
	}
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitMeshCylinder(void)
{
	LPMESHCYLINDER pCylinder = &g_aCylinder[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pCylinder++)
	{
		/*** テクスチャの破棄 ***/
		RELEASE(pCylinder->pTexture);

		/*** 頂点バッファの破棄 ***/
		RELEASE(pCylinder->pVtxBuff);

		/*** インデックスバッファの破棄 ***/
		RELEASE(pCylinder->pIdxBuff);
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateMeshCylinder(void)
{

}

//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawMeshCylinder(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMESHCYLINDER pCylinder = &g_aCylinder[0];
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pCylinder++)
	{
		if (pCylinder->bUse == true)
		{
			LIGHTING(pDevice, pCylinder->bUseLight);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pCylinder->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pCylinder->rot.y,			// Y軸回転
				pCylinder->rot.x,			// X軸回転
				pCylinder->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pCylinder->mtxWorld, &pCylinder->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pCylinder->pos.x,
				pCylinder->pos.y,
				pCylinder->pos.z);

			D3DXMatrixMultiply(&pCylinder->mtxWorld, &pCylinder->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pCylinder->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pCylinder->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** インデックスバッファをデータストリームに設定 ***/
			pDevice->SetIndices(pCylinder->pIdxBuff);

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pCylinder->nIdxTexture));

			/*** インデックスを利用したポリゴンの描画 ***/
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
				0,
				0,
				VTXMESH(pCylinder->nXBlock, pCylinder->nYBlock),		// 頂点数
				0,
				DRAWPRIM_NUM(pCylinder->nXBlock, pCylinder->nYBlock));	// 描画するプリミティブ(三角ポリゴン)の数

			LIGHTING(pDevice, TRUE);
		}
	}
}

//================================================================================================================
// --- メッシュシリンダーの設置処理 ---
//================================================================================================================
int SetMeshCylinder(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fHeight, float fRadius, int nXBlock, int nYBlock, CULLINGTYPE type, int nIdxTexture, bool bUseLight)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	WORD* pIdx;							// インデックス情報へのポインタ
	HRESULT hr;
	LPMESHCYLINDER pCylinder = &g_aCylinder[0];

	for (int nCntCylinder = 0; nCntCylinder < MAX_MESH; nCntCylinder++, pCylinder++)
	{
		if (pCylinder->bUse == false)
		{
			pCylinder->pos = pos;
			pCylinder->rot = rot;
			pCylinder->fRadius = fRadius;
			pCylinder->fHeight = fHeight;
			pCylinder->nXBlock = MESHCYLINDER_XNUM;
			pCylinder->nYBlock = MESHCYLINDER_ZNUM;
			pCylinder->nIdxTexture = nIdxTexture;
			pCylinder->bUse = true;
			pCylinder->bUseLight = bUseLight;

			/*** 頂点バッファの生成 ***/
			hr = pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * VTXMESH(pCylinder->nXBlock, pCylinder->nYBlock),
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pCylinder->pVtxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pCylinder->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			if (SUCCEEDED(hr))
			{
				for (int nCntHeight = 0; nCntHeight < (pCylinder->nYBlock + 1); nCntHeight++)
				{
					float fHeight = (pCylinder->fHeight / (float)(pCylinder->nYBlock)) * ((pCylinder->nYBlock) - nCntHeight);

					/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
					for (int nCntX = 0; nCntX < (pCylinder->nXBlock + 1); nCntX++)
					{
						float fAngle = 0.0f;

						switch (type)
						{
						case CULLINGTYPE_FRONT:

							fAngle = (D3DX_2PI / (float)pCylinder->nXBlock) * nCntX;

							break;

						case CULLINGTYPE_BACK:

							fAngle = (-D3DX_2PI / (float)pCylinder->nXBlock) * nCntX;

							break;

						default:
							break;
						}

						D3DXVECTOR3 nor = {};
						D3DXVECTOR3 pos = {};
						D3DXVECTOR2 tex = {};
						int nCn = nCntX + (nCntHeight * (pCylinder->nXBlock + 1));

						pos.x = sinf(fAngle) * (pCylinder->fRadius * 0.5f);
						pos.y = fHeight;
						pos.z = cosf(fAngle) * (pCylinder->fRadius * 0.5f);

						pVtx[nCn].pos = pos;

						pVtx[nCn].col = D3DXCOLOR_NULL;

						nor.x = pos.x - pCylinder->pos.x;
						nor.y = fHeight;
						nor.z = pos.z - pCylinder->pos.z;

						D3DXVec3Normalize(&nor, &nor);

						pVtx[nCn].nor = nor;

						pVtx[nCn].tex.x = (1.0f / (float)pCylinder->nXBlock) * nCntX;
						pVtx[nCn].tex.y = (1.0f * (float)nCntHeight);
					}
				}
			}

			/*** 頂点バッファの設定を終了 ***/
			pCylinder->pVtxBuff->Unlock();

			/*** インデックスバッファの生成 ***/
			hr = pDevice->CreateIndexBuffer(sizeof(WORD) * INDEX_BUFFER(VTXMESH(pCylinder->nXBlock, pCylinder->nYBlock), pCylinder->nXBlock, pCylinder->nYBlock),		// sizeof(WORD) * 必要なインデックス数
				D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16,
				D3DPOOL_MANAGED,
				&pCylinder->pIdxBuff,
				NULL);

			/*** インデックスバッファをロックし、頂点番号データを取得 ***/
			pCylinder->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			if (SUCCEEDED(hr))
			{
				/*** 頂点番号データの設定 ***/
				SetIndex(pIdx, pCylinder->nXBlock, pCylinder->nYBlock);
			}

			/*** インデックスバッファをアンロック ***/
			pCylinder->pIdxBuff->Unlock();

			return nCntCylinder;
		}
	}

	return -1;
}

//================================================================================================================
// --- メッシュフィールドの当たり判定処理 ---
//================================================================================================================
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
	//g_pVtxBuffMeshCylinder->Lock(0, 0, (void**)&pVtx, D3DLOCK_READONLY);

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
	//g_pVtxBuffMeshCylinder->Unlock();
}
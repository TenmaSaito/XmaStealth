//================================================================================================================
//
// DirectXのメッシュスフィア用表示処理 [meshSphere.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "meshSphere.h"
#include "effect.h"
#include "XmasTree.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MESH			(256)		// メッシュシリンダーの最大設置数
#define VTXMESH_SPHERE(nNumX, nNumY)	(((nNumX + 1) * (nNumY + 1)) + ((nNumX + 1) * 2))	// VTXBUFF確保
#define INDEXNUM_TOP(xBlock)			(xBlock + 1)		// 蓋の頂点数
#define INDEX_BUFFER(vertexNum, xBlock, zBlock)	(vertexNum + ((xBlock + 1) * (zBlock - 1)) + ((zBlock - 1) * 2))	// インデックスバッファ確保用		
#define DRAWPRIM_NUM(xBlock, yBlock)	(((xBlock * yBlock) * 2) + ((yBlock - 1) * 4))	// 描画プリミティブ用
#define INDEXNUM_DOWN(xBlock)			(xBlock + 1)		// 下の頂点数
#define VERTEX_BUFFER(xBlock, zBlock)	((xBlock + 1) * (zBlock + 1))	// 頂点バッファの確保用

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
MeshSphere g_aMeshSphere[MAX_MESH] = {};
int g_nCounterSphere;						// 合計数

//================================================================================================================
// --- メッシュスフィアの初期化処理 ---
//================================================================================================================
void InitMeshSphere(void)
{
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];
	ZeroMemory(&g_aMeshSphere[0], sizeof(MeshSphere) * MAX_MESH);

	g_nCounterSphere = 0;
}

//================================================================================================================
// --- メッシュスフィアの終了処理 ---
//================================================================================================================
void UninitMeshSphere(void)
{
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pSphere++)
	{
		/*** テクスチャの破棄 ***/
		RELEASE(pSphere->pTexture);

		/*** 頂点バッファの破棄 ***/
		RELEASE(pSphere->pVtxBuff);

		/*** インデックスバッファの破棄 ***/
		RELEASE(pSphere->pIdxBuff);
	}
}

//================================================================================================================
// --- メッシュスフィアの更新処理 ---
//================================================================================================================
void UpdateMeshSphere(void)
{
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];
	PXMASTREE pXmas = GetXmasTree();
	D3DXMATRIX mtxSphere;
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス

	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++, pSphere++)
	{
		if ((pSphere->bCollision == true)
			&& GetMode() == MODE_GAME)
		{
			
		}
	}
}

//================================================================================================================
// --- メッシュスフィアの描画処理 ---
//================================================================================================================
void DrawMeshSphere(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	DWORD dwCurrentTime;						// 現在時刻
	DWORD dwExecLastTime;						// 最後に処理した時刻
	DWORD dwFrameCount;							// フレームカウント
	DWORD dwFPSLastTime;						// 最後にFPSを計測した時刻

	dwCurrentTime = 0;							// 初期化
	dwExecLastTime = timeGetTime();				// 現在時刻を取得
	dwFrameCount = 0;
	dwFPSLastTime = timeGetTime();

	// SetCullingType(D3DCULL_NONE);

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pSphere++)
	{
		if (pSphere->bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pSphere->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pSphere->rot.y,			// Y軸回転
				pSphere->rot.x,			// X軸回転
				pSphere->rot.z);		// Z軸回転

			D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pSphere->pos.x,
				pSphere->pos.y,
				pSphere->pos.z);

			D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, &mtxTrans);

			if (pSphere->pMtxParent != NULL)
			{
				D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, pSphere->pMtxParent);
			}

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pSphere->mtxWorld);

			if (pSphere->bDisp == true)
			{
				/*** 頂点バッファをデータストリームに設定 ***/
				pDevice->SetStreamSource(0, pSphere->pVtxBuff, 0, sizeof(VERTEX_3D));

				/*** インデックスバッファをデータストリームに設定 ***/
				pDevice->SetIndices(pSphere->pIdxBuff);

				/*** 頂点フォーマットの設定 ***/
				pDevice->SetFVF(FVF_VERTEX_3D);

				/*** テクスチャの設定 ***/
				pDevice->SetTexture(0, GetTexture(-1));

				// ポリゴンの描画
				//pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN,
				//	0,
				//	0,
				//	INDEXNUM_TOP(pSphere->nXBlock),		// 頂点数
				//	0,
				//	pSphere->nXBlock);		// 描画するプリミティブ(三角ポリゴン)の数

				/*** インデックスを利用したポリゴンの描画 ***/
				pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
					0,
					0,
					VERTEX_BUFFER(pSphere->nXBlock, pSphere->nYBlock),		// 頂点数
					pSphere->nXBlock + 2,
					DRAWPRIM_NUM(pSphere->nXBlock, pSphere->nYBlock));	// 描画するプリミティブ(三角ポリゴン)の数

				// ポリゴンの描画
				//pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN,
				//	0,
				//	0,
				//	INDEXNUM_DOWN(pSphere->nXBlock),		// 頂点数
				//	(pSphere->nXBlock + 2) + INDEX_BUFFER(((pSphere->nXBlock + 1) * (pSphere->nYBlock + 1)), pSphere->nXBlock, pSphere->nYBlock),
				//	pSphere->nXBlock);		// 描画するプリミティブ(三角ポリゴン)の数
			}
		}
	}
}

//================================================================================================================
// --- メッシュスフィアの描画設定処理 ---
//================================================================================================================
int SetMeshSphere(_OffSet_ D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	WORD* pIdx;							// インデックス情報へのポインタ
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];

	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++, pSphere++)
	{
		if (pSphere->bUse == false)
		{
			pSphere->pos = pos;
			pSphere->rot = rot;
			pSphere->col = col;
			pSphere->fRadius = fRadius;
			pSphere->nXBlock = nXBlock;
			pSphere->nYBlock = nYBlock;
			pSphere->bUse = true;
			pSphere->bDisp = bDisp;
			pSphere->bCollision = false;

			/*** 頂点バッファの生成 ***/
			pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * (VTXMESH_SPHERE(nXBlock, nYBlock)),
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pSphere->pVtxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pSphere->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			int nSphere = 0;

			/*** 球の天井の座標設定 ***/
			for (int nCntSphere = 0; nCntSphere < (pSphere->nXBlock + 1); nCntSphere++, pVtx++)
			{
				float fAngle = 0.0f;
				float fRadiusDiv = sinf((float)(nSphere) * (D3DX_PI / (float)(pSphere->nYBlock))) * pSphere->fRadius;
				float fHeightDiv = cosf((float)(nSphere) * (D3DX_PI / (float)(pSphere->nYBlock))) * (pSphere->fRadius);

				fAngle = (D3DX_2PI / (float)pSphere->nXBlock) * (nCntSphere - 1) * -1;

				if (nCntSphere == 0)
				{
					pVtx->pos.x = 0.0f;
					pVtx->pos.y = fHeightDiv;
					pVtx->pos.z = 0.0f;
					nSphere++;
				}
				else
				{
					pVtx->pos.x = sinf(fAngle) * (fRadiusDiv);
					pVtx->pos.y = fHeightDiv;
					pVtx->pos.z = cosf(fAngle) * (fRadiusDiv);
				}

				D3DXVECTOR3 nor = (pVtx->pos - pSphere->pos);
				pVtx->col = col;
				D3DXVec3Normalize(&nor, &nor);
				pVtx->nor = nor;
			}

			/*** 球の中腹の座標設定 ***/
			for (int nCntY = 0; nCntY < (nYBlock + 1); nCntY++)
			{
				/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
				for (int nCntX = 0; nCntX < (nXBlock + 1); nCntX++, pVtx++)
				{
					float fAngle = 0.0f;
					float fRadiusDiv = sinf((float)(nCntY + 1) * (D3DX_PI / (float)(nYBlock))) * fRadius;
					float fHeightDiv = cosf((float)(nCntY + 1) * (D3DX_PI / (float)(nYBlock))) * (fRadius);

					fAngle = (D3DX_2PI / (float)nXBlock) * nCntX;

					D3DXVECTOR3 norDiv = {};
					D3DXVECTOR3 posDiv = {};
					D3DXVECTOR2 texDiv = {};

					posDiv.x = sinf(fAngle) * (fRadiusDiv);
					posDiv.y = fHeightDiv;
					posDiv.z = cosf(fAngle) * (fRadiusDiv);

					pVtx->pos = posDiv;

					pVtx->col = col;

					norDiv.x = posDiv.x - pos.x;
					norDiv.y = posDiv.y - pos.y;
					norDiv.z = posDiv.z - pos.z;

					D3DXVec3Normalize(&norDiv, &norDiv);

					pVtx->nor = norDiv;

					pVtx->tex.x = (1.0f / (float)nXBlock) * nCntX;
					pVtx->tex.y = (1.0f * (float)nCntY);
				}
			}

			nSphere = 0;
			InitedVec3(vec[300]);

			/*** 球の下の座標設定 ***/
			for (int nCntSphere = 0; nCntSphere < (pSphere->nXBlock + 1); nCntSphere++, pVtx++)
			{
				float fAngle = 0.0f;
				float fRadiusDiv = sinf((float)(nSphere) * (D3DX_PI / (float)(pSphere->nYBlock))) * pSphere->fRadius;
				float fHeightDiv = cosf((float)(nSphere) * (D3DX_PI / (float)(pSphere->nYBlock))) * (pSphere->fRadius);

				fAngle = (D3DX_2PI / (float)pSphere->nXBlock) * (nCntSphere - 1);

				if (nCntSphere == 0)
				{
					pVtx->pos.x = 0.0f;
					pVtx->pos.y = -fHeightDiv;
					pVtx->pos.z = 0.0f;
					nSphere++;
				}
				else
				{
					pVtx->pos.x = sinf(fAngle) * (fRadiusDiv);
					pVtx->pos.y = -fHeightDiv;
					pVtx->pos.z = cosf(fAngle) * (fRadiusDiv);
					vec[nCntSphere] = pVtx->pos + pos;
				}

				D3DXVECTOR3 nor = (pVtx->pos - pSphere->pos);
				pVtx->col = col;
				D3DXVec3Normalize(&nor, &nor);
				pVtx->nor = nor;
			}

			/*** 頂点バッファの設定を終了 ***/
			pSphere->pVtxBuff->Unlock();

			/*** インデックスバッファの生成 ***/
			pDevice->CreateIndexBuffer(sizeof(WORD) * (INDEX_BUFFER(((nXBlock + 1) * (nYBlock + 1)), nXBlock, nYBlock) + ((nXBlock + 2) * 2)),		// sizeof(WORD) * 必要なインデックス数
				D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16,
				D3DPOOL_MANAGED,
				&pSphere->pIdxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pSphere->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			/*** 球の天井のインデックス設定 ***/
			for (int nCntSphere = 0; nCntSphere < (nXBlock + 2); nCntSphere++, pIdx++)
			{
				*pIdx = (nCntSphere % (nXBlock + 1)) + (nCntSphere / (nXBlock + 1));
			}

			int nCount = 0;
			int nCounts = 0;
			int nLine = 0;
			int nOffSet = (nXBlock + 1);

			for (int nCntIndex = 0; nCntIndex < INDEX_BUFFER(((nXBlock + 1) * (nYBlock + 1)), nXBlock, nYBlock); nCntIndex++, pIdx++)
			{
				if (nCntIndex != 0 && nCounts == (nXBlock + 1))
				{ // 0ではなく、且つ上辺の点が右端に着いたとき
					nCounts--;
					*pIdx = nCounts + ((nXBlock + 1) * nLine);

					*pIdx += nOffSet;

					nCounts = 0;
				}
				else if (nCntIndex != 0 && nCount == (nXBlock + 1) && nCounts == 0)
				{ // 0ではなく、且つ右端の多重設定が済んでいる且つ、下辺の点が右端に着いたとき
					nCount = 0;
					nLine++;
					*pIdx = nCount + ((nXBlock + 1) * (nLine + 1));

					*pIdx += nOffSet;
				}
				else
				{
					if (nCntIndex == 0)
					{ // 初回限定0の場合
						*pIdx = nCount + ((nXBlock + 1) * (nLine + 1));

						*pIdx += nOffSet;

						nCount++;
					}
					else if (nCntIndex % 2 == 0)
					{ // 偶数の場合	
						*pIdx = nCount + ((nXBlock + 1) * (nLine + 1));

						*pIdx += nOffSet;

						nCount++;
					}
					else
					{ // 奇数の場合
						*pIdx = nCounts + ((nXBlock + 1) * nLine);

						*pIdx += nOffSet;

						nCounts++;
					}
				}
			}

			/*** 球の下のインデックス設定 ***/
			for (int nCntSphere = 0; nCntSphere < (nXBlock + 2); nCntSphere++, pIdx++)
			{
				if (nCntSphere == 0)
				{
					*pIdx = VTXMESH_SPHERE(nXBlock, nYBlock) - (nXBlock + 1);
				}
				else
				{
					*pIdx = (nCntSphere % (nXBlock + 1)) + (VTXMESH_SPHERE(nXBlock, nYBlock) - (nXBlock + 1)) + (nCntSphere / (nXBlock + 1));
				}
			}

			/*** インデックスバッファをアンロック ***/
			pSphere->pIdxBuff->Unlock();

			g_nCounterSphere++;

			return nCntSphere;
		}
	}

	return -1;
}

//================================================================================================================
// --- 親子関係付きメッシュスフィアの設置処理 ---
//================================================================================================================
int SetParentSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp, D3DXMATRIX* pMtxParent)
{
	int nIdxSphere = SetMeshSphere(pos, rot, col, nXBlock, nYBlock, fRadius, bDisp);

	if(nIdxSphere == -1) return -1;
	LPMESHSPHERE pSphere = &g_aMeshSphere[nIdxSphere];

	pSphere->pMtxParent = pMtxParent;
	pSphere->bCollision = true;

	return nIdxSphere;
}

//================================================================================================================
// --- メッシュスフィアの取得処理 ---
//================================================================================================================
LPMESHSPHERE GetMeshSphere(int nIdxSphere)
{
	if (nIdxSphere < 0 || nIdxSphere >= g_nCounterSphere)
	{
		return NULL;
	}

	return &g_aMeshSphere[nIdxSphere];
}

//================================================================================================================
// --- メッシュスフィアの当たり判定処理 ---
//================================================================================================================
void CollisionSphere(D3DXVECTOR3 pos, float fRadius)
{
	LPMESHSPHERE pSphere = &g_aMeshSphere[0];

	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++, pSphere++)
	{
		if (pSphere->bUse == true)
		{
			InitedVec3(length);
			InitedVec3(posSphere);
			float fLenght = 0.0f;

			if (pSphere->pMtxParent != NULL)
			{
				D3DXVec3TransformCoord(&posSphere, &pSphere->pos, pSphere->pMtxParent);
			}
			else
			{
				posSphere = pSphere->pos;
			}

			length = pos - pSphere->pos;
			fLenght = D3DXVec3Length(&length);
			if (D3DXVec3Length(&length) <= (fRadius + (pSphere->fRadius)))
			{
				PrintDebugProc("Hit %d\n", nCntSphere);
			}
		}
	}
}

bool CollisionSphereToSphere(LPMESHSPHERE pSphere)
{
	LPMESHSPHERE pSphereCol = &g_aMeshSphere[0];

	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++, pSphereCol++)
	{
		if (pSphereCol->bUse == true && pSphere->bCollision == true && pSphereCol->pVtxBuff != pSphere->pVtxBuff)
		{
			InitedVec3(length);
			InitedVec3(posSphere);
			InitedVec3(posSphereCol);
			float fLenght = 0.0f;

			if (pSphere->pMtxParent != NULL)
			{
				D3DXVec3TransformCoord(&posSphere, &pSphere->pos, pSphere->pMtxParent);
			}
			else
			{
				posSphere = pSphere->pos;
			}

			if (pSphereCol->pMtxParent != NULL)
			{
				D3DXVec3TransformCoord(&posSphereCol, &pSphereCol->pos, pSphereCol->pMtxParent);
			}
			else
			{
				posSphereCol = pSphereCol->pos;
			}

			length = posSphere - posSphereCol;
			fLenght = D3DXVec3Length(&length);
			if (D3DXVec3Length(&length) <= (pSphere->fRadius + (pSphereCol->fRadius)))
			{
				PrintDebugProc("Hit %d\n", nCntSphere);
				return true;
			}
		}
	}

	return false;
}

//================================================================================================================
// --- 指定した2つのメッシュスフィアの当たり判定処理 ---
//================================================================================================================
bool CollisionSphereToSphere(LPMESHSPHERE pSphere, LPMESHSPHERE pSphereAnother)
{
	InitedVec3(length);
	InitedVec3(posSphere);
	InitedVec3(posSphereAnother);
	float fLenght = 0.0f;

	if (pSphere == NULL || pSphereAnother == NULL) return false;

	if (pSphere->pMtxParent != NULL)
	{
		//D3DXVec3TransformCoord(&posSphere, &pSphere->pos, pSphere->pMtxParent);
		posSphere.x = pSphere->mtxWorld._41;
		posSphere.y = pSphere->mtxWorld._42;
		posSphere.z = pSphere->mtxWorld._43;
	}
	else
	{
		posSphere = pSphere->pos;
	}

	if (pSphereAnother->pMtxParent != NULL)
	{
		D3DXVec3TransformCoord(&posSphereAnother, &pSphereAnother->pos, pSphereAnother->pMtxParent);
	}
	else
	{
		posSphereAnother = pSphereAnother->pos;
	}

	length = posSphere - posSphereAnother;
	fLenght = D3DXVec3Length(&length);
	if (D3DXVec3Length(&length) <= (pSphere->fRadius + (pSphereAnother->fRadius)))
	{
		return true;
	}

	return false;
}

//================================================================================================================
// --- 指定したメッシュスフィアとの当たり判定処理 ---
//================================================================================================================
bool CollisionIndexSphere(int nIndexSphere, D3DXVECTOR3 pos, float fRadius)
{
	LPMESHSPHERE pSphere = &g_aMeshSphere[nIndexSphere];

	if (pSphere->bUse == true)
	{
		InitedVec3(length);
		Vec3(posRepair = pSphere->pos);
		float fLength = 0.0f;

		D3DXVec3TransformCoord(&posRepair, &posRepair, &pSphere->mtxWorld);

		length = pos - posRepair;
		fLength = D3DXVec3Length(&length);
		if (fLength <= (fRadius + (pSphere->fRadius)))
		{
			return true;
		}
	}

	return false;
}

//================================================================================================================
// --- 指定したメッシュスフィアとの当たり判定処理 ---
//================================================================================================================
bool CollisionLPSphere(LPMESHSPHERE pSphere, D3DXVECTOR3 pos, float fMag)
{
	if (pSphere == NULL) return false;

	if (pSphere->bUse == true)
	{
		InitedVec3(length);
		Vec3(posRepair = pSphere->pos);
		float fLength = 0.0f;

		D3DXVec3TransformCoord(&posRepair, &posRepair, &pSphere->mtxWorld);

		length = pos - posRepair;
		fLength = D3DXVec3Length(&length);
		if (fLength <= pSphere->fRadius * fMag)
		{
			return true;
		}
	}

	return false;
}
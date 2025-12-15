//================================================================================================================
//
// DirectXのXファイル読み込み表示処理 [xmodel.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "xmodel.h"
#include "mathUtil.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
XM_INFO g_aXmInfo[MODE_MAX][MAX_XMODEL];		// オブジェクト情報

//================================================================================================================
// --- 初期化 ---
//================================================================================================================
void InitXmodel(void)
{
	ZeroMemory(&g_aXmInfo[0][0], sizeof(XM_INFO) * (MAX_XMODEL * MODE_MAX));
}

//================================================================================================================
// --- モデルの読み込み ---
//================================================================================================================
HRESULT LoadXmodel(_In_ const char* pXFileName, MODE type)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	LPXM_INFO pXmInfo = &g_aXmInfo[type][0];
	D3DXMATERIAL* pMat = NULL;		// マテリアルへのポインタ
	HRESULT hr = E_FAIL;			// 読み込み成功判定
	int nNumVtx = 0;				// 頂点数
	DWORD dwSizeFVF = 0;			// 頂点フォーマットのサイズ
	BYTE* pVtxBuff = NULL;			// 頂点バッファへのポインタ

	/*** 過去に読み込んでいないか確認 ***/
	for (int nCntXmodel = 0; nCntXmodel < MAX_XMODEL; nCntXmodel++, pXmInfo++)
	{
		if (strcmp(&pXmInfo->aXFileName[0], pXFileName) == NULL)
		{
			return S_OK;
		}
	}

	pXmInfo = &g_aXmInfo[type][0];

	/*** Xファイルの読み込み ***/
	for (int nCntObject = 0; nCntObject < MAX_XMODEL; nCntObject++, pXmInfo++)
	{
		if (pXmInfo->bUse != true)
		{
			pXmInfo->bUse = true;

			pMat = NULL;

			hr = D3DXLoadMeshFromX(pXFileName,			// 読み込むXファイル名
				D3DXMESH_SYSTEMMEM,
				pDevice,								// デバイスポインタ
				NULL,
				&pXmInfo->pBuffMat,	// マテリアルへのポインタ
				NULL,
				&pXmInfo->dwNumMat,	// マテリアルの数
				&pXmInfo->pMesh);		// メッシュへのポインタ

			if (FAILED(hr))
			{
				MessageBox(GetHandleWindow(), "Xファイルの読み込みに失敗しました。", "Error(3)", MB_ICONERROR);

				return E_FAIL;
			}

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)pXmInfo->pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)pXmInfo->dwNumMat; nCntMat++)
			{
				if (pMat[nCntMat].pTextureFilename != NULL)
				{
					if (CheckPath(pMat[nCntMat].pTextureFilename))
					{
						GenerateMessageBox(MB_ICONERROR,
							"Warnings!",
							"対象Xファイルパス : %s",
							pXFileName);
					}

					/*** テクスチャの読み込み ***/
					hr = D3DXCreateTextureFromFile(pDevice,
						pMat[nCntMat].pTextureFilename,
						&pXmInfo->apTexture[nCntMat]);

					if (FAILED(hr))
					{
						GenerateMessageBox(MB_ICONERROR,
							"Error (4)",
							"Xファイルのテクスチャ読み込みに失敗しました。\n対象パス : %s",
							&pMat[nCntMat].pTextureFilename[0]);
					}
				}
			}

			/*** 頂点数を取得 ***/
			nNumVtx = pXmInfo->pMesh->GetNumVertices();

			/*** 頂点フォーマットのサイズを取得 ***/
			dwSizeFVF = D3DXGetFVFVertexSize(pXmInfo->pMesh->GetFVF());

			/*** 頂点バッファをロック ***/
			pXmInfo->pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

			/*** 頂点の最大、最小値を取得 ***/
			for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++)
			{
				D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;	// 頂点座標の代入

				/*** 最小値を取得 ***/
				if (pXmInfo->mtxMin.x > vtx.x)
				{
					pXmInfo->mtxMin.x = vtx.x;
					RepairFloat(&pXmInfo->mtxMin.x);
					pXmInfo->mtxXYZ[0] = vtx;
				}
				if (pXmInfo->mtxMin.y > vtx.y)
				{
					pXmInfo->mtxMin.y = vtx.y;
					RepairFloat(&pXmInfo->mtxMin.y);
					pXmInfo->mtxXYZ[2] = vtx;
				}
				if (pXmInfo->mtxMin.z > vtx.z)
				{
					pXmInfo->mtxMin.z = vtx.z;
					RepairFloat(&pXmInfo->mtxMin.z);
					pXmInfo->mtxXYZ[4] = vtx;
				}

				/*** 最大値を取得 ***/
				if (pXmInfo->mtxMax.x < vtx.x)
				{
					pXmInfo->mtxMax.x = vtx.x;
					RepairFloat(&pXmInfo->mtxMax.x);
					pXmInfo->mtxXYZ[1] = vtx;
				}
				if (pXmInfo->mtxMax.y < vtx.y)
				{
					pXmInfo->mtxMax.y = vtx.y;
					RepairFloat(&pXmInfo->mtxMax.y);
					pXmInfo->mtxXYZ[3] = vtx;
				}
				if (pXmInfo->mtxMax.z < vtx.z)
				{
					pXmInfo->mtxMax.z = vtx.z;
					RepairFloat(&pXmInfo->mtxMax.z);
					pXmInfo->mtxXYZ[5] = vtx;
				}

				pVtxBuff += dwSizeFVF;		// 頂点フォーマットのサイズ分ポインタを進める
			}

			/*** 頂点バッファをアンロック ***/
			pXmInfo->pMesh->UnlockVertexBuffer();

			pXmInfo->bSafe = true;

			ZeroMemory(&pXmInfo->aXFileName[0], sizeof(char) * MAX_PATH);
			strcpy(&pXmInfo->aXFileName[0], pXFileName);

			/*** 成功 ***/
			return S_OK;
		}
	}

	/*** 上限オーバー ***/
	return E_FAIL;
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitXmodel(void)
{
	for (int nCntType = 0; nCntType < MODE_MAX; nCntType++)
	{
		LPXM_INFO pXmInfo = &g_aXmInfo[nCntType][0];

		for (int nCntObject = 0; nCntObject < MAX_XMODEL; nCntObject++, pXmInfo++)
		{
			/*** メッシュの破棄 ***/
			RELEASE(pXmInfo->pMesh);

			/*** マテリアルの破棄 ***/
			RELEASE(pXmInfo->pBuffMat);

			/*** テクスチャの破棄 ***/
			for (int nCntTex = 0; nCntTex < MAX_XMTEXTURE; nCntTex++)
			{
				RELEASE(pXmInfo->apTexture[nCntTex]);
			}
		}
	}
}

//================================================================================================================
// --- オブジェクト情報の取得 ---
//================================================================================================================
LPXM_INFO GetXMInfo(_In_ int nType, MODE type)
{
	LPXM_INFO pXmInfo = &g_aXmInfo[type][nType];
	if (pXmInfo->bSafe == true)
	{
		return pXmInfo;
	}
	else
	{
		return NULL;
	}
}

D3DXVECTOR3 GetXMVector(int nType, int nXYZ, MODE type)
{
	LPXM_INFO pXmInfo = &g_aXmInfo[type][nType];

	return pXmInfo->mtxXYZ[nXYZ];
}
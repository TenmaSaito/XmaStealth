//================================================================================================================
//
// DirectXのXファイル表示処理 [object.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "object.h"
#include "wall.h"
#include "effect.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define OBJECT_SIZE_X		(100)		// モデルの基本サイズ - X
#define OBJECT_SIZE_Z		(150)		// モデルの基本サイズ - Y
#define OBJECT_SPD			(2.0f)		// モデルの移動スピード
#define OBJECT_ROTSPD		(0.1f)		// モデルの回転スピード
#define OBJECT_WDSPD		(0.1f)		// モデルの拡縮スピード
#define NUM_OBJECT			(5)			// 読み込むモデル数
#define MAX_TEXTURE			(14)		// 読み込むテクスチャの最大数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
ObjectInfo g_aObjectInfo[MAX_OBJECT];		// オブジェクト情報
Object g_aObject[MAX_OBJECT];				// モデルの情報
LPD3DXFONT g_pFontObj = NULL;				// フォントへのポインタ
D3DXVECTOR3 g_aPos[MAX_OBJECT][6];			// 最大最小値の各座標取得用
int g_nCurrentObject;						// 現在のモデル数
int g_nCountCollision;						// モデルにぶつかった回数

//================================================================================================================
// --- Xファイル表示の初期化処理 ---
//================================================================================================================
void InitObject(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** 各変数の初期化 ***/
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		g_aObject[nCntObject].pos = D3DXVECTOR3_NULL;
		g_aObject[nCntObject].move = D3DXVECTOR3_NULL;
		g_aObject[nCntObject].rot = D3DXVECTOR3_NULL;
		g_aObject[nCntObject].fWidth = 0.0f;
		g_aObject[nCntObject].fHeight = 0.0f;
		g_aObject[nCntObject].fDepth = 0.0f;
		memset(g_aObject[nCntObject].aCode, NULL, sizeof(char) * MAX_PATH);
		g_aObject[nCntObject].bUse = false;

		g_aObjectInfo[nCntObject].pMesh = NULL;
		g_aObjectInfo[nCntObject].pBuffMat = NULL;
		for (int nCntTex = 0; nCntTex < MAX_OBJTEXTURE; nCntTex++)
		{
			g_aObjectInfo[nCntObject].apTexture[nCntTex] = NULL;
		}

		g_aObjectInfo[nCntObject].dwNumMat = 0;
		g_aObjectInfo[nCntObject].mtxMin = D3DXVECTOR3(10000.0f, 10000.0f, 10000.0f);
		g_aObjectInfo[nCntObject].mtxMax = D3DXVECTOR3(-10000.0f, -10000.0f, -10000.0f);
		g_aObjectInfo[nCntObject].bUse = false;
		g_aObjectInfo[nCntObject].bSafe = false;
	}

	g_nCurrentObject = 0;
	g_nCountCollision = 0;

	// デバッグ用フォントの生成
	D3DXCreateFont(pDevice,
		23,
		0,
		0,
		0,
		FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		"PixelMplus12",
		&g_pFontObj);
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitObject(void)
{
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		/*** メッシュの破棄 ***/
		RELEASE(g_aObjectInfo[nCntObject].pMesh);
	
		if ((g_aObjectInfo[nCntObject].pMesh) != NULL) 
		{ 
			(g_aObjectInfo[nCntObject].pMesh)->Release();
			(g_aObjectInfo[nCntObject].pMesh) = nullptr;
		}

		/*** マテリアルの破棄 ***/
		RELEASE(g_aObjectInfo[nCntObject].pBuffMat);
	
		if ((g_aObjectInfo[nCntObject].pBuffMat) != NULL)
		{
			(g_aObjectInfo[nCntObject].pBuffMat)->Release();
			(g_aObjectInfo[nCntObject].pBuffMat) = nullptr;
		}

		/*** テクスチャの破棄 ***/
		for (int nCntTex = 0; nCntTex < MAX_OBJTEXTURE; nCntTex++)
		{
			RELEASE(g_aObjectInfo[nCntObject].apTexture[nCntTex]);

			if ((g_aObjectInfo[nCntObject].apTexture[nCntTex]) != NULL)
			{
				(g_aObjectInfo[nCntObject].apTexture[nCntTex])->Release();
				(g_aObjectInfo[nCntObject].apTexture[nCntTex]) = nullptr;
			}
		}
	}
}

//================================================================================================================
// --- Xファイル表示の更新 ---
//================================================================================================================
void UpdateObject(void)
{
	LPOBJECT pObj = &g_aObject[0];
	PrintDebugProc("ObjectNum : [%d]\n", g_nCurrentObject);

	for (int nCntObj = 0; nCntObj < MAX_OBJECT; nCntObj++, pObj++)
	{
		if (pObj->bUse == false)
		{
			continue;
		}
	}
}

//================================================================================================================
// --- Xファイル表示の描画 ---
//================================================================================================================
void DrawObject(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL *pMat;					// マテリアルデータへのポインタ

	for (int nCntWall = 0; nCntWall < MAX_OBJECT; nCntWall++)
	{
		if (g_aObject[nCntWall].bUse == true && g_aObjectInfo[g_aObject[nCntWall].nType].bSafe == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aObject[nCntWall].mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				g_aObject[nCntWall].rot.y,			// Y軸回転
				g_aObject[nCntWall].rot.x,			// X軸回転
				g_aObject[nCntWall].rot.z);			// Z軸回転

			D3DXMatrixMultiply(&g_aObject[nCntWall].mtxWorld, &g_aObject[nCntWall].mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aObject[nCntWall].pos.x,
				g_aObject[nCntWall].pos.y,
				g_aObject[nCntWall].pos.z);

			D3DXMatrixMultiply(&g_aObject[nCntWall].mtxWorld, &g_aObject[nCntWall].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aObject[nCntWall].mtxWorld);

			/*** 現在のマテリアルを保存 ***/
			pDevice->GetMaterial(&matDef);

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)g_aObjectInfo[g_aObject[nCntWall].nType].pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)g_aObjectInfo[g_aObject[nCntWall].nType].dwNumMat; nCntMat++)
			{
				/*** マテリアルの設定 ***/
				pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

				/*** テクスチャの設定 ***/
				pDevice->SetTexture(0, g_aObjectInfo[g_aObject[nCntWall].nType].apTexture[nCntMat]);

				/*** モデル(パーツ)の描画 ***/
				g_aObjectInfo[g_aObject[nCntWall].nType].pMesh->DrawSubset(nCntMat);
			}

			/*** 保存していたマテリアルを戻す！ ***/
			pDevice->SetMaterial(&matDef);
		}
	}
}

//================================================================================================================
// --- モデルの読み込み ---
//================================================================================================================
HRESULT LoadObject(_In_ const char *pXFileName)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATERIAL* pMat = NULL;		// マテリアルへのポインタ
	HRESULT hr = E_FAIL;			// 読み込み成功判定
	int nNumVtx = 0;				// 頂点数
	DWORD dwSizeFVF = 0;			// 頂点フォーマットのサイズ
	BYTE* pVtxBuff = NULL;			// 頂点バッファへのポインタ

	/*** Xファイルの読み込み ***/
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		if (g_aObjectInfo[nCntObject].bUse != true)
		{
			g_aObjectInfo[nCntObject].bUse = true;

			pMat = NULL;

			hr = D3DXLoadMeshFromX(pXFileName,			// 読み込むXファイル名
				D3DXMESH_SYSTEMMEM,
				pDevice,								// デバイスポインタ
				NULL,
				&g_aObjectInfo[nCntObject].pBuffMat,	// マテリアルへのポインタ
				NULL,
				&g_aObjectInfo[nCntObject].dwNumMat,	// マテリアルの数
				&g_aObjectInfo[nCntObject].pMesh);		// メッシュへのポインタ

			if (FAILED(hr))
			{
				MessageBox(GetHandleWindow(), "Xファイルの読み込みに失敗しました。", "Error(3)", MB_ICONERROR);

				return E_FAIL;
			}

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)g_aObjectInfo[nCntObject].pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)g_aObjectInfo[nCntObject].dwNumMat; nCntMat++)
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
						&g_aObjectInfo[nCntObject].apTexture[nCntMat]);

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
			nNumVtx = g_aObjectInfo[nCntObject].pMesh->GetNumVertices();

			/*** 頂点フォーマットのサイズを取得 ***/
			dwSizeFVF = D3DXGetFVFVertexSize(g_aObjectInfo[nCntObject].pMesh->GetFVF());

			/*** 頂点バッファをロック ***/
			g_aObjectInfo[nCntObject].pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

			/*** 頂点の最大、最小値を取得 ***/
			for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++)
			{
				D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;	// 頂点座標の代入

				/*** 最小値を取得 ***/
				if (g_aObjectInfo[nCntObject].mtxMin.x > vtx.x)
				{
					g_aObjectInfo[nCntObject].mtxMin.x = vtx.x;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMin.x);
					g_aPos[nCntObject][0] = vtx;
				}
				if (g_aObjectInfo[nCntObject].mtxMin.y > vtx.y)
				{
					g_aObjectInfo[nCntObject].mtxMin.y = vtx.y;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMin.y);
					g_aPos[nCntObject][2] = vtx;
				}
				if (g_aObjectInfo[nCntObject].mtxMin.z > vtx.z)
				{
					g_aObjectInfo[nCntObject].mtxMin.z = vtx.z;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMin.z);
					g_aPos[nCntObject][4] = vtx;
				}

				/*** 最大値を取得 ***/
				if (g_aObjectInfo[nCntObject].mtxMax.x < vtx.x)
				{
					g_aObjectInfo[nCntObject].mtxMax.x = vtx.x;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMax.x);
					g_aPos[nCntObject][1] = vtx;
				}
				if (g_aObjectInfo[nCntObject].mtxMax.y < vtx.y)
				{
					g_aObjectInfo[nCntObject].mtxMax.y = vtx.y;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMax.y);
					g_aPos[nCntObject][3] = vtx;
				}
				if (g_aObjectInfo[nCntObject].mtxMax.z < vtx.z)
				{
					g_aObjectInfo[nCntObject].mtxMax.z = vtx.z;
					RepairFloat(&g_aObjectInfo[nCntObject].mtxMax.z);
					g_aPos[nCntObject][5] = vtx;
				}

				pVtxBuff += dwSizeFVF;		// 頂点フォーマットのサイズ分ポインタを進める
			}

			/*** 頂点バッファをアンロック ***/
			g_aObjectInfo[nCntObject].pMesh->UnlockVertexBuffer();

			g_aObjectInfo[nCntObject].bSafe = true;

			break;
		}
	}

	/*** 成功 ***/
	return S_OK;
}

//================================================================================================================
// --- モデルの設置 ---
//================================================================================================================
void SetObject(_In_ D3DXVECTOR3 pos, _In_ D3DXVECTOR3 rot, _In_ int nType, _In_ COLLISIONTYPE collisionType, _In_opt_ const char *pCode)
{
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		if (g_aObject[nCntObject].bUse == false)
		{
			/*** モデルの情報を適用 ***/
			g_aObject[nCntObject].pos = pos;
			g_aObject[nCntObject].rot = rot;
			g_aObject[nCntObject].nType = nType;
			g_aObject[nCntObject].collisionType = collisionType;

			g_aObject[nCntObject].bUse = true;
			g_aObject[nCntObject].fWidth = g_aObjectInfo[nType].mtxMax.x - g_aObjectInfo[nType].mtxMin.x;
			g_aObject[nCntObject].fHeight = g_aObjectInfo[nType].mtxMax.y - g_aObjectInfo[nType].mtxMin.y;
			g_aObject[nCntObject].fDepth = g_aObjectInfo[nType].mtxMax.z - g_aObjectInfo[nType].mtxMin.z;

			g_nCurrentObject++;

			break;
		}
	}
}

//================================================================================================================
// --- モデルの取得 ---
//================================================================================================================
bool CollisionObject(_Inout_ D3DXVECTOR3 *pPos, _Inout_ D3DXVECTOR3 *pPosOld, _Inout_ D3DXVECTOR3 *pMove)
{
	D3DXVECTOR3 posVtx[4] = {};		// オブジェクトの四辺の頂点
	D3DXVECTOR3 vecMove = D3DXVECTOR3_NULL;		// 移動ベクトル
	D3DXVECTOR3 vecLine = D3DXVECTOR3_NULL;		// 境界線ベクトル
	D3DXVECTOR3 vecToPos = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosOld = D3DXVECTOR3_NULL;	// 過去位置と境界線のはじめを結んだベクトル 
	D3DXVECTOR3 vecLineNor = D3DXVECTOR3_NULL;	// 正規化した境界線ベクトル
	float fVecPos = 0.0f;
	float fVecPosOld = 0.0f;
	float fPosToMove = 0.0f;					// vecToPosとの外積
	float fLineToMove = 0.0f;					// vecLineとの外積
	float fRate = 0.0f;							// 面積比率
	float fVecPosToNor = 0.0f;					// 逆法線との外積
	bool bLand = false;

	g_nCountCollision = 0;

	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		/*** オブジェクトが使用されているか判定 ***/
		if (g_aObject[nCntObject].bUse == true && g_aObject[nCntObject].collisionType == COLLISIONTYPE_RECT)
		{
			/*** オブジェクトにめり込んでるか判定 ***/
			if (pPos->x >= g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.x && pPos->x <= g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.x
				&& pPos->y <= g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.y && pPos->y >= g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.y
				&& pPos->z >= g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.z && pPos->z <= g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.z)
			{
				if (pPosOld->x <= g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.x)
				{/** 過去の位置が、モデルのXの最小値よりも小さい位置にいた場合 **/
					pPos->x = g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.x;
				}
				else if (pPosOld->x >= g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.x)
				{/** 過去の位置が、モデルのXの最大値よりも大さい位置にいた場合 **/
					pPos->x = g_aObject[nCntObject].pos.x + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.x;
				}

				if (pPosOld->y < g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.y)
				{/** 過去の位置が、モデルのYの最小値よりも小さい位置にいた場合 **/
					pPos->y = g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.y;
				}
				else if (pPosOld->y >= g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.y)
				{/** 過去の位置が、モデルのYの最大値よりも大さい位置にいた場合 **/
					pPos->y = g_aObject[nCntObject].pos.y + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.y;
					bLand = true;
					pMove->y = 0.0f;
				}

				if (pPosOld->z <= g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.z)
				{/** 過去の位置が、モデルのZの最小値よりも小さい位置にいた場合 **/
					pPos->z = g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMin.z;
					g_nCountCollision++;
				}
				else if (pPosOld->z >= g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.z)
				{/** 過去の位置が、モデルのZの最大値よりも大さい位置にいた場合 **/
					pPos->z = g_aObject[nCntObject].pos.z + g_aObjectInfo[g_aObject[nCntObject].nType].mtxMax.z;
					g_nCountCollision++;
				}
			}
		}
	}

	return bLand;
}

//================================================================================================================
// --- モデルの取得 ---
//================================================================================================================
Object *GetObject(void)
{
	return &g_aObject[0];
}

//================================================================================================================
// --- オブジェクト情報の取得 ---
//================================================================================================================
ObjectInfo* GetObjectInfo(_In_ int nType)
{
	LPOBJECTINFO pInfo = &g_aObjectInfo[nType];
	if (pInfo->bSafe == true)
	{
		return &g_aObjectInfo[nType];
	}
	else
	{
		return NULL;
	}
}

D3DXVECTOR3 GetObjectMVector(int nType, int nXYZ)
{
	return g_aPos[nType][nXYZ];
}
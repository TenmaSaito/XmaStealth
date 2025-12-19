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
#include <assert.h>

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
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitObject(void)
{
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		/*** メッシュの破棄 ***/
		if ((g_aObjectInfo[nCntObject].pMesh) != NULL) 
		{ 
			g_aObjectInfo[nCntObject].pMesh->Release();
			g_aObjectInfo[nCntObject].pMesh = NULL;
		}

		/*** マテリアルの破棄 ***/
		if ((g_aObjectInfo[nCntObject].pBuffMat) != NULL)
		{
			g_aObjectInfo[nCntObject].pBuffMat->Release();
			g_aObjectInfo[nCntObject].pBuffMat = NULL;
		}

		/*** テクスチャの破棄 ***/
		for (int nCntTex = 0; nCntTex < MAX_OBJTEXTURE; nCntTex++)
		{
			if ((g_aObjectInfo[nCntObject].apTexture[nCntTex]) != NULL)
			{
				g_aObjectInfo[nCntObject].apTexture[nCntTex]->Release();
				g_aObjectInfo[nCntObject].apTexture[nCntTex] = NULL;
			}
		}
	}
}

//================================================================================================================
// --- Xファイル表示の更新 ---
//================================================================================================================
void UpdateObject(void)
{
	
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
_Check_return_ HRESULT LoadObject(_In_ const char *pXFileName)
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

			CheckPath(pXFileName);

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
#ifdef _DEBUG
				MessageBox(GetHandleWindow(), "Xファイルの読み込みに失敗しました。", "Error(3)", MB_ICONERROR);
#endif
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
#ifdef _DEBUG
						GenerateMessageBox(MB_ICONERROR,
							"Warnings!",
							"対象Xファイルパス : %s",
							pXFileName);
#endif
					}

					/*** テクスチャの読み込み ***/
					hr = D3DXCreateTextureFromFile(pDevice,
						pMat[nCntMat].pTextureFilename,
						&g_aObjectInfo[nCntObject].apTexture[nCntMat]);

					if (FAILED(hr))
					{
#ifdef _DEBUG
						GenerateMessageBox(MB_ICONERROR,
							"Error (4)",
							"Xファイルのテクスチャ読み込みに失敗しました。\n対象パス : %s",
							&pMat[nCntMat].pTextureFilename[0]);
#endif
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
			if (g_aObjectInfo[nType].bSafe == false)
			{
				//assert(0);
				return;
			}

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
bool CollisionObject(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight, bool *pColl)
{
	LPOBJECT pObject = &g_aObject[0];
	D3DXVECTOR3 posVtx[4] = {};		// オブジェクトの四辺の頂点
	D3DXVECTOR3 vecMove = D3DXVECTOR3_NULL;		// 移動ベクトル
	D3DXVECTOR3 vecLine = D3DXVECTOR3_NULL;		// 境界線ベクトル
	D3DXVECTOR3 vecLineA[4] = {};	// 境界線ベクトル
	D3DXVECTOR3 vecToPos = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosA[4] = {};	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosOld = D3DXVECTOR3_NULL;	// 過去位置と境界線のはじめを結んだベクトル 
	D3DXVECTOR3 vecLineNor = D3DXVECTOR3_NULL;	// 正規化した境界線ベクトル
	float fVecPos = 0.0f;
	float fVecPosA[4] = {};
	float fVecPosOld = 0.0f;
	float fPosToMove = 0.0f;					// vecToPosとの外積
	float fLineToMove = 0.0f;					// vecLineとの外積
	float fRate = 0.0f;							// 面積比率
	float fVecPosToNor = 0.0f;					// 逆法線との外積
	bool bLand = false;

	g_nCountCollision = 0;

	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++, pObject++)
	{
		/*** オブジェクトが使用されているか判定 ***/
		if (g_aObject[nCntObject].bUse == true && GetMode() == MODE_GAME)
		{
			switch (g_aObject[nCntObject].collisionType)
			{
			case COLLISIONTYPE_RECT:

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

					if (pColl != NULL)
					{
						*pColl = true;
					}
				}

				break;

			case COLLISIONTYPE_CROSS:

				POBJECTINFO pObjInfo = &g_aObjectInfo[pObject->nType];

				if (pPos->y + fHeight >= pObject->pos.y + pObjInfo->mtxMin.y
					&& pPos->y <= pObject->pos.y + pObjInfo->mtxMax.y)
				{
					Vec3(Length);
					float fLength = 0.0f;
					float fAngle = 0.0f;
					bool bCollision[4] = {};

					posVtx[0].x = pObject->pos.x + pObjInfo->mtxMin.x;
					posVtx[0].y = pObject->pos.y;
					posVtx[0].z = pObject->pos.z + pObjInfo->mtxMin.z;

					Length = posVtx[0] - pObject->pos;

					fLength = D3DXVec3Length(&Length);
					fAngle = GetPosToPos(posVtx[0], pObject->pos);

					posVtx[0].x = pObject->pos.x + (sinf(fAngle + pObject->rot.y) * fLength);
					posVtx[0].y = pObject->pos.y + pObjInfo->mtxMax.y;
					posVtx[0].z = pObject->pos.z + (cosf(fAngle + pObject->rot.y) * fLength);

					posVtx[1].x = pObject->pos.x + pObjInfo->mtxMax.x;
					posVtx[1].y = pObject->pos.y;
					posVtx[1].z = pObject->pos.z + pObjInfo->mtxMin.z;

					Length = posVtx[1] - pObject->pos;

					fLength = D3DXVec3Length(&Length);
					fAngle = GetPosToPos(posVtx[1], pObject->pos);

					posVtx[1].x = pObject->pos.x + (sinf(fAngle + pObject->rot.y) * fLength);
					posVtx[1].y = pObject->pos.y;
					posVtx[1].z = pObject->pos.z + (cosf(fAngle + pObject->rot.y) * fLength);

					posVtx[2].x = pObject->pos.x + pObjInfo->mtxMax.x;
					posVtx[2].y = pObject->pos.y;
					posVtx[2].z = pObject->pos.z + pObjInfo->mtxMax.z;

					Length = posVtx[2] - pObject->pos;

					fLength = D3DXVec3Length(&Length);
					fAngle = GetPosToPos(posVtx[2], pObject->pos);

					posVtx[2].x = pObject->pos.x + (sinf(fAngle + pObject->rot.y) * fLength);
					posVtx[2].y = pObject->pos.y;
					posVtx[2].z = pObject->pos.z + (cosf(fAngle + pObject->rot.y) * fLength);

					posVtx[3].x = pObject->pos.x + pObjInfo->mtxMin.x;
					posVtx[3].y = pPos->y;
					posVtx[3].z = pObject->pos.z + pObjInfo->mtxMax.z;

					Length = posVtx[3] - pObject->pos;

					fLength = D3DXVec3Length(&Length);
					fAngle = GetPosToPos(posVtx[3], pObject->pos);

					posVtx[3].x = pObject->pos.x + (sinf(fAngle + pObject->rot.y) * fLength);
					posVtx[3].y = pObject->pos.y;
					posVtx[3].z = pObject->pos.z + (cosf(fAngle + pObject->rot.y) * fLength);

					/*** 移動ベクトルの取得 ***/
					vecMove = *pPos - *pPosOld;

					for (int nCntCollision = 0; nCntCollision < 4; nCntCollision++)
					{
						D3DXVECTOR3 vecNor = D3DXVECTOR3_NULL;		// 壁の逆法線ベクトル
						D3DXVECTOR3 vecCutLine = D3DXVECTOR3_NULL;	// 交点からの境界線ベクトル

						vecLine = posVtx[(nCntCollision + 1) % 4] - posVtx[nCntCollision];
						
						RepairFloat(&vecLine.x);
						RepairFloat(&vecLine.z);

						/*** 現在位置との関係を外積を使い求める ***/
						vecToPos = *pPos - posVtx[nCntCollision];

						fVecPos = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);

						RepairFloat(&fVecPos);

						/*** 過去位置との関係を外積を使い求める ***/
						vecToPosOld = *pPosOld - posVtx[nCntCollision];

						fVecPosOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);

						RepairFloat(&fVecPosOld);

						/*** 強制位置の判定 ***/

						/** 現在位置との外積 **/
						fPosToMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);

						/** 最大値との外積 **/
						fLineToMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);

						/** 面積比率の計算 **/
						fRate = fPosToMove / fLineToMove;

						vecNor.x = vecLine.z;
						vecNor.z = -vecLine.x;

						D3DXVec3Normalize(&vecNor, &vecNor);

						fVecPosToNor = (vecNor.z * vecMove.x) - (vecNor.x * vecMove.z);

						D3DXVECTOR3 vecF;
						vecCutLine = vecLine;

						if (fVecPosToNor == 0)
						{
							fVecPosToNor = fabsf(fVecPosToNor);
							D3DXVec3Normalize(&vecCutLine, &vecCutLine);

							vecF = vecCutLine * fVecPosToNor;
						}
						else if (fVecPosToNor > 0)
						{
							fVecPosToNor = fabsf(fVecPosToNor);
							D3DXVec3Normalize(&vecCutLine, &vecCutLine);

							vecF = -vecCutLine * fVecPosToNor;
						}
						else if (fVecPosToNor < 0)
						{
							fVecPosToNor = fabsf(fVecPosToNor);
							D3DXVec3Normalize(&vecCutLine, &vecCutLine);

							vecF = vecCutLine * fVecPosToNor;
						}

						/*** プレイヤーの壁のめり込み判定 ***/
						if (fVecPos < 0 && fVecPosOld >= 0)
						{
							/*** もしも比率が範囲内であれば,衝突 ***/
							if (fRate >= 0.0f && fRate <= 1.0f)
							{
								pPos->x = posVtx[nCntCollision].x + (vecLine.x * fRate) + vecF.x;
								pPos->z = posVtx[nCntCollision].z + (vecLine.z * fRate) + vecF.z;

								//pMove->x = pMove->x * 0.95f;
								//pMove->z = pMove->z * 0.95f;
							}
						}

						if (fVecPos < 0 && fVecPosOld < 0)
						{
							bCollision[nCntCollision] = true;
						}
					}

					if (bCollision[0] == true
						&& bCollision[1] == true
						&& bCollision[2] == true
						&& bCollision[3] == true)
					{
						pPos->y = pObject->pos.y + pObjInfo->mtxMax.y;
						bLand = true;
						pMove->y = 0.0f;

						if (pColl != NULL)
						{
							*pColl = true;
						}
					}
				}

				break;
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
	if (nType == -1)
	{
		//assert(0);
		return NULL;
	}

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
	if (nType == -1)
	{
		//assert(0);
		return VECNULL;
	}

	if (nXYZ < 0 || nXYZ >= 6)
	{
		//assert(0);
		return VECNULL;
	}

	return g_aPos[nType][nXYZ];
}
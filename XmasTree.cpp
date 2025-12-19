//================================================================================================================
//
// DirectXのプレイヤー表示処理 [player.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "XmasTree.h"
#include "ornament.h"
#include "shadow.h"
#include "object.h"
#include "timer.h"
#include "mesh.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_TEXTURE			(14)		// 読み込むテクスチャの最大数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
XmasTree g_xmasTree;		// クリスマスツリー
static float g_fRadiusCollisionTree = 15.0f;		// 当たり判定球の半径
int g_nCntCol;

//================================================================================================================
// --- Xファイル表示の初期化処理 ---
//================================================================================================================
void InitXmasTree(void)
{
	/*** 各変数の初期化 ***/
	ZeroMemory(&g_xmasTree, sizeof(XmasTree));

	g_nCntCol = 0;
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitXmasTree(void)
{

}

//================================================================================================================
// --- Xファイル表示の更新 ---
//================================================================================================================
void UpdateXmasTree(void)
{
	PXMASTREE pXmas = &g_xmasTree;
	LPMESHSPHERE pSphere = NULL;

	if ((pXmas->bUse & true) & (pXmas->bDisp & true))
	{
		for (int nCntCollision = 0; nCntCollision < g_nCntCol; nCntCollision++)
		{
			pSphere = GetMeshSphere(pXmas->aIdxCollision[nCntCollision]);
			CollisionTree(pSphere, &pSphere->mtxWorld, pSphere->pos);
		}

		/*** 影の移動 ***/
		SetPositionShadow(pXmas->nIdShadow, pXmas->pos, pXmas->rot);

		if (GetKeyboardPress(DIK_B))
		{
			pXmas->rot.x += 0.01f;
		}

		pXmas->rot.x += (pXmas->rotDest.x - pXmas->rot.x) * 0.0045f;
		pXmas->rot.z += (pXmas->rotDest.z - pXmas->rot.z) * 0.0045f;

		pXmas->rot = RepairedRot(pXmas->rot);
		if (pXmas->rotDest.x <= -D3DX_HALFPI)
		{
			pXmas->rotDest.x = -D3DX_HALFPI;

		}
		else if (pXmas->rotDest.x >= D3DX_HALFPI)
		{
			pXmas->rotDest.x = D3DX_HALFPI;
		}

		if (pXmas->rotDest.z <= -D3DX_HALFPI)
		{
			pXmas->rotDest.z = -D3DX_HALFPI;

		}
		else if (pXmas->rotDest.z >= D3DX_HALFPI)
		{
			pXmas->rotDest.z = D3DX_HALFPI;
		}

		PrintDebugProc("countOnrament %d\n", pXmas->nCntXmasTrees);

		Vec3(head);
		Vec3(headOut);
		head = D3DXVECTOR3(0.0f, pXmas->fHead, 0.0f);
		D3DXVec3TransformCoord(&headOut, &head, &pXmas->mtxWorld);

		if (headOut.y <= 10.0f && GetMode() == MODE_GAME)
		{
			SetGameState(GAMESTATE_BADEND, 120);
		}

		if (GetTimer() <= 0)
		{
			if (pXmas->nCntXmasTrees >= 30)
			{
				SetGameState(GAMESTATE_GOODEND, 120);
			}
			else
			{
				SetGameState(GAMESTATE_BADEND, 120);
			}
		}
	}
}

//================================================================================================================
// --- Xファイル表示の描画 ---
//================================================================================================================
void DrawXmasTree(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL* pMat;					// マテリアルデータへのポインタ
	D3DMATERIAL9 mat;					// マテリアルデータのコピー
	LPOBJECTINFO pObj = NULL;			// オブジェクト情報へのポインタ
	PXMASTREE pXmas = &g_xmasTree;

	if (pXmas->bUse && pXmas->bDisp)
	{
		/*** オブジェクト情報へのポインタ取得 ***/
		pObj = GetObjectInfo(pXmas->nType);

		/*** ワールドマトリックスの初期化 ***/
		D3DXMatrixIdentity(&pXmas->mtxWorld);

		/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
		D3DXMatrixRotationYawPitchRoll(&mtxRot,
			pXmas->rot.y,			// Y軸回転
			pXmas->rot.x,			// X軸回転
			pXmas->rot.z);			// Z軸回転

		D3DXMatrixMultiply(&pXmas->mtxWorld, &pXmas->mtxWorld, &mtxRot);

		/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
		D3DXMatrixTranslation(&mtxTrans,
			pXmas->pos.x,
			pXmas->pos.y,
			pXmas->pos.z);

		D3DXMatrixMultiply(&pXmas->mtxWorld, &pXmas->mtxWorld, &mtxTrans);

		/*** ワールドマトリックスの設定 ***/
		pDevice->SetTransform(D3DTS_WORLD, &pXmas->mtxWorld);

		/*** 現在のマテリアルを保存 ***/
		pDevice->GetMaterial(&matDef);

		/*** マテリアルデータへのポインタを取得 ***/
		pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();

		for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
		{
			/*** マテリアルの設定 ***/
			pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

			/*** マテリアルの設定 ***/
			pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, pObj->apTexture[nCntMat]);

			/*** モデル(パーツ)の描画 ***/
			pObj->pMesh->DrawSubset(nCntMat);
		}

		/*** 保存していたマテリアルを戻す！ ***/
		pDevice->SetMaterial(&matDef);
	}
}

//================================================================================================================
// --- 敵の設置 ---
//================================================================================================================
void SetXmasTree(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType)
{
	/*** オブジェクト情報へのポインタ取得 ***/
	LPOBJECTINFO pObj = GetObjectInfo(nType);
	PXMASTREE pXmas = &g_xmasTree;

	if (pObj == NULL)
	{
		return;
	}

	/*** Xファイルの読み込み ***/
	if (pXmas->bUse != true)
	{
		/*** 影の生成 ***/
		pXmas->nIdShadow = SetShadow(50.0f, 50.0f);

		/*** 敵の情報の適用 ***/
		pXmas->pos = pos;
		pXmas->rot = rot;
		pXmas->nType = nType;
		pXmas->fRadius = (pObj->mtxMax.x - pObj->mtxMin.x) * 0.5f;
		ZeroMemory(&pXmas->aIdxCollision[0], sizeof(int) * MAX_COLLISION);
		pXmas->nCntXmasTrees = 0;
		pXmas->fHead = pObj->mtxMax.y;
		pXmas->bDisp = true;
		pXmas->bUse = true;

		pXmas->aIdxCollision[0] = SetParentSphere(VEC_Y(pObj->mtxMax.y),
			VECNULL,
			D3DXCOLOR_NULL,
			6, 
			6, 
			5.0f, 
			true, 
			&pXmas->mtxWorld);

		g_nCntCol++;

		D3DXVECTOR3 posBottom = GetObjectMVector(nType, 0);

		// 円錐の体積
		float fVolume = (1.0f / 3.0f) * (powf(pXmas->fRadius, 2.0f) * D3DX_PI) * (pObj->mtxMax.y - posBottom.y);
		float fHeight = (pObj->mtxMax.y - posBottom.y);	// 高さ
		float fPosY = pObj->mtxMax.y - 10.0f;
		float fChange = 20.0f;

		while (fPosY >= posBottom.y)
		{
			float fRadius = (pXmas->fRadius * ((pObj->mtxMax.y - fPosY) / fHeight)) * 1.05f;

			pXmas->aIdxCollision[g_nCntCol] = SetParentSphere(VEC_Y(fPosY),
				VECNULL,
				D3DXCOLOR_NULL,
				6,
				6,
				fRadius,
				true,
				&pXmas->mtxWorld);

			fPosY -= fChange;

			g_nCntCol++;
		}
	}
}

//================================================================================================================
// --- オーナメントの設置 ---
//================================================================================================================
void PutOrnament(float fAngle, float fWeight)
{
	PXMASTREE pXmas = &g_xmasTree;

	pXmas->rotDest.x += cosf(fAngle) * (fWeight * 0.01f);
	pXmas->rotDest.z -= sinf(fAngle) * (fWeight * 0.01f);

	pXmas->rotDest =  RepairedRot(pXmas->rotDest);

	pXmas->nCntXmasTrees++;
}

//================================================================================================================
// --- モデルの取得 ---
//================================================================================================================
PXMASTREE GetXmasTree(void)
{
	return &g_xmasTree;
}
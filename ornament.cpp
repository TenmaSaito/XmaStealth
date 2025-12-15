//================================================================================================================
//
// DirectXの投擲物表示 [ornament.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "ornament.h"
#include "player.h"
#include "shadow.h"
#include "object.h"
#include "effect.h"
#include "wall.h"
#include "XmasTree.h"
#include "mesh.h"
#include "field.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define ORNAMENT_SIZE_X		(10)		// プレイヤーの基本サイズ - X
#define ORNAMENT_SIZE_Z		(10)		// プレイヤーの基本サイズ - Y
#define ORNAMENT_SPD		(2.0f)		// プレイヤーの移動スピード
#define ORNAMENT_ROTSPD		(0.1f)		// プレイヤーの回転スピード
#define ORNAMENT_WDSPD		(0.1f)		// プレイヤーの拡縮スピード
#define MAX_TEXTURE			(14)		// 読み込むテクスチャの最大数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
Ornament g_aOrnament[MAX_ORNAMENT];				// モデルの情報
ORNAMENT_INFO g_aOrnamentInfo[MAX_ORNAMENT];	// 情報
int g_nCounterOrnament;							// 合計数
InitedVec3(g_pos);

//================================================================================================================
// --- 投擲物の初期化処理 ---
//================================================================================================================
void InitOrnament(void)
{
	/*** 各変数の初期化 ***/
	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		g_aOrnament[nCntOrnament].pos = D3DXVECTOR3_NULL;
		g_aOrnament[nCntOrnament].posOld = D3DXVECTOR3_NULL;
		g_aOrnament[nCntOrnament].pos.y = 15.0f;
		g_aOrnament[nCntOrnament].move = D3DXVECTOR3_NULL;
		g_aOrnament[nCntOrnament].rot = D3DXVECTOR3_NULL;
		g_aOrnament[nCntOrnament].pMtxParent = NULL;
		g_aOrnament[nCntOrnament].nIdShadow = -1;
		g_aOrnament[nCntOrnament].nType = 0;
		g_aOrnament[nCntOrnament].bDisp = false;
		g_aOrnament[nCntOrnament].bUse = false;
		g_aOrnament[nCntOrnament].bCatched = false;
		g_aOrnament[nCntOrnament].bReady = true;
	}

	ZeroMemory(&g_aOrnamentInfo[0], sizeof(ORNAMENT_INFO) * MAX_ORNAMENT);

	g_nCounterOrnament = 0;
}

//================================================================================================================
// --- 投擲物の終了 ---
//================================================================================================================
void UninitOrnament(void)
{

}

//================================================================================================================
// --- 投擲物の更新 ---
//================================================================================================================
void UpdateOrnament(void)
{
	Player *pPlayer = GetPlayer();
	PORNAMENT pOrnament = &g_aOrnament[0];
	int nCounterPutted = 0;			// 木に付けられたオーナメント数

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse == true)
		{
			pOrnament->posOld = pOrnament->pos;

			if (pOrnament->pMtxParent == NULL)
			{
				/*** オーナメントの移動量の適用 ***/
				pOrnament->pos.x += pOrnament->move.x;
				pOrnament->pos.y += pOrnament->move.y;
				pOrnament->pos.z += pOrnament->move.z;

				/*** オーナメントの反射 ***/
				ReflectWall(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move);

				if (CollisionMeshField(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move) == true)
				{
					pOrnament->move = VECNULL;
					pOrnament->bReady = true;
				}

				if (CollisionFloor(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move, g_aOrnamentInfo[pOrnament->nType].fRadius))
				{
					pOrnament->move = VECNULL;
					pOrnament->bReady = true;
				}

				/*** オーナメントの重力適用 ***/
				pOrnament->move.y += WORLD_GRAVITY * 0.05f;

				if (pOrnament->pos.y < 0.0f)
				{ // もし地面に着いた場合、静止してつかめるように
					pOrnament->pos.y = 0.0f;
					pOrnament->move = VECNULL;
					pOrnament->bReady = true;
				}

				SetPositionShadow(pOrnament->nIdShadow, pOrnament->pos, pOrnament->rot);
			}
			else
			{
				Vec3(posShadow);
				D3DXVec3TransformCoord(&posShadow, &pOrnament->pos, &pOrnament->mtxWorld);

				SetPositionShadow(pOrnament->nIdShadow, posShadow, pPlayer->rot);
			}

			if (pOrnament->bCatchedTree == true)
			{
				nCounterPutted++;
			}
		}
	}

	if (nCounterPutted >= g_nCounterOrnament)
	{ // もし存在するオーナメントが全て設置されたら、ゲームクリア
		SetGameState(GAMESTATE_GOODEND, 120);
	}
}

//================================================================================================================
// --- 投擲物の描画 ---
//================================================================================================================
void DrawOrnament(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL *pMat;					// マテリアルデータへのポインタ
	D3DMATERIAL9 mat;					// マテリアルデータのコピー
	LPOBJECTINFO pObj = NULL;			// オブジェクト情報へのポインタ
	PORNAMENT pOrnament = &g_aOrnament[0];

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse && pOrnament->bDisp && g_aOrnamentInfo[pOrnament->nType].bSafe == true)
		{
			/*** オブジェクト情報へのポインタ取得 ***/
			pObj = GetObjectInfo(g_aOrnamentInfo[pOrnament->nType].nType);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pOrnament->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pOrnament->rot.y,			// Y軸回転
				pOrnament->rot.x,			// X軸回転
				pOrnament->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pOrnament->pos.x,
				pOrnament->pos.y,
				pOrnament->pos.z);

			D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, &mtxTrans);

			if (pOrnament->pMtxParent != nullptr)
			{
				D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, pOrnament->pMtxParent);
			}

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pOrnament->mtxWorld);

			/*** 現在のマテリアルを保存 ***/
			pDevice->GetMaterial(&matDef);

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
			{
				if (FALSE)
				{ // マテリアルカラー変更
					mat = pMat[nCntMat].MatD3D;

					mat.Diffuse = GetRandomColor(false);

					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&mat);
				}
				else
				{
					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
				}

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
}

//================================================================================================================
// --- 投擲物情報の登録 ---
//================================================================================================================
void SettingOrnamentInfo(int nType, float fRadius, float fWeight)
{
	PORNAMENT_INFO pInfo = &g_aOrnamentInfo[0];

	for (int nCntInfo = 0; nCntInfo < MAX_ORNAMENT; nCntInfo++, pInfo++)
	{
		if (pInfo->bUse == false)
		{
			if (GetObjectInfo(nType) != NULL)
			{
				pInfo->nType = nType;
				pInfo->fRadius = fRadius;
				pInfo->fWeight = fWeight;
				pInfo->bSafe = true;
			}
			else
			{
				pInfo->bSafe = false;
			}

			pInfo->bUse = true;

			break;
		}
	}
}

//================================================================================================================
// --- 投擲物の設置 ---
//================================================================================================================
void SetOrnament(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType, float fSpd, float fRadius)
{
	/*** オブジェクト情報へのポインタ取得 ***/
	LPOBJECTINFO pObj = GetObjectInfo(g_aOrnamentInfo[nType].nType);
	PORNAMENT pOrnament = &g_aOrnament[0];

	if (pObj == NULL)
	{
		return;
	}

	/*** Xファイルの読み込み ***/
	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse != true)
		{
			/*** 影の生成 ***/
			pOrnament->nIdShadow = SetShadow((pObj->mtxMax.x - pObj->mtxMin.x), (pObj->mtxMax.z - pObj->mtxMin.z));

			/*** 敵の情報の適用 ***/
			pOrnament->pos = pos;
			pOrnament->rot = rot;
			pOrnament->fSpd = fSpd;
			pOrnament->nType = nType;
			pOrnament->pMtxParent = NULL;
			pOrnament->bDisp = true;
			pOrnament->bUse = true;
			pOrnament->bCatched = false;
			pOrnament->bCatchedTree = false;
			pOrnament->bReady = true;

			// 当たり判定生成
			pOrnament->nIdxCollision = SetParentSphere(VECNULL, VECNULL, D3DXCOLOR(1.0f, 0.0f, 0.0f, 0.3f), 12, 12, g_aOrnamentInfo[nType].fRadius, true, &pOrnament->mtxWorld);

			break;
		}
	}

	g_nCounterOrnament++;
}

//================================================================================================================
// --- 投擲物との衝突処理(最大数は1) ---
//================================================================================================================
int CollisionOrnament(LPMESHSPHERE pCollisionSphere)
{
	PORNAMENT pOrnament = &g_aOrnament[0];

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse == true)
		{
			if (CollisionSphereToSphere(pCollisionSphere, GetMeshSphere(pOrnament->nIdxCollision)))
			{
				return nCntOrnament;
			}
		}
	}

	return -1;
}

//================================================================================================================
// --- 投擲物との衝突処理(点と球の判定版) ---
//================================================================================================================
int CollisionOrnament(D3DXVECTOR3 pos)
{
	PORNAMENT pOrnament = &g_aOrnament[0];

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse == true)
		{
			if (CollisionLPSphere(GetMeshSphere(pOrnament->nIdxCollision), pos, 3.0f))
			{
				return nCntOrnament;
			}
		}
	}

	return -1;
}

//================================================================================================================
// --- クリスマスツリー限定の衝突処理(ぶつかったら止まる) ---
//================================================================================================================
void CollisionTree(LPMESHSPHERE pCollisionSphere, LPD3DXMATRIX pMtxXmasTree, D3DXVECTOR3 posTree)
{
	PORNAMENT pOrnament = &g_aOrnament[0];
	PXMASTREE pXmas = GetXmasTree();
	int nCollision = 0;

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse == true && pOrnament->bCatchedTree == false)
		{
			if (CollisionSphereToSphere(pCollisionSphere, GetMeshSphere(pOrnament->nIdxCollision)))
			{
				Vec3(posRepairOrnament);
				Vec3(posRepairTreeSphere);

				D3DXVec3TransformCoord(&posRepairOrnament, &pOrnament->pos, &pOrnament->mtxWorld);
				D3DXVec3TransformCoord(&posRepairTreeSphere, &pCollisionSphere->pos, &pCollisionSphere->mtxWorld);
				float fAngle = GetPosToPos(posRepairTreeSphere, pOrnament->pos);
				RepairRot(&fAngle, &fAngle);
				fAngle = InverseRot(fAngle);

				Vec3(posOrnament);

				if ((pCollisionSphere == GetMeshSphere(pXmas->aIdxCollision[0]))
					&& pXmas->bTop == false)
				{
					posOrnament.x = 0.0f;
					posOrnament.y = pCollisionSphere->fRadius * 0.85f;
					posOrnament.z = 0.0f;

					fAngle = 0.0f;
					pXmas->bTop = true;
				}
				else
				{
					posOrnament.x = sinf(fAngle) * pCollisionSphere->fRadius;

					posOrnament.y = pOrnament->pos.y - pCollisionSphere->pos.y;

					posOrnament.z = cosf(fAngle) * pCollisionSphere->fRadius;

					PutOrnament(fAngle, g_aOrnamentInfo[pOrnament->nType].fWeight);
				}

				pOrnament->pos = posOrnament;
				pOrnament->move = VECNULL;
				pOrnament->bReady = false;
				pOrnament->bCatchedTree = true;
				pOrnament->pMtxParent = pMtxXmasTree;
			}
		}
	}
}

//================================================================================================================
// --- 投擲物の親設定処理 ---
//================================================================================================================
void SetParentOrnament(int nIdxOrnament, LPD3DXMATRIX pMtxParent)
{
	if (nIdxOrnament < 0 || nIdxOrnament >= g_nCounterOrnament) return;
	if (g_aOrnament[nIdxOrnament].bReady == false) return;

	g_aOrnament[nIdxOrnament].pos = VECNULL;
	g_aOrnament[nIdxOrnament].rot = VECNULL;
	g_aOrnament[nIdxOrnament].move = VECNULL;
	g_aOrnament[nIdxOrnament].bCatched = true;
	g_aOrnament[nIdxOrnament].bReady = false;
	g_aOrnament[nIdxOrnament].pMtxParent = pMtxParent;
}

//================================================================================================================
// --- 投擲物の親解除処理 ---
//================================================================================================================
void RemoveParentOrnament(int nIdxOrnament)
{
	if (nIdxOrnament < 0 || nIdxOrnament >= g_nCounterOrnament) return;

	g_aOrnament[nIdxOrnament].bCatched = false;
	g_aOrnament[nIdxOrnament].bReady = false;
	g_aOrnament[nIdxOrnament].pMtxParent = NULL;
}

//================================================================================================================
// --- 投擲物の投擲処理！ ---
//================================================================================================================
void ShotOrnament(int nIdxOrnament, D3DXVECTOR3 vec, float fSpeed)
{
	if (nIdxOrnament < 0 || nIdxOrnament >= g_nCounterOrnament) return;

	PORNAMENT pOrnament = &g_aOrnament[nIdxOrnament];
	Player *pPlayer = GetPlayer();
	if (pOrnament->bCatched != true) return;
	pOrnament->move.x = sinf(vec.y + D3DX_PI) * fSpeed;
	pOrnament->move.z = cosf(vec.y + D3DX_PI) * fSpeed;

	D3DXVec3TransformCoord(&pOrnament->pos, &pOrnament->pos, &pOrnament->mtxWorld);
	pOrnament->rot = pPlayer->rot;

	RemoveParentOrnament(nIdxOrnament);
}

//================================================================================================================
// --- 投擲物の取得処理 ---
//================================================================================================================
PORNAMENT GetOrnament(int nIdxOrnament)
{
	if (nIdxOrnament < 0 || nIdxOrnament >= g_nCounterOrnament) return NULL;

	return &g_aOrnament[nIdxOrnament];
}
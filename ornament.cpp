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
#include "line.h"
#include "billboard.h"

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
int g_nCounterOrnamentInfo;

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
		g_aOrnament[nCntOrnament].nCounter = 0;
		g_aOrnament[nCntOrnament].bDisp = false;
		g_aOrnament[nCntOrnament].bUse = false;
		g_aOrnament[nCntOrnament].bCatched = false;
		g_aOrnament[nCntOrnament].bReady = true;
		g_aOrnament[nCntOrnament].bLand = false;
		g_aOrnamentInfo[nCntOrnament].nType = -1;
		g_aOrnamentInfo[nCntOrnament].nOrnamentType = -1;
		g_aOrnamentInfo[nCntOrnament].bUse = false;
		g_aOrnamentInfo[nCntOrnament].bSafe = false;
		g_aOrnamentInfo[nCntOrnament].fRadius = 0.0f;
		g_aOrnamentInfo[nCntOrnament].fWeight = 0.0f;
	}

	g_nCounterOrnament = 0;
	g_nCounterOrnamentInfo = 0;
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
	PORNAMENT pOrnament = &g_aOrnament[0];
	int nCounterPutted = 0;			// 木に付けられたオーナメント数

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse == true && 
			(GetMode() == MODE_GAME
			|| GetMode() == MODE_TUTORIAL))
		{
			pOrnament->posOld = pOrnament->pos;

			if (pOrnament->pMtxParent == NULL)
			{
				/*** オーナメントの移動量の適用 ***/
				pOrnament->pos.x += pOrnament->move.x;
				pOrnament->pos.y += pOrnament->move.y;
				pOrnament->pos.z += pOrnament->move.z;

				if (CollisionObject(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move, g_aOrnamentInfo[pOrnament->nType].fRadius))
				{
					pOrnament->move = VECNULL;
					pOrnament->bReady = true;
				}

				/*** オーナメントの反射 ***/
				if (GetMode() == MODE_GAME)
				{
					ReflectWall(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move);

					/*if (CollisionMeshField(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move) == true)
					{
						pOrnament->move = VECNULL;
						pOrnament->bReady = true;
					}*/

					if (CollisionFloor(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move, g_aOrnamentInfo[pOrnament->nType].fRadius))
					{
						pOrnament->pos.y += g_aOrnamentInfo[pOrnament->nType].fRadius;
						pOrnament->move = VECNULL;
						pOrnament->bReady = true;
						pOrnament->bLand = true;
					}
				}

				/*** オーナメントの重力適用 ***/
				pOrnament->move.y += WORLD_GRAVITY * 0.05f;

				if (pOrnament->pos.y < g_aOrnamentInfo[pOrnament->nType].fRadius)
				{ // もし地面に着いた場合、静止してつかめるように
					pOrnament->pos.y = g_aOrnamentInfo[pOrnament->nType].fRadius;
					pOrnament->move = VECNULL;
					pOrnament->bReady = true;
					pOrnament->bLand = true;
				}

				SetPositionShadow(pOrnament->nIdShadow, pOrnament->pos, pOrnament->rot);

				Vec3(pos = pOrnament->pos);
				SetEffect(pos, GetRandomVector3(100, 100, 100), 1.5f, 1.0f, 1.0f);

				if (pOrnament->bLand == true)
				{
					if (pOrnament->nIdxBill == -1)
					{
						Vec3(posBill = pos);
						posBill.y += 50.0f;
						pOrnament->nIdxBill = SetBillBoard(posBill, 15.0f, 40.0f, 3);
					}

					pOrnament->move.y = 0.5f;
					pOrnament->bLand = false;

					pos.y = pOrnament->pos.y + 0.5f;
					SetMeshRing(pos, VECNULL, 0.0f, 1.0f, 8, 0.75f, 1.25f, 75);
				}

				if (GetMode() == MODE_TUTORIAL)
				{
					D3DXVECTOR4 rect = D3DXVECTOR4(-500.0f, 500.0f, -200.0f, 700.0f);
					if (!CollisionBoxZ(rect, pOrnament->pos))
					{
						if (pOrnament->pos.x <= -500.0f)
						{
							pOrnament->pos.x = -500.0f;
						}
						else if (pOrnament->pos.x >= 500.0f)
						{
							pOrnament->pos.x = 500.0f;
						}

						if (pOrnament->pos.z <= -200.0f)
						{
							pOrnament->pos.z = -200.0f;
						}
						else if (pOrnament->pos.z >= 700.0f)
						{
							pOrnament->pos.z = 700.0f;
						}
					}
				}
			}
			//else
			//{
			//	Vec3(posShadow);
			//	PrintDebugProc("%~3f\n", pOrnament->pos.x, pOrnament->pos.y, pOrnament->pos.z);
			//	//D3DXVec3TransformCoord(&posShadow, &pOrnament->pos, &pOrnament->mtxWorld);

			//	InvisibleShadow(pOrnament->nIdShadow, 0.0f);
			//}

			if (pOrnament->bCatchedTree == true)
			{
				nCounterPutted++;
			}
		}
	}

	if (nCounterPutted >= g_nCounterOrnament && GetMode() == MODE_GAME)
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
	HRESULT hr;

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++, pOrnament++)
	{
		if (pOrnament->bUse && pOrnament->bDisp && g_aOrnamentInfo[pOrnament->nType].bSafe == true)
		{
			/*** オブジェクト情報へのポインタ取得 ***/
			pObj = GetObjectInfo(g_aOrnamentInfo[pOrnament->nType].nType);
			if (pObj == NULL)
			{
				continue;
			}

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

			if (pOrnament->pMtxParent != NULL)
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
				/*** マテリアルの設定 ***/
				pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

				/*** テクスチャの設定 ***/
				pDevice->SetTexture(0, pObj->apTexture[nCntMat]);

				/*** モデル(パーツ)の描画 ***/
				hr = pObj->pMesh->DrawSubset(nCntMat);
			}

			/*** 保存していたマテリアルを戻す！ ***/
			pDevice->SetMaterial(&matDef);
		}
	}
}

//================================================================================================================
// --- 投擲物情報の登録 ---
//================================================================================================================
void SettingOrnamentInfo(int nOrnamentType, int nType, float fRadius, float fWeight)
{
	PORNAMENT_INFO pInfo = &g_aOrnamentInfo[0];

	for (int nCntInfo = 0; nCntInfo < MAX_ORNAMENT; nCntInfo++, pInfo++)
	{
		if (pInfo->bUse == false)
		{
			if (GetObjectInfo(nType) != NULL)
			{
				if (nOrnamentType < 0 || nOrnamentType >= 3)
				{
					pInfo->nOrnamentType = 0;
				}
				else
				{
					pInfo->nOrnamentType = nOrnamentType;
				}
				
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
			g_nCounterOrnamentInfo++;

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
			pOrnament->nIdxCollision = SetParentSphere(VECNULL, VECNULL, D3DXCOLOR(1.0f, 0.0f, 0.0f, 0.3f), 12, 12, g_aOrnamentInfo[nType].fRadius, false, &pOrnament->mtxWorld);
			D3DXVECTOR3 pos = pOrnament->pos;
			pos.y += 50.0f;
			pOrnament->nIdxBill = SetBillBoard(pos, 15.0f, 40.0f, 3);
			g_nCounterOrnament++;
			break;
		}
	}
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
		if (pOrnament->bUse == true && pOrnament->bReady == true)
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
		if (pOrnament->bUse == true && pOrnament->bCatchedTree == false && pOrnament->bCatched == false)
		{
			if (CollisionSphereToSphere(pCollisionSphere, GetMeshSphere(pOrnament->nIdxCollision)))
			{
				Vec3(posRepairOrnament = pOrnament->pos);
				Vec3(posRepairTreeSphere);

				posRepairTreeSphere.x = pCollisionSphere->mtxWorld._41;
				posRepairTreeSphere.y = pCollisionSphere->mtxWorld._42;
				posRepairTreeSphere.z = pCollisionSphere->mtxWorld._43;

				float fAngle = GetPosToPos(pOrnament->pos, posRepairTreeSphere);

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
					Vec3(vecMove = pOrnament->move);
					D3DXVec3Normalize(&vecMove, &vecMove);
					vecMove *= -1;

					posOrnament.x = sinf(fAngle) * pCollisionSphere->fRadius;

					posOrnament.y = pOrnament->pos.y - posRepairTreeSphere.y;

					posOrnament.z = cosf(fAngle) * pCollisionSphere->fRadius;

					PutOrnament(fAngle, g_aOrnamentInfo[pOrnament->nType].fWeight);
				}

				pOrnament->pos = posOrnament;
				pOrnament->move = VECNULL;
				pOrnament->bReady = false;
				pOrnament->bCatchedTree = true;
				pOrnament->pMtxParent = &pCollisionSphere->mtxWorld;
				pXmas->nCntXmasTrees++;
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
	InvisibleShadow(g_aOrnament[nIdxOrnament].nIdShadow, 0.0f);
	KillBillboard(g_aOrnament[nIdxOrnament].nIdxBill);
	g_aOrnament[nIdxOrnament].nIdxBill = -1;
}

//================================================================================================================
// --- 投擲物の親解除処理 ---
//================================================================================================================
void RemoveParentOrnament(int nIdxOrnament)
{
	if (nIdxOrnament < 0 || nIdxOrnament >= g_nCounterOrnament) return;

	g_aOrnament[nIdxOrnament].bCatched = false;
	g_aOrnament[nIdxOrnament].pMtxParent = NULL;
	InvisibleShadow(g_aOrnament[nIdxOrnament].nIdShadow, 1.0f);
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
	g_aOrnament[nIdxOrnament].bReady = false;
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

//================================================================================================================
// --- 投擲物のランダム配置処理 ---
//================================================================================================================
void SetOrnamentRand(int nNumOrnament)
{
	int nCntOrnament = 0;
	MODE modeNext = GetModeNext();

	if (nNumOrnament >= MAX_ORNAMENT) nNumOrnament = MAX_ORNAMENT;

	while(nCntOrnament < nNumOrnament)
	{
		if (modeNext == MODE_GAME)
		{
			D3DXVECTOR3 pos = GetRandomVector3(800, 300, 900);
			D3DXVECTOR3 move;

			if (pos.z < 500.0f && pos.z > -400.0f
				&& pos.x > -400.0f && pos.x < 400.0f)
			{
				if (pos.z < 295.0f && pos.x > 100.0f)
				{ // リビングの右端
					continue;
				}

				if (pos.x > 100.0f && pos.z < 300.0f)
				{
					continue;
				}

				CollisionFloor(&pos, &pos, &move, 0.0f);

				CollisionMeshField(&pos, &pos, &move);

				if (CollisionObject(&pos, &pos, &move, 0.0f))
				{
					continue;
				}

				int nType = rand() % g_nCounterOrnamentInfo;
				SetOrnament(pos, VECNULL, nType, 0.0f, g_aOrnamentInfo[nType].fRadius);
				nCntOrnament++;
			}
		}
		else if (modeNext == MODE_TUTORIAL)
		{
			D3DXVECTOR3 pos = GetRandomVector3(1000, 2, 1000);

			D3DXVECTOR4 rect = D3DXVECTOR4(-500.0f, 500.0f, -200.0f, 700.0f);
			if (CollisionBoxZ(rect, pos))
			{
				int nType = rand() % g_nCounterOrnamentInfo;
				SetOrnament(pos, VECNULL, nType, 0.0f, g_aOrnamentInfo[nType].fRadius);

				nCntOrnament++;
			}
		}
	}

	return;
}
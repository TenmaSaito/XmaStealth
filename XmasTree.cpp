//==================================================================================
//
// クリスマスツリーの処理 [XmasTree.cpp]
// Author : TENMA SAITO
//
// ・ツリーの形に沿って球形の当たり判定を縦に並べ、オーナメントとの接触を判定する
// ・付けたオーナメントの重さと角度に応じてツリーが傾く
// ・頭(てっぺん)が一定の高さ以下まで傾く or 時間切れでゲーム終了
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "XmasTree.h"
#include "ornament.h"
#include "shadow.h"
#include "object.h"
#include "timer.h"
#include "mesh.h"
#include "score.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TREE_TILT_FOLLOW		(0.0045f)	// 目標の傾きへ近づく割合(1フレームあたり)
#define TREE_TILT_WEIGHT		(0.01f)		// オーナメントの重さ1あたりの傾き(ラジアン)
#define TREE_TILT_DEBUG			(0.01f)		// デバッグキー(B)で傾ける量
#define TREE_FALL_HEIGHT		(45.0f)		// 頭がこの高さ以下になったら倒れたとみなす
#define TREE_END_FRAME			(120)		// ゲーム終了状態にしてから画面遷移するまでのフレーム
#define TREE_SHADOW_SIZE		(50.0f)		// 影の大きさ

// --- 当たり判定球の配置 ---
#define TREE_TOP_RADIUS			(5.0f)		// てっぺんの当たり判定球の半径
#define TREE_TOP_OFFSET			(10.0f)		// 2つ目の球をてっぺんからどれだけ下げるか
#define TREE_SPHERE_INTERVAL	(20.0f)		// 当たり判定球を並べる間隔(高さ)
#define TREE_RADIUS_SCALE		(1.05f)		// 当たり判定球を見た目より少し大きくする倍率
#define TREE_SPHERE_BLOCK		(6)			// 当たり判定球の分割数(描画しないため粗くてよい)

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static XmasTree g_xmasTree;		// クリスマスツリーの情報
static int g_nCntCol;			// ツリーに作った当たり判定球の数

//==================================================================================
// --- クリスマスツリーの初期化処理 ---
//==================================================================================
void InitXmasTree(void)
{
	ZeroMemory(&g_xmasTree, sizeof(XmasTree));

	g_nCntCol = 0;
}

//==================================================================================
// --- クリスマスツリーの終了処理 ---
//==================================================================================
void UninitXmasTree(void)
{

}

//==================================================================================
// --- クリスマスツリーの更新処理 ---
//==================================================================================
void UpdateXmasTree(void)
{
	PXMASTREE pXmas = &g_xmasTree;		// クリスマスツリー

	if (pXmas->bUse == false || pXmas->bDisp == false)
	{ // 未設置 or 非表示
		return;
	}

	/*** オーナメントとの当たり判定 ***/
	for (int nCntCollision = 0; nCntCollision < g_nCntCol; nCntCollision++)
	{
		LPMESHSPHERE pSphere = GetMeshSphere(pXmas->aIdxCollision[nCntCollision]);		// 当たり判定球

		CollisionTree(pSphere, &pSphere->mtxWorld, pSphere->pos);
	}

	/*** 影の移動 ***/
	SetPositionShadow(pXmas->nIdShadow, pXmas->pos, pXmas->rot);

	if (GetKeyboardPress(DIK_B))
	{ // デバッグ用 : ツリーを傾ける
		pXmas->rot.x += TREE_TILT_DEBUG;
	}

	/*** 目標の傾きへ少しずつ近づける ***/
	pXmas->rot.x += (pXmas->rotDest.x - pXmas->rot.x) * TREE_TILT_FOLLOW;
	pXmas->rot.z += (pXmas->rotDest.z - pXmas->rot.z) * TREE_TILT_FOLLOW;
	pXmas->rot = RepairedRot(pXmas->rot);

	/*** 目標の傾きは真横(±90度)まで ***/
	if (pXmas->rotDest.x <= -D3DX_HALFPI)		pXmas->rotDest.x = -D3DX_HALFPI;
	else if (pXmas->rotDest.x >= D3DX_HALFPI)	pXmas->rotDest.x = D3DX_HALFPI;

	if (pXmas->rotDest.z <= -D3DX_HALFPI)		pXmas->rotDest.z = -D3DX_HALFPI;
	else if (pXmas->rotDest.z >= D3DX_HALFPI)	pXmas->rotDest.z = D3DX_HALFPI;

	/*** 頭(てっぺんの当たり判定球)の高さを取得 ***/
	float fHeadHeight = GetMeshSphere(pXmas->aIdxCollision[0])->mtxWorld._42;		// 頭のワールド高さ

	if (fHeadHeight <= TREE_FALL_HEIGHT && GetMode() == MODE_GAME)
	{ // 頭の位置が一定以下になればゲームオーバー
		SetGameState(GAMESTATE_BADEND, TREE_END_FRAME);
	}

	int nIdxTimer = -1;		// 制限時間のスコア番号
	if (GetTimerIndex(&nIdxTimer) && GetScore(nIdxTimer) <= 0)
	{ // 制限時間終了時 : オーナメントが1つでも付いていればクリア
		if (GetOrnamentNum() <= 0)
		{
			SetGameState(GAMESTATE_BADEND, TREE_END_FRAME);
		}
		else
		{
			SetGameState(GAMESTATE_GOODEND, TREE_END_FRAME);
		}
	}
}

//==================================================================================
// --- クリスマスツリーの描画処理 ---
//==================================================================================
void DrawXmasTree(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	D3DMATERIAL9 matDef;							// 現在のマテリアル保存用
	D3DXMATERIAL *pMat = NULL;						// マテリアルデータへのポインタ
	PXMASTREE pXmas = &g_xmasTree;					// クリスマスツリー

	if (pXmas->bUse == false || pXmas->bDisp == false)
	{ // 未設置 or 非表示
		return;
	}

	LPOBJECTINFO pObj = GetObjectInfo(pXmas->nType);		// 使用するモデル
	if (pObj == NULL)
	{ // モデルが無い
		return;
	}

	/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
	D3DXMatrixIdentity(&pXmas->mtxWorld);

	D3DXMatrixRotationYawPitchRoll(&mtxRot, pXmas->rot.y, pXmas->rot.x, pXmas->rot.z);
	D3DXMatrixMultiply(&pXmas->mtxWorld, &pXmas->mtxWorld, &mtxRot);

	D3DXMatrixTranslation(&mtxTrans, pXmas->pos.x, pXmas->pos.y, pXmas->pos.z);
	D3DXMatrixMultiply(&pXmas->mtxWorld, &pXmas->mtxWorld, &mtxTrans);

	pDevice->SetTransform(D3DTS_WORLD, &pXmas->mtxWorld);

	/*** 現在のマテリアルを保存 ***/
	pDevice->GetMaterial(&matDef);

	/*** マテリアルごとにモデルを描画 ***/
	pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();

	for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
	{
		pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
		pDevice->SetTexture(0, pObj->apTexture[nCntMat]);
		pObj->pMesh->DrawSubset(nCntMat);
	}

	/*** 保存していたマテリアルを戻す ***/
	pDevice->SetMaterial(&matDef);
}

//==================================================================================
// --- クリスマスツリーの設置 (1本のみ) ---
//==================================================================================
void SetXmasTree(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType)
{
	LPOBJECTINFO pObj = GetObjectInfo(nType);		// 使用するモデル
	PXMASTREE pXmas = &g_xmasTree;					// クリスマスツリー

	if (pObj == NULL || pXmas->bUse == true)
	{ // モデルが無い or 設置済み
		return;
	}

	/*** 影の生成 ***/
	pXmas->nIdShadow = SetShadow(TREE_SHADOW_SIZE, TREE_SHADOW_SIZE);

	/*** 情報の設定 ***/
	pXmas->pos = pos;
	pXmas->rot = rot;
	pXmas->nType = nType;
	pXmas->fRadius = (pObj->mtxMax.x - pObj->mtxMin.x) * 0.5f;
	ZeroMemory(&pXmas->aIdxCollision[0], sizeof pXmas->aIdxCollision);
	pXmas->nCntXmasTrees = 0;
	pXmas->fHead = pObj->mtxMax.y;
	pXmas->bDisp = true;
	pXmas->bUse = true;

	/*** てっぺんの当たり判定球 ***/
	pXmas->aIdxCollision[0] = SetParentSphere(VEC_Y(pObj->mtxMax.y),
		VECNULL,
		D3DXCOLOR_NULL,
		TREE_SPHERE_BLOCK,
		TREE_SPHERE_BLOCK,
		TREE_TOP_RADIUS,
		false,
		&pXmas->mtxWorld);

	g_nCntCol++;

	/*** 葉っぱの部分に沿って当たり判定球を並べる (下に行くほど大きく) ***/
	D3DXVECTOR3 posBottom = GetObjectMVector(nType, 0);		// 葉っぱの最下点
	pXmas->fBottom = posBottom.y;

	float fLeafHeight = (pObj->mtxMax.y - posBottom.y);		// 葉っぱ部分の高さ
	float fPosY = pObj->mtxMax.y - TREE_TOP_OFFSET;			// 球を置く高さ

	while (fPosY >= posBottom.y && g_nCntCol < MAX_COLLISION)
	{
		// てっぺんからの距離に比例した半径 (円錐の断面)
		float fRadius = (pXmas->fRadius * ((pObj->mtxMax.y - fPosY) / fLeafHeight)) * TREE_RADIUS_SCALE;

		pXmas->aIdxCollision[g_nCntCol] = SetParentSphere(VEC_Y(fPosY),
			VECNULL,
			D3DXCOLOR_NULL,
			TREE_SPHERE_BLOCK,
			TREE_SPHERE_BLOCK,
			fRadius,
			false,
			&pXmas->mtxWorld);

		fPosY -= TREE_SPHERE_INTERVAL;
		g_nCntCol++;
	}
}

//==================================================================================
// --- オーナメントをツリーに付けた時の処理 (重さと角度の分だけ傾ける) ---
//==================================================================================
void PutOrnament(float fAngle, float fWeight)
{
	PXMASTREE pXmas = &g_xmasTree;		// クリスマスツリー

	pXmas->rotDest.x += cosf(fAngle) * (fWeight * TREE_TILT_WEIGHT);
	pXmas->rotDest.z -= sinf(fAngle) * (fWeight * TREE_TILT_WEIGHT);
	pXmas->rotDest = RepairedRot(pXmas->rotDest);

	pXmas->nCntXmasTrees++;		// 付いているオーナメント数を増加
}

//==================================================================================
// --- クリスマスツリーの取得 ---
//==================================================================================
PXMASTREE GetXmasTree(void)
{
	return &g_xmasTree;
}

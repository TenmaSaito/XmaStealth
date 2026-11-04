//==================================================================================
//
// メッシュスフィア(球・球形の当たり判定)の処理 [meshSphere.cpp]
// Author : TENMA SAITO
//
// 主にツリーやオーナメントの当たり判定として使う(描画しない球)。
// 描画する球の頂点バッファの構成 : [上の蓋(扇)] [側面の格子] [下の蓋(扇)]
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "meshSphere.h"
#include "effect.h"
#include "XmasTree.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MESH						(512)		// メッシュスフィアの最大設置数
#define SPHERE_CAP_VTX_NUM(nXBlock)		((nXBlock) + 1)		// 蓋(扇)1つの頂点数
#define SPHERE_VTX_NUM(nXBlock, nYBlock)	(MESH_VTX_NUM(nXBlock, nYBlock) + (SPHERE_CAP_VTX_NUM(nXBlock) * 2))	// 全体の頂点数
#define SPHERE_IDX_NUM(nXBlock, nYBlock)	(MESH_IDX_NUM(nXBlock, nYBlock) + (((nXBlock) + 2) * 2))				// 全体のインデックス数

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void CreateSphereBuffer(LPMESHSPHERE pSphere);
static D3DXVECTOR3 GetSphereWorldPos(const MeshSphere *pSphere, bool bUseMtxWorld);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static MeshSphere g_aMeshSphere[MAX_MESH] = {};		// メッシュスフィアの情報
static int g_nCounterSphere;						// 設置した球の数

//==================================================================================
// --- メッシュスフィアの初期化処理 ---
//==================================================================================
void InitMeshSphere(void)
{
	ZeroMemory(&g_aMeshSphere[0], sizeof g_aMeshSphere);

	g_nCounterSphere = 0;
}

//==================================================================================
// --- メッシュスフィアの終了処理 ---
//==================================================================================
void UninitMeshSphere(void)
{
	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++)
	{
		/*** 頂点バッファ・インデックスバッファの破棄 ***/
		RELEASE(g_aMeshSphere[nCntSphere].pVtxBuff);
		RELEASE(g_aMeshSphere[nCntSphere].pIdxBuff);
	}
}

//==================================================================================
// --- メッシュスフィアの更新処理 ---
//==================================================================================
void UpdateMeshSphere(void)
{

}

//==================================================================================
// --- メッシュスフィアの描画処理 ---
// ※ 描画しない球もワールドマトリックスはここで更新する (当たり判定で使うため)
//==================================================================================
void DrawMeshSphere(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++)
	{
		LPMESHSPHERE pSphere = &g_aMeshSphere[nCntSphere];		// 描画する球

		if (pSphere->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ワールドマトリックスの計算 (回転 → 平行移動 → 親の順) ***/
		D3DXMatrixIdentity(&pSphere->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pSphere->rot.y, pSphere->rot.x, pSphere->rot.z);
		D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pSphere->pos.x, pSphere->pos.y, pSphere->pos.z);
		D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, &mtxTrans);

		if (pSphere->pMtxParent != NULL)
		{
			D3DXMatrixMultiply(&pSphere->mtxWorld, &pSphere->mtxWorld, pSphere->pMtxParent);
		}

		pDevice->SetTransform(D3DTS_WORLD, &pSphere->mtxWorld);

		if (pSphere->bDisp == false)
		{ // 当たり判定用の球は描画しない
			continue;
		}

		int nNumVtx = SPHERE_VTX_NUM(pSphere->nXBlock, pSphere->nYBlock);		// 全体の頂点数
		int nNumIdxCap = pSphere->nXBlock + 2;									// 蓋1つ分のインデックス数

		/*** 頂点バッファ・インデックスバッファの設定 ***/
		pDevice->SetStreamSource(0, pSphere->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetIndices(pSphere->pIdxBuff);
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, NULL);

		/*** 上の蓋 → 側面 → 下の蓋 の順に描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, nNumVtx, 0, pSphere->nXBlock);
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, nNumVtx, nNumIdxCap, MESH_PRIM_NUM(pSphere->nXBlock, pSphere->nYBlock));
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, nNumVtx, nNumIdxCap + MESH_IDX_NUM(pSphere->nXBlock, pSphere->nYBlock), pSphere->nXBlock);
	}
}

//==================================================================================
// --- メッシュスフィアの設置処理 ---
// 戻り値 : 設置した球の番号 (空きが無ければ -1)
//==================================================================================
int SetMeshSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp)
{
	for (int nCntSphere = 0; nCntSphere < MAX_MESH; nCntSphere++)
	{
		LPMESHSPHERE pSphere = &g_aMeshSphere[nCntSphere];		// 設置先の球

		if (pSphere->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pSphere->pos = pos;
		pSphere->rot = rot;
		pSphere->col = col;
		pSphere->fRadius = fRadius;
		pSphere->nXBlock = nXBlock;
		pSphere->nYBlock = nYBlock;
		pSphere->bUse = true;
		pSphere->bDisp = bDisp;
		pSphere->bCollision = false;

		if (bDisp == true)
		{ // 描画する球のみ頂点・インデックスを作る
			CreateSphereBuffer(pSphere);
		}

		g_nCounterSphere++;

		return nCntSphere;
	}

	return -1;
}

//==================================================================================
// --- 親マトリックスに追従する当たり判定用の球の設置処理 ---
// 戻り値 : 設置した球の番号 (失敗したら -1)
//==================================================================================
int SetParentSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp, D3DXMATRIX *pMtxParent)
{
	if (pMtxParent == NULL)
	{ // 親が無い
		return -1;
	}

	int nIdxSphere = SetMeshSphere(pos, rot, col, nXBlock, nYBlock, fRadius, bDisp);		// 設置した球の番号
	if (nIdxSphere == -1)
	{ // 空きが無い
		return -1;
	}

	g_aMeshSphere[nIdxSphere].pMtxParent = pMtxParent;
	g_aMeshSphere[nIdxSphere].bCollision = true;

	return nIdxSphere;
}

//==================================================================================
// --- メッシュスフィアの取得 (範囲外は NULL) ---
//==================================================================================
LPMESHSPHERE GetMeshSphere(int nIdxSphere)
{
	if (nIdxSphere < 0 || nIdxSphere >= g_nCounterSphere)
	{
		return NULL;
	}

	return &g_aMeshSphere[nIdxSphere];
}

//==================================================================================
// --- 2つの球が重なっているか ---
//==================================================================================
bool CollisionSphereToSphere(LPMESHSPHERE pSphere, LPMESHSPHERE pSphereAnother)
{
	if (pSphere == NULL || pSphereAnother == NULL)
	{
		return false;
	}

	// ※ 1つ目は描画時に計算したワールド座標、2つ目は親マトリックスから計算する
	D3DXVECTOR3 posSphere = GetSphereWorldPos(pSphere, true);					// 1つ目の中心
	D3DXVECTOR3 posSphereAnother = GetSphereWorldPos(pSphereAnother, false);	// 2つ目の中心
	D3DXVECTOR3 vecLength = posSphere - posSphereAnother;						// 中心同士のベクトル

	return D3DXVec3Length(&vecLength) <= (pSphere->fRadius + pSphereAnother->fRadius);
}

//==================================================================================
// --- 点が球(半径 × fMag)の中にあるか ---
//==================================================================================
bool CollisionLPSphere(LPMESHSPHERE pSphere, D3DXVECTOR3 pos, float fMag)
{
	if (pSphere == NULL || pSphere->bUse == false)
	{
		return false;
	}

	D3DXVECTOR3 posSphere = D3DXVECTOR3(pSphere->mtxWorld._41, pSphere->mtxWorld._42, pSphere->mtxWorld._43);	// 球の中心
	D3DXVECTOR3 vecLength = pos - posSphere;																	// 中心からのベクトル

	return D3DXVec3Length(&vecLength) <= pSphere->fRadius * fMag;
}

//==================================================================================
// --- 球の中心のワールド座標 ---
// bUseMtxWorld : true なら描画時に計算したワールドマトリックスから取得する
//==================================================================================
static D3DXVECTOR3 GetSphereWorldPos(const MeshSphere *pSphere, bool bUseMtxWorld)
{
	if (pSphere->pMtxParent == NULL)
	{ // 親が無ければ位置そのもの
		return pSphere->pos;
	}

	if (bUseMtxWorld)
	{
		return D3DXVECTOR3(pSphere->mtxWorld._41, pSphere->mtxWorld._42, pSphere->mtxWorld._43);
	}

	D3DXVECTOR3 posWorld;		// 親マトリックスで変換した位置
	D3DXVec3TransformCoord(&posWorld, &pSphere->pos, pSphere->pMtxParent);
	return posWorld;
}

//==================================================================================
// --- 描画する球の頂点バッファ・インデックスバッファの生成 ---
//==================================================================================
static void CreateSphereBuffer(LPMESHSPHERE pSphere)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	int nXBlock = pSphere->nXBlock;					// 横の分割数
	int nYBlock = pSphere->nYBlock;					// 縦の分割数
	float fAngleY = D3DX_PI / (float)nYBlock;		// 縦1段あたりの角度
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ
	WORD *pIdx = NULL;								// インデックス情報へのポインタ

	/*** 頂点バッファ ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * SPHERE_VTX_NUM(nXBlock, nYBlock),
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&pSphere->pVtxBuff,
		NULL);

	pSphere->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntCap = 0; nCntCap < 2; nCntCap++)
	{ // 0 : 上の蓋 / 1 : 下の蓋 (間に側面を挟む)
		if (nCntCap == 1)
		{ // 側面の格子 (上から順に)
			for (int nCntY = 0; nCntY <= nYBlock; nCntY++)
			{
				float fRingRadius = sinf((float)(nCntY + 1) * fAngleY) * pSphere->fRadius;		// この段の半径
				float fHeight = cosf((float)(nCntY + 1) * fAngleY) * pSphere->fRadius;			// この段の高さ

				for (int nCntX = 0; nCntX <= nXBlock; nCntX++, pVtx++)
				{
					float fAngle = (D3DX_2PI / (float)nXBlock) * nCntX;		// 円周上の角度
					D3DXVECTOR3 nor;										// 法線

					pVtx->pos = D3DXVECTOR3(sinf(fAngle) * fRingRadius, fHeight, cosf(fAngle) * fRingRadius);
					nor = pVtx->pos - pSphere->pos;
					D3DXVec3Normalize(&nor, &nor);

					pVtx->nor = nor;
					pVtx->col = pSphere->col;
					pVtx->tex.x = (1.0f / (float)nXBlock) * nCntX;
					pVtx->tex.y = (float)nCntY;
				}
			}
		}

		/*** 蓋 : [0] が極、[1]～ が1段目の円周 (上は逆回り) ***/
		float fSign = (nCntCap == 0) ? 1.0f : -1.0f;		// 上下の向き

		for (int nCntVtx = 0; nCntVtx < SPHERE_CAP_VTX_NUM(nXBlock); nCntVtx++, pVtx++)
		{
			int nRing = (nCntVtx == 0) ? 0 : 1;													// 0 : 極 / 1 : 1段目
			float fRingRadius = sinf(nRing * fAngleY) * pSphere->fRadius;						// この段の半径
			float fHeight = cosf(nRing * fAngleY) * pSphere->fRadius * fSign;					// この段の高さ
			float fAngle = (D3DX_2PI / (float)nXBlock) * (nCntVtx - 1) * ((nCntCap == 0) ? -1.0f : 1.0f);	// 円周上の角度
			D3DXVECTOR3 nor;																	// 法線

			pVtx->pos = (nCntVtx == 0)
				? D3DXVECTOR3(0.0f, fHeight, 0.0f)
				: D3DXVECTOR3(sinf(fAngle) * fRingRadius, fHeight, cosf(fAngle) * fRingRadius);

			nor = pVtx->pos - pSphere->pos;
			D3DXVec3Normalize(&nor, &nor);

			pVtx->nor = nor;
			pVtx->col = pSphere->col;
		}
	}

	pSphere->pVtxBuff->Unlock();

	/*** インデックスバッファ ***/
	pDevice->CreateIndexBuffer(sizeof(WORD) * SPHERE_IDX_NUM(nXBlock, nYBlock),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&pSphere->pIdxBuff,
		NULL);

	pSphere->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

	int nCapVtx = SPHERE_CAP_VTX_NUM(nXBlock);						// 蓋1つの頂点数
	int nIdxBottomTop = SPHERE_VTX_NUM(nXBlock, nYBlock) - nCapVtx;	// 下の蓋の先頭頂点

	/*** 上の蓋 : 0, 1, 2, ... , nXBlock, 1 ***/
	for (int nCntIdx = 0; nCntIdx < nXBlock + 2; nCntIdx++, pIdx++)
	{
		*pIdx = (WORD)((nCntIdx % nCapVtx) + (nCntIdx / nCapVtx));
	}

	/*** 側面 : 格子は上の蓋の後ろに並んでいるため、その分ずらす ***/
	SetIndex(pIdx, nXBlock, nYBlock, nCapVtx);
	pIdx += MESH_IDX_NUM(nXBlock, nYBlock);

	/*** 下の蓋 ***/
	for (int nCntIdx = 0; nCntIdx < nXBlock + 2; nCntIdx++, pIdx++)
	{
		*pIdx = (WORD)((nCntIdx == 0) ? nIdxBottomTop : (nCntIdx % nCapVtx) + nIdxBottomTop + (nCntIdx / nCapVtx));
	}

	pSphere->pIdxBuff->Unlock();
}

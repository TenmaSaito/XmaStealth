//==================================================================================
//
// Xファイルオブジェクト(家具などの配置物)の処理 [object.cpp]
// Author : TENMA SAITO
//
// g_aObjectInfo : 読み込んだモデル(メッシュ・マテリアル・テクスチャ・大きさ)
// g_aObject     : 配置したオブジェクト(どのモデルをどこに置いたか)
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "object.h"
#include "wall.h"
#include "effect.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define VTX_MIN_INIT		(10000.0f)		// 頂点の最小値を求める時の初期値
#define VTX_MAX_INIT		(-10000.0f)		// 頂点の最大値を求める時の初期値

// --- g_aPos の添字 (各軸の最小・最大を取った頂点) ---
#define VTXIDX_MIN_X		(0)				// X が最小の頂点
#define VTXIDX_MAX_X		(1)				// X が最大の頂点
#define VTXIDX_MIN_Y		(2)				// Y が最小の頂点
#define VTXIDX_MAX_Y		(3)				// Y が最大の頂点
#define VTXIDX_MIN_Z		(4)				// Z が最小の頂点
#define VTXIDX_MAX_Z		(5)				// Z が最大の頂点
#define VTXIDX_MAX			(6)				// 添字の数

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void LoadObjectTexture(LPOBJECTINFO pInfo, const char *pXFileName);
static void CalcObjectSize(int nIdxInfo);
static bool CollisionObjectRect(const Object *pObject, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, bool *pColl);
static bool CollisionObjectCross(const Object *pObject, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight, bool *pColl);
static D3DXVECTOR3 GetRotatedCorner(const Object *pObject, float fOffsetX, float fOffsetZ);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static ObjectInfo g_aObjectInfo[MAX_OBJECT];			// 読み込んだモデルの情報
static Object g_aObject[MAX_OBJECT];					// 配置したオブジェクトの情報
static D3DXVECTOR3 g_aPos[MAX_OBJECT][VTXIDX_MAX];		// 各軸の最小・最大を取った頂点(モデルごと)
static int g_nCurrentObject;							// 配置したオブジェクトの数
static int g_nCountCollision;							// Z方向に押し戻された回数(直近の判定)
static int g_nCounterObjectInfo;						// 読み込んだモデルの数

//==================================================================================
// --- オブジェクトの初期化処理 ---
//==================================================================================
void InitObject(void)
{
	ZeroMemory(&g_aObject[0], sizeof g_aObject);
	ZeroMemory(&g_aObjectInfo[0], sizeof g_aObjectInfo);
	ZeroMemory(&g_aPos[0][0], sizeof g_aPos);

	for (int nCntInfo = 0; nCntInfo < MAX_OBJECT; nCntInfo++)
	{
		g_aObjectInfo[nCntInfo].mtxMin = D3DXVECTOR3(VTX_MIN_INIT, VTX_MIN_INIT, VTX_MIN_INIT);
		g_aObjectInfo[nCntInfo].mtxMax = D3DXVECTOR3(VTX_MAX_INIT, VTX_MAX_INIT, VTX_MAX_INIT);
	}

	g_nCurrentObject = 0;
	g_nCountCollision = 0;
	g_nCounterObjectInfo = 0;
}

//==================================================================================
// --- オブジェクトの終了処理 (読み込んだモデルの破棄) ---
//==================================================================================
void UninitObject(void)
{
	for (int nCntInfo = 0; nCntInfo < MAX_OBJECT; nCntInfo++)
	{
		LPOBJECTINFO pInfo = &g_aObjectInfo[nCntInfo];		// 破棄するモデル

		/*** メッシュ・マテリアル・テクスチャの破棄 ***/
		RELEASE(pInfo->pMesh);
		RELEASE(pInfo->pBuffMat);

		for (int nCntTex = 0; nCntTex < MAX_OBJTEXTURE; nCntTex++)
		{
			RELEASE(pInfo->apTexture[nCntTex]);
		}
	}
}

//==================================================================================
// --- オブジェクトの更新処理 ---
//==================================================================================
void UpdateObject(void)
{
	PrintDebugProc("%d\n", g_nCounterObjectInfo);
}

//==================================================================================
// --- オブジェクトの描画処理 ---
//==================================================================================
void DrawObject(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	D3DMATERIAL9 matDef;							// 現在のマテリアル保存用

	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		Object *pObject = &g_aObject[nCntObject];				// 描画するオブジェクト
		LPOBJECTINFO pInfo = &g_aObjectInfo[pObject->nType];	// オブジェクトのモデル

		if (pObject->bUse == false || pInfo->bSafe == false)
		{ // 未使用 or モデルが読み込めていない
			continue;
		}

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pObject->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pObject->rot.y, pObject->rot.x, pObject->rot.z);
		D3DXMatrixMultiply(&pObject->mtxWorld, &pObject->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pObject->pos.x, pObject->pos.y, pObject->pos.z);
		D3DXMatrixMultiply(&pObject->mtxWorld, &pObject->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pObject->mtxWorld);

		/*** マテリアルごとに描画 ***/
		pDevice->GetMaterial(&matDef);

		D3DXMATERIAL *pMat = (D3DXMATERIAL*)pInfo->pBuffMat->GetBufferPointer();		// マテリアルデータ

		for (int nCntMat = 0; nCntMat < (int)pInfo->dwNumMat; nCntMat++)
		{
			pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
			pDevice->SetTexture(0, pInfo->apTexture[nCntMat]);
			pInfo->pMesh->DrawSubset(nCntMat);
		}

		pDevice->SetMaterial(&matDef);
	}
}

//==================================================================================
// --- モデル(Xファイル)の読み込み ---
// 空いている枠に読み込む (読み込んだ順に 0, 1, 2 ... の番号になる)
//==================================================================================
_Check_return_ HRESULT LoadObject(_In_ const char *pXFileName)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	for (int nCntInfo = 0; nCntInfo < MAX_OBJECT; nCntInfo++)
	{
		LPOBJECTINFO pInfo = &g_aObjectInfo[nCntInfo];		// 読み込み先のモデル

		if (pInfo->bUse == true)
		{ // 使用中 (読み込みに失敗した枠も使用中のまま)
			continue;
		}

		pInfo->bUse = true;
		g_nCounterObjectInfo++;

		CheckPath(pXFileName);

		/*** Xファイルの読み込み ***/
		if (FAILED(D3DXLoadMeshFromX(pXFileName,
			D3DXMESH_SYSTEMMEM,
			pDevice,
			NULL,
			&pInfo->pBuffMat,		// マテリアル
			NULL,
			&pInfo->dwNumMat,		// マテリアルの数
			&pInfo->pMesh)))		// メッシュ
		{ // 読み込み失敗
#ifdef _DEBUG
			MessageBox(GetHandleWindow(), "Xファイルの読み込みに失敗しました。", "Error(3)", MB_ICONERROR);
#endif
			return E_FAIL;
		}

		if (pInfo->dwNumMat > MAX_OBJTEXTURE)
		{ // テクスチャの配列に収まらないマテリアルは描画しない (配列外への書き込み防止)
			GenerateMessageBox(MB_ICONWARNING, "Warnings!", "マテリアル数が多すぎます(%d個まで)。\n対象Xファイルパス : %s", MAX_OBJTEXTURE, pXFileName);
			pInfo->dwNumMat = MAX_OBJTEXTURE;
		}

		LoadObjectTexture(pInfo, pXFileName);
		CalcObjectSize(nCntInfo);

		pInfo->bSafe = true;		// 参照可能にする

		break;
	}

	return S_OK;
}

//==================================================================================
// --- オブジェクトの配置 ---
// ※ pCode は現在未使用
//==================================================================================
void SetObject(_In_ D3DXVECTOR3 pos, _In_ D3DXVECTOR3 rot, _In_ int nType, _In_ COLLISIONTYPE collisionType, _In_opt_ const char *pCode)
{
	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		Object *pObject = &g_aObject[nCntObject];		// 配置先のオブジェクト

		if (pObject->bUse == true)
		{ // 使用中
			continue;
		}

		if (g_aObjectInfo[nType].bSafe == false)
		{ // モデルが読み込めていない
			return;
		}

		LPOBJECTINFO pInfo = &g_aObjectInfo[nType];		// 配置するモデル

		pObject->pos = pos;
		pObject->rot = rot;
		pObject->nType = nType;
		pObject->collisionType = collisionType;
		pObject->bUse = true;
		pObject->fWidth = pInfo->mtxMax.x - pInfo->mtxMin.x;
		pObject->fHeight = pInfo->mtxMax.y - pInfo->mtxMin.y;
		pObject->fDepth = pInfo->mtxMax.z - pInfo->mtxMin.z;

		g_nCurrentObject++;

		return;
	}
}

//==================================================================================
// --- オブジェクトとの当たり判定 (ゲーム画面のみ) ---
// pColl : 何かに当たったら true が入る
// 戻り値 : オブジェクトの上に乗ったか
//==================================================================================
bool CollisionObject(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight, bool *pColl)
{
	bool bLand = false;		// 上に乗ったか

	g_nCountCollision = 0;

	if (GetMode() != MODE_GAME)
	{
		return false;
	}

	for (int nCntObject = 0; nCntObject < MAX_OBJECT; nCntObject++)
	{
		const Object *pObject = &g_aObject[nCntObject];		// 判定するオブジェクト

		if (pObject->bUse == false)
		{ // 未使用
			continue;
		}

		switch (pObject->collisionType)
		{
		case COLLISIONTYPE_RECT:		// 回転を考慮しない箱
			if (CollisionObjectRect(pObject, pPos, pPosOld, pMove, pColl)) bLand = true;
			break;

		case COLLISIONTYPE_CROSS:		// Y軸回転を考慮した箱 (外積で判定)
			if (CollisionObjectCross(pObject, pPos, pPosOld, pMove, fHeight, pColl)) bLand = true;
			break;

		default:
			break;
		}
	}

	return bLand;
}

//==================================================================================
// --- 配置したオブジェクトの取得 (先頭) ---
//==================================================================================
Object *GetObject(void)
{
	return &g_aObject[0];
}

//==================================================================================
// --- 読み込んだモデルの取得 (範囲外・読み込めていない場合は NULL) ---
//==================================================================================
ObjectInfo *GetObjectInfo(_In_ int nType)
{
	if (nType < 0 || nType >= MAX_OBJECT)
	{
		return NULL;
	}

	if (g_aObjectInfo[nType].bSafe == false)
	{
		return NULL;
	}

	return &g_aObjectInfo[nType];
}

//==================================================================================
// --- モデルの、指定した軸で最小・最大を取った頂点の取得 ---
// nXYZ : 0 = X最小 / 1 = X最大 / 2 = Y最小 / 3 = Y最大 / 4 = Z最小 / 5 = Z最大
//==================================================================================
D3DXVECTOR3 GetObjectMVector(int nType, int nXYZ)
{
	if (nType < 0 || nType >= MAX_OBJECT || nXYZ < 0 || nXYZ >= VTXIDX_MAX)
	{
		return VECNULL;
	}

	return g_aPos[nType][nXYZ];
}

//==================================================================================
//
// ここから内部処理
//
//==================================================================================

//==================================================================================
// --- モデルのマテリアルに指定されたテクスチャの読み込み ---
//==================================================================================
static void LoadObjectTexture(LPOBJECTINFO pInfo, const char *pXFileName)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();									// デバイス
	D3DXMATERIAL *pMat = (D3DXMATERIAL*)pInfo->pBuffMat->GetBufferPointer();	// マテリアルデータ

	for (int nCntMat = 0; nCntMat < (int)pInfo->dwNumMat; nCntMat++)
	{
		if (pMat[nCntMat].pTextureFilename == NULL)
		{ // テクスチャ無し
			continue;
		}

		if (CheckPath(pMat[nCntMat].pTextureFilename))
		{ // 絶対パスになっている
#ifdef _DEBUG
			GenerateMessageBox(MB_ICONERROR, "Warnings!", "対象Xファイルパス : %s", pXFileName);
#endif
		}

		if (FAILED(D3DXCreateTextureFromFile(pDevice, pMat[nCntMat].pTextureFilename, &pInfo->apTexture[nCntMat])))
		{ // テクスチャ読み込み失敗
#ifdef _DEBUG
			GenerateMessageBox(MB_ICONERROR, "Error (4)", "Xファイルのテクスチャ読み込みに失敗しました。\n対象パス : %s", pMat[nCntMat].pTextureFilename);
#endif
		}
	}
}

//==================================================================================
// --- 全頂点を調べて、モデルの最小・最大座標を求める ---
// ※ 値は RepairFloat で整数に丸める
//==================================================================================
static void CalcObjectSize(int nIdxInfo)
{
	LPOBJECTINFO pInfo = &g_aObjectInfo[nIdxInfo];							// 調べるモデル
	D3DXVECTOR3 *pPosVtx = &g_aPos[nIdxInfo][0];							// 最小・最大を取った頂点の格納先
	int nNumVtx = pInfo->pMesh->GetNumVertices();							// 頂点数
	DWORD dwSizeFVF = D3DXGetFVFVertexSize(pInfo->pMesh->GetFVF());			// 頂点1つ分のサイズ
	BYTE *pVtxBuff = NULL;													// 頂点バッファ

	pInfo->pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

	for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++, pVtxBuff += dwSizeFVF)
	{
		D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;		// 頂点座標 (頂点データの先頭)

		/*** 最小値 ***/
		if (pInfo->mtxMin.x > vtx.x) { pInfo->mtxMin.x = vtx.x; RepairFloat(&pInfo->mtxMin.x); pPosVtx[VTXIDX_MIN_X] = vtx; }
		if (pInfo->mtxMin.y > vtx.y) { pInfo->mtxMin.y = vtx.y; RepairFloat(&pInfo->mtxMin.y); pPosVtx[VTXIDX_MIN_Y] = vtx; }
		if (pInfo->mtxMin.z > vtx.z) { pInfo->mtxMin.z = vtx.z; RepairFloat(&pInfo->mtxMin.z); pPosVtx[VTXIDX_MIN_Z] = vtx; }

		/*** 最大値 ***/
		if (pInfo->mtxMax.x < vtx.x) { pInfo->mtxMax.x = vtx.x; RepairFloat(&pInfo->mtxMax.x); pPosVtx[VTXIDX_MAX_X] = vtx; }
		if (pInfo->mtxMax.y < vtx.y) { pInfo->mtxMax.y = vtx.y; RepairFloat(&pInfo->mtxMax.y); pPosVtx[VTXIDX_MAX_Y] = vtx; }
		if (pInfo->mtxMax.z < vtx.z) { pInfo->mtxMax.z = vtx.z; RepairFloat(&pInfo->mtxMax.z); pPosVtx[VTXIDX_MAX_Z] = vtx; }
	}

	pInfo->pMesh->UnlockVertexBuffer();
}

//==================================================================================
// --- 回転を考慮しない箱との当たり判定 ---
// 箱の中に入ったら、前フレームにいた側の面まで押し戻す
// 戻り値 : 上に乗ったか
//==================================================================================
static bool CollisionObjectRect(const Object *pObject, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, bool *pColl)
{
	const ObjectInfo *pInfo = &g_aObjectInfo[pObject->nType];		// オブジェクトのモデル
	D3DXVECTOR3 posMin = pObject->pos + pInfo->mtxMin;				// 箱の最小座標
	D3DXVECTOR3 posMax = pObject->pos + pInfo->mtxMax;				// 箱の最大座標
	bool bLand = false;												// 上に乗ったか

	if (pPos->x < posMin.x || pPos->x > posMax.x
		|| pPos->y > posMax.y || pPos->y < posMin.y
		|| pPos->z < posMin.z || pPos->z > posMax.z)
	{ // 箱の外
		return false;
	}

	/*** X : 前フレームにいた側へ押し戻す ***/
	if (pPosOld->x <= posMin.x)			pPos->x = posMin.x;
	else if (pPosOld->x >= posMax.x)	pPos->x = posMax.x;

	/*** Y : 上から入ったら上に乗せる ***/
	if (pPosOld->y < posMin.y)
	{
		pPos->y = posMin.y;
	}
	else if (pPosOld->y >= posMax.y)
	{
		pPos->y = posMax.y;
		pMove->y = 0.0f;
		bLand = true;
	}

	/*** Z ***/
	if (pPosOld->z <= posMin.z)
	{
		pPos->z = posMin.z;
		g_nCountCollision++;
	}
	else if (pPosOld->z >= posMax.z)
	{
		pPos->z = posMax.z;
		g_nCountCollision++;
	}

	if (pColl != NULL)
	{
		*pColl = true;
	}

	return bLand;
}

//==================================================================================
// --- Y軸回転を考慮した箱との当たり判定 (4辺それぞれと外積で判定) ---
// 辺を外から内へまたいだら辺の上に押し戻し、4辺全ての内側にいれば上に乗せる
// 戻り値 : 上に乗ったか
//==================================================================================
static bool CollisionObjectCross(const Object *pObject, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight, bool *pColl)
{
	const ObjectInfo *pInfo = &g_aObjectInfo[pObject->nType];		// オブジェクトのモデル

	if (pPos->y + fHeight < pObject->pos.y + pInfo->mtxMin.y
		|| pPos->y > pObject->pos.y + pInfo->mtxMax.y)
	{ // 高さが重なっていない
		return false;
	}

	/*** 回転させた箱の4隅 (左手前 → 右手前 → 右奥 → 左奥) ***/
	D3DXVECTOR3 aPosCorner[4] =
	{
		GetRotatedCorner(pObject, pInfo->mtxMin.x, pInfo->mtxMin.z),
		GetRotatedCorner(pObject, pInfo->mtxMax.x, pInfo->mtxMin.z),
		GetRotatedCorner(pObject, pInfo->mtxMax.x, pInfo->mtxMax.z),
		GetRotatedCorner(pObject, pInfo->mtxMin.x, pInfo->mtxMax.z),
	};

	D3DXVECTOR3 vecMove = *pPos - *pPosOld;		// 移動ベクトル
	bool abInside[4] = {};						// 各辺の内側にいるか

	for (int nCntLine = 0; nCntLine < 4; nCntLine++)
	{
		const D3DXVECTOR3 *pStart = &aPosCorner[nCntLine];					// 辺の始点
		D3DXVECTOR3 vecLine = aPosCorner[(nCntLine + 1) % 4] - *pStart;		// 辺のベクトル

		RepairFloat(&vecLine.x);
		RepairFloat(&vecLine.z);

		/*** 現在・前フレームの位置が辺のどちら側か (外積 : 負なら内側) ***/
		D3DXVECTOR3 vecToPos = *pPos - *pStart;				// 始点 → 現在位置
		D3DXVECTOR3 vecToPosOld = *pPosOld - *pStart;		// 始点 → 前フレームの位置

		float fCross = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);				// 現在位置との外積
		float fCrossOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);	// 前フレームの位置との外積
		RepairFloat(&fCross);
		RepairFloat(&fCrossOld);

		/*** 移動ベクトルと辺の交点の位置 (辺の始点からの割合) ***/
		float fCrossPosMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);		// 始点→現在位置 と 移動ベクトルの外積
		float fCrossLineMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);		// 辺 と 移動ベクトルの外積
		float fRate = fCrossPosMove / fCrossLineMove;									// 交点の割合

		/*** 押し戻し後に辺に沿って滑らせる量 ***/
		D3DXVECTOR3 vecNor = D3DXVECTOR3(vecLine.z, 0.0f, -vecLine.x);		// 辺の法線
		D3DXVec3Normalize(&vecNor, &vecNor);

		float fCrossNorMove = (vecNor.z * vecMove.x) - (vecNor.x * vecMove.z);		// 法線と移動ベクトルの外積
		D3DXVECTOR3 vecLineDir;														// 辺の向き(単位ベクトル)
		D3DXVec3Normalize(&vecLineDir, &vecLine);

		D3DXVECTOR3 vecSlide = (fCrossNorMove > 0)
			? -vecLineDir * fabsf(fCrossNorMove)
			: vecLineDir * fabsf(fCrossNorMove);									// 滑らせる量

		if (fCross <= 0 && fCrossOld >= 0 && fRate >= 0.0f && fRate <= 1.0f)
		{ // 外から内へ辺をまたいだ → 交点まで押し戻して滑らせる
			pPos->x = pStart->x + (vecLine.x * fRate) + vecSlide.x;
			pPos->z = pStart->z + (vecLine.z * fRate) + vecSlide.z;
		}

		if (fCross < 0 && fCrossOld < 0)
		{ // ずっと内側にいる
			abInside[nCntLine] = true;
		}
	}

	if (abInside[0] && abInside[1] && abInside[2] && abInside[3])
	{ // 4辺全ての内側 → 上に乗せる
		pPos->y = pObject->pos.y + pInfo->mtxMax.y;
		pMove->y = 0.0f;

		if (pColl != NULL)
		{
			*pColl = true;
		}

		return true;
	}

	return false;
}

//==================================================================================
// --- オブジェクトのローカル座標(XZ)の隅を、オブジェクトの向きで回転させたワールド座標 ---
//==================================================================================
static D3DXVECTOR3 GetRotatedCorner(const Object *pObject, float fOffsetX, float fOffsetZ)
{
	D3DXVECTOR3 posCorner = D3DXVECTOR3(pObject->pos.x + fOffsetX, pObject->pos.y, pObject->pos.z + fOffsetZ);	// 回転前の隅
	D3DXVECTOR3 vecToCorner = posCorner - pObject->pos;																// 中心 → 隅
	float fLength = D3DXVec3Length(&vecToCorner);																	// 中心からの距離
	float fAngle = GetPosToPos(posCorner, pObject->pos);															// 中心から見た隅の方向

	return D3DXVECTOR3(pObject->pos.x + (sinf(fAngle + pObject->rot.y) * fLength),
		pObject->pos.y,
		pObject->pos.z + (cosf(fAngle + pObject->rot.y) * fLength));
}

//==================================================================================
//
// メッシュフィールド(分割された地面・坂)の処理 [meshfield.cpp]
// Author : TENMA SAITO
//
// 当たり判定のため、インデックスの並びを構造体にも控えておき、
// 三角形ごとに「真上から見て内側にいるか」を外積で判定して高さを求める。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "meshfield.h"
#include "effect.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MESH				(64)		// メッシュフィールドの最大設置数
#define MAX_MESHFIELD_INDEX		(1000)		// 1枚あたりのインデックスの最大数 (20×20分割で878)

// --- 当たり判定を行う範囲 (中心からの倍率 / 元の調整値) ---
#define COLLISION_RANGE_LEFT	(0.5f)		// 左側 (幅に対する倍率)
#define COLLISION_RANGE_RIGHT	(1.4f)		// 右側 (幅に対する倍率)
#define COLLISION_RANGE_DEPTH	(0.65f)		// 手前・奥 (奥行に対する倍率)

//**********************************************************************************
// *** メッシュフィールド構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;						// 位置
	D3DXVECTOR3 rot;						// 角度
	D3DXMATRIX mtxWorld;					// ワールドマトリックス
	WORD aIdx[MAX_MESHFIELD_INDEX];			// インデックスの控え(当たり判定用)
	D3DCULL cullType;						// カリングタイプ
	int nXBlock;							// 横の分割数
	int nZBlock;							// 奥の分割数
	int nIdxTexture;						// テクスチャ番号
	float fWidth;							// 横幅
	float fDepth;							// 奥行
	bool bUse;								// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;		// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;		// インデックスバッファへのポインタ
} MeshField, *PMESHFIELD;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static bool GetTriangleHeight(const D3DXVECTOR3 *pPos, const D3DXVECTOR3 aPosTri[3], bool bOdd, float *pHeight, D3DXVECTOR3 *pNor);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static MeshField g_aMField[MAX_MESH];		// メッシュフィールドの情報

//==================================================================================
// --- メッシュフィールドの初期化処理 ---
//==================================================================================
void InitMeshfield(void)
{
	ZeroMemory(&g_aMField[0], sizeof g_aMField);
}

//==================================================================================
// --- メッシュフィールドの終了処理 ---
//==================================================================================
void UninitMeshfield(void)
{
	for (int nCntField = 0; nCntField < MAX_MESH; nCntField++)
	{
		/*** 頂点バッファ・インデックスバッファの破棄 ***/
		RELEASE(g_aMField[nCntField].pVtxBuff);
		RELEASE(g_aMField[nCntField].pIdxBuff);
	}
}

//==================================================================================
// --- メッシュフィールドの更新処理 ---
//==================================================================================
void UpdateMeshfield(void)
{

}

//==================================================================================
// --- メッシュフィールドの描画処理 ---
//==================================================================================
void DrawMeshfield(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	for (int nCntField = 0; nCntField < MAX_MESH; nCntField++)
	{
		PMESHFIELD pField = &g_aMField[nCntField];		// 描画するメッシュフィールド

		if (pField->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		// ※ 当たり判定でもこの行列を使うため、描画より先に判定すると1フレーム前の行列になる
		D3DXMatrixIdentity(&pField->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pField->rot.y, pField->rot.x, pField->rot.z);
		D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pField->pos.x, pField->pos.y, pField->pos.z);
		D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pField->mtxWorld);

		/*** 頂点バッファ・インデックスバッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, pField->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetIndices(pField->pIdxBuff);
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pField->nIdxTexture));

		/*** インデックスを利用したポリゴンの描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
			0,
			0,
			MESH_VTX_NUM(pField->nXBlock, pField->nZBlock),		// 頂点数
			0,
			MESH_PRIM_NUM(pField->nXBlock, pField->nZBlock));	// 描画する三角形の数
	}
}

//==================================================================================
// --- メッシュフィールドの設置処理 ---
//==================================================================================
void SetMeshField(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fDepth, int nXBlock, int nZBlock, int nIdxTexture, D3DCULL culllType)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	if (MESH_IDX_NUM(nXBlock, nZBlock) > MAX_MESHFIELD_INDEX)
	{ // 分割数が多すぎて、当たり判定用のインデックスが控えきれない
		GenerateMessageBox(MB_ICONERROR, "Error (MeshField)", "分割数が多すぎます。(%d × %d)", nXBlock, nZBlock);
		return;
	}

	for (int nCntField = 0; nCntField < MAX_MESH; nCntField++)
	{
		PMESHFIELD pField = &g_aMField[nCntField];		// 設置先のメッシュフィールド

		if (pField->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		ZeroMemory(pField, sizeof(MeshField));

		pField->pos = pos;
		pField->rot = RepairedRot(rot);
		pField->fWidth = fWidth;
		pField->fDepth = fDepth;
		pField->nXBlock = nXBlock;
		pField->nZBlock = nZBlock;
		pField->nIdxTexture = nIdxTexture;
		pField->cullType = culllType;
		pField->bUse = true;

		/*** 頂点バッファの生成・設定 ***/
		if (SUCCEEDED(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * MESH_VTX_NUM(nXBlock, nZBlock),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_3D,
			D3DPOOL_MANAGED,
			&pField->pVtxBuff,
			NULL)))
		{
			VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ
			pField->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			SetIndexedVertex(pVtx, pField->pos, nXBlock, nZBlock, fWidth, fDepth);

			pField->pVtxBuff->Unlock();
		}

		/*** インデックスバッファの生成・設定 (当たり判定用に控えも取る) ***/
		if (SUCCEEDED(pDevice->CreateIndexBuffer(sizeof(WORD) * MESH_IDX_NUM(nXBlock, nZBlock),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&pField->pIdxBuff,
			NULL)))
		{
			WORD *pIdx = NULL;		// インデックス情報へのポインタ
			pField->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			SetIndex(pIdx, nXBlock, nZBlock, 0, &pField->aIdx[0]);

			pField->pIdxBuff->Unlock();
		}

		return;
	}
}

//==================================================================================
// --- メッシュフィールドとの当たり判定 ---
// 足元が地面より下にあれば地面の高さまで持ち上げる
// 戻り値 : 着地したか (pVecNor には着地した面の法線が入る)
//==================================================================================
bool CollisionMeshField(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, D3DXVECTOR3 *pVecNor)
{
	bool bLand = false;		// 着地したか

	if (pPos == NULL || pPosOld == NULL || pMove == NULL)
	{
		return false;
	}

	for (int nCntField = 0; nCntField < MAX_MESH; nCntField++)
	{
		PMESHFIELD pField = &g_aMField[nCntField];		// 判定するメッシュフィールド

		if (pField->bUse == false)
		{ // 未使用
			continue;
		}

		/*** 大まかな範囲内か (x:左端 y:右端 z:手前端 w:奥端) ***/
		D3DXVECTOR4 rect;
		rect.x = pField->pos.x - (pField->fWidth * COLLISION_RANGE_LEFT);
		rect.y = pField->pos.x + (pField->fWidth * COLLISION_RANGE_RIGHT);
		rect.z = pField->pos.z - (pField->fDepth * COLLISION_RANGE_DEPTH);
		rect.w = pField->pos.z + (pField->fDepth * COLLISION_RANGE_DEPTH);

		if (CollisionBoxZ(rect, *pPos) == false)
		{ // 範囲外
			continue;
		}

		/*** 三角形ごとに判定 ***/
		VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ
		pField->pVtxBuff->Lock(0, 0, (void**)&pVtx, D3DLOCK_READONLY);

		int nNumPrim = MESH_PRIM_NUM(pField->nXBlock, pField->nZBlock);		// 三角形の数

		for (int nCntPrim = 0; nCntPrim < nNumPrim; nCntPrim++)
		{
			const WORD *pIdxTri = &pField->aIdx[nCntPrim];		// この三角形のインデックス(3つ)

			if (pIdxTri[0] == pIdxTri[1] || pIdxTri[1] == pIdxTri[2])
			{ // 縮退三角形(段の折り返し)は無視
				continue;
			}

			/*** 三角形の頂点をワールド座標で取得 ***/
			D3DXVECTOR3 aPosTri[3];		// 三角形の頂点
			for (int nCntVtx = 0; nCntVtx < 3; nCntVtx++)
			{
				D3DXVec3TransformCoord(&aPosTri[nCntVtx], &pVtx[pIdxTri[nCntVtx]].pos, &pField->mtxWorld);
			}

			float fHeight = 0.0f;					// この地点の地面の高さ
			D3DXVECTOR3 nor = D3DXVECTOR3_NULL;		// 面の法線

			if (GetTriangleHeight(pPos, aPosTri, (nCntPrim % 2) == 1, &fHeight, &nor) && fHeight >= pPos->y)
			{ // 三角形の内側で、地面より下にいる → 地面の高さへ
				pPos->y = fHeight;
				bLand = true;

				if (pVecNor != NULL)
				{
					*pVecNor = nor;
				}
			}
		}

		pField->pVtxBuff->Unlock();
	}

	return bLand;
}

//==================================================================================
// --- 真上から見て三角形の内側にいれば、その地点の面の高さと法線を求める ---
// bOdd : トライアングルストリップの奇数番目の三角形か (頂点の並びが逆回りになる)
// 戻り値 : 三角形の内側にいるか
//==================================================================================
static bool GetTriangleHeight(const D3DXVECTOR3 *pPos, const D3DXVECTOR3 aPosTri[3], bool bOdd, float *pHeight, D3DXVECTOR3 *pNor)
{
	float afCross[3] = {};		// 各辺と位置の外積(XZ平面)

	for (int nCntLine = 0; nCntLine < 3; nCntLine++)
	{
		D3DXVECTOR3 vecLine = aPosTri[(nCntLine + 1) % 3] - aPosTri[nCntLine];		// 辺のベクトル
		D3DXVECTOR3 vecToPos = *pPos - aPosTri[nCntLine];							// 辺の始点 → 位置

		afCross[nCntLine] = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);
	}

	/*** 内側判定 (偶数番目は全て正、奇数番目は全て負で内側) ***/
	if (bOdd == false && (afCross[0] > 0 && afCross[1] > 0 && afCross[2] > 0) == false) return false;
	if (bOdd == true && (afCross[0] < 0 && afCross[1] < 0 && afCross[2] < 0) == false) return false;

	/*** 面の法線 (奇数番目は裏返す) ***/
	D3DXVECTOR3 vecEdge0 = aPosTri[1] - aPosTri[0];		// 辺0
	D3DXVECTOR3 vecEdge1 = aPosTri[2] - aPosTri[0];		// 辺1
	D3DXVECTOR3 nor;										// 法線

	D3DXVec3Cross(&nor, &vecEdge0, &vecEdge1);
	if (bOdd) nor *= -1;
	D3DXVec3Normalize(&nor, &nor);

	/*** 平面の方程式から高さを求める ***/
	float fHeight = aPosTri[0].y - ((1.0f / nor.y) * ((nor.x * (pPos->x - aPosTri[0].x)) + (nor.z * (pPos->z - aPosTri[0].z))));

	if (bOdd == false)
	{ // 偶数番目は絶対値を取る (元の仕様)
		fHeight = fabsf(fHeight);
	}

	*pHeight = fHeight;
	*pNor = nor;

	return true;
}

//================================================================================================================
//
// DirectXのメッシュフィールド用表示処理 [meshfield.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "meshfield.h"
#include "effect.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_MESH				(32)		// メッシュの最大設置数
#define MESHFIELD_SIZE_X		(300)		// ポリゴンの基本サイズ - X
#define MESHFIELD_SIZE_Z		(300)		// ポリゴンの基本サイズ - Y
#define MESHFIELD_SPD			(2.0f)		// ポリゴンの移動スピード
#define MESHFIELD_ROTSPD		(0.1f)		// ポリゴンの回転スピード
#define MESHFIELD_WDSPD			(0.1f)		// ポリゴンの拡縮スピード
#define MESH_X_NUM				(5)			// ポリゴンの分割数 (X)※頂点数ではない
#define MESH_Z_NUM				(5)			// ポリゴンの分割数 (Z)※頂点数ではない
#define VERTEX_BUFFER(xBlock, zBlock)	((xBlock + 1) * (zBlock + 1))	// 頂点バッファの確保用
#define INDEX_BUFFER(vertexNum, xBlock, zBlock)	(vertexNum + ((xBlock + 1) * (zBlock - 1)) + ((zBlock - 1) * 2))	// インデックスバッファ確保用		
#define DRAWPRIM_NUM(xBlock, zBlock)	(((xBlock * zBlock) * 2) + ((zBlock - 1) * 4))	// 描画プリミティブ用

//*************************************************************************************************
//*** メッシュフィールド構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// ポリゴンの位置
	D3DXVECTOR3 rot;		// ポリゴンの角度
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	WORD nIdx[1000];		// インデックス管理
	D3DCULL cullType;		// カリングタイプ
	int nXBlock;			// 分割数
	int nZBlock;			// 分割数
	int nIdxTexture;		// テクスチャ番号
	float fWidth;			// ポリゴンの横幅
	float fDepth;			// ポリゴンの奥行
	bool bUse;				// 使用状況
	LPDIRECT3DTEXTURE9		pTexture;	// テクスチャへのポインタ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshField, *PMESHFIELD;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureMeshfield = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffMeshfield = NULL;	// 頂点バッファのポインタ
LPDIRECT3DINDEXBUFFER9 g_pIdxBuffMeshField = NULL;	// インデックスバッファへのポインタ
MeshField g_aMField[MAX_MESH];		// メッシュフィールド
WORD g_wIdx[MAX_MESH][INDEX_BUFFER(VERTEX_BUFFER(MESH_X_NUM, MESH_Z_NUM), MESH_X_NUM, MESH_Z_NUM)] = {};

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitMeshfield(void)
{
	for (int nCntMeshField = 0; nCntMeshField < MAX_MESH; nCntMeshField++)
	{
		ZeroMemory(&g_aMField[nCntMeshField], sizeof(MeshField));
	}
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitMeshfield(void)
{
	PMESHFIELD pField = &g_aMField[0];

	for (int nCntRelease = 0; nCntRelease < MAX_MESH; nCntRelease++, pField++)
	{
		/*** テクスチャの破棄 ***/
		RELEASE(pField->pTexture);

		/*** 頂点バッファの破棄 ***/
		RELEASE(pField->pVtxBuff);

		/*** インデックスバッファの破棄 ***/
		RELEASE(pField->pIdxBuff);
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateMeshfield(void)
{

}

//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawMeshfield(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	PMESHFIELD pField = &g_aMField[0];
	DWORD cullTypeDef;

	for (int nCntMField = 0; nCntMField < MAX_MESH; nCntMField++, pField++)
	{
		if (pField->bUse == true)
		{
			pDevice->GetRenderState(D3DRS_CULLMODE, &cullTypeDef);

			SetCullingType(pField->cullType);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pField->mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				pField->rot.y,			// Y軸回転
				pField->rot.x,			// X軸回転
				pField->rot.z);			// Z軸回転

			D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pField->pos.x,
				pField->pos.y,
				pField->pos.z);

			D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pField->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pField->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** インデックスバッファをデータストリームに設定 ***/
			pDevice->SetIndices(pField->pIdxBuff);

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pField->nIdxTexture));

			/*** インデックスを利用したポリゴンの描画 ***/
			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
				0,
				0,
				VERTEX_BUFFER(pField->nXBlock, pField->nZBlock),		// 頂点数
				0,
				DRAWPRIM_NUM(pField->nXBlock, pField->nZBlock));	// 描画するプリミティブ(三角ポリゴン)の数

			SetCullingType((D3DCULL)cullTypeDef);
		}
	}
}

//================================================================================================================
// --- メッシュフィールドの設置処理 ---
//================================================================================================================
void SetMeshField(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fDepth, int nXBlock, int nZBlock, int nIdxTexture, D3DCULL culllType)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	HRESULT hr = E_FAIL;				// 例外判定
	WORD* pIdx;							// インデックス情報へのポインタ
	PMESHFIELD pField = &g_aMField[0];

	for (int nCntField = 0; nCntField < MAX_MESH; nCntField++, pField++)
	{
		if (pField->bUse == false)
		{
			/*** 各変数の初期化 ***/
			ZeroMemory(&g_aMField[nCntField], sizeof(MeshField));

			pField->pos = pos;
			pField->rot = RepairedRot(rot);
			pField->fWidth = fWidth;
			pField->fDepth = fDepth;
			pField->nXBlock = nXBlock;
			pField->nZBlock = nZBlock;
			pField->nIdxTexture = nIdxTexture;
			pField->cullType = culllType;
			pField->bUse = true;

			/*** 頂点バッファの生成 ***/
			hr = pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * VERTEX_BUFFER(nXBlock, nZBlock),
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pField->pVtxBuff,
				NULL);

			/*** 頂点バッファの設定 ***/
			pField->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			if (SUCCEEDED(hr))
			{
				/*** 頂点座標の設定の設定 + テクスチャ座標の設定 ***/
				SetIndexedVertex(pVtx, pField->pos, nXBlock, nZBlock, fWidth, fDepth);
			}

			/*** 頂点バッファの設定を終了 ***/
			pField->pVtxBuff->Unlock();

			/*** インデックスバッファの生成 ***/
			hr = pDevice->CreateIndexBuffer(sizeof(WORD) * INDEX_BUFFER(VERTEX_BUFFER(nXBlock, nZBlock), nXBlock, nZBlock),		// sizeof(WORD) * 必要なインデックス数
				D3DUSAGE_WRITEONLY,
				D3DFMT_INDEX16,
				D3DPOOL_MANAGED,
				&pField->pIdxBuff,
				NULL);

			/*** インデックスバッファをロックし、頂点番号データを取得 ***/
			pField->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			if (SUCCEEDED(hr))
			{
				/*** 頂点番号データの設定 ***/
				SetIndex(pIdx, nXBlock, nZBlock, 0, &pField->nIdx[0]);
			}

			/*** インデックスバッファをアンロック ***/
			pField->pIdxBuff->Unlock();

			break;
		}
	}
}

//================================================================================================================
// --- メッシュフィールドの当たり判定処理 ---
//================================================================================================================
bool CollisionMeshField(D3DXVECTOR3* pPos, D3DXVECTOR3* pPosOld, D3DXVECTOR3* pMove)
{
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ
	D3DXVECTOR3 pos[3];							// 境界線ベクトルの始点と終点
	D3DXVECTOR3 vecLine[3] = {};	// 境界線ベクトル
	D3DXVECTOR3 vecToPos[3] = {};	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecNor = {};
	D3DXVECTOR3 vecGai[2] = {};
	D3DXVECTOR3 vecMeshLine = {};	// 面のベクトル
	D3DXVECTOR3 vecToPosTriangle = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	bool bInside[3];
	bool bLand = false;
	float fVecPos[3] = {};
	float fVecPosTriangle = 0.0f;
	PMESHFIELD pField = &g_aMField[0];

	if (pPos == NULL || pPosOld == NULL || pMove == NULL)
	{
		return false;
	}

	int n = (sizeof g_aMField / sizeof(MeshField));

	for (int nCntMesh = 0; nCntMesh < MAX_MESH; nCntMesh++, pField++)
	{
		if (pField->bUse == true)
		{ // 使用状況チェック
			/*** 頂点バッファの読み込み ***/
			pField->pVtxBuff->Lock(0, 0, (void**)&pVtx, D3DLOCK_READONLY);

			for (int nCntPrimitive = 0; nCntPrimitive < DRAWPRIM_NUM(pField->nXBlock, pField->nZBlock); nCntPrimitive++)
			{ // プリミティブ描画数分、当たり判定を確認
				if (pField->nIdx[nCntPrimitive] == pField->nIdx[nCntPrimitive + 1]
					|| pField->nIdx[nCntPrimitive + 1] == pField->nIdx[nCntPrimitive + 2])
				{ // インデックスが重なっている三角ポリゴンは無視
					continue;
				}

				/*** 各三角形の頂点を取得 ***/
				pos[0] = pVtx[pField->nIdx[nCntPrimitive]].pos;
				pos[1] = pVtx[pField->nIdx[nCntPrimitive + 1]].pos;
				pos[2] = pVtx[pField->nIdx[nCntPrimitive + 2]].pos;

				D3DXVec3TransformCoord(&pos[0], &pos[0], &pField->mtxWorld);
				D3DXVec3TransformCoord(&pos[1], &pos[1], &pField->mtxWorld);
				D3DXVec3TransformCoord(&pos[2], &pos[2], &pField->mtxWorld);

				/*** 境界線ベクトルを計算 ***/
				vecLine[0] = pos[1] - pos[0];
				vecLine[1] = pos[2] - pos[1];
				vecLine[2] = pos[0] - pos[2];

				/*** 現在位置との関係を外積を使い求める ***/
				vecToPos[0] = *pPos - pos[0];
				vecToPos[1] = *pPos - pos[1];
				vecToPos[2] = *pPos - pos[2];

				fVecPos[0] = (vecLine[0].z * vecToPos[0].x) - (vecLine[0].x * vecToPos[0].z);	// This is 外積
				fVecPos[1] = (vecLine[1].z * vecToPos[1].x) - (vecLine[1].x * vecToPos[1].z);	// This is 外積
				fVecPos[2] = (vecLine[2].z * vecToPos[2].x) - (vecLine[2].x * vecToPos[2].z);	// This is 外積

				/*** 三角形の番号によって外積判定が変わるため、場合分け ***/
				if ((fVecPos[0] > 0 && fVecPos[1] > 0 && fVecPos[2] > 0) 
					&& (nCntPrimitive % 2) == 0)
				{ // もし偶数番目の三角形なら、境界線ベクトルの内側にいれば
					/** その三角形の内側にいる判定 **/
					/*** 外積用ベクトルを用意 ***/
					vecGai[0] = pos[1] - pos[0];
					vecGai[1] = pos[2] - pos[0];

					/** ベクトルを用いて３次元ベクトルの外積を求める **/
					vecNor.x = (vecGai[0].y * vecGai[1].z) - (vecGai[0].z * vecGai[1].y);
					vecNor.y = (vecGai[0].z * vecGai[1].x) - (vecGai[0].x * vecGai[1].z);
					vecNor.z = (vecGai[0].x * vecGai[1].y) - (vecGai[0].y * vecGai[1].x);

					/** 法線ベクトルを正規化 **/
					D3DXVec3Normalize(&vecNor, &vecNor);

					vecMeshLine = (pos[1] + vecLine[1] * 0.5f) - pos[0];

					vecToPosTriangle = *pPos - pos[0];

					fVecPosTriangle = (vecMeshLine.z * vecToPosTriangle.x) - (vecMeshLine.x * vecToPosTriangle.z);

					float fPosY = pos[0].y - ((1.0f / vecNor.y) * ((vecNor.x * (pPos->x - pos[0].x)) + (vecNor.z * (pPos->z - pos[0].z))));

					fPosY = fabsf(fPosY);

					if (fPosY >= pPos->y)
					{
						pPos->y = fPosY;
						//pMove->y = 0.0f;
						bLand = true;
						PrintDebugProc("Landing...\n");
					}
				}

				if ((fVecPos[0] < 0 && fVecPos[1] < 0 && fVecPos[2] < 0)
					&& (nCntPrimitive % 2) == 1)
				{ // もし奇数個の三角形なら、境界線ベクトルの外側にいれば
					/** その三角形の内側にいる判定 **/
					vecGai[0] = pos[1] - pos[0];
					vecGai[1] = pos[2] - pos[0];

					vecNor.x = (vecGai[0].y * vecGai[1].z) - (vecGai[0].z * vecGai[1].y);
					vecNor.y = (vecGai[0].z * vecGai[1].x) - (vecGai[0].x * vecGai[1].z);
					vecNor.z = (vecGai[0].x * vecGai[1].y) - (vecGai[0].y * vecGai[1].x);

					vecNor *= -1;

					D3DXVec3Normalize(&vecNor, &vecNor);

					float fPosY = pos[0].y - ((1.0f / vecNor.y) * ((vecNor.x * (pPos->x - pos[0].x)) + (vecNor.z * (pPos->z - pos[0].z))));

					if (fPosY >= pPos->y)
					{
						pPos->y = fPosY;
						//pMove->y = 0.0f;
						bLand = true;
						PrintDebugProc("Landing...\n");
					}
				}
			}

			/*** 頂点バッファの設定を終了 ***/
			pField->pVtxBuff->Unlock();
		}
	}

	return bLand;
}
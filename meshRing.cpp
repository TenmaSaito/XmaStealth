//==================================================================================
//
// メッシュリング(広がる輪の演出)の処理 [meshRing.cpp]
// Author : TENMA SAITO
//
// 内周と外周の2段の頂点で作った平たい輪。寿命に合わせて広がりながら透明になる。
// 頂点・インデックスバッファは Init で全リング分を作っておき、使い回す。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "meshRing.h"
#include "effect.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MESH			(2048)		// メッシュリングの最大数
#define MESHRING_XNUM		(16)		// 円周の分割数
#define MESHRING_YNUM		(1)			// 縦の分割数 (内周と外周の1段)

//**********************************************************************************
// *** メッシュリング構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置(中心)
	D3DXVECTOR3 rot;					// 角度
	D3DXCOLOR col;						// 色
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fRadiusInner;					// 内周の直径
	float fRadiusOuter;					// 外周の直径
	float fSpdInner;					// 内周の拡大速度
	float fSpdOuter;					// 外周の拡大速度
	int nIdxTexture;					// テクスチャ番号
	int nLife;							// 残り寿命
	int nMaxLife;						// 最大寿命
	bool bUse;							// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
} MeshRing, *LPMESHRING;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void SetVertexMeshRing(LPMESHRING pRing, D3DXCOLOR col);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static MeshRing g_aRing[MAX_MESH];		// メッシュリングの情報
static int g_nIdxTextureRing;			// リングのテクスチャ番号(全リング共通)

//==================================================================================
// --- メッシュリングの初期化処理 ---
//==================================================================================
void InitMeshRing(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	g_nIdxTextureRing = 0;

	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++)
	{
		LPMESHRING pRing = &g_aRing[nCntRing];		// 初期化するリング

		pRing->pos = VECNULL;
		pRing->rot = VECNULL;
		pRing->col = D3DXCOLOR_NULL;
		pRing->fRadiusInner = 0.0f;
		pRing->fRadiusOuter = 0.0f;
		pRing->fSpdInner = 0.0f;
		pRing->fSpdOuter = 0.0f;
		pRing->nIdxTexture = -1;
		pRing->nLife = 0;
		pRing->nMaxLife = 0;
		pRing->bUse = false;

		/*** 頂点バッファの生成 ***/
		if (SUCCEEDED(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * MESH_VTX_NUM(MESHRING_XNUM, MESHRING_YNUM),
			D3DUSAGE_WRITEONLY,
			FVF_VERTEX_3D,
			D3DPOOL_MANAGED,
			&pRing->pVtxBuff,
			NULL)))
		{
			SetVertexMeshRing(pRing, D3DXCOLOR_NULL);
		}

		/*** インデックスバッファの生成 ***/
		if (SUCCEEDED(pDevice->CreateIndexBuffer(sizeof(WORD) * MESH_IDX_NUM(MESHRING_XNUM, MESHRING_YNUM),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&pRing->pIdxBuff,
			NULL)))
		{
			WORD *pIdx = NULL;		// インデックス情報へのポインタ
			pRing->pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

			SetIndex(pIdx, MESHRING_XNUM, MESHRING_YNUM);

			pRing->pIdxBuff->Unlock();
		}
	}
}

//==================================================================================
// --- メッシュリングの終了処理 ---
//==================================================================================
void UninitMeshRing(void)
{
	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++)
	{
		/*** 頂点バッファ・インデックスバッファの破棄 ***/
		RELEASE(g_aRing[nCntRing].pVtxBuff);
		RELEASE(g_aRing[nCntRing].pIdxBuff);
	}
}

//==================================================================================
// --- メッシュリングの更新処理 (広げながら透明にしていく) ---
//==================================================================================
void UpdateMeshRing(void)
{
	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++)
	{
		LPMESHRING pRing = &g_aRing[nCntRing];		// 更新するリング

		if (pRing->bUse == false)
		{ // 未使用
			continue;
		}

		/*** 内周・外周を広げ、残り寿命に応じて透明にする ***/
		pRing->fRadiusInner += pRing->fSpdInner;
		pRing->fRadiusOuter += pRing->fSpdOuter;
		pRing->col.a = (float)pRing->nLife / (float)pRing->nMaxLife;

		SetVertexMeshRing(pRing, pRing->col);

		pRing->nLife--;
		if (pRing->nLife <= 0)
		{ // 寿命が尽きたら消える
			pRing->bUse = false;
		}
	}
}

//==================================================================================
// --- メッシュリングの描画処理 (加算合成) ---
//==================================================================================
void DrawMeshRing(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	pDevice->LightEnable(0, false);

	/*** Zバッファへの書き込みを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	/*** アルファブレンディングを加算合成に設定 ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++)
	{
		LPMESHRING pRing = &g_aRing[nCntRing];		// 描画するリング

		if (pRing->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pRing->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pRing->rot.y, pRing->rot.x, pRing->rot.z);
		D3DXMatrixMultiply(&pRing->mtxWorld, &pRing->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pRing->pos.x, pRing->pos.y, pRing->pos.z);
		D3DXMatrixMultiply(&pRing->mtxWorld, &pRing->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pRing->mtxWorld);

		/*** 頂点バッファ・インデックスバッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, pRing->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetIndices(pRing->pIdxBuff);
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pRing->nIdxTexture));

		/*** インデックスを利用したポリゴンの描画 ***/
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
			0,
			0,
			MESH_VTX_NUM(MESHRING_XNUM, MESHRING_YNUM),		// 頂点数
			0,
			MESH_PRIM_NUM(MESHRING_XNUM, MESHRING_YNUM));	// 描画する三角形の数
	}

	/*** アルファブレンディングを通常に戻す (重要！) ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zバッファの設定を戻す ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	pDevice->LightEnable(0, true);
}

//==================================================================================
// --- メッシュリングの発生処理 ---
// ※ nIdxTexture は現在未使用 (SetIndexTextureRing で設定した番号を使う)
//==================================================================================
void SetMeshRing(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fRadiusInner, float fRadiusOuter, int nIdxTexture, float fSpdInner, float fSpdOuter, int nLife, D3DXCOLOR col)
{
	for (int nCntRing = 0; nCntRing < MAX_MESH; nCntRing++)
	{
		LPMESHRING pRing = &g_aRing[nCntRing];		// 発生させるリング

		if (pRing->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pRing->pos = pos;
		pRing->rot = rot;
		pRing->col = col;
		pRing->fRadiusInner = fRadiusInner;
		pRing->fRadiusOuter = fRadiusOuter;
		pRing->nIdxTexture = g_nIdxTextureRing;
		pRing->fSpdInner = fSpdInner;
		pRing->fSpdOuter = fSpdOuter;
		pRing->nLife = nLife;
		pRing->nMaxLife = nLife;
		pRing->bUse = true;

		SetVertexMeshRing(pRing, col);

		return;
	}
}

//==================================================================================
// --- リングのテクスチャ番号の設定 ---
//==================================================================================
void SetIndexTextureRing(int nIdxTexture)
{
	g_nIdxTextureRing = nIdxTexture;
}

//==================================================================================
// --- リングのテクスチャ番号の取得 ---
//==================================================================================
int GetIndexTextureRing(void)
{
	return g_nIdxTextureRing;
}

//==================================================================================
// --- 現在の内周・外周の大きさで頂点情報を設定 ---
//==================================================================================
static void SetVertexMeshRing(LPMESHRING pRing, D3DXCOLOR col)
{
	VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ

	pRing->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntY = 0; nCntY <= MESHRING_YNUM; nCntY++)
	{ // 0段目 : 外周 / 1段目 : 内周
		float fRadius = (nCntY == 0) ? pRing->fRadiusOuter : pRing->fRadiusInner;		// この段の直径

		for (int nCntX = 0; nCntX <= MESHRING_XNUM; nCntX++)
		{
			float fAngle = (D3DX_2PI / (float)MESHRING_XNUM) * nCntX;		// 円周上の角度
			int nIdxVtx = nCntX + (nCntY * (MESHRING_XNUM + 1));			// 頂点番号

			D3DXVECTOR3 posVtx = D3DXVECTOR3(sinf(fAngle) * (fRadius * 0.5f), 0.0f, cosf(fAngle) * (fRadius * 0.5f));	// 頂点座標
			D3DXVECTOR3 nor = D3DXVECTOR3(posVtx.x - pRing->pos.x, 0.0f, posVtx.z - pRing->pos.z);					// 法線
			D3DXVec3Normalize(&nor, &nor);

			pVtx[nIdxVtx].pos = posVtx;
			pVtx[nIdxVtx].nor = nor;
			pVtx[nIdxVtx].col = col;
			pVtx[nIdxVtx].tex.x = (1.0f / (float)MESHRING_XNUM) * nCntX;
			pVtx[nIdxVtx].tex.y = (float)nCntY;
		}
	}

	pRing->pVtxBuff->Unlock();
}

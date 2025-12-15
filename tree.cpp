//================================================================================================================
//
// DirectXのビルボード木表示処理 [tree.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "tree.h"
#include "shadow.h"
#include "mathUtil.h"
#include "loadxfile.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define TREE_SIZE_X		(10)		// ポリゴンの基本サイズ - X
#define TREE_SIZE_Z		(10)		// ポリゴンの基本サイズ - Y
#define TREE_SPD			(2.0f)		// ポリゴンの移動スピード
#define TREE_WDSPD		(0.1f)		// ポリゴンの拡縮スピード
#define MAX_TREE			(300)		// 弾丸の最大出現数
#define TREE_LIFE			(100)		// 弾丸の体力

//*************************************************************************************************
//*** 弾丸構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	float fWidth;			// 幅
	float fHeight;			// 高さ
	int nIdShadow;			// 影
	int nIndexTexture;		// テクスチャインデックス
	bool bZFunc;			// Zテストの有無
	bool bUse;				// 使っているか
}Tree;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureTree = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffTree = NULL;	// 頂点バッファのポインタ
Tree g_aTree[MAX_TREE];						// 弾丸の情報
D3DXVECTOR3 g_posTree;							// ポリゴンの位置
D3DXMATRIX g_mtxWorldTree;						// ワールドマトリックス

//================================================================================================================
// --- ビルボード木初期化処理 ---
//================================================================================================================
void InitTree(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		g_aTree[nCntTree].pos = D3DXVECTOR3_NULL;
		g_aTree[nCntTree].move = D3DXVECTOR3_NULL;
		g_aTree[nCntTree].fWidth = 10.0f;
		g_aTree[nCntTree].fHeight = 10.0f;
		g_aTree[nCntTree].nIdShadow = -1;
		g_aTree[nCntTree].nIndexTexture = 0;
		g_aTree[nCntTree].bZFunc = false;
		g_aTree[nCntTree].bUse = false;
	}

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
							  "data\\TEXTURE\\Tree000.png",
							  &g_pTextureTree);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_TREE,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffTree,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffTree->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	for (int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		pVtx[0].pos.x = -g_aTree[nCntTree].fWidth;
		pVtx[0].pos.y = g_aTree[nCntTree].fHeight;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aTree[nCntTree].fWidth;
		pVtx[1].pos.y = g_aTree[nCntTree].fHeight;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = -g_aTree[nCntTree].fWidth;
		pVtx[2].pos.y = -g_aTree[nCntTree].fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aTree[nCntTree].fWidth;
		pVtx[3].pos.y = -g_aTree[nCntTree].fHeight;
		pVtx[3].pos.z = 0.0f;

		/*** 法線ベクトルの設定 ***/
		pVtx[0].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVtx[1].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVtx[2].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVtx[3].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		/*** 頂点カラー設定 ***/
		pVtx[0].col = D3DXCOLOR_NULL;
		pVtx[1].col = D3DXCOLOR_NULL;
		pVtx[2].col = D3DXCOLOR_NULL;
		pVtx[3].col = D3DXCOLOR_NULL;

		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffTree->Unlock();
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitTree(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureTree != NULL)
	{
		g_pTextureTree->Release();
		g_pTextureTree = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffTree != NULL)
	{
		g_pVtxBuffTree->Release();
		g_pVtxBuffTree = NULL;
	}
}

//================================================================================================================
// --- ビルボード木更新処理 ---
//================================================================================================================
void UpdateTree(void)
{
	
}

//================================================================================================================
// --- ビルボード木描画処理 ---
//================================================================================================================
void DrawTree(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用

	for (int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		if (g_aTree[nCntTree].bUse == true)
		{
			if (g_aTree[nCntTree].bZFunc == true)
			{
				/*** Zテストを無効にする ***/
				pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			}

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aTree[nCntTree].mtxWorld);

			/*** カメラのビューマトリックスを取得 ***/
			pDevice->GetTransform(D3DTS_VIEW, &mtxView);

			/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixInverse(&g_aTree[nCntTree].mtxWorld, NULL, &mtxView);
			/** 逆行列によって入ってしまった位置情報を初期化 **/
			g_aTree[nCntTree].mtxWorld._41 = 0.0f;
			g_aTree[nCntTree].mtxWorld._42 = 0.0f;
			g_aTree[nCntTree].mtxWorld._43 = 0.0f;

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aTree[nCntTree].pos.x,
				g_aTree[nCntTree].pos.y,
				g_aTree[nCntTree].pos.z);

			D3DXMatrixMultiply(&g_aTree[nCntTree].mtxWorld, &g_aTree[nCntTree].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aTree[nCntTree].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffTree, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_aTree[nCntTree].nIndexTexture));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntTree,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数

			if (g_aTree[nCntTree].bZFunc == true)
			{
				/*** Zテストを無効にする ***/
				pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			}
		}
	}
}

//================================================================================================================
// --- ビルボード木発射処理 ---
//================================================================================================================
void SetTree(D3DXVECTOR3 pos, float fWidth, float fHeight, int nIndexTexture, bool bZFunc)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffTree->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntTree = 0; nCntTree < MAX_TREE; nCntTree++)
	{
		if (g_aTree[nCntTree].bUse != true)
		{
			g_aTree[nCntTree].pos = pos;
			g_aTree[nCntTree].fWidth = fWidth;
			g_aTree[nCntTree].fHeight = fHeight;
			g_aTree[nCntTree].nIndexTexture = nIndexTexture;
			g_aTree[nCntTree].bZFunc = bZFunc;

			pVtx[0].pos.x = -g_aTree[nCntTree].fWidth * 0.5f;
			pVtx[0].pos.y = g_aTree[nCntTree].fHeight;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = g_aTree[nCntTree].fWidth * 0.5f;
			pVtx[1].pos.y = g_aTree[nCntTree].fHeight;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -g_aTree[nCntTree].fWidth * 0.5f;
			pVtx[2].pos.y = 0.0f;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = g_aTree[nCntTree].fWidth * 0.5f;
			pVtx[3].pos.y = 0.0f;
			pVtx[3].pos.z = 0.0f;

			g_aTree[nCntTree].bUse = true;

			g_aTree[nCntTree].nIdShadow = SetShadow(TREE_SIZE_X, TREE_SIZE_Z);

			break;
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffTree->Unlock();
}
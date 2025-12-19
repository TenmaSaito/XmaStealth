//================================================================================================================
//
// DirectXの影表示処理 [shadow.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "shadow.h"
#include "mathUtil.h"
#include "loadxfile.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define SHADOW_SIZE_X		(10)		// ポリゴンの基本サイズ - X
#define SHADOW_SIZE_Z		(10)		// ポリゴンの基本サイズ - Y
#define SHADOW_SPD			(2.0f)		// ポリゴンの移動スピード
#define SHADOW_ROTSPD		(0.1f)		// ポリゴンの回転スピード
#define SHADOW_WDSPD		(0.1f)		// ポリゴンの拡縮スピード
#define MAX_SHADOW			(300)		// 影の最大数

//*************************************************************************************************
//*** 影の構造体定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 影の位置
	D3DXVECTOR3 rot;					// 影の向き
	D3DXCOLOR col;						// 影の濃さ
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fWidth;						// 幅
	float fDepth;						// 奥行
	float fWidthSet;					// 幅(保存用)
	float fDepthSet;					// 奥行(保存用)
	bool bUse;
}Shadow;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffShadow = NULL;	// 頂点バッファのポインタ
Shadow g_aShadow[MAX_SHADOW];						// 影の情報
int g_nIndexTextureShadow;							// 影のテクスチャインデックス

//================================================================================================================
// --- 影の初期化処理 ---
//================================================================================================================
void InitShadow(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		g_aShadow[nCntShadow].pos = D3DXVECTOR3_NULL;
		g_aShadow[nCntShadow].rot = D3DXVECTOR3_NULL;
		g_aShadow[nCntShadow].fWidth = SHADOW_SIZE_X;
		g_aShadow[nCntShadow].fDepth = SHADOW_SIZE_Z;
		g_aShadow[nCntShadow].fWidthSet = SHADOW_SIZE_X;
		g_aShadow[nCntShadow].fDepthSet = SHADOW_SIZE_Z;
		g_aShadow[nCntShadow].col = D3DXCOLOR_NULL;
		g_aShadow[nCntShadow].bUse = false;
	}

	g_nIndexTextureShadow = 0;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_SHADOW,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffShadow,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aShadow[nCntShadow].pos.x - g_aShadow[nCntShadow].fWidth;
		pVtx[0].pos.y = 0.15f;
		pVtx[0].pos.z = g_aShadow[nCntShadow].pos.z + g_aShadow[nCntShadow].fDepth;

		pVtx[1].pos.x = g_aShadow[nCntShadow].pos.x + g_aShadow[nCntShadow].fWidth;
		pVtx[1].pos.y = 0.15f;
		pVtx[1].pos.z = g_aShadow[nCntShadow].pos.z + g_aShadow[nCntShadow].fDepth;

		pVtx[2].pos.x = g_aShadow[nCntShadow].pos.x - g_aShadow[nCntShadow].fWidth;
		pVtx[2].pos.y = 0.15f;
		pVtx[2].pos.z = g_aShadow[nCntShadow].pos.z - g_aShadow[nCntShadow].fDepth;

		pVtx[3].pos.x = g_aShadow[nCntShadow].pos.x + g_aShadow[nCntShadow].fWidth;
		pVtx[3].pos.y = 0.15f;
		pVtx[3].pos.z = g_aShadow[nCntShadow].pos.z - g_aShadow[nCntShadow].fDepth;

		/*** 法線ベクトルの設定 ***/
		pVtx[0].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[1].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[2].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[3].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

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
	g_pVtxBuffShadow->Unlock();
}

//================================================================================================================
// --- 影の終了処理 ---
//================================================================================================================
void UninitShadow(void)
{
	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffShadow != NULL)
	{
		g_pVtxBuffShadow->Release();
		g_pVtxBuffShadow = NULL;
	}
}

//================================================================================================================
// --- 影の更新処理 ---
//================================================================================================================
void UpdateShadow(void)
{

}

//================================================================================================================
// --- 影の描画処理 ---
//================================================================================================================
void DrawShadow(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	/*** アルファブレンディングを減算合成に設定！ ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		if (g_aShadow[nCntShadow].bUse)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aShadow[nCntShadow].mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				g_aShadow[nCntShadow].rot.y,		// Y軸回転
				g_aShadow[nCntShadow].rot.x,		// X軸回転
				g_aShadow[nCntShadow].rot.z);		// Z軸回転

			D3DXMatrixMultiply(&g_aShadow[nCntShadow].mtxWorld, &g_aShadow[nCntShadow].mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aShadow[nCntShadow].pos.x,
				g_aShadow[nCntShadow].pos.y,
				g_aShadow[nCntShadow].pos.z);

			D3DXMatrixMultiply(&g_aShadow[nCntShadow].mtxWorld, &g_aShadow[nCntShadow].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aShadow[nCntShadow].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffShadow, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_nIndexTextureShadow));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntShadow,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}

	/*** アルファブレンディングを元に戻す！！(重要！) ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//================================================================================================================
// --- 影の設定処理 ---
//================================================================================================================
int SetShadow(float fWidth, float fDepth)
{
	int nErrorShadow = -1;
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		if (g_aShadow[nCntShadow].bUse != true)
		{
			g_aShadow[nCntShadow].fWidth = fWidth;
			g_aShadow[nCntShadow].fWidthSet = fWidth;
			g_aShadow[nCntShadow].fDepth = fDepth;
			g_aShadow[nCntShadow].fDepthSet = fDepth;
			g_aShadow[nCntShadow].col.a = 1.0f;

			/*** 頂点バッファの設定 ***/
			g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);

			pVtx += 4 * nCntShadow;

			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = - g_aShadow[nCntShadow].fWidth;
			pVtx[0].pos.y = 0.1f;
			pVtx[0].pos.z = g_aShadow[nCntShadow].fDepth;

			pVtx[1].pos.x = g_aShadow[nCntShadow].fWidth;
			pVtx[1].pos.y = 0.1f;
			pVtx[1].pos.z = g_aShadow[nCntShadow].fDepth;

			pVtx[2].pos.x =  - g_aShadow[nCntShadow].fWidth;
			pVtx[2].pos.y = 0.1f;
			pVtx[2].pos.z =  - g_aShadow[nCntShadow].fDepth;

			pVtx[3].pos.x = g_aShadow[nCntShadow].fWidth;
			pVtx[3].pos.y = 0.1f;
			pVtx[3].pos.z = - g_aShadow[nCntShadow].fDepth;

			/*** 頂点カラー設定 ***/
			pVtx[0].col = g_aShadow[nCntShadow].col;
			pVtx[1].col = g_aShadow[nCntShadow].col;
			pVtx[2].col = g_aShadow[nCntShadow].col;
			pVtx[3].col = g_aShadow[nCntShadow].col;

			g_aShadow[nCntShadow].bUse = true;

			/*** 頂点バッファの設定を終了***/
			g_pVtxBuffShadow->Unlock();

			return nCntShadow;
		}
	}

	return nErrorShadow;
}

//================================================================================================================
// --- 影のテクスチャ設定処理 ---
//================================================================================================================
void SetIndexTextureShadow(int nIndexTexture)
{
	g_nIndexTextureShadow = nIndexTexture;
}

//================================================================================================================
// --- 影の位置設定処理 ---
//================================================================================================================
void SetPositionShadow(int nIdShadow, D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	if (nIdShadow == -1)
	{
		return;
	}

	/*** 影の位置を更新！ ***/
	g_aShadow[nIdShadow].pos.x = pos.x;
	g_aShadow[nIdShadow].pos.z = pos.z;

	g_aShadow[nIdShadow].rot.y = rot.y;

	/*** プレイヤーの高度によって、影のサイズを調整！ ***/
	g_aShadow[nIdShadow].fWidth = ((pos.y * 0.1f) + g_aShadow[nIdShadow].fWidthSet);
	g_aShadow[nIdShadow].fDepth = ((pos.y * 0.1f) + g_aShadow[nIdShadow].fDepthSet);

	/*** プレイヤーの高度によって、影の濃度を調整！ ***/
	g_aShadow[nIdShadow].col.a = 1.0f - (pos.y * 0.001f);

	/*** 影の濃度の上下限を設定！ ***/
	if (g_aShadow[nIdShadow].col.a > 0.001f)
	{
		if (g_aShadow[nIdShadow].col.a <= 0.001f)
		{
			g_aShadow[nIdShadow].col.a = 0.001f;
		}
	}
	else if (g_aShadow[nIdShadow].col.a < 1.0f)
	{
		if (g_aShadow[nIdShadow].col.a >= 1.0f)
		{
			g_aShadow[nIdShadow].col.a = 1.0f;
		}
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);

	pVtx += 4 * nIdShadow;

	/*** 頂点座標の設定 (※ 位置を使うと影がずれる...使用は控えて！ <- 位置はマトリックスによって既に移動している為) ***/
	pVtx[0].pos.x = -g_aShadow[nIdShadow].fWidth;
	pVtx[0].pos.y = 0.1f;
	pVtx[0].pos.z = g_aShadow[nIdShadow].fDepth;

	pVtx[1].pos.x = g_aShadow[nIdShadow].fWidth;
	pVtx[1].pos.y = 0.1f;
	pVtx[1].pos.z = g_aShadow[nIdShadow].fDepth;

	pVtx[2].pos.x = -g_aShadow[nIdShadow].fWidth;
	pVtx[2].pos.y = 0.1f;
	pVtx[2].pos.z = -g_aShadow[nIdShadow].fDepth;

	pVtx[3].pos.x = g_aShadow[nIdShadow].fWidth;
	pVtx[3].pos.y = 0.1f;
	pVtx[3].pos.z = -g_aShadow[nIdShadow].fDepth;

	/*** 頂点カラー設定 ***/
	pVtx[0].col = g_aShadow[nIdShadow].col;
	pVtx[1].col = g_aShadow[nIdShadow].col;
	pVtx[2].col = g_aShadow[nIdShadow].col;
	pVtx[3].col = g_aShadow[nIdShadow].col;

	/*** 頂点バッファの設定を終了***/
	g_pVtxBuffShadow->Unlock();
}

//================================================================================================================
// --- 影の解放処理 ---
//================================================================================================================
void DestroyShadow(int nIdShadow)
{
	if (g_aShadow[nIdShadow].bUse == true)
	{
		g_aShadow[nIdShadow].bUse = false;
	}
}

//================================================================================================================
// --- 影のサイズ変更処理 ---
//================================================================================================================
float AddShadowSize(int nIdShadow, SHADOW_SIZE type, float fValue)
{
	float fSize = 0.0f;

	if (nIdShadow == -1)
	{
		return fSize;
	}
	
	if (FAILED(CheckIndex(SHADOW_SIZE_MAX, type)))
	{
		return fSize;
	}

	/*** 影のサイズ変更***/
	switch (type)
	{
	case SHADOW_SIZE_WIDTH:

		fSize = g_aShadow[nIdShadow].fWidth;
		g_aShadow[nIdShadow].fWidth += fValue;
		fSize += fValue;

		break;

	case SHADOW_SIZE_DEPTH:

		fSize = g_aShadow[nIdShadow].fDepth;
		g_aShadow[nIdShadow].fDepth += fValue;
		fSize += fValue;

		break;
	}

	return fSize;
}

//================================================================================================================
// --- 影の描画変更処理 ---
//================================================================================================================
void InvisibleShadow(int nIdxShadow, float fAlpha)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	// 存在しないインデックスなら終了
	if (nIdxShadow < 0 || nIdxShadow >= MAX_SHADOW)
	{
		return;
	}

	g_aShadow[nIdxShadow].col.a = fAlpha;	// 影の透明度を指定された値に設定

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点バッファをインデックス分ずらす
	pVtx += 4 * nIdxShadow;

	/*** 頂点カラー設定 ***/
	pVtx[0].col = g_aShadow[nIdxShadow].col;
	pVtx[1].col = g_aShadow[nIdxShadow].col;
	pVtx[2].col = g_aShadow[nIdxShadow].col;
	pVtx[3].col = g_aShadow[nIdxShadow].col;

	/*** 頂点バッファの設定を終了***/
	g_pVtxBuffShadow->Unlock();
}
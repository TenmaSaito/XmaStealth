//================================================================================================================
//
// DirectXのビルボードエフェクト表示処理 [effect.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "effect.h"
#include "loadxfile.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define EFFECT_SIZE_X		(3)			// エフェクトの基本サイズ - X
#define EFFECT_SIZE_Z		(3)			// エフェクトの基本サイズ - Y
#define EFFECT_SPD			(2.0f)		// エフェクトの移動スピード
#define EFFECT_WDSPD		(0.1f)		// エフェクトの拡縮スピード
#define MAX_EFFECT			(4096)		// エフェクトの最大出現数
#define EFFECT_LIFE			(10)		// エフェクトの体力

//*************************************************************************************************
//*** 弾丸構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	D3DXCOLOR col;			// 色
	int nLife;				// 体力
	int nDrawNum;			// 描画順番
	float fWidth;			// 幅
	float fHeight;			// 高さ
	bool bUse;				// 使っているか
}Effect;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffEffect = NULL;	// 頂点バッファのポインタ
Effect g_aEffect[MAX_EFFECT];						// 弾丸の情報
D3DXVECTOR3 g_posEffect;							// ポリゴンの位置
D3DXMATRIX g_mtxWorldEffect;						// ワールドマトリックス
int g_nIndexTextureEffect;							// テクスチャインデックス

//================================================================================================================
// --- エフェクトの初期化処理 ---
//================================================================================================================
void InitEffect(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		g_aEffect[nCntEffect].pos = D3DXVECTOR3_NULL;
		g_aEffect[nCntEffect].move = D3DXVECTOR3_NULL;
		g_aEffect[nCntEffect].nLife = 0;
		g_aEffect[nCntEffect].fWidth = 0.0f;
		g_aEffect[nCntEffect].fHeight = 0.0f;
		g_aEffect[nCntEffect].bUse = false;
	}

	g_nIndexTextureEffect = 0;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_EFFECT,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffEffect,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffEffect->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		pVtx[0].pos.x = -g_aEffect[nCntEffect].fWidth;
		pVtx[0].pos.y = g_aEffect[nCntEffect].fHeight;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aEffect[nCntEffect].fWidth;
		pVtx[1].pos.y = g_aEffect[nCntEffect].fHeight;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = -g_aEffect[nCntEffect].fWidth;
		pVtx[2].pos.y = -g_aEffect[nCntEffect].fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aEffect[nCntEffect].fWidth;
		pVtx[3].pos.y = -g_aEffect[nCntEffect].fHeight;
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
	g_pVtxBuffEffect->Unlock();
}

//================================================================================================================
// --- エフェクトの終了処理 ---
//================================================================================================================
void UninitEffect(void)
{
	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffEffect != NULL)
	{
		g_pVtxBuffEffect->Release();
		g_pVtxBuffEffect = NULL;
	}
}

//================================================================================================================
// --- エフェクトの更新処理 ---
//================================================================================================================
void UpdateEffect(void)
{
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		if (g_aEffect[nCntEffect].bUse == true)
		{
			g_aEffect[nCntEffect].pos.x += g_aEffect[nCntEffect].move.x;
			g_aEffect[nCntEffect].pos.y += g_aEffect[nCntEffect].move.y;
			g_aEffect[nCntEffect].pos.z += g_aEffect[nCntEffect].move.z;

			g_aEffect[nCntEffect].nLife--;
			if (g_aEffect[nCntEffect].nLife <= 0)
			{
				g_aEffect[nCntEffect].bUse = false;
			}
		}
	}
}
//================================================================================================================
// --- エフェクトの描画処理 ---
//================================================================================================================
void DrawEffect(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用
	
	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	// アルファブレンディングを加算合成に設定！
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	/*** アルファテストを有効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);		// アルファテストを有効
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);	// 基準値よりも大きい場合にZバッファに書き込み
	pDevice->SetRenderState(D3DRS_ALPHAREF, 254);				// 基準値

	LIGHTING(pDevice, FALSE);

	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		if (g_aEffect[nCntEffect].bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aEffect[nCntEffect].mtxWorld);

			/*** カメラのビューマトリックスを取得 ***/
			pDevice->GetTransform(D3DTS_VIEW, &mtxView);

			/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixInverse(&g_aEffect[nCntEffect].mtxWorld, NULL, &mtxView);
			/** 逆行列によって入ってしまった位置情報を初期化 **/
			g_aEffect[nCntEffect].mtxWorld._41 = 0.0f;
			g_aEffect[nCntEffect].mtxWorld._42 = 0.0f;
			g_aEffect[nCntEffect].mtxWorld._43 = 0.0f;

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aEffect[nCntEffect].pos.x,
				g_aEffect[nCntEffect].pos.y,
				g_aEffect[nCntEffect].pos.z);

			D3DXMatrixMultiply(&g_aEffect[nCntEffect].mtxWorld, &g_aEffect[nCntEffect].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aEffect[nCntEffect].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffEffect, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_nIndexTextureEffect));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntEffect,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}

	/*** アルファテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);		// アルファテストを無効化
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);	// 無条件にZバッファに書き込み
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);					// 基準値

	// アルファブレンディングを元に戻す！！(重要！)
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	LIGHTING(pDevice, TRUE);
}

//================================================================================================================
// --- エフェクトの発生処理 ---
//================================================================================================================
void SetEffect(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd, float fWidth, float fHeight, int nLife)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffEffect->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		if (g_aEffect[nCntEffect].bUse != true)
		{
			D3DXVec3Normalize(&rot, &rot);

			g_aEffect[nCntEffect].pos = pos;
			g_aEffect[nCntEffect].fWidth = fWidth;
			g_aEffect[nCntEffect].fHeight = fHeight;
			g_aEffect[nCntEffect].move.x = rot.x * fSpd;
			g_aEffect[nCntEffect].move.y = rot.y * fSpd;
			g_aEffect[nCntEffect].move.z = rot.z * fSpd;
			g_aEffect[nCntEffect].col = D3DXCOLOR_NULL;
			g_aEffect[nCntEffect].nLife = EFFECT_LIFE;

			if (nLife != -1)
			{
				g_aEffect[nCntEffect].nLife = nLife;
				g_aEffect[nCntEffect].col = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
			}

			g_aEffect[nCntEffect].bUse = true;

			pVtx[0].pos.x = -g_aEffect[nCntEffect].fWidth;
			pVtx[0].pos.y = g_aEffect[nCntEffect].fHeight;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = g_aEffect[nCntEffect].fWidth;
			pVtx[1].pos.y = g_aEffect[nCntEffect].fHeight;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -g_aEffect[nCntEffect].fWidth;
			pVtx[2].pos.y = -g_aEffect[nCntEffect].fHeight;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = g_aEffect[nCntEffect].fWidth;
			pVtx[3].pos.y = -g_aEffect[nCntEffect].fHeight;
			pVtx[3].pos.z = 0.0f;

			pVtx[0].col = g_aEffect[nCntEffect].col;
			pVtx[1].col = g_aEffect[nCntEffect].col;
			pVtx[2].col = g_aEffect[nCntEffect].col;
			pVtx[3].col = g_aEffect[nCntEffect].col;

			break;
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffEffect->Unlock();
}

//================================================================================================================
// --- エフェクトのテクスチャインデックス設定処理 ---
//================================================================================================================
void SetIndexTextureEffect(int nIndexTexture)
{
	g_nIndexTextureEffect = nIndexTexture;
}
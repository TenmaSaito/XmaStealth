//==================================================================================
//
// エフェクト(加算合成のビルボード粒子)の処理 [effect.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "effect.h"
#include "loadxfile.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_EFFECT			(8000)		// エフェクトの最大出現数
#define EFFECT_LIFE			(10)		// 寿命を指定しなかった時の寿命(フレーム)
#define EFFECT_ALPHA_REF	(254)		// アルファテストの基準値
#define EFFECT_LIFE_COLOR	D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)	// 寿命を指定した時の色(緑)

//**********************************************************************************
// *** エフェクト構造体の定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	D3DXCOLOR col;			// 色
	int nLife;				// 寿命(フレーム)
	float fWidth;			// 幅の半分
	float fHeight;			// 高さの半分
	bool bUse;				// 使っているか
} Effect;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffEffect = NULL;		// 頂点バッファのポインタ(全エフェクト分)
static Effect g_aEffect[MAX_EFFECT];						// エフェクトの情報
static int g_nIndexTextureEffect;							// テクスチャ番号

//==================================================================================
// --- エフェクトの初期化処理 ---
//==================================================================================
void InitEffect(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

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

	/*** 頂点情報の初期設定 (大きさ0の板) ***/
	g_pVtxBuffEffect->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++, pVtx += 4)
	{
		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].pos = D3DXVECTOR3_NULL;
			pVtx[nCntVtx].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);		// 法線(手前向き)
			pVtx[nCntVtx].col = D3DXCOLOR_NULL;
		}

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
	}

	g_pVtxBuffEffect->Unlock();
}

//==================================================================================
// --- エフェクトの終了処理 ---
//==================================================================================
void UninitEffect(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffEffect);
}

//==================================================================================
// --- エフェクトの更新処理 (移動と寿命) ---
//==================================================================================
void UpdateEffect(void)
{
	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		Effect *pEffect = &g_aEffect[nCntEffect];		// 更新するエフェクト

		if (pEffect->bUse == false)
		{ // 未使用
			continue;
		}

		pEffect->pos += pEffect->move;

		pEffect->nLife--;
		if (pEffect->nLife <= 0)
		{ // 寿命が尽きたら消える
			pEffect->bUse = false;
		}
	}
}

//==================================================================================
// --- エフェクトの描画処理 ---
//==================================================================================
void DrawEffect(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxTrans;							// 平行移動用マトリックス
	D3DXMATRIX mtxView;								// カメラのビューマトリックス

	/*** Zバッファへの書き込みを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	/*** アルファブレンディングを加算合成に設定 ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	/*** アルファテストを有効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	pDevice->SetRenderState(D3DRS_ALPHAREF, EFFECT_ALPHA_REF);

	LIGHTING(pDevice, FALSE);

	/*** カメラのビューマトリックスを取得 ***/
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	/*** 頂点バッファ・テクスチャの設定 (全エフェクト共通) ***/
	pDevice->SetStreamSource(0, g_pVtxBuffEffect, 0, sizeof(VERTEX_3D));
	pDevice->SetFVF(FVF_VERTEX_3D);
	pDevice->SetTexture(0, GetTexture(g_nIndexTextureEffect));

	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		Effect *pEffect = &g_aEffect[nCntEffect];		// 描画するエフェクト

		if (pEffect->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ビューの逆行列で常にカメラの方を向かせる (平行移動成分は消す) ***/
		D3DXMatrixInverse(&pEffect->mtxWorld, NULL, &mtxView);
		pEffect->mtxWorld._41 = 0.0f;
		pEffect->mtxWorld._42 = 0.0f;
		pEffect->mtxWorld._43 = 0.0f;

		/*** 位置を反映 ***/
		D3DXMatrixTranslation(&mtxTrans, pEffect->pos.x, pEffect->pos.y, pEffect->pos.z);
		D3DXMatrixMultiply(&pEffect->mtxWorld, &pEffect->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pEffect->mtxWorld);

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntEffect, 2);
	}

	/*** アルファテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);

	/*** アルファブレンディングを通常に戻す (重要！) ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zバッファの設定を戻す ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	LIGHTING(pDevice, TRUE);
}

//==================================================================================
// --- エフェクトの発生処理 ---
// nLife を省略(-1)すると白色・寿命 EFFECT_LIFE、指定すると緑色・指定の寿命になる
//==================================================================================
void SetEffect(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd, float fWidth, float fHeight, int nLife)
{
	if (nLife == -1)
	{
		SetEffectEx(pos, rot, D3DXCOLOR_NULL, fSpd, fWidth, fHeight, EFFECT_LIFE);
	}
	else
	{
		SetEffectEx(pos, rot, EFFECT_LIFE_COLOR, fSpd, fWidth, fHeight, nLife);
	}
}

//==================================================================================
// --- エフェクトの発生処理 (色・寿命を指定) ---
// rot : 移動方向 (正規化して使う)
//==================================================================================
void SetEffectEx(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, float fSpd, float fWidth, float fHeight, int nLife)
{
	VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ

	for (int nCntEffect = 0; nCntEffect < MAX_EFFECT; nCntEffect++)
	{
		Effect *pEffect = &g_aEffect[nCntEffect];		// 発生させるエフェクト

		if (pEffect->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		D3DXVec3Normalize(&rot, &rot);

		pEffect->pos = pos;
		pEffect->fWidth = fWidth;
		pEffect->fHeight = fHeight;
		pEffect->move = rot * fSpd;
		pEffect->col = col;
		pEffect->nLife = nLife;
		pEffect->bUse = true;

		/*** このエフェクトの頂点座標・頂点カラーの設定 ***/
		g_pVtxBuffEffect->Lock(0, 0, (void**)&pVtx, 0);
		pVtx += 4 * nCntEffect;

		pVtx[0].pos = D3DXVECTOR3(-fWidth, fHeight, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(fWidth, fHeight, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(-fWidth, -fHeight, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(fWidth, -fHeight, 0.0f);

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].col = col;
		}

		g_pVtxBuffEffect->Unlock();

		return;
	}
}

//==================================================================================
// --- エフェクトのテクスチャ番号の設定 ---
//==================================================================================
void SetIndexTextureEffect(int nIndexTexture)
{
	g_nIndexTextureEffect = nIndexTexture;
}

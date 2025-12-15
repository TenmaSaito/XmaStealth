//================================================================================================================
//
// DirectXのビルボード爆発表示処理 [explosion.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "bullet.h"
#include "effect.h"
#include "shadow.h"
#include "mathUtil.h"
#include "loadxfile.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define EXPLOSION_SIZE_X		(10)		// 爆発の基本サイズ - X
#define EXPLOSION_SIZE_Z		(10)		// 爆発の基本サイズ - Y
#define EXPLOSION_SPD			(2.0f)		// 爆発の移動スピード
#define EXPLOSION_WDSPD			(0.1f)		// 爆発の拡縮スピード
#define MAX_EXPLOSION			(300)		// 爆発の最大出現数
#define MAX_TEXTURE				(8)			// アニメーション数
#define EXPLOSION_COUNT			(3)			// 爆発のアニメーションカウント

//*************************************************************************************************
//*** 爆発構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	float fWidth;			// 幅
	float fHeight;			// 高さ
	int nCounterAnim;		// アニメーションカウンター
	int nPatternAnim;		// アニメーションパターン
	int nSpeeedAnim;		// アニメーションスピード
	int nIdShadow;			// 影
	bool bUse;				// 使っているか
}Explosion;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureExplosion = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffExplosion = NULL;	// 頂点バッファのポインタ
Explosion g_aExplosion[MAX_EXPLOSION];				// 弾丸の情報
D3DXVECTOR3 g_posExplosion;							// ポリゴンの位置
D3DXMATRIX g_mtxWorldExplosion;						// ワールドマトリックス
int g_nIndexTextureExplosion;						// テクスチャインデックス
int g_nTextureAnimCount;							// アニメーション数

//================================================================================================================
// --- 爆発初期化処理 ---
//================================================================================================================
void InitExplosion(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntExplosion = 0; nCntExplosion < MAX_EXPLOSION; nCntExplosion++)
	{
		g_aExplosion[nCntExplosion].pos = D3DXVECTOR3_NULL;
		g_aExplosion[nCntExplosion].move = D3DXVECTOR3_NULL;
		g_aExplosion[nCntExplosion].fWidth = 0.0f;
		g_aExplosion[nCntExplosion].fHeight = 0.0f;
		g_aExplosion[nCntExplosion].nCounterAnim = 0;
		g_aExplosion[nCntExplosion].nPatternAnim = 0;
		g_aExplosion[nCntExplosion].nIdShadow = -1;
		g_aExplosion[nCntExplosion].bUse = false;
	}

	g_nIndexTextureExplosion = 0;
	g_nTextureAnimCount = 1;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
							  "data\\TEXTURE\\explosion000.png",
							  &g_pTextureExplosion);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_EXPLOSION,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffExplosion,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffExplosion->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	for (int nCntExplosion = 0; nCntExplosion < MAX_EXPLOSION; nCntExplosion++)
	{
		pVtx[0].pos.x = -EXPLOSION_SIZE_X;
		pVtx[0].pos.y = EXPLOSION_SIZE_Z;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = EXPLOSION_SIZE_X;
		pVtx[1].pos.y = EXPLOSION_SIZE_Z;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = -EXPLOSION_SIZE_X;
		pVtx[2].pos.y = -EXPLOSION_SIZE_Z;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = EXPLOSION_SIZE_X;
		pVtx[3].pos.y = -EXPLOSION_SIZE_Z;
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
		pVtx[0].tex = D3DXVECTOR2(1.0f / g_nTextureAnimCount, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f / g_nTextureAnimCount, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(1.0f / g_nTextureAnimCount, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f / g_nTextureAnimCount, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffExplosion->Unlock();
}

//================================================================================================================
// --- 爆発終了処理 ---
//================================================================================================================
void UninitExplosion(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureExplosion != NULL)
	{
		g_pTextureExplosion->Release();
		g_pTextureExplosion = NULL;
	}

	RELEASE(g_pVtxBuffExplosion);

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffExplosion != NULL)
	{
		g_pVtxBuffExplosion->Release();
		g_pVtxBuffExplosion = NULL;
	}
}

//================================================================================================================
// --- 爆発更新処理 ---
//================================================================================================================
void UpdateExplosion(void)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffExplosion->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntExplosion = 0; nCntExplosion < MAX_EXPLOSION; nCntExplosion++)
	{
		if (g_aExplosion[nCntExplosion].bUse == true)
		{
			pVtx[0].pos.x = -g_aExplosion[nCntExplosion].fWidth;
			pVtx[0].pos.y = g_aExplosion[nCntExplosion].fHeight;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = g_aExplosion[nCntExplosion].fWidth;
			pVtx[1].pos.y = g_aExplosion[nCntExplosion].fHeight;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -g_aExplosion[nCntExplosion].fWidth;
			pVtx[2].pos.y = -g_aExplosion[nCntExplosion].fHeight;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = g_aExplosion[nCntExplosion].fWidth;
			pVtx[3].pos.y = -g_aExplosion[nCntExplosion].fHeight;
			pVtx[3].pos.z = 0.0f;

			SetPositionShadow(g_aExplosion[nCntExplosion].nIdShadow, g_aExplosion[nCntExplosion].pos, D3DXVECTOR3_NULL);

			g_aExplosion[nCntExplosion].nCounterAnim++;
			if (g_aExplosion[nCntExplosion].nCounterAnim % g_aExplosion[nCntExplosion].nSpeeedAnim == 0)
			{
				g_aExplosion[nCntExplosion].nPatternAnim++;
				if (g_aExplosion[nCntExplosion].nPatternAnim >= g_nTextureAnimCount)
				{
					DestroyShadow(g_aExplosion[nCntExplosion].nIdShadow);
					g_aExplosion[nCntExplosion].nIdShadow = -1;
					g_aExplosion[nCntExplosion].bUse = false;
				}
			}

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2((1.0f / g_nTextureAnimCount) * g_aExplosion[nCntExplosion].nPatternAnim, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(((1.0f / g_nTextureAnimCount) * g_aExplosion[nCntExplosion].nPatternAnim) + (1.0f / g_nTextureAnimCount), 0.0f);
			pVtx[2].tex = D3DXVECTOR2((1.0f / g_nTextureAnimCount) * g_aExplosion[nCntExplosion].nPatternAnim, 1.0f);
			pVtx[3].tex = D3DXVECTOR2(((1.0f / g_nTextureAnimCount) * g_aExplosion[nCntExplosion].nPatternAnim) + (1.0f / g_nTextureAnimCount), 1.0f);
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffExplosion->Unlock();
}

//================================================================================================================
// --- 爆発描画処理 ---
//================================================================================================================
void DrawExplosion(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用

	for (int nCntExplosion = 0; nCntExplosion < MAX_EXPLOSION; nCntExplosion++)
	{
		if (g_aExplosion[nCntExplosion].bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aExplosion[nCntExplosion].mtxWorld);

			/*** カメラのビューマトリックスを取得 ***/
			pDevice->GetTransform(D3DTS_VIEW, &mtxView);

			/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixInverse(&g_aExplosion[nCntExplosion].mtxWorld, NULL, &mtxView);
			/** 逆行列によって入ってしまった位置情報を初期化 **/
			g_aExplosion[nCntExplosion].mtxWorld._41 = 0.0f;
			g_aExplosion[nCntExplosion].mtxWorld._42 = 0.0f;
			g_aExplosion[nCntExplosion].mtxWorld._43 = 0.0f;

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aExplosion[nCntExplosion].pos.x,
				g_aExplosion[nCntExplosion].pos.y,
				g_aExplosion[nCntExplosion].pos.z);

			D3DXMatrixMultiply(&g_aExplosion[nCntExplosion].mtxWorld, &g_aExplosion[nCntExplosion].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aExplosion[nCntExplosion].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffExplosion, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_nIndexTextureExplosion));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntExplosion,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}
}

//================================================================================================================
// --- 爆発発生処理 ---
//================================================================================================================
void SetExplosion(D3DXVECTOR3 pos, float fWidth, float fHeight, int nSpeedAnim)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffExplosion->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntExplosion = 0; nCntExplosion < MAX_EXPLOSION; nCntExplosion++)
	{
		if (g_aExplosion[nCntExplosion].bUse != true)
		{
			g_aExplosion[nCntExplosion].pos = pos;
			g_aExplosion[nCntExplosion].fWidth = fWidth;
			g_aExplosion[nCntExplosion].fHeight = fHeight;
			g_aExplosion[nCntExplosion].nCounterAnim = 0;
			g_aExplosion[nCntExplosion].nPatternAnim = 0;
			g_aExplosion[nCntExplosion].nSpeeedAnim = nSpeedAnim;
			g_aExplosion[nCntExplosion].bUse = true;

			pVtx[0].pos.x = -g_aExplosion[nCntExplosion].fWidth;
			pVtx[0].pos.y = g_aExplosion[nCntExplosion].fHeight;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = g_aExplosion[nCntExplosion].fWidth;
			pVtx[1].pos.y = g_aExplosion[nCntExplosion].fHeight;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -g_aExplosion[nCntExplosion].fWidth;
			pVtx[2].pos.y = -g_aExplosion[nCntExplosion].fHeight;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = g_aExplosion[nCntExplosion].fWidth;
			pVtx[3].pos.y = -g_aExplosion[nCntExplosion].fHeight;
			pVtx[3].pos.z = 0.0f;

			g_aExplosion[nCntExplosion].nIdShadow = SetShadow(EXPLOSION_SIZE_X, EXPLOSION_SIZE_Z);

			break;
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffExplosion->Unlock();
}

//================================================================================================================
// --- 爆発のテクスチャインデックス及びアニメーション数設定処理 ---
//================================================================================================================
void SetIndexTextureExplosion(int nIndexTexture, int nAnimationCount)
{
	g_nIndexTextureExplosion = nIndexTexture;
	g_nTextureAnimCount = nAnimationCount;
}
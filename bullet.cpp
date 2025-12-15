//================================================================================================================
//
// DirectXのビルボードバレット表示処理 [bullet.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "bullet.h"
#include "effect.h"
#include "wall.h"
#include "shadow.h"
#include "explosion.h"
#include "mathUtil.h"
#include "loadxfile.h"
#include "meshSphere.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define BULLET_SIZE_X		(10)		// ポリゴンの基本サイズ - X
#define BULLET_SIZE_Z		(10)		// ポリゴンの基本サイズ - Y
#define BULLET_SPD			(2.0f)		// ポリゴンの移動スピード
#define BULLET_WDSPD		(0.1f)		// ポリゴンの拡縮スピード
#define MAX_BULLET			(300)		// 弾丸の最大出現数
#define BULLET_LIFE			(1000)		// 弾丸の体力
#define UNBILLBOARD			// ビルボード解除

//*************************************************************************************************
//*** 弾丸構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXVECTOR3 posOld;		// 過去位置
	D3DXVECTOR3 move;		// 移動量
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	int nLife;				// 体力
	int nIdShadow;			// 影
	bool bUse;				// 使っているか
}Bullet;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffBullet = NULL;	// 頂点バッファのポインタ
Bullet g_aBullet[MAX_BULLET];						// 弾丸の情報
D3DXVECTOR3 g_posBullet;							// ポリゴンの位置
D3DXMATRIX g_mtxWorldBullet;						// ワールドマトリックス
int g_nIndexTextureBullet;							// テクスチャインデックス

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitBullet(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntBullet = 0; nCntBullet < MAX_BULLET; nCntBullet++)
	{
		g_aBullet[nCntBullet].pos = D3DXVECTOR3_NULL;
		g_aBullet[nCntBullet].move = D3DXVECTOR3_NULL;
		g_aBullet[nCntBullet].nLife = 0;
		g_aBullet[nCntBullet].nIdShadow = -1;
		g_aBullet[nCntBullet].bUse = false;
	}

	g_nIndexTextureBullet = 0;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_BULLET,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffBullet,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffBullet->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	for (int nCntBullet = 0; nCntBullet < MAX_BULLET; nCntBullet++)
	{
		pVtx[0].pos.x = -BULLET_SIZE_X;
		pVtx[0].pos.y = BULLET_SIZE_Z;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = BULLET_SIZE_X;
		pVtx[1].pos.y = BULLET_SIZE_Z;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = -BULLET_SIZE_X;
		pVtx[2].pos.y = -BULLET_SIZE_Z;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = BULLET_SIZE_X;
		pVtx[3].pos.y = -BULLET_SIZE_Z;
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
	g_pVtxBuffBullet->Unlock();
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitBullet(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffBullet);

	if ((g_pVtxBuffBullet) != NULL)
	{
		(g_pVtxBuffBullet)->Release();
		(g_pVtxBuffBullet) = nullptr;
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateBullet(void)
{
	D3DXVECTOR3 Vector3;

	for (int nCntBullet = 0; nCntBullet < MAX_BULLET; nCntBullet++)
	{
		if (g_aBullet[nCntBullet].bUse == true)
		{
			g_aBullet[nCntBullet].posOld = g_aBullet[nCntBullet].pos;

			Vector3 = GetRandomVector3(100, 100, 100);

			g_aBullet[nCntBullet].pos.x += g_aBullet[nCntBullet].move.x;
			g_aBullet[nCntBullet].pos.z += g_aBullet[nCntBullet].move.z;
			SetPositionShadow(g_aBullet[nCntBullet].nIdShadow, g_aBullet[nCntBullet].pos, D3DXVECTOR3_NULL);
			SetEffect(g_aBullet[nCntBullet].pos, D3DXVECTOR3_NULL, 3.0f, BULLET_SIZE_X, BULLET_SIZE_Z);

			g_aBullet[nCntBullet].nLife--;
			if (g_aBullet[nCntBullet].nLife <= 0)
			{
				SetExplosion(g_aBullet[nCntBullet].pos, 10.0f, 10.0f, 5);
				DestroyShadow(g_aBullet[nCntBullet].nIdShadow);
				g_aBullet[nCntBullet].nIdShadow = -1;
				g_aBullet[nCntBullet].bUse = false;
			}

			ReflectWall(&g_aBullet[nCntBullet].pos, &g_aBullet[nCntBullet].posOld, &g_aBullet[nCntBullet].move);

			g_aBullet[nCntBullet].move.x += (0.0f - g_aBullet[nCntBullet].move.x) * 0.001f;
			g_aBullet[nCntBullet].move.z += (0.0f - g_aBullet[nCntBullet].move.z) * 0.001f;
		}
	}
}
//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawBullet(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用

	// レンダリングステートの設定
	LIGHTING(pDevice, FALSE) // ライティング無効化

#ifndef UNBILLBOARD
	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
#endif

	/*** アルファテストを有効にする ***/
	AbleAlphaFunction(D3DCMP_GREATER, 30);

	//pDevice->GetRenderState()

	for (int nCntBullet = 0; nCntBullet < MAX_BULLET; nCntBullet++)
	{
		if (g_aBullet[nCntBullet].bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aBullet[nCntBullet].mtxWorld);

			/*** カメラのビューマトリックスを取得 ***/
			pDevice->GetTransform(D3DTS_VIEW, &mtxView);

			/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixInverse(&g_aBullet[nCntBullet].mtxWorld, NULL, &mtxView);
			/** 逆行列によって入ってしまった位置情報を初期化 **/
			g_aBullet[nCntBullet].mtxWorld._41 = 0.0f;
			g_aBullet[nCntBullet].mtxWorld._42 = 0.0f;
			g_aBullet[nCntBullet].mtxWorld._43 = 0.0f;

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aBullet[nCntBullet].pos.x,
				g_aBullet[nCntBullet].pos.y,
				g_aBullet[nCntBullet].pos.z);

			D3DXMatrixMultiply(&g_aBullet[nCntBullet].mtxWorld, &g_aBullet[nCntBullet].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aBullet[nCntBullet].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffBullet, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_nIndexTextureBullet));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntBullet,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}

	/*** アルファテストを無効にする ***/
	EndFunction();

#ifndef UNBILLBOARD
	/*** Zテストを有効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
#endif

	// レンダリングステートの設定
	LIGHTING(pDevice, TRUE) // ライティング有効化
}

//================================================================================================================
// --- 弾丸の発射処理 ---
//================================================================================================================
void SetBullet(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd)
{
	for (int nCntBullet = 0; nCntBullet < MAX_BULLET; nCntBullet++)
	{
		if (g_aBullet[nCntBullet].bUse != true)
		{
			g_aBullet[nCntBullet].pos = pos;
			g_aBullet[nCntBullet].move.x = sinf(rot.y + D3DX_PI) * fSpd;
			g_aBullet[nCntBullet].move.z = cosf(rot.y + D3DX_PI) * fSpd;
			g_aBullet[nCntBullet].nLife = BULLET_LIFE;
			g_aBullet[nCntBullet].bUse = true;

			g_aBullet[nCntBullet].nIdShadow = SetShadow(BULLET_SIZE_X, BULLET_SIZE_Z);

			break;
		}
	}
}

//================================================================================================================
// --- 弾丸のテクスチャインデックス設定処理 ---
//================================================================================================================
void SetIndexTextureBullet(int nIndexTexture)
{
	g_nIndexTextureBullet = nIndexTexture;
}
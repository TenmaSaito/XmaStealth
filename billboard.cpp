//================================================================================================================
//
// DirectXのポリゴン表示処理 [billboard.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "billboard.h"
#include "shadow.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define BILLBOARD_SIZE_X		(20)		// ポリゴンの基本サイズ - X
#define BILLBOARD_SIZE_Z		(20)		// ポリゴンの基本サイズ - Y
#define BILLBOARD_SPD			(2.0f)		// ポリゴンの移動スピード
#define BILLBOARD_WDSPD			(0.1f)		// ポリゴンの拡縮スピード
#define MAX_BILLBOARD			(256)		// ビルボード最大数

//*************************************************************************************************
//*** ビルボード構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	float fWidth;			// 幅
	float fHeight;			// 高さ
	int nIdxTexture;		// テクスチャインデックス
	int nIdxShadow;			// 影のインデックス
	bool bUse;				// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
}BillBoard, *PBILLBOARD;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
BillBoard g_aBillboard[MAX_BILLBOARD];		// ビルボードの情報

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitBillBoard(void)
{
	/*** 各変数の初期化 ***/
	ZeroMemory(&g_aBillboard[0], sizeof(BillBoard) * MAX_BILLBOARD);
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitBillBoard(void)
{
	PBILLBOARD pBill = &g_aBillboard[0];

	for (int nCntRelease = 0; nCntRelease < MAX_BILLBOARD; nCntRelease++, pBill++)
	{
		RELEASE(pBill->pVtxBuff);
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateBillBoard(void)
{
	PBILLBOARD pBill = &g_aBillboard[0];

	for (int nCntRelease = 0; nCntRelease < MAX_BILLBOARD; nCntRelease++, pBill++)
	{
		if (pBill->bUse == true)
		{
			SetPositionShadow(pBill->nIdxShadow, pBill->pos, D3DXVECTOR3_NULL);
		}
	}
}
//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawBillBoard(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxTrans;		// 計算用マトリックス
	D3DXMATRIX mtxView;			// ビューマトリックスの取得用
	D3DXMATRIX mtxBill;
	PBILLBOARD pBill = &g_aBillboard[0];

	/*** カメラのビューマトリックスを取得 ***/
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	/*** アルファテストを有効にする ***/
	AbleAlphaFunction(D3DCMP_GREATER, 30);

	LIGHTING(pDevice, FALSE);

	for (int nCntRelease = 0; nCntRelease < MAX_BILLBOARD; nCntRelease++, pBill++)
	{
		if (pBill->bUse == true)
		{
			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&pBill->mtxWorld);
			D3DXMatrixIdentity(&mtxBill);

			/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/

			/*** Y軸の回転 ***/
			pBill->mtxWorld._11 = mtxView._11;
			pBill->mtxWorld._12 = mtxView._21;
			pBill->mtxWorld._13 = mtxView._31;

			/*** X軸の回転 ***/
			/*g_mtxWorldBillBoard._21 = mtxView._12;
			g_mtxWorldBillBoard._22 = mtxView._22;
			g_mtxWorldBillBoard._23 = mtxView._32;*/

			/*** Z軸の回転? ***/
			pBill->mtxWorld._31 = mtxView._13;
			pBill->mtxWorld._32 = mtxView._23;
			pBill->mtxWorld._33 = mtxView._33;

			/*** 位置の反映 ***/
			pBill->mtxWorld._41 = 0.0f;
			pBill->mtxWorld._42 = 0.0f;
			pBill->mtxWorld._43 = 0.0f;

			/*** 逆行列の計算に必要 ***/


			/*** Y軸の逆行列 ***/

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				pBill->pos.x,
				pBill->pos.y,
				pBill->pos.z);

			D3DXMatrixMultiply(&pBill->mtxWorld, &pBill->mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &pBill->mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, pBill->pVtxBuff, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(pBill->nIdxTexture));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				0,											// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}

	/*** アルファテストを無効にする ***/
	EndFunction();

	LIGHTING(pDevice, TRUE);
}

int SetBillBoard(D3DXVECTOR3 pos, float fWidth, float fHeight, int nIdxTexture)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ
	PBILLBOARD pBill = &g_aBillboard[0];

	for (int nCntRelease = 0; nCntRelease < MAX_BILLBOARD; nCntRelease++, pBill++)
	{
		if (pBill->bUse == false)
		{
			pBill->pos = pos;
			pBill->fWidth = fWidth;
			pBill->fHeight = fHeight;
			pBill->nIdxTexture = nIdxTexture;

			if (pBill->pVtxBuff == NULL)
			{
				/*** 頂点バッファの生成 ***/
				pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4,
					D3DUSAGE_WRITEONLY,
					FVF_VERTEX_3D,
					D3DPOOL_MANAGED,
					&pBill->pVtxBuff,
					NULL);
			}

			/*** 頂点バッファの設定 ***/
			pBill->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = -fWidth * 0.5f;
			pVtx[0].pos.y = fHeight * 0.5f;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = fWidth * 0.5f;
			pVtx[1].pos.y = fHeight * 0.5f;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -fWidth * 0.5f;
			pVtx[2].pos.y = -fHeight * 0.5f;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = fWidth * 0.5f;
			pVtx[3].pos.y = -fHeight * 0.5f;
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

			/*** 頂点バッファの設定を終了 ***/
			pBill->pVtxBuff->Unlock();

			pBill->bUse = true;

			return nCntRelease;
		}
	}

	return -1;
}

void KillBillboard(int nIdxBill)
{
	PBILLBOARD pBill = &g_aBillboard[nIdxBill];
	pBill->bUse = false;
}
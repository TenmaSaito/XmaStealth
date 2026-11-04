//==================================================================================
//
// ビルボード(常にカメラを向く板ポリゴン)の処理 [billboard.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "billboard.h"
#include "shadow.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_BILLBOARD			(256)		// ビルボードの最大数
#define BILLBOARD_ALPHA_REF		(30)		// アルファテストの基準値(これ以下のαは描画しない)

//**********************************************************************************
// *** ビルボード構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fWidth;						// 幅
	float fHeight;						// 高さ
	int nIdxTexture;					// テクスチャ番号
	int nIdxShadow;						// 影の番号
	bool bUse;							// 使用状況
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
} BillBoard, *PBILLBOARD;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static BillBoard g_aBillboard[MAX_BILLBOARD];		// ビルボードの情報

//==================================================================================
// --- ビルボードの初期化処理 ---
//==================================================================================
void InitBillBoard(void)
{
	ZeroMemory(&g_aBillboard[0], sizeof g_aBillboard);
}

//==================================================================================
// --- ビルボードの終了処理 ---
//==================================================================================
void UninitBillBoard(void)
{
	for (int nCntBill = 0; nCntBill < MAX_BILLBOARD; nCntBill++)
	{
		/*** 頂点バッファの破棄 ***/
		RELEASE(g_aBillboard[nCntBill].pVtxBuff);
	}
}

//==================================================================================
// --- ビルボードの更新処理 ---
//==================================================================================
void UpdateBillBoard(void)
{
	for (int nCntBill = 0; nCntBill < MAX_BILLBOARD; nCntBill++)
	{
		PBILLBOARD pBill = &g_aBillboard[nCntBill];		// 更新するビルボード

		if (pBill->bUse == true)
		{ // 影を追従させる
			SetPositionShadow(pBill->nIdxShadow, pBill->pos, D3DXVECTOR3_NULL);
		}
	}
}

//==================================================================================
// --- ビルボードの描画処理 ---
//==================================================================================
void DrawBillBoard(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxTrans;							// 平行移動用マトリックス
	D3DXMATRIX mtxView;								// カメラのビューマトリックス

	/*** カメラのビューマトリックスを取得 ***/
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	/*** アルファテスト有効・ライティング無効 ***/
	AbleAlphaFunction(D3DCMP_GREATER, BILLBOARD_ALPHA_REF);
	LIGHTING(pDevice, FALSE);

	for (int nCntBill = 0; nCntBill < MAX_BILLBOARD; nCntBill++)
	{
		PBILLBOARD pBill = &g_aBillboard[nCntBill];		// 描画するビルボード

		if (pBill->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ビューマトリックスの回転成分を転置してカメラの方を向かせる ***/
		// ※ 2行目(X軸回りの回転)はコピーしないため、Y軸回りにのみカメラを向く
		D3DXMatrixIdentity(&pBill->mtxWorld);

		pBill->mtxWorld._11 = mtxView._11;
		pBill->mtxWorld._12 = mtxView._21;
		pBill->mtxWorld._13 = mtxView._31;

		pBill->mtxWorld._31 = mtxView._13;
		pBill->mtxWorld._32 = mtxView._23;
		pBill->mtxWorld._33 = mtxView._33;

		/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
		D3DXMatrixTranslation(&mtxTrans, pBill->pos.x, pBill->pos.y, pBill->pos.z);
		D3DXMatrixMultiply(&pBill->mtxWorld, &pBill->mtxWorld, &mtxTrans);

		/*** ワールドマトリックス・頂点バッファ・テクスチャの設定 ***/
		pDevice->SetTransform(D3DTS_WORLD, &pBill->mtxWorld);
		pDevice->SetStreamSource(0, pBill->pVtxBuff, 0, sizeof(VERTEX_3D));
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pBill->nIdxTexture));

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	/*** アルファテスト無効・ライティング有効に戻す ***/
	EndFunction();
	LIGHTING(pDevice, TRUE);
}

//==================================================================================
// --- ビルボードの設置処理 ---
// 戻り値 : 設置したビルボードの番号 (空きが無ければ -1)
//==================================================================================
int SetBillBoard(D3DXVECTOR3 pos, float fWidth, float fHeight, int nIdxTexture, D3DXCOLOR col)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

	for (int nCntBill = 0; nCntBill < MAX_BILLBOARD; nCntBill++)
	{
		PBILLBOARD pBill = &g_aBillboard[nCntBill];		// 設置先のビルボード

		if (pBill->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pBill->pos = pos;
		pBill->fWidth = fWidth;
		pBill->fHeight = fHeight;
		pBill->nIdxTexture = nIdxTexture;

		if (pBill->pVtxBuff == NULL)
		{ // 頂点バッファが未生成なら生成 (消去済みの枠は使い回す)
			pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4,
				D3DUSAGE_WRITEONLY,
				FVF_VERTEX_3D,
				D3DPOOL_MANAGED,
				&pBill->pVtxBuff,
				NULL);
		}

		/*** 頂点情報の設定 (原点中心の板) ***/
		pBill->pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

		float fHalfWidth = fWidth * 0.5f;		// 幅の半分
		float fHalfHeight = fHeight * 0.5f;		// 高さの半分

		pVtx[0].pos = D3DXVECTOR3(-fHalfWidth, fHalfHeight, 0.0f);		// 左上
		pVtx[1].pos = D3DXVECTOR3(fHalfWidth, fHalfHeight, 0.0f);		// 右上
		pVtx[2].pos = D3DXVECTOR3(-fHalfWidth, -fHalfHeight, 0.0f);		// 左下
		pVtx[3].pos = D3DXVECTOR3(fHalfWidth, -fHalfHeight, 0.0f);		// 右下

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);		// 法線(手前向き)
			pVtx[nCntVtx].col = col;								// 頂点カラー
		}

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pBill->pVtxBuff->Unlock();

		pBill->bUse = true;

		return nCntBill;
	}

	return -1;
}

//==================================================================================
// --- ビルボードの消去処理 (頂点バッファは次回の設置で使い回す) ---
//==================================================================================
void KillBillboard(int nIdxBill)
{
	if (nIdxBill < 0 || nIdxBill >= MAX_BILLBOARD)
	{ // 番号が範囲外
		return;
	}

	g_aBillboard[nIdxBill].bUse = false;
}

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

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureBillBoard = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffBillBoard = NULL;	// 頂点バッファのポインタ
D3DXVECTOR3 g_posBillBoard;							// ポリゴンの位置
D3DXMATRIX g_mtxWorldBillBoard;						// ワールドマトリックス
int g_nIdShadow = -1;		// 影

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitBillBoard(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	g_posBillBoard = D3DXVECTOR3(0.0f, 25.0f, 0.0f);

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
							  "data\\TEXTURE\\ball000.png",
							  &g_pTextureBillBoard);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffBillBoard,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffBillBoard->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	pVtx[0].pos.x = -BILLBOARD_SIZE_X;
	pVtx[0].pos.y = BILLBOARD_SIZE_Z;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = BILLBOARD_SIZE_X;
	pVtx[1].pos.y = BILLBOARD_SIZE_Z;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = -BILLBOARD_SIZE_X;
	pVtx[2].pos.y = -BILLBOARD_SIZE_Z;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = BILLBOARD_SIZE_X;
	pVtx[3].pos.y = -BILLBOARD_SIZE_Z;
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
	g_pVtxBuffBillBoard->Unlock();

	g_nIdShadow = SetShadow(BILLBOARD_SIZE_X, BILLBOARD_SIZE_Z);
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitBillBoard(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureBillBoard != NULL)
	{
		g_pTextureBillBoard->Release();
		g_pTextureBillBoard = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffBillBoard != NULL)
	{
		g_pVtxBuffBillBoard->Release();
		g_pVtxBuffBillBoard = NULL;
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateBillBoard(void)
{
	if (GetKeyboardPress(DIK_W))
	{
		g_posBillBoard.z += 5.0f;
	}
	if (GetKeyboardPress(DIK_S))
	{
		g_posBillBoard.z -= 5.0f;
	}
	if (GetKeyboardPress(DIK_A))
	{
		g_posBillBoard.x -= 5.0f;
	}
	if (GetKeyboardPress(DIK_D))
	{
		g_posBillBoard.x += 5.0f;
	}

	SetPositionShadow(g_nIdShadow, g_posBillBoard, D3DXVECTOR3_NULL);
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

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	/*** ワールドマトリックスの初期化 ***/
	D3DXMatrixIdentity(&g_mtxWorldBillBoard);
	D3DXMatrixIdentity(&mtxBill);

	/*** カメラのビューマトリックスを取得 ***/
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	/*** マトリックスの逆行列を求める (※ 位置を反映する前に必ず行うこと！) ***/

	/*** Y軸の回転 ***/
	g_mtxWorldBillBoard._11 = mtxView._11;
	g_mtxWorldBillBoard._12 = mtxView._21;
	g_mtxWorldBillBoard._13 = mtxView._31;

	/*** X軸の回転 ***/
	/*g_mtxWorldBillBoard._21 = mtxView._12;
	g_mtxWorldBillBoard._22 = mtxView._22;
	g_mtxWorldBillBoard._23 = mtxView._32;*/

	/*** Z軸の回転? ***/
	g_mtxWorldBillBoard._31 = mtxView._13;
	g_mtxWorldBillBoard._32 = mtxView._23;
	g_mtxWorldBillBoard._33 = mtxView._33;

	/*** 位置の反映 ***/
	g_mtxWorldBillBoard._41 = 0.0f;
	g_mtxWorldBillBoard._42 = 0.0f;
	g_mtxWorldBillBoard._43 = 0.0f;

	/*** 逆行列の計算に必要 ***/
	

	/*** Y軸の逆行列 ***/

	/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
	D3DXMatrixTranslation(&mtxTrans,
		g_posBillBoard.x,
		g_posBillBoard.y,
		g_posBillBoard.z);

	D3DXMatrixMultiply(&g_mtxWorldBillBoard, &g_mtxWorldBillBoard, &mtxTrans);

	/*** ワールドマトリックスの設定 ***/
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorldBillBoard);

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffBillBoard, 0, sizeof(VERTEX_3D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_3D);
	
	/*** テクスチャの設定 ***/
	pDevice->SetTexture(0, g_pTextureBillBoard);

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
		0,											// 描画する最初の頂点インデックス
		2);											// 描画するプリミティブの数

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//================================================================================================================
// --- リザルト用背景の描画設定処理 ---
//================================================================================================================
void SetTextureIndex(int nTextureIndex)
{
	//g_bUseBillBoard = bUse;
}
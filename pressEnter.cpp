//================================================================================================================
//
// DirectXのPressEnter演出処理 [pressEnter.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "pressEnter.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define PRESSENTER_WIDTH	(700.0f)		// 幅
#define PRESSENTER_HEIGHT	(360.0f)		// 高さ
#define PRESSENTER_ANIM_V	(6)				// V座標の数
#define ANIM_COUT			(15)			// アニメーション変化秒数

//*************************************************************************************************
//*** PressEnter演出 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXCOLOR col;			// 色
	float fWidth;			// 幅
	float fHeight;			// 高さ
	int nCounterAnim;		// アニメーションカウンター
	int nPatternAnim;		// アニメーションNo
	bool bDisp;				// 描画しているか
}PRESSENTER;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTexturePressEnter = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffPressEnter = NULL;	// 頂点バッファのポインタ
PRESSENTER g_pressEnter;			// 情報
bool g_bEndPressEnter;

//================================================================================================================
// --- PressEnter演出の初期化処理 ---
//================================================================================================================
void InitPressEnter(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D *pVtx;					// 頂点情報へのポインタ

	g_pressEnter.pos = WINDOW_MID;
	g_pressEnter.col = D3DXCOLOR_INV;
	g_pressEnter.fWidth = PRESSENTER_WIDTH;
	g_pressEnter.fHeight = PRESSENTER_HEIGHT;
	g_pressEnter.nCounterAnim = 0;
	g_pressEnter.nPatternAnim = 0;
	g_pressEnter.bDisp = false;
	g_bEndPressEnter = false;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
							  "data\\TEXTURE\\PRESSENTER_ANIM.png",
							  &g_pTexturePressEnter);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_2D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffPressEnter,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffPressEnter->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標の設定の設定 ***/
	pVtx[0].pos.x = g_pressEnter.pos.x;
	pVtx[0].pos.y = g_pressEnter.pos.y;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = g_pressEnter.pos.x + g_pressEnter.fWidth;
	pVtx[1].pos.y = g_pressEnter.pos.y;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = g_pressEnter.pos.x;
	pVtx[2].pos.y = g_pressEnter.pos.y + g_pressEnter.fHeight;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = g_pressEnter.pos.x + g_pressEnter.fWidth;
	pVtx[3].pos.y = g_pressEnter.pos.y + g_pressEnter.fHeight;
	pVtx[3].pos.z = 0.0f;

	/*** rhwの設定 ***/
	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	/*** 頂点カラー設定 ***/
	pVtx[0].col = g_pressEnter.col;
	pVtx[1].col = g_pressEnter.col;
	pVtx[2].col = g_pressEnter.col;
	pVtx[3].col = g_pressEnter.col;

	/*** テクスチャ座標の設定 ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, (1.0f / PRESSENTER_ANIM_V) * g_pressEnter.nPatternAnim);
	pVtx[1].tex = D3DXVECTOR2(1.0f, (1.0f / PRESSENTER_ANIM_V) * g_pressEnter.nPatternAnim);
	pVtx[2].tex = D3DXVECTOR2(0.0f, (1.0f / PRESSENTER_ANIM_V) * (g_pressEnter.nPatternAnim + 1));
	pVtx[3].tex = D3DXVECTOR2(1.0f, (1.0f / PRESSENTER_ANIM_V) * (g_pressEnter.nPatternAnim + 1));

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffPressEnter->Unlock();
}

//================================================================================================================
// --- PressEnter演出の終了処理 ---
//================================================================================================================
void UninitPressEnter(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTexturePressEnter != NULL)
	{
		g_pTexturePressEnter->Release();
		g_pTexturePressEnter = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffPressEnter != NULL)
	{
		g_pVtxBuffPressEnter->Release();
		g_pVtxBuffPressEnter = NULL;
	}
}

//================================================================================================================
// --- PressEnter演出の更新処理 ---
//================================================================================================================
void UpdatePressEnter(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ

	if (g_pressEnter.bDisp)
	{
		if (GetKeyboardTrigger(DIK_RETURN)
			|| GetJoypadTrigger(JOYKEY_A)
			|| GetJoypadTrigger(JOYKEY_START))
		{
			g_pressEnter.col.a = 1.0f;
			g_bEndPressEnter = true;
		}

		if(g_pressEnter.col.a < 1.0f)
		{
			g_pressEnter.col.a += 0.02f;
			if (g_pressEnter.col.a >= 1.0f)
			{
				g_pressEnter.col.a = 1.0f;
				g_bEndPressEnter = true;
			}
		}

		g_pressEnter.nCounterAnim++;
		if (g_pressEnter.nCounterAnim % ANIM_COUT == 0)
		{
			g_pressEnter.nPatternAnim++;			
		}

		/*** 頂点バッファの設定 ***/
		g_pVtxBuffPressEnter->Lock(0, 0, (void**)&pVtx, 0);

		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_pressEnter.pos.x;
		pVtx[0].pos.y = g_pressEnter.pos.y;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_pressEnter.pos.x + g_pressEnter.fWidth;
		pVtx[1].pos.y = g_pressEnter.pos.y;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_pressEnter.pos.x;
		pVtx[2].pos.y = g_pressEnter.pos.y + g_pressEnter.fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_pressEnter.pos.x + g_pressEnter.fWidth;
		pVtx[3].pos.y = g_pressEnter.pos.y + g_pressEnter.fHeight;
		pVtx[3].pos.z = 0.0f;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = g_pressEnter.col;
		pVtx[1].col = g_pressEnter.col;
		pVtx[2].col = g_pressEnter.col;
		pVtx[3].col = g_pressEnter.col;

		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2(0.0f, (1.0f / PRESSENTER_ANIM_V) * (g_pressEnter.nPatternAnim % PRESSENTER_ANIM_V));
		pVtx[1].tex = D3DXVECTOR2(1.0f, (1.0f / PRESSENTER_ANIM_V) * (g_pressEnter.nPatternAnim % PRESSENTER_ANIM_V));
		pVtx[2].tex = D3DXVECTOR2(0.0f, (1.0f / PRESSENTER_ANIM_V) * ((g_pressEnter.nPatternAnim % PRESSENTER_ANIM_V) + 1));
		pVtx[3].tex = D3DXVECTOR2(1.0f, (1.0f / PRESSENTER_ANIM_V) * ((g_pressEnter.nPatternAnim % PRESSENTER_ANIM_V) + 1));

		/*** 頂点バッファの設定を終了 ***/
		g_pVtxBuffPressEnter->Unlock();
	}
}

//================================================================================================================
// --- PressEnter演出の描画処理 ---
//================================================================================================================
void DrawPressEnter(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffPressEnter, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	if (g_pressEnter.bDisp)
	{
		/*** テクスチャの設定 ***/
		pDevice->SetTexture(0, g_pTexturePressEnter);

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
			0,											// 描画する最初の頂点インデックス
			2);											// 描画するプリミティブの数
	}
}

//================================================================================================================
// --- PressEnter演出の描画設定処理 ---
//================================================================================================================
void SetPressEnterEffect(D3DXVECTOR3 pos)
{
	g_pressEnter.pos = pos;
	g_pressEnter.col.a = 0.0f;
	g_pressEnter.bDisp = true;
}

//================================================================================================================
// --- PressEnter演出の取得処理 ---
//================================================================================================================
bool GetPressEnterEffect(void)
{
	return g_pressEnter.bDisp;
}

//================================================================================================================
// --- PressEnter演出の取得処理 ---
//================================================================================================================
bool GetEndPressEnterEffect(void)
{
	return g_bEndPressEnter;
}
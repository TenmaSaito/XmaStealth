//================================================================================================================
//
// DirectXのタイトル背景表示処理 [titleLogo.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "titleLogo.h"
#include "pressEnter.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define LOGO_WIDTH			(640.0f)		// ロゴの横幅
#define ANIMATION_TIME_C	(120)			// ロゴのアニメーション時間(CHAR)
#define ANIMATION_TIME_U	(30)			// ロゴのアニメーション時間(UNDERLINE)

//**********************************************************************************
//*** ロゴの構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置
	D3DXCOLOR col;			// 色
	float fWidth;			// 幅
	float fHeight;			// 高さ
	LOGOTYPE type;			// 種類
	int nCounterLogo;		// カウンター
	bool bDisp;				// 描画しているか
}LOGO;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_apTextureTitleLogo[LOGOTYPE_MAX] = {};	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffTitleLogo = NULL;					// 頂点バッファのポインタ
LOGO g_aLogo[LOGOTYPE_MAX];				// ロゴの情報
int g_nCounterTitleLogo;				// ロゴカウンター
bool g_bReverseFirstLogo;				// タイトルロゴが反転表示されたか
bool g_bReverseDFirstLogo;				// 過去にタイトルロゴが反転表示されたか
const LOGO g_aLogoInfo[] =
{
	{D3DXVECTOR3(640.0f - (LOGO_WIDTH * 0.5f), 75.0f + ANIMATION_TIME_C, 0.0f), D3DXCOLOR_INV, LOGO_WIDTH, 275.0f, LOGOTYPE_CHAR, 0, true},
	{D3DXVECTOR3(640.0f - (LOGO_WIDTH * 0.5f), 320.0f, 0.0f), D3DXCOLOR_INV, LOGO_WIDTH, 50.0f, LOGOTYPE_UNDERLINE, 0, false}
};

//**********************************************************************************
//*** テクスチャ ***
//**********************************************************************************
const char* g_aLogoTex[]
{
	"data\\TEXTURE\\LOGO\\Logo.png",
	"data\\TEXTURE\\LOGO\\LOGO_SHOOTINGSTAR.png",
};

//================================================================================================================
// --- タイトル用背景の初期化処理 ---
//================================================================================================================
void InitTitleLogo(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D *pVtx;					// 頂点情報へのポインタ

	for (int nCntTex = 0; nCntTex < (sizeof g_aLogoTex / sizeof(const char*)); nCntTex++)
	{
		g_aLogo[nCntTex] = g_aLogoInfo[nCntTex];

		/*** テクスチャの読み込み ***/
		D3DXCreateTextureFromFile(pDevice,
			g_aLogoTex[nCntTex],
			&g_apTextureTitleLogo[nCntTex]);
	}

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * LOGOTYPE_MAX,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_2D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffTitleLogo,
								NULL);

	g_nCounterTitleLogo = 0;
	g_bReverseFirstLogo = false;
	g_bReverseDFirstLogo = false;

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffTitleLogo->Lock(0, 0, (void**)&pVtx, 0);
	
	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		g_aLogo[nCntLogo].col = D3DXCOLOR_INV;

		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aLogo[nCntLogo].pos.x;
		pVtx[0].pos.y = g_aLogo[nCntLogo].pos.y;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aLogo[nCntLogo].pos.x + g_aLogo[nCntLogo].fWidth;
		pVtx[1].pos.y = g_aLogo[nCntLogo].pos.y;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_aLogo[nCntLogo].pos.x;
		pVtx[2].pos.y = g_aLogo[nCntLogo].pos.y + g_aLogo[nCntLogo].fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aLogo[nCntLogo].pos.x + g_aLogo[nCntLogo].fWidth;
		pVtx[3].pos.y = g_aLogo[nCntLogo].pos.y + g_aLogo[nCntLogo].fHeight;
		pVtx[3].pos.z = 0.0f;

		/*** rhwの設定 ***/
		pVtx[0].rhw = 1.0f;
		pVtx[1].rhw = 1.0f;
		pVtx[2].rhw = 1.0f;
		pVtx[3].rhw = 1.0f;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = g_aLogo[nCntLogo].col;
		pVtx[1].col = g_aLogo[nCntLogo].col;
		pVtx[2].col = g_aLogo[nCntLogo].col;
		pVtx[3].col = g_aLogo[nCntLogo].col;

		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffTitleLogo->Unlock();
}

//================================================================================================================
// --- タイトル用背景の終了処理 ---
//================================================================================================================
void UninitTitleLogo(void)
{
	/*** テクスチャの破棄 ***/
	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		if (g_apTextureTitleLogo[nCntLogo] != NULL)
		{
			g_apTextureTitleLogo[nCntLogo]->Release();
			g_apTextureTitleLogo[nCntLogo] = NULL;
		}
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffTitleLogo != NULL)
	{
		g_pVtxBuffTitleLogo->Release();
		g_pVtxBuffTitleLogo = NULL;
	}
}

//================================================================================================================
// --- タイトル用背景の更新処理 ---
//================================================================================================================
void UpdateTitleLogo(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	float fWidth;

	if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_A) || GetJoypadTrigger(JOYKEY_START))
	{
		g_aLogo[LOGOTYPE_CHAR].pos.y = 44.0f;
		g_aLogo[LOGOTYPE_CHAR].col.a = 1.0f;
		g_aLogo[LOGOTYPE_UNDERLINE].col.a = 1.0f;
		g_aLogo[LOGOTYPE_CHAR].nCounterLogo = ANIMATION_TIME_C;
		g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo = ANIMATION_TIME_U;
		g_bReverseFirstLogo = true;
		g_bReverseDFirstLogo = true;
	}

	if (g_aLogo[LOGOTYPE_CHAR].nCounterLogo < ANIMATION_TIME_C)
	{
		g_aLogo[LOGOTYPE_CHAR].nCounterLogo++;
		g_aLogo[LOGOTYPE_CHAR].col.a += 1.0f / ANIMATION_TIME_C;
		g_aLogo[LOGOTYPE_CHAR].pos.y -= 0.5f;
		if (g_aLogo[LOGOTYPE_CHAR].nCounterLogo >= ANIMATION_TIME_C)
		{
			g_aLogo[LOGOTYPE_CHAR].nCounterLogo = ANIMATION_TIME_C;
			g_bReverseFirstLogo = true;
		}
	}
	else
	{
		g_aLogo[LOGOTYPE_CHAR].nCounterLogo = ANIMATION_TIME_C;
		g_aLogo[LOGOTYPE_UNDERLINE].bDisp = true;
		g_aLogo[LOGOTYPE_UNDERLINE].col.a = 1.0f;
		if (g_nCounterTitleLogo <= ANIMATION_TIME_C)
		{
			g_nCounterTitleLogo++;
		}
	}

	if (g_bReverseFirstLogo == true)
	{
		if (g_bReverseDFirstLogo == true)
		{
			if (g_aLogo[LOGOTYPE_CHAR].col.a < 1.0f)
			{
				if (g_aLogo[LOGOTYPE_CHAR].pos.y >= (1.0f * (ANIMATION_TIME_C - g_aLogo[LOGOTYPE_CHAR].nCounterLogo)))
				{
					g_aLogo[LOGOTYPE_CHAR].nCounterLogo++;
					g_aLogo[LOGOTYPE_CHAR].pos.y -= 0.45f;
				}
				else
				{
					printf("");
				}
				g_aLogo[LOGOTYPE_CHAR].col.a += 0.01f;
				if (g_aLogo[LOGOTYPE_CHAR].col.a >= 1.0f)
				{
					g_aLogo[LOGOTYPE_CHAR].col.a = 1.0f;
				}
			}
		}
		else
		{
			if (g_aLogo[LOGOTYPE_CHAR].col.a > 0.0f)
			{
				if (g_aLogo[LOGOTYPE_CHAR].pos.y >= (1.0f * (ANIMATION_TIME_C - g_aLogo[LOGOTYPE_CHAR].nCounterLogo)))
				{
					g_aLogo[LOGOTYPE_CHAR].nCounterLogo++;
					g_aLogo[LOGOTYPE_CHAR].pos.y -= 0.45f;
				}
				g_aLogo[LOGOTYPE_CHAR].col.a -= 0.01f;
				if (g_aLogo[LOGOTYPE_CHAR].col.a <= 0.0f)
				{
					g_aLogo[LOGOTYPE_CHAR].col.a = 0.0f;
					g_bReverseDFirstLogo = true;
				}
			}
		}
	}

	if (g_aLogo[LOGOTYPE_UNDERLINE].bDisp && g_nCounterTitleLogo >= ANIMATION_TIME_C && (g_bReverseDFirstLogo == true && g_aLogo[LOGOTYPE_CHAR].col.a == 1.0f))
	{
		if (g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo < ANIMATION_TIME_C)
		{
			if (g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo == 0)
			{
				PlaySound(SOUND_LABEL_SE_ENTER);
			}
			g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo++;
			if (g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo >= ANIMATION_TIME_U)
			{
				g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo = ANIMATION_TIME_U;
				if (GetPressEnterEffect() == false)
				{
					SetPressEnterEffect(PRESSENTER_POS);
				}
			}
		}
		else
		{
			g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo = ANIMATION_TIME_U;
		}
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffTitleLogo->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		fWidth = g_aLogo[nCntLogo].fWidth;
		if (nCntLogo == LOGOTYPE_UNDERLINE)
		{
			fWidth = fWidth * ((1.0f / ANIMATION_TIME_U) * g_aLogo[LOGOTYPE_UNDERLINE].nCounterLogo);
		}

		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aLogo[nCntLogo].pos.x;
		pVtx[0].pos.y = g_aLogo[nCntLogo].pos.y;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aLogo[nCntLogo].pos.x + fWidth;
		pVtx[1].pos.y = g_aLogo[nCntLogo].pos.y;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_aLogo[nCntLogo].pos.x;
		pVtx[2].pos.y = g_aLogo[nCntLogo].pos.y + g_aLogo[nCntLogo].fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aLogo[nCntLogo].pos.x + fWidth;
		pVtx[3].pos.y = g_aLogo[nCntLogo].pos.y + g_aLogo[nCntLogo].fHeight;
		pVtx[3].pos.z = 0.0f;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = g_aLogo[nCntLogo].col;
		pVtx[1].col = g_aLogo[nCntLogo].col;
		pVtx[2].col = g_aLogo[nCntLogo].col;
		pVtx[3].col = g_aLogo[nCntLogo].col;

		/*** テクスチャ座標の設定 ***/
		if (nCntLogo == LOGOTYPE_UNDERLINE && g_aLogo[nCntLogo].nCounterLogo <= ANIMATION_TIME_C)
		{
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2((1.0f / ANIMATION_TIME_U) * g_aLogo[nCntLogo].nCounterLogo, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
			pVtx[3].tex = D3DXVECTOR2((1.0f / ANIMATION_TIME_U) * g_aLogo[nCntLogo].nCounterLogo, 1.0f);
		}
		else
		{
			if (g_bReverseDFirstLogo == false)
			{
				pVtx[3].tex = D3DXVECTOR2(0.01f, 0.01f);
				pVtx[2].tex = D3DXVECTOR2(1.0f, 0.01f);
				pVtx[1].tex = D3DXVECTOR2(0.01f, 1.0f);
				pVtx[0].tex = D3DXVECTOR2(1.0f, 1.0f);
			}
			else
			{
				pVtx[0].tex = D3DXVECTOR2(0.01f, 0.01f);
				pVtx[1].tex = D3DXVECTOR2(1.0f, 0.01f);
				pVtx[2].tex = D3DXVECTOR2(0.01f, 1.0f);
				pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
			}
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffTitleLogo->Unlock();
}

//================================================================================================================
// --- タイトル用背景の描画処理 ---
//================================================================================================================
void DrawTitleLogo(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffTitleLogo, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		/*** テクスチャの設定 ***/
		pDevice->SetTexture(0, g_apTextureTitleLogo[g_aLogo[nCntLogo].type]);

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
			4 * nCntLogo,								// 描画する最初の頂点インデックス
			2);											// 描画するプリミティブの数
	}
}
//==================================================================================
//
// タイトルロゴの処理 [titleLogo.cpp]
// Author : TENMA SAITO
//
// 演出の流れ :
//   1. 上下逆さま(180度回転)のロゴが、上へ移動しながらフェードイン
//   2. 出現し終わったら、逆さまのままフェードアウト
//   3. 消えたら正しい向きに切り替えて、再びフェードイン → PressEnter 演出を表示
// ※ 決定入力があると、途中を飛ばして 3 の完了状態にする
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "titleLogo.h"
#include "pressEnter.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define LOGO_WIDTH			(640.0f)		// ロゴの横幅
#define LOGO_HEIGHT			(275.0f)		// ロゴの縦幅
#define LOGO_POS_X			(350.0f - (LOGO_WIDTH * 0.5f))		// ロゴの左端
#define LOGO_POS_Y_SKIP		(44.0f)			// スキップした時のロゴの上端
#define ANIMATION_TIME_C	(120)			// 最初の出現アニメーションの時間(フレーム)
#define APPEAR_MOVE_Y		(0.5f)			// 最初の出現中の1フレームあたりの上昇量
#define FLIP_MOVE_Y			(0.45f)			// 消える・再出現中の1フレームあたりの上昇量
#define FLIP_FADE_SPEED		(0.01f)			// 消える・再出現中の1フレームあたりのα値の変化量
#define LOGO_TEX_MARGIN		(0.01f)			// テクスチャ端のにじみ防止の余白

//**********************************************************************************
// *** ロゴの構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置(左上)
	D3DXCOLOR col;			// 色
	float fWidth;			// 幅
	float fHeight;			// 高さ
	LOGOTYPE type;			// 種類
	int nCounterLogo;		// アニメーションカウンター
	bool bDisp;				// 描画しているか
} LOGO;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_apTextureTitleLogo[LOGOTYPE_MAX] = {};	// テクスチャへのポインタ
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffTitleLogo = NULL;			// 頂点バッファのポインタ
static LOGO g_aLogo[LOGOTYPE_MAX];									// ロゴの情報
static int g_nCounterTitleLogo;										// 出現後の経過フレーム
static bool g_bAppearEnd;											// 最初の出現アニメーションが終わったか
static bool g_bFlipEnd;												// 正しい向きに切り替わったか

// --- 各ロゴの初期値 ---
static const LOGO g_aLogoInfo[LOGOTYPE_MAX] =
{
	{ D3DXVECTOR3(LOGO_POS_X, 70.0f + ANIMATION_TIME_C, 0.0f), D3DXCOLOR_INV, LOGO_WIDTH, LOGO_HEIGHT, LOGOTYPE_CHAR, 0, true },
};

// --- 各ロゴのテクスチャ ---
static const char *g_apLogoTexturePath[LOGOTYPE_MAX] =
{
	"data/TEXTURE/LOGO/Logo.png",
};

//==================================================================================
// --- タイトルロゴの初期化処理 ---
//==================================================================================
void InitTitleLogo(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_2D *pVtx = NULL;							// 頂点情報へのポインタ

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		g_aLogo[nCntLogo] = g_aLogoInfo[nCntLogo];
		g_aLogo[nCntLogo].col = D3DXCOLOR_INV;

		/*** テクスチャの読み込み ***/
		D3DXCreateTextureFromFile(pDevice, g_apLogoTexturePath[nCntLogo], &g_apTextureTitleLogo[nCntLogo]);
	}

	g_nCounterTitleLogo = 0;
	g_bAppearEnd = false;
	g_bFlipEnd = false;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * LOGOTYPE_MAX,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffTitleLogo,
		NULL);

	/*** 頂点情報の設定 ***/
	g_pVtxBuffTitleLogo->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++, pVtx += 4)
	{
		LOGO *pLogo = &g_aLogo[nCntLogo];		// 設定するロゴ

		pVtx[0].pos = D3DXVECTOR3(pLogo->pos.x, pLogo->pos.y, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(pLogo->pos.x + pLogo->fWidth, pLogo->pos.y, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(pLogo->pos.x, pLogo->pos.y + pLogo->fHeight, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(pLogo->pos.x + pLogo->fWidth, pLogo->pos.y + pLogo->fHeight, 0.0f);

		SetDefaultPolygon2D(pVtx, pLogo->col);
	}

	g_pVtxBuffTitleLogo->Unlock();
}

//==================================================================================
// --- タイトルロゴの終了処理 ---
//==================================================================================
void UninitTitleLogo(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		RELEASE(g_apTextureTitleLogo[nCntLogo]);
	}

	RELEASE(g_pVtxBuffTitleLogo);
}

//==================================================================================
// --- タイトルロゴの更新処理 ---
//==================================================================================
void UpdateTitleLogo(void)
{
	LOGO *pLogo = &g_aLogo[LOGOTYPE_CHAR];		// タイトルロゴ
	VERTEX_2D *pVtx = NULL;						// 頂点情報へのポインタ

	if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_A) || GetJoypadTrigger(JOYKEY_START))
	{ // 決定入力 : 演出を飛ばして完了状態に
		pLogo->pos.y = LOGO_POS_Y_SKIP;
		pLogo->col.a = 1.0f;
		pLogo->nCounterLogo = ANIMATION_TIME_C;
		g_bAppearEnd = true;
		g_bFlipEnd = true;
		SetPressEnterEffect(PRESSENTER_POS);
	}

	if (pLogo->nCounterLogo < ANIMATION_TIME_C)
	{ // 1. 最初の出現 : 上へ移動しながらフェードイン
		pLogo->nCounterLogo++;
		pLogo->col.a += 1.0f / ANIMATION_TIME_C;
		pLogo->pos.y -= APPEAR_MOVE_Y;

		if (pLogo->nCounterLogo >= ANIMATION_TIME_C)
		{ // 出現し終わった
			pLogo->nCounterLogo = ANIMATION_TIME_C;
			g_bAppearEnd = true;
		}
	}
	else
	{
		pLogo->nCounterLogo = ANIMATION_TIME_C;
		if (g_nCounterTitleLogo <= ANIMATION_TIME_C)
		{
			g_nCounterTitleLogo++;
		}
	}

	if (g_bAppearEnd == true)
	{
		// 上端がこの高さより下にある間は、少しずつ上へ移動する
		float fMoveLimitY = (float)(ANIMATION_TIME_C - pLogo->nCounterLogo);		// 移動を止める高さ

		if (g_bFlipEnd == true)
		{ // 3. 正しい向きで再出現 (フェードイン)
			if (pLogo->col.a < 1.0f)
			{
				if (pLogo->pos.y >= fMoveLimitY)
				{
					pLogo->nCounterLogo++;
					pLogo->pos.y -= FLIP_MOVE_Y;
				}

				pLogo->col.a += FLIP_FADE_SPEED;
				if (pLogo->col.a >= 1.0f)
				{ // 出現完了 → PressEnter 演出を表示
					pLogo->col.a = 1.0f;
					if (GetPressEnterEffect() == false)
					{
						SetPressEnterEffect(PRESSENTER_POS);
					}
				}
			}
		}
		else
		{ // 2. 逆さまのまま消える (フェードアウト)
			if (pLogo->col.a > 0.0f)
			{
				if (pLogo->pos.y >= fMoveLimitY)
				{
					pLogo->nCounterLogo++;
					pLogo->pos.y -= FLIP_MOVE_Y;
				}

				pLogo->col.a -= FLIP_FADE_SPEED;
				if (pLogo->col.a <= 0.0f)
				{ // 消えたら正しい向きへ
					pLogo->col.a = 0.0f;
					g_bFlipEnd = true;
				}
			}
		}
	}

	/*** 頂点情報の更新 ***/
	g_pVtxBuffTitleLogo->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++, pVtx += 4)
	{
		LOGO *pLogoVtx = &g_aLogo[nCntLogo];		// 更新するロゴ

		pVtx[0].pos = D3DXVECTOR3(pLogoVtx->pos.x, pLogoVtx->pos.y, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(pLogoVtx->pos.x + pLogoVtx->fWidth, pLogoVtx->pos.y, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(pLogoVtx->pos.x, pLogoVtx->pos.y + pLogoVtx->fHeight, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(pLogoVtx->pos.x + pLogoVtx->fWidth, pLogoVtx->pos.y + pLogoVtx->fHeight, 0.0f);

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].col = pLogoVtx->col;
		}

		/*** テクスチャ座標 (切り替え前は180度回転させて貼る) ***/
		D3DXVECTOR2 aTex[4] =
		{
			D3DXVECTOR2(LOGO_TEX_MARGIN, LOGO_TEX_MARGIN),
			D3DXVECTOR2(1.0f, LOGO_TEX_MARGIN),
			D3DXVECTOR2(LOGO_TEX_MARGIN, 1.0f),
			D3DXVECTOR2(1.0f, 1.0f),
		};

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].tex = (g_bFlipEnd == true) ? aTex[nCntVtx] : aTex[3 - nCntVtx];
		}
	}

	g_pVtxBuffTitleLogo->Unlock();
}

//==================================================================================
// --- タイトルロゴの描画処理 ---
//==================================================================================
void DrawTitleLogo(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** 頂点バッファ・頂点フォーマットの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffTitleLogo, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);

	for (int nCntLogo = 0; nCntLogo < LOGOTYPE_MAX; nCntLogo++)
	{
		/*** テクスチャの設定・ポリゴンの描画 ***/
		pDevice->SetTexture(0, g_apTextureTitleLogo[g_aLogo[nCntLogo].type]);
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntLogo, 2);
	}
}

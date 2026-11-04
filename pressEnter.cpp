//==================================================================================
//
// タイトル画面の「PressEnter」演出の処理 [pressEnter.cpp]
// Author : TENMA SAITO
//
// 縦に並んだアニメーションのコマ(PRESSENTER_ANIM_V枚)を順に切り替えながら、
// 透明な状態からフェードインして表示する。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "pressEnter.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define PRESSENTER_TEXTURE	"data/TEXTURE/PRESSENTER_ANIM.png"	// テクスチャのパス
#define PRESSENTER_WIDTH	(500.0f)		// 幅
#define PRESSENTER_HEIGHT	(290.0f)		// 高さ
#define PRESSENTER_ANIM_V	(6)				// テクスチャの縦のコマ数
#define ANIM_COUNT			(15)			// コマを切り替える間隔(フレーム)
#define FADEIN_SPEED		(0.02f)			// 1フレームあたりのα値の増加量

//**********************************************************************************
// *** PressEnter演出構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置(左上)
	D3DXCOLOR col;			// 色
	float fWidth;			// 幅
	float fHeight;			// 高さ
	int nCounterAnim;		// アニメーションカウンター
	int nPatternAnim;		// アニメーションのコマ番号
	bool bDisp;				// 描画しているか
} PRESSENTER;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void UpdateVertexPressEnter(void);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_pTexturePressEnter = NULL;		// テクスチャへのポインタ
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffPressEnter = NULL;	// 頂点バッファのポインタ
static PRESSENTER g_pressEnter;								// PressEnter演出の情報
static bool g_bEndPressEnter;								// フェードインし終えたか

//==================================================================================
// --- PressEnter演出の初期化処理 ---
//==================================================================================
void InitPressEnter(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	g_pressEnter.pos = WINDOW_MID;
	g_pressEnter.col = D3DXCOLOR_INV;
	g_pressEnter.fWidth = PRESSENTER_WIDTH;
	g_pressEnter.fHeight = PRESSENTER_HEIGHT;
	g_pressEnter.nCounterAnim = 0;
	g_pressEnter.nPatternAnim = 0;
	g_pressEnter.bDisp = false;
	g_bEndPressEnter = false;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice, PRESSENTER_TEXTURE, &g_pTexturePressEnter);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffPressEnter,
		NULL);

	/*** 頂点情報の設定 ***/
	VERTEX_2D *pVtx = NULL;		// 頂点情報へのポインタ
	g_pVtxBuffPressEnter->Lock(0, 0, (void**)&pVtx, 0);
	SetDefaultPolygon2D(pVtx, g_pressEnter.col);
	g_pVtxBuffPressEnter->Unlock();

	UpdateVertexPressEnter();
}

//==================================================================================
// --- PressEnter演出の終了処理 ---
//==================================================================================
void UninitPressEnter(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	RELEASE(g_pTexturePressEnter);
	RELEASE(g_pVtxBuffPressEnter);
}

//==================================================================================
// --- PressEnter演出の更新処理 ---
//==================================================================================
void UpdatePressEnter(void)
{
	if (g_pressEnter.bDisp == false)
	{ // 表示していない
		return;
	}

	if (GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
	{ // 決定入力でフェードインをスキップ
		g_pressEnter.col.a = 1.0f;
		g_bEndPressEnter = true;
	}

	if (g_pressEnter.col.a < 1.0f)
	{ // フェードイン
		g_pressEnter.col.a += FADEIN_SPEED;
		if (g_pressEnter.col.a >= 1.0f)
		{
			g_pressEnter.col.a = 1.0f;
			g_bEndPressEnter = true;
		}
	}

	/*** 一定間隔でコマを進める ***/
	g_pressEnter.nCounterAnim++;
	if (g_pressEnter.nCounterAnim % ANIM_COUNT == 0)
	{
		g_pressEnter.nPatternAnim++;
	}

	UpdateVertexPressEnter();
}

//==================================================================================
// --- PressEnter演出の描画処理 ---
//==================================================================================
void DrawPressEnter(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	if (g_pressEnter.bDisp == false)
	{ // 表示していない
		return;
	}

	/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffPressEnter, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);
	pDevice->SetTexture(0, g_pTexturePressEnter);

	/*** ポリゴンの描画 ***/
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

//==================================================================================
// --- PressEnter演出の表示開始 ---
//==================================================================================
void SetPressEnterEffect(D3DXVECTOR3 pos)
{
	g_pressEnter.pos = pos;
	g_pressEnter.col.a = 0.0f;
	g_pressEnter.bDisp = true;
}

//==================================================================================
// --- PressEnter演出を表示しているか ---
//==================================================================================
bool GetPressEnterEffect(void)
{
	return g_pressEnter.bDisp;
}

//==================================================================================
// --- PressEnter演出のフェードインが終わったか ---
//==================================================================================
bool GetEndPressEnterEffect(void)
{
	return g_bEndPressEnter;
}

//==================================================================================
// --- 頂点座標・頂点カラー・テクスチャ座標(現在のコマ)の更新 ---
//==================================================================================
static void UpdateVertexPressEnter(void)
{
	VERTEX_2D *pVtx = NULL;										// 頂点情報へのポインタ
	D3DXVECTOR3 pos = g_pressEnter.pos;							// 左上の位置
	float fTexHeight = 1.0f / PRESSENTER_ANIM_V;				// 1コマ分のテクスチャの高さ
	int nPattern = g_pressEnter.nPatternAnim % PRESSENTER_ANIM_V;	// 表示するコマ

	g_pVtxBuffPressEnter->Lock(0, 0, (void**)&pVtx, 0);

	/*** 頂点座標 (pos を左上とする) ***/
	pVtx[0].pos = D3DXVECTOR3(pos.x, pos.y, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(pos.x + g_pressEnter.fWidth, pos.y, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(pos.x, pos.y + g_pressEnter.fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(pos.x + g_pressEnter.fWidth, pos.y + g_pressEnter.fHeight, 0.0f);

	/*** 頂点カラー ***/
	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].col = g_pressEnter.col;
	}

	/*** テクスチャ座標 (現在のコマを切り出す) ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, fTexHeight * nPattern);
	pVtx[1].tex = D3DXVECTOR2(1.0f, fTexHeight * nPattern);
	pVtx[2].tex = D3DXVECTOR2(0.0f, fTexHeight * (nPattern + 1));
	pVtx[3].tex = D3DXVECTOR2(1.0f, fTexHeight * (nPattern + 1));

	g_pVtxBuffPressEnter->Unlock();
}

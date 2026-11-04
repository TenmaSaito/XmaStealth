//==================================================================================
//
// 丸影(減算合成の板ポリゴン)の処理 [shadow.cpp]
// Author : TENMA SAITO
//
// 持ち主の高さに応じて大きさと濃さが変わる。プレイヤーの影だけは毎フレーム
// 床・メッシュフィールドに落として、坂の傾きに合わせて回転させる。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "shadow.h"
#include "mathUtil.h"
#include "loadxfile.h"
#include "player.h"
#include "field.h"
#include "mesh.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_SHADOW				(300)		// 影の最大数
#define SHADOW_SIZE_X			(10)		// 影の初期の幅(半分)
#define SHADOW_SIZE_Z			(10)		// 影の初期の奥行(半分)
#define SHADOW_VTX_HEIGHT		(0.1f)		// 板ポリゴンの高さ(地面とのちらつき防止)
#define SHADOW_SCALE_RATE		(0.1f)		// 高さ1あたりの大きさの増加量
#define SHADOW_ALPHA_RATE		(0.001f)	// 高さ1あたりの濃さの減少量
#define SHADOW_ALPHA_MIN		(0.001f)	// 濃さの下限
#define SHADOW_ALPHA_MAX		(1.0f)		// 濃さの上限
#define SHADOW_FLOOR_OFFSET		(1.1f)		// 床に落とした時に浮かせる高さ
#define SHADOW_MESH_OFFSET		(1.3f)		// メッシュフィールドに落とした時に浮かせる高さ
#define SHADOW_HIGH_FLOOR_Y		(199.2f)	// プレイヤー以外の影 : この高さ以上なら2階の床に置く
#define SHADOW_LOW_FLOOR_Y		(0.3f)		// プレイヤー以外の影 : それ以外は1階の床に置く

//**********************************************************************************
// *** 影の構造体定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置
	D3DXVECTOR3 posOffset;				// 基準の高さ(大きさの計算に使う)
	D3DXVECTOR3 move;					// 移動量(落下用)
	D3DXVECTOR3 rot;					// 向き
	D3DXCOLOR col;						// 色(αが濃さ)
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fWidth;						// 現在の幅(半分)
	float fDepth;						// 現在の奥行(半分)
	float fWidthSet;					// 設置時の幅(半分)
	float fDepthSet;					// 設置時の奥行(半分)
	bool bUse;							// 使用しているか
} Shadow, *LPSHADOW;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void SetVertexShadow(int nIdxShadow);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffShadow = NULL;		// 頂点バッファのポインタ(全影分)
static Shadow g_aShadow[MAX_SHADOW];						// 影の情報
static int g_nIndexTextureShadow;							// 影のテクスチャ番号

//==================================================================================
// --- 影の初期化処理 ---
//==================================================================================
void InitShadow(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		LPSHADOW pShadow = &g_aShadow[nCntShadow];		// 初期化する影

		pShadow->pos = D3DXVECTOR3_NULL;
		pShadow->rot = D3DXVECTOR3_NULL;
		pShadow->fWidth = SHADOW_SIZE_X;
		pShadow->fDepth = SHADOW_SIZE_Z;
		pShadow->fWidthSet = SHADOW_SIZE_X;
		pShadow->fDepthSet = SHADOW_SIZE_Z;
		pShadow->col = D3DXCOLOR_NULL;
		pShadow->bUse = false;
	}

	g_nIndexTextureShadow = 0;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_SHADOW,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffShadow,
		NULL);

	/*** 法線・頂点カラー・テクスチャ座標の初期設定 (座標は SetVertexShadow で設定) ***/
	g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++, pVtx += 4)
	{
		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);		// 法線(上向き)
			pVtx[nCntVtx].col = D3DXCOLOR_NULL;
		}

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
	}

	g_pVtxBuffShadow->Unlock();

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		SetVertexShadow(nCntShadow);
	}
}

//==================================================================================
// --- 影の終了処理 ---
//==================================================================================
void UninitShadow(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffShadow);
}

//==================================================================================
// --- 影の更新処理 (ゲーム画面のプレイヤーの影を地面に落とす) ---
//==================================================================================
void UpdateShadow(void)
{
	Player *pPlayer = GetPlayer();		// プレイヤー

	if (GetMode() != MODE_GAME)
	{ // ゲーム画面以外では行わない
		return;
	}

	int nIdxShadow = pPlayer->nIdShadow;		// プレイヤーの影の番号
	if (nIdxShadow < 0 || nIdxShadow >= MAX_SHADOW || g_aShadow[nIdxShadow].bUse == false)
	{ // プレイヤーの影が無い
		return;
	}

	LPSHADOW pShadow = &g_aShadow[nIdxShadow];		// プレイヤーの影
	D3DXVECTOR3 posShadowOld = pShadow->pos;		// 落下前の位置

	PrintDebugProc("sPos : %~3f\n", pShadow->pos.x, pShadow->pos.y, pShadow->pos.z);

	/*** 落下させて、床に当たったらそこに置く ***/
	pShadow->pos.y += pShadow->move.y;

	if (CollisionFloor(&pShadow->pos, &posShadowOld, &pShadow->move, 0.0f))
	{
		pShadow->pos.y += SHADOW_FLOOR_OFFSET;
		pShadow->posOffset.y = pShadow->pos.y;
	}

	/*** メッシュフィールド(坂)に当たったら、面の傾きに合わせて回転させる ***/
	D3DXVECTOR3 vecNor;		// 当たった面の法線
	pShadow->rot = VECNULL;

	if (CollisionMeshField(&pShadow->pos, &posShadowOld, &pShadow->move, &vecNor))
	{
		pShadow->pos.y += SHADOW_MESH_OFFSET;
		pShadow->posOffset.y = pShadow->pos.y;

		vecNor *= D3DX_HALFPI;
		RepairRot(&vecNor, &vecNor);
		pShadow->rot.x = vecNor.z;
		pShadow->rot.z = vecNor.x;
	}

	PrintDebugProc("sPosNext : %~3f\n", pShadow->pos.x, pShadow->pos.y, pShadow->pos.z);

	/*** 重力 ***/
	pShadow->move.y += WORLD_GRAVITY;
	if (pShadow->move.y <= TERMINAL_VELOCITY)
	{
		pShadow->move.y = TERMINAL_VELOCITY;
	}
}

//==================================================================================
// --- 影の描画処理 (減算合成) ---
//==================================================================================
void DrawShadow(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス

	/*** Zバッファへの書き込みを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	/*** アルファブレンディングを減算合成に設定 ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	/*** 頂点バッファ・テクスチャの設定 (全影共通) ***/
	pDevice->SetStreamSource(0, g_pVtxBuffShadow, 0, sizeof(VERTEX_3D));
	pDevice->SetFVF(FVF_VERTEX_3D);
	pDevice->SetTexture(0, GetTexture(g_nIndexTextureShadow));

	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		LPSHADOW pShadow = &g_aShadow[nCntShadow];		// 描画する影

		if (pShadow->bUse == false)
		{ // 未使用
			continue;
		}

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pShadow->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pShadow->rot.y, pShadow->rot.x, pShadow->rot.z);
		D3DXMatrixMultiply(&pShadow->mtxWorld, &pShadow->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pShadow->pos.x, pShadow->pos.y, pShadow->pos.z);
		D3DXMatrixMultiply(&pShadow->mtxWorld, &pShadow->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pShadow->mtxWorld);

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntShadow, 2);
	}

	/*** アルファブレンディングを通常に戻す (重要！) ***/
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	/*** Zバッファの設定を戻す ***/
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

//==================================================================================
// --- 影の設置処理 ---
// ※ posOffset は現在未使用
// 戻り値 : 設置した影の番号 (空きが無ければ -1)
//==================================================================================
int SetShadow(float fWidth, float fDepth, D3DXVECTOR3 posOffset)
{
	for (int nCntShadow = 0; nCntShadow < MAX_SHADOW; nCntShadow++)
	{
		LPSHADOW pShadow = &g_aShadow[nCntShadow];		// 設置先の影

		if (pShadow->bUse == true)
		{ // 使用中
			continue;
		}

		pShadow->fWidth = fWidth;
		pShadow->fWidthSet = fWidth;
		pShadow->fDepth = fDepth;
		pShadow->fDepthSet = fDepth;
		pShadow->col.a = 1.0f;
		pShadow->bUse = true;

		SetVertexShadow(nCntShadow);

		return nCntShadow;
	}

	return -1;
}

//==================================================================================
// --- 影のテクスチャ番号の設定 ---
//==================================================================================
void SetIndexTextureShadow(int nIndexTexture)
{
	g_nIndexTextureShadow = nIndexTexture;
}

//==================================================================================
// --- 影の位置の設定 (持ち主の高さから大きさ・濃さも更新) ---
//==================================================================================
void SetPositionShadow(int nIdShadow, D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	Player *pPlayer = GetPlayer();		// プレイヤー

	if (nIdShadow < 0 || nIdShadow >= MAX_SHADOW)
	{ // 番号が範囲外
		return;
	}

	LPSHADOW pShadow = &g_aShadow[nIdShadow];		// 設定する影

	/*** 位置・向きの更新 ***/
	pShadow->pos.x = pos.x;
	pShadow->pos.z = pos.z;
	pShadow->rot.y = rot.y;

	if (nIdShadow != pPlayer->nIdShadow)
	{ // プレイヤー以外の影は、持ち主の高さで置く床を決める (プレイヤーの影は UpdateShadow で落とす)
		pShadow->pos.y = (pos.y >= SHADOW_HIGH_FLOOR_Y) ? SHADOW_HIGH_FLOOR_Y : SHADOW_LOW_FLOOR_Y;
	}

	/*** 持ち主が高いほど大きく、薄くする ***/
	float fHeight = pos.y - pShadow->posOffset.y;		// 基準からの高さ

	pShadow->fWidth = (fHeight * SHADOW_SCALE_RATE) + pShadow->fWidthSet;
	pShadow->fDepth = (fHeight * SHADOW_SCALE_RATE) + pShadow->fDepthSet;

	pShadow->col.a = 1.0f - (pos.y * SHADOW_ALPHA_RATE);
	if (pShadow->col.a < SHADOW_ALPHA_MIN)			pShadow->col.a = SHADOW_ALPHA_MIN;
	else if (pShadow->col.a > SHADOW_ALPHA_MAX)		pShadow->col.a = SHADOW_ALPHA_MAX;

	SetVertexShadow(nIdShadow);
}

//==================================================================================
// --- 影の消去処理 ---
//==================================================================================
void DestroyShadow(int nIdShadow)
{
	if (nIdShadow < 0 || nIdShadow >= MAX_SHADOW)
	{ // 番号が範囲外
		return;
	}

	g_aShadow[nIdShadow].bUse = false;
}

//==================================================================================
// --- 影のサイズの加算 ---
// 戻り値 : 加算後のサイズ (番号・種類が範囲外なら 0)
// ※ 頂点には反映しない (次の SetPositionShadow で再計算される)
//==================================================================================
float AddShadowSize(int nIdShadow, SHADOW_SIZE type, float fValue)
{
	if (nIdShadow < 0 || nIdShadow >= MAX_SHADOW || FAILED(CheckIndex(SHADOW_SIZE_MAX, type)))
	{ // 番号・種類が範囲外
		return 0.0f;
	}

	LPSHADOW pShadow = &g_aShadow[nIdShadow];		// 変更する影

	switch (type)
	{
	case SHADOW_SIZE_WIDTH:		// 幅
		pShadow->fWidth += fValue;
		return pShadow->fWidth;

	case SHADOW_SIZE_DEPTH:		// 奥行
		pShadow->fDepth += fValue;
		return pShadow->fDepth;

	default:
		return 0.0f;
	}
}

//==================================================================================
// --- 影の濃さの設定 ---
//==================================================================================
void InvisibleShadow(int nIdxShadow, float fAlpha)
{
	if (nIdxShadow < 0 || nIdxShadow >= MAX_SHADOW)
	{ // 番号が範囲外
		return;
	}

	g_aShadow[nIdxShadow].col.a = fAlpha;

	SetVertexShadow(nIdxShadow);
}

//==================================================================================
// --- 影1つ分の頂点座標・頂点カラーを現在の大きさ・濃さで設定 ---
// ※ 位置はワールドマトリックスで反映するため、頂点は原点中心で置く
//==================================================================================
static void SetVertexShadow(int nIdxShadow)
{
	LPSHADOW pShadow = &g_aShadow[nIdxShadow];		// 設定する影
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

	g_pVtxBuffShadow->Lock(0, 0, (void**)&pVtx, 0);
	pVtx += 4 * nIdxShadow;

	pVtx[0].pos = D3DXVECTOR3(-pShadow->fWidth, SHADOW_VTX_HEIGHT, pShadow->fDepth);
	pVtx[1].pos = D3DXVECTOR3(pShadow->fWidth, SHADOW_VTX_HEIGHT, pShadow->fDepth);
	pVtx[2].pos = D3DXVECTOR3(-pShadow->fWidth, SHADOW_VTX_HEIGHT, -pShadow->fDepth);
	pVtx[3].pos = D3DXVECTOR3(pShadow->fWidth, SHADOW_VTX_HEIGHT, -pShadow->fDepth);

	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].col = pShadow->col;
	}

	g_pVtxBuffShadow->Unlock();
}

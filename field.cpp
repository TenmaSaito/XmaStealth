//==================================================================================
//
// 床(1枚板ポリゴン)の処理 [field.cpp]
// Author : TENMA SAITO
//
// スクリプトの FIELDSET で配置される。テクスチャは分割数分だけ繰り返して貼り、
// MOVE を指定するとテクスチャがスクロールする。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "field.h"
#include "loadxfile.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_FIELD			(128)		// 床の最大数
#define FIELD_SIZE_X		(1000)		// 床の初期の横幅
#define FIELD_SIZE_Z		(1000)		// 床の初期の奥行

//**********************************************************************************
// *** 床構造体の定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;			// 位置(中心)
	D3DXVECTOR3 move;			// テクスチャのスクロール量(1フレームあたり)
	D3DXVECTOR3 rot;			// 角度
	D3DXMATRIX mtxWorld;		// ワールドマトリックス
	float fWidth;				// 横幅
	float fDepth;				// 奥行
	int nXBlock;				// 横方向のテクスチャの繰り返し数
	int nZBlock;				// 奥方向のテクスチャの繰り返し数
	int nIndexTexture;			// テクスチャ番号
	int nCounterAnim;			// スクロール用カウンター
	D3DCULL type;				// カリングタイプ
	bool bUse;					// 使われているか
} Field;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffField = NULL;	// 頂点バッファのポインタ(全床分)
static Field g_aField[MAX_FIELD];						// 床の情報

//==================================================================================
// --- 床の初期化処理 ---
//==================================================================================
void InitField(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		Field *pField = &g_aField[nCntField];		// 初期化する床

		pField->pos = D3DXVECTOR3_NULL;
		pField->move = D3DXVECTOR3_NULL;
		pField->rot = D3DXVECTOR3_NULL;
		pField->fWidth = FIELD_SIZE_X;
		pField->fDepth = FIELD_SIZE_Z;
		pField->nXBlock = 0;
		pField->nZBlock = 0;
		pField->nIndexTexture = 0;
		pField->nCounterAnim = 0;
		pField->bUse = false;
	}

	/*** 頂点バッファの生成 (全床分の四角形) ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_FIELD,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffField,
		NULL);

	/*** 頂点情報の初期設定 ***/
	g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++, pVtx += 4)
	{
		float fHalfWidth = g_aField[nCntField].fWidth * 0.5f;		// 横幅の半分
		float fHalfDepth = g_aField[nCntField].fDepth * 0.5f;		// 奥行の半分

		pVtx[0].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, fHalfDepth);		// 左奥
		pVtx[1].pos = D3DXVECTOR3(fHalfWidth, 0.0f, fHalfDepth);		// 右奥
		pVtx[2].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, -fHalfDepth);		// 左手前
		pVtx[3].pos = D3DXVECTOR3(fHalfWidth, 0.0f, -fHalfDepth);		// 右手前

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

	g_pVtxBuffField->Unlock();
}

//==================================================================================
// --- 床の終了処理 ---
//==================================================================================
void UninitField(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffField);
}

//==================================================================================
// --- 床の更新処理 (テクスチャのスクロール) ---
//==================================================================================
void UpdateField(void)
{
	VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ

	g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++, pVtx += 4)
	{
		Field *pField = &g_aField[nCntField];		// 更新する床

		if (pField->bUse == false)
		{ // 未使用
			continue;
		}

		pField->nCounterAnim++;

		float fScrollU = pField->move.x * pField->nCounterAnim;		// 横方向のスクロール量
		float fScrollV = pField->move.z * pField->nCounterAnim;		// 奥方向のスクロール量
		float fRepeatU = (float)pField->nXBlock;					// 横方向の繰り返し数
		float fRepeatV = (float)pField->nZBlock;					// 奥方向の繰り返し数

		pVtx[0].tex = D3DXVECTOR2(fScrollU, fScrollV);
		pVtx[1].tex = D3DXVECTOR2(fRepeatU + fScrollU, fScrollV);
		pVtx[2].tex = D3DXVECTOR2(fScrollU, fRepeatV + fScrollV);
		pVtx[3].tex = D3DXVECTOR2(fRepeatU + fScrollU, fRepeatV + fScrollV);
	}

	g_pVtxBuffField->Unlock();
}

//==================================================================================
// --- 床の描画処理 ---
//==================================================================================
void DrawField(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	DWORD dwCullDef = 0;							// 描画前のカリングタイプ

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		Field *pField = &g_aField[nCntField];		// 描画する床

		if (pField->bUse == false)
		{ // 未使用
			continue;
		}

		/*** この床のカリングタイプに変更 ***/
		pDevice->GetRenderState(D3DRS_CULLMODE, &dwCullDef);
		pDevice->SetRenderState(D3DRS_CULLMODE, pField->type);

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pField->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pField->rot.y, pField->rot.x, pField->rot.z);
		D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pField->pos.x, pField->pos.y, pField->pos.z);
		D3DXMatrixMultiply(&pField->mtxWorld, &pField->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pField->mtxWorld);

		/*** 頂点バッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, g_pVtxBuffField, 0, sizeof(VERTEX_3D));
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pField->nIndexTexture));

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntField, 2);

		/*** カリングタイプを戻す ***/
		pDevice->SetRenderState(D3DRS_CULLMODE, dwCullDef);
	}
}

//==================================================================================
// --- 床の設置処理 ---
// fWidth / fDepth : テクスチャ1枚分の大きさ (実際の大きさは繰り返し数倍になる)
//==================================================================================
void SetField(D3DXVECTOR3 pos, D3DXVECTOR3 move, D3DXVECTOR3 rot, float fWidth, float fDepth, int nIndexTexture, int nXblock, int nZblock, D3DCULL type)
{
	VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		Field *pField = &g_aField[nCntField];		// 設置先の床

		if (pField->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pField->pos = pos;
		pField->move = move;
		pField->rot = rot;
		pField->fWidth = fWidth * nXblock;
		pField->fDepth = fDepth * nZblock;
		pField->nXBlock = nXblock;
		pField->nZBlock = nZblock;
		pField->nIndexTexture = nIndexTexture;
		pField->type = type;
		pField->nCounterAnim = 0;
		pField->bUse = true;

		/*** この床の頂点情報の設定 ***/
		g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);
		pVtx += 4 * nCntField;

		float fHalfWidth = pField->fWidth * 0.5f;		// 横幅の半分
		float fHalfDepth = pField->fDepth * 0.5f;		// 奥行の半分

		pVtx[0].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, fHalfDepth);
		pVtx[1].pos = D3DXVECTOR3(fHalfWidth, 0.0f, fHalfDepth);
		pVtx[2].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, -fHalfDepth);
		pVtx[3].pos = D3DXVECTOR3(fHalfWidth, 0.0f, -fHalfDepth);

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f * nXblock, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f * nZblock);
		pVtx[3].tex = D3DXVECTOR2(1.0f * nXblock, 1.0f * nZblock);

		g_pVtxBuffField->Unlock();

		return;
	}
}

//==================================================================================
// --- 床との当たり判定 (プレイヤーのすり抜け防止) ---
// 戻り値 : 床の上に着地したか
//==================================================================================
bool CollisionFloor(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight)
{
	bool bLand = false;		// 着地したか

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		Field *pField = &g_aField[nCntField];		// 判定する床

		if (pField->bUse == false)
		{ // 未使用
			continue;
		}

		/*** 床の範囲 (x:左端 y:右端 z:手前端 w:奥端) ***/
		D3DXVECTOR4 rect;
		rect.x = pField->pos.x - (pField->fWidth * 0.5f);
		rect.y = pField->pos.x + (pField->fWidth * 0.5f);
		rect.z = pField->pos.z - (pField->fDepth * 0.5f);
		rect.w = pField->pos.z + (pField->fDepth * 0.5f);

		if (CollisionBoxZ(rect, *pPos) == false)
		{ // 床の範囲外
			continue;
		}

		if (pPos->y < pField->pos.y && pPosOld->y >= pField->pos.y)
		{ // 上から床を突き抜けた → 床の上に乗せる
			pPos->y = pField->pos.y;
			bLand = true;
		}
		else if (pPos->y + fHeight > pField->pos.y && pPosOld->y + fHeight <= pField->pos.y)
		{ // 下から頭が床を突き抜けた → 床の下に戻す
			pPos->y = pField->pos.y - fHeight;
		}
	}

	return bLand;
}

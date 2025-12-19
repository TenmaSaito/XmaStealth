//================================================================================================================
//
// DirectXの床の表示処理 [field.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "field.h"
#include "loadxfile.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define MAX_FIELD			(128)		// 床の最大数
#define FIELD_SIZE_X		(1000)		// ポリゴンの基本サイズ - X
#define FIELD_SIZE_Z		(1000)		// ポリゴンの基本サイズ - Y
#define FIELD_SPD			(2.0f)		// ポリゴンの移動スピード
#define FIELD_ROTSPD		(0.1f)		// ポリゴンの回転スピード
#define FIELD_WDSPD			(0.1f)		// ポリゴンの拡縮スピード
#define COLLISION_CODE		"FLOOR"		// 床の判定用文字列

//*************************************************************************************************
//*** 床構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;							// ポリゴンの位置
	D3DXVECTOR3 move;							// 移動量
	D3DXVECTOR3 rot;							// ポリゴンの角度
	D3DXMATRIX mtxWorld;						// ワールドマトリックス
	float fWidth;								// ポリゴンの横幅
	float fDepth;								// ポリゴンの奥行
	int nXBlock;								// 拡大倍率X
	int nZBlock;								// 拡大倍率Y
	int nIndexTexture;							// テクスチャの種類
	int nCounterAnim;							// アニメーションカウンター
	int nPatternAnim;							// アニメーションパターン
	char aCodename[MAX_PATH];					// 判定用名称
	D3DCULL type;								// カリングタイプ
	bool bUse;									// 使われているか
}Field;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureField = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffField = NULL;	// 頂点バッファのポインタ
Field g_aField[MAX_FIELD];

//================================================================================================================
// --- 床の初期化処理 ---
//================================================================================================================
void InitField(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		g_aField[nCntField].pos = D3DXVECTOR3_NULL;
		g_aField[nCntField].move = D3DXVECTOR3_NULL;
		g_aField[nCntField].rot = D3DXVECTOR3_NULL;
		g_aField[nCntField].fWidth = FIELD_SIZE_X;
		g_aField[nCntField].fDepth = FIELD_SIZE_Z;
		g_aField[nCntField].nXBlock = 0;
		g_aField[nCntField].nZBlock = 0;
		g_aField[nCntField].nIndexTexture = 0;
		g_aField[nCntField].nCounterAnim = 0;
		g_aField[nCntField].nPatternAnim = 0;
		g_aField[nCntField].bUse = false;
	}

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_FIELD,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffField,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aField[nCntField].pos.x - g_aField[nCntField].fWidth * 0.5f;
		pVtx[0].pos.y = 0.0f;
		pVtx[0].pos.z = g_aField[nCntField].pos.z + g_aField[nCntField].fDepth * 0.5f;

		pVtx[1].pos.x = g_aField[nCntField].pos.x + g_aField[nCntField].fWidth * 0.5f;
		pVtx[1].pos.y = 0.0f;
		pVtx[1].pos.z = g_aField[nCntField].pos.z + g_aField[nCntField].fDepth * 0.5f;

		pVtx[2].pos.x = g_aField[nCntField].pos.x - g_aField[nCntField].fWidth * 0.5f;
		pVtx[2].pos.y = 0.0f;
		pVtx[2].pos.z = g_aField[nCntField].pos.z - g_aField[nCntField].fDepth * 0.5f;

		pVtx[3].pos.x = g_aField[nCntField].pos.x + g_aField[nCntField].fWidth * 0.5f;
		pVtx[3].pos.y = 0.0f;
		pVtx[3].pos.z = g_aField[nCntField].pos.z - g_aField[nCntField].fDepth * 0.5f;

		/*** 法線ベクトルの設定 ***/
		pVtx[0].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[1].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[2].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVtx[3].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

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
	g_pVtxBuffField->Unlock();
}

//================================================================================================================
// --- 床の終了処理 ---
//================================================================================================================
void UninitField(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureField != NULL)
	{
		g_pTextureField->Release();
		g_pTextureField = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffField != NULL)
	{
		g_pVtxBuffField->Release();
		g_pVtxBuffField = NULL;
	}
}

//================================================================================================================
// --- 床の更新処理 ---
//================================================================================================================
void UpdateField(void)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		// 使われていれば
		if (g_aField[nCntField].bUse == true)
		{
			g_aField[nCntField].nCounterAnim++;

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f + (g_aField[nCntField].move.x * g_aField[nCntField].nCounterAnim), 0.0f + (g_aField[nCntField].move.z * g_aField[nCntField].nCounterAnim));
			pVtx[1].tex = D3DXVECTOR2((1.0f * g_aField[nCntField].nXBlock) + (g_aField[nCntField].move.x * g_aField[nCntField].nCounterAnim), 0.0f + (g_aField[nCntField].move.z * g_aField[nCntField].nCounterAnim));
			pVtx[2].tex = D3DXVECTOR2(0.0f + (g_aField[nCntField].move.x * g_aField[nCntField].nCounterAnim), (1.0f * g_aField[nCntField].nZBlock) + (g_aField[nCntField].move.z * g_aField[nCntField].nCounterAnim));
			pVtx[3].tex = D3DXVECTOR2((1.0f * g_aField[nCntField].nXBlock) + (g_aField[nCntField].move.x * g_aField[nCntField].nCounterAnim), (1.0f * g_aField[nCntField].nZBlock) + (g_aField[nCntField].move.z * g_aField[nCntField].nCounterAnim));
		}

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffField->Unlock();
}
//================================================================================================================
// --- 床の描画処理 ---
//================================================================================================================
void DrawField(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	DWORD typeDef;						// カリングタイプ保存用

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		if (g_aField[nCntField].bUse == true)
		{
			pDevice->GetRenderState(D3DRS_CULLMODE, &typeDef);
			pDevice->SetRenderState(D3DRS_CULLMODE, g_aField[nCntField].type);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aField[nCntField].mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				g_aField[nCntField].rot.y,		// Y軸回転
				g_aField[nCntField].rot.x,		// X軸回転
				g_aField[nCntField].rot.z);		// Z軸回転

			D3DXMatrixMultiply(&g_aField[nCntField].mtxWorld, &g_aField[nCntField].mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aField[nCntField].pos.x,
				g_aField[nCntField].pos.y,
				g_aField[nCntField].pos.z);

			D3DXMatrixMultiply(&g_aField[nCntField].mtxWorld, &g_aField[nCntField].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aField[nCntField].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffField, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_aField[nCntField].nIndexTexture));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntField,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数

			pDevice->SetRenderState(D3DRS_CULLMODE, typeDef);
		}
	}
}

//================================================================================================================
// --- 床の設置処理 ---
//================================================================================================================
void SetField(D3DXVECTOR3 pos, D3DXVECTOR3 move, D3DXVECTOR3 rot, float fWidth, float fDepth, int nIndexTexture, int nXblock, int nZblock, D3DCULL type)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		// 床構造体が使われていなければ
		if (g_aField[nCntField].bUse != true)
		{
			/*** 頂点バッファの設定 ***/
			g_pVtxBuffField->Lock(0, 0, (void**)&pVtx, 0);

			pVtx += 4 * nCntField;		// 頂点バッファをずらす

			// 引数の情報を適用
			g_aField[nCntField].pos = pos;
			g_aField[nCntField].move = move;
			g_aField[nCntField].rot = rot;
			g_aField[nCntField].fWidth = fWidth * nXblock;
			g_aField[nCntField].fDepth = fDepth * nZblock;
			g_aField[nCntField].nXBlock = nXblock;
			g_aField[nCntField].nZBlock = nZblock;
			g_aField[nCntField].nIndexTexture = nIndexTexture;
			g_aField[nCntField].type = type;
			g_aField[nCntField].nCounterAnim = 0;

			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = -g_aField[nCntField].fWidth * 0.5f;
			pVtx[0].pos.y = 0.0f;
			pVtx[0].pos.z = g_aField[nCntField].fDepth * 0.5f;

			pVtx[1].pos.x = g_aField[nCntField].fWidth * 0.5f;
			pVtx[1].pos.y = 0.0f;
			pVtx[1].pos.z = g_aField[nCntField].fDepth * 0.5f;

			pVtx[2].pos.x = -g_aField[nCntField].fWidth * 0.5f;
			pVtx[2].pos.y = 0.0f;
			pVtx[2].pos.z = -g_aField[nCntField].fDepth * 0.5f;

			pVtx[3].pos.x = g_aField[nCntField].fWidth * 0.5f;
			pVtx[3].pos.y = 0.0f;
			pVtx[3].pos.z = -g_aField[nCntField].fDepth * 0.5f;

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(1.0f * nXblock, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f * nZblock);
			pVtx[3].tex = D3DXVECTOR2(1.0f * nXblock, 1.0f * nZblock);

			g_aField[nCntField].bUse = true;		// 使用状態に設定

			/*** 頂点バッファの設定を終了 ***/
			g_pVtxBuffField->Unlock();

			break;
		}
	}
}

//================================================================================================================
// --- 床の判定処理(プレイヤーの脱走防止) ---
//================================================================================================================
bool CollisionFloor(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight)
{
	bool bLand = false;

	for (int nCntField = 0; nCntField < MAX_FIELD; nCntField++)
	{
		// 使われていれば
		if (g_aField[nCntField].bUse == true)
		{
			D3DXVECTOR4 rect;
			rect.x = g_aField[nCntField].pos.x - (g_aField[nCntField].fWidth * 0.5f);
			rect.y = g_aField[nCntField].pos.x + (g_aField[nCntField].fWidth * 0.5f);
			rect.z = g_aField[nCntField].pos.z - (g_aField[nCntField].fDepth * 0.5f);
			rect.w = g_aField[nCntField].pos.z + (g_aField[nCntField].fDepth * 0.5f);

			if (CollisionBoxZ(rect, *pPos))
			{
				if (pPos->y < g_aField[nCntField].pos.y
					&& pPosOld->y >= g_aField[nCntField].pos.y)
				{
					pPos->y = g_aField[nCntField].pos.y;
					bLand = true;
				}
				else if(pPos->y + fHeight > g_aField[nCntField].pos.y
					&& pPosOld->y + fHeight <= g_aField[nCntField].pos.y)
				{
					pPos->y = g_aField[nCntField].pos.y - fHeight;
				}
			}
		}
	}

	return bLand;
}
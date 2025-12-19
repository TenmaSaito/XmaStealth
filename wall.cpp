//================================================================================================================
//
// DirectXの壁表示処理 [wall.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "wall.h"
#include "loadxfile.h"
#include "player.h"
#include "debugproc.h"
#include "game.h"
#include "line.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define WALL_SIZE_X		(100)		// ポリゴンの基本サイズ - X
#define WALL_SIZE_Y		(150)		// ポリゴンの基本サイズ - Y
#define WALL_SPD		(2.0f)		// ポリゴンの移動スピード
#define WALL_ROTSPD		(0.1f)		// ポリゴンの回転スピード
#define WALL_WDSPD		(0.1f)		// ポリゴンの拡縮スピード
#define MAX_WALL		(128)		// 壁の数

//*************************************************************************************************
//*** 壁の構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;							// 位置
	D3DXVECTOR3 rot;							// 角度
	D3DXCOLOR col;								// 色
	D3DXMATRIX mtxWorld;						// ワールドマトリックス
	float fWidth;								// 横幅
	float fHeight;								// 奥行
	int nIndexTexture;							// テクスチャの種類
	bool bUse;									// 使用状況
	D3DCULL type;								// カリングタイプ
} Wall;

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureWall = NULL;	// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffWall = NULL;	// 頂点バッファのポインタ
Wall g_aWall[MAX_WALL];							// 壁の情報
float g_f[MAX_WALL];

//================================================================================================================
// --- リザルト用背景の初期化処理 ---
//================================================================================================================
void InitWall(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_3D *pVtx;					// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		g_aWall[nCntWall].pos = D3DXVECTOR3_NULL;
		g_aWall[nCntWall].rot = D3DXVECTOR3_NULL;
		g_aWall[nCntWall].col = D3DXCOLOR_NULL;
		g_aWall[nCntWall].fWidth = WALL_SIZE_X;
		g_aWall[nCntWall].fHeight = WALL_SIZE_Y;
		g_aWall[nCntWall].nIndexTexture = 0;
		g_aWall[nCntWall].bUse = false;
	}

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_WALL,
								D3DUSAGE_WRITEONLY,
								FVF_VERTEX_3D,
								D3DPOOL_MANAGED,
								&g_pVtxBuffWall,
								NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffWall->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aWall[nCntWall].pos.x - g_aWall[nCntWall].fWidth * 0.5f;
		pVtx[0].pos.y = g_aWall[nCntWall].pos.y + g_aWall[nCntWall].fHeight;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aWall[nCntWall].pos.x + g_aWall[nCntWall].fWidth * 0.5f;
		pVtx[1].pos.y = g_aWall[nCntWall].pos.y + g_aWall[nCntWall].fHeight;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_aWall[nCntWall].pos.x - g_aWall[nCntWall].fWidth * 0.5f;
		pVtx[2].pos.y = 0.0f;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aWall[nCntWall].pos.x + g_aWall[nCntWall].fWidth * 0.5f;
		pVtx[3].pos.y = 0.0f;
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

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffWall->Unlock();
}

//================================================================================================================
// --- リザルト用背景の終了処理 ---
//================================================================================================================
void UninitWall(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureWall != NULL)
	{
		g_pTextureWall->Release();
		g_pTextureWall = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffWall != NULL)
	{
		g_pVtxBuffWall->Release();
		g_pVtxBuffWall = NULL;
	}
}

//================================================================================================================
// --- リザルト用背景の更新処理 ---
//================================================================================================================
void UpdateWall(void)
{

}

//================================================================================================================
// --- リザルト用背景の描画処理 ---
//================================================================================================================
void DrawWall(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	DWORD typeDef;						// カリングタイプ保存用

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		if (g_aWall[nCntWall].bUse)
		{
			pDevice->GetRenderState(D3DRS_CULLMODE, &typeDef);
			pDevice->SetRenderState(D3DRS_CULLMODE, g_aWall[nCntWall].type);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aWall[nCntWall].mtxWorld);
			D3DXMatrixIdentity(&mtxRot);
			D3DXMatrixIdentity(&mtxTrans);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				g_aWall[nCntWall].rot.y,			// Y軸回転
				g_aWall[nCntWall].rot.x,			// X軸回転
				g_aWall[nCntWall].rot.z);			// Z軸回転

			D3DXMatrixMultiply(&g_aWall[nCntWall].mtxWorld, &g_aWall[nCntWall].mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aWall[nCntWall].pos.x,
				g_aWall[nCntWall].pos.y,
				g_aWall[nCntWall].pos.z);

			D3DXMatrixMultiply(&g_aWall[nCntWall].mtxWorld, &g_aWall[nCntWall].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aWall[nCntWall].mtxWorld);

			/*** 頂点バッファをデータストリームに設定 ***/
			pDevice->SetStreamSource(0, g_pVtxBuffWall, 0, sizeof(VERTEX_3D));

			/*** 頂点フォーマットの設定 ***/
			pDevice->SetFVF(FVF_VERTEX_3D);

			/*** テクスチャの設定 ***/
			pDevice->SetTexture(0, GetTexture(g_aWall[nCntWall].nIndexTexture));

			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntWall,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数

			pDevice->SetRenderState(D3DRS_CULLMODE, typeDef);
		}
	}
}

//================================================================================================================
// --- 壁の設置処理 ---
//================================================================================================================
void SetWall(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fHeight, int nIndexTexture, int nXblock, int nZblock, D3DXCOLOR col, D3DCULL type)
{
	VERTEX_3D* pVtx;					// 頂点情報へのポインタ

	/** 角度の修正! **/
	if (rot.x < -D3DX_PI)
	{
		rot.x += D3DX_PI * 2.0f;
	}
	else if (rot.x >= D3DX_PI)
	{
		rot.x -= D3DX_PI * 2.0f;
	}

	/** 角度の修正! **/
	if (rot.y < -D3DX_PI)
	{
		rot.y += D3DX_PI * 2.0f;
	}
	else if (rot.y >= D3DX_PI)
	{
		rot.y -= D3DX_PI * 2.0f;
	}

	/** 角度の修正! **/
	if (rot.z < -D3DX_PI)
	{
		rot.z += D3DX_PI * 2.0f;
	}
	else if (rot.z >= D3DX_PI)
	{
		rot.z -= D3DX_PI * 2.0f;
	}

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		if (g_aWall[nCntWall].bUse != true)
		{
			/*** 頂点バッファの設定 ***/
			g_pVtxBuffWall->Lock(0, 0, (void**)&pVtx, 0);

			pVtx += 4 * nCntWall;

			g_aWall[nCntWall].pos = pos;
			g_aWall[nCntWall].rot = rot;
			g_aWall[nCntWall].col = col;
			g_aWall[nCntWall].fWidth = fWidth * nXblock;
			g_aWall[nCntWall].fHeight = fHeight * nZblock;
			g_aWall[nCntWall].nIndexTexture = nIndexTexture;
			g_aWall[nCntWall].type = type;

			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = -g_aWall[nCntWall].fWidth * 0.5f;
			pVtx[0].pos.y = g_aWall[nCntWall].fHeight;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = g_aWall[nCntWall].fWidth * 0.5f;
			pVtx[1].pos.y = g_aWall[nCntWall].fHeight;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = -g_aWall[nCntWall].fWidth * 0.5f;
			pVtx[2].pos.y = 0.0f;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = g_aWall[nCntWall].fWidth * 0.5f;
			pVtx[3].pos.y = 0.0f;
			pVtx[3].pos.z = 0.0f;

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(1.0f * nXblock, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f * nZblock);
			pVtx[3].tex = D3DXVECTOR2(1.0f * nXblock, 1.0f * nZblock);

			/*** 頂点カラー設定 ***/
			pVtx[0].col = col;
			pVtx[1].col = col;
			pVtx[2].col = col;
			pVtx[3].col = col;

			g_aWall[nCntWall].bUse = true;

			/*** 頂点バッファの設定を終了 ***/
			g_pVtxBuffWall->Unlock();

			break;
		}
	}
}

//================================================================================================================
// --- 外積を利用した壁の当たり判定処理 ---
//================================================================================================================
void CollisionWall(D3DXVECTOR3* pPos, D3DXVECTOR3* pPosOld, D3DXVECTOR3* pMove)
{
	D3DXVECTOR3 pos[2];
	D3DXVECTOR3 vecMove = D3DXVECTOR3_NULL;		// 移動ベクトル
	D3DXVECTOR3 vecLine = D3DXVECTOR3_NULL;		// 境界線ベクトル
	D3DXVECTOR3 vecToPos = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosOld = D3DXVECTOR3_NULL;	// 過去位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecLostMove = D3DXVECTOR3_NULL;	// 壁にぶつかったことで失った、本来は壁の先にある残りのベクトル
	D3DXVECTOR3 vecToMove = D3DXVECTOR3_NULL;	// 最終的な移動ベクトル
	D3DXVECTOR3 rotVecLost = D3DXVECTOR3_NULL;	// 残りのベクトルの方向
	D3DXVECTOR3 vecCutLine = D3DXVECTOR3_NULL;	// 交点からの境界線ベクトル
	D3DXVECTOR3 vecLineNor = D3DXVECTOR3_NULL;	// 正規化した境界線ベクトル
	D3DXVECTOR3 vecLostMoveElse = D3DXVECTOR3_NULL;	// 壁にぶつかったことで失った、本来は壁の先にある残りのベクトル
	D3DXVECTOR3 vecNor = D3DXVECTOR3_NULL;		// 壁の逆法線ベクトル
	D3DXVECTOR3 vecToPosNor = D3DXVECTOR3_NULL;	// 壁の逆法線ベクトルの開始位置と現在位置を結んだベクトル
	Player *pP = GetPlayer();
	float fVecPos = 0.0f;
	float fVecPosOld = 0.0f;
	float fPosToMove = 0.0f;					// vecToPosとの外積
	float fLineToMove = 0.0f;					// vecLineとの外積
	float fPosToResult = 0.0f;
	float fPosOldToResult = 0.0f;
	float fRate = 0.0f;							// 面積比率
	float fRateMove = 0.0f;						// ベクトル比率
	float rotMoveToWall = 0.0f;					// 移動ベクトルと壁の角度の関係
	float fDot = 0.0f;
	float fLengthLine = 0.0f;
	float fLengthMove = 0.0f;					// 移動量
	float fRateLength = 0.0f;
	float fAngleMoveToLine = 0.0f;
	float fVecPosToNor = 0.0f;					// 逆法線との外積

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		if (g_aWall[nCntWall].bUse == true && pPos->y >= g_aWall[nCntWall].pos.y && pPos->y <= g_aWall[nCntWall].pos.y + g_aWall[nCntWall].fHeight)
		{
			/*** 移動ベクトルの取得 ***/
			vecMove = *pPos - *pPosOld;

			/*** 境界線ベクトルの取得の為、頂点を計算 ***/
			pos[0].x = g_aWall[nCntWall].pos.x - ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[0].y = g_aWall[nCntWall].pos.y;
			pos[0].z = g_aWall[nCntWall].pos.z + ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			pos[1].x = g_aWall[nCntWall].pos.x + ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[1].y = g_aWall[nCntWall].pos.y;
			pos[1].z = g_aWall[nCntWall].pos.z - ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			/*** 境界線ベクトルを計算 ***/
			vecLine = pos[1] - pos[0];

			RepairFloat(&vecLine.x);
			RepairFloat(&vecLine.z);

			/*** 現在位置との関係を外積を使い求める ***/
			vecToPos = *pPos - pos[0];

			fVecPos = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);

			RepairFloat(&fVecPos);

			/*** 過去位置との関係を外積を使い求める ***/
			vecToPosOld = *pPosOld - pos[0];

			fVecPosOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);

			RepairFloat(&fVecPosOld);

			/*** 強制位置の判定 ***/

			/** 現在位置との外積 **/
			fPosToMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);

			/** 最大値との外積 **/
			fLineToMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);

			/** 面積比率の計算 **/
			fRate = fPosToMove / fLineToMove;

			vecNor.x = vecLine.z;
			vecNor.z = -vecLine.x;

			D3DXVec3Normalize(&vecNor, &vecNor);

			fVecPosToNor = (vecNor.z * vecMove.x) - (vecNor.x * vecMove.z);

			D3DXVECTOR3 vecF;
			vecCutLine = vecLine;

			if (fVecPosToNor == 0)
			{
				fVecPosToNor = fabsf(fVecPosToNor);
				D3DXVec3Normalize(&vecCutLine, &vecCutLine);

				vecF = vecCutLine * fVecPosToNor;
			}
			else if (fVecPosToNor > 0)
			{
				fVecPosToNor = fabsf(fVecPosToNor);
				D3DXVec3Normalize(&vecCutLine, &vecCutLine);

				vecF = -vecCutLine * fVecPosToNor;
			}
			else if (fVecPosToNor < 0)
			{
				fVecPosToNor = fabsf(fVecPosToNor);
				D3DXVec3Normalize(&vecCutLine, &vecCutLine);

				vecF = vecCutLine * fVecPosToNor;
			}

			/*** プレイヤーの壁のめり込み判定 ***/
			if (fVecPos < 0 && fVecPosOld >= 0)
			{
				/*** もしも比率が範囲内であれば,衝突 ***/
				if (fRate >= 0.0f && fRate <= 1.0f)
				{
					pPos->x = pos[0].x + (vecLine.x * fRate) + vecF.x;
					pPos->z = pos[0].z + (vecLine.z * fRate) + vecF.z;

					//pMove->x = pMove->x * 0.95f;
					//pMove->z = pMove->z * 0.95f;
				}
			}
		}
	}
}

//================================================================================================================
// --- 外積及び内積を利用した壁の反射処理 ---
//================================================================================================================
void ReflectWall(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove)
{
	D3DXVECTOR3 pos[2];
	D3DXVECTOR3 vecMove = D3DXVECTOR3_NULL;		// 移動ベクトル
	D3DXVECTOR3 vecLine = D3DXVECTOR3_NULL;		// 境界線ベクトル
	D3DXVECTOR3 vecToPos = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosOld = D3DXVECTOR3_NULL;	// 過去位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecNor = D3DXVECTOR3_NULL;		// 境界線ベクトルと垂直な法線ベクトル
	D3DXVECTOR3 vecDotNor = D3DXVECTOR3_NULL;	// 大きさを考慮した法線ベクトル
	D3DXVECTOR3 vecMoveRef = D3DXVECTOR3_NULL;	// 最終的な移動ベクトル
	D3DXVECTOR3 posIntersect = D3DXVECTOR3_NULL;// 交点の座標
	D3DXVECTOR3 vecLostMove = D3DXVECTOR3_NULL;	// 壁にぶつかったことで失った、本来は壁の先にある残りのベクトル
	D3DXVECTOR3 vecLostMoveRef = D3DXVECTOR3_NULL;	// 角度を直した残りのベクトル
	D3DXVECTOR3 vecCutLine = D3DXVECTOR3_NULL;	// 交点からの境界線ベクトル
	float fRate = 0.0f;			// 面積比率
	float fDot = 0.0f;			// 内積の結果
	float fVecPos = 0.0f;
	float fVecPosOld = 0.0f;
	float fPosToMove = 0.0f;	// vecToPosとの外積
	float fLineToMove = 0.0f;	// vecLineとの外積
	float fLengthVec = 0.0f;	// 境界線ベクトルの長さ
	float fDotLost = 0.0f;
	float fLengthLine = 0.0f;
	float fLengthMove = 0.0f;					// 移動量
	float fRateLength = 0.0f;
	float fAngleMoveToLine = 0.0f;
	float fLengthMoveRef = 0.0f;	// 反射時のベクトルの大きさ(moveから計算)

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		if (g_aWall[nCntWall].bUse == true)
		{
			/*** 移動ベクトルの取得 ***/
			vecMove = *pPos - *pPosOld;

			/*** 境界線ベクトルの取得の為、頂点を計算 ***/
			pos[0].x = g_aWall[nCntWall].pos.x - ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[0].y = g_aWall[nCntWall].pos.y;
			pos[0].z = g_aWall[nCntWall].pos.z + ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			pos[1].x = g_aWall[nCntWall].pos.x + ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[1].y = g_aWall[nCntWall].pos.y;
			pos[1].z = g_aWall[nCntWall].pos.z - ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			/*** 境界線ベクトルを計算 ***/
			vecLine = pos[1] - pos[0];
			fLengthVec = sqrtf(powf(vecLine.x, 2.0f) + powf(vecLine.z, 2.0f));

			/*** 法線ベクトルを計算 ***/
			vecNor.x = vecLine.x * cosf(-(D3DX_PI * 0.5f)) - vecLine.z * sinf(-(D3DX_PI * 0.5f));
			vecNor.y = 0.0f;
			vecNor.z = vecLine.x * sinf(-(D3DX_PI * 0.5f)) + vecLine.z * cosf(-(D3DX_PI * 0.5f));

			/** 法線ベクトルを正規化 **/
			D3DXVec3Normalize(&vecNor, &vecNor);

			/*** 現在位置との関係を外積を使い求める ***/
			vecToPos = *pPos - pos[0];
			fVecPos = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);

			/*** 過去位置との関係を外積を使い求める ***/
			vecToPosOld = *pPosOld - pos[0];
			fVecPosOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);

			/*** 強制位置の判定 ***/

			/** 現在位置との外積 **/
			fPosToMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);

			/** 最大値との外積 **/
			fLineToMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);

			/** 面積比率の計算 **/
			fRate = fPosToMove / fLineToMove;

			/* 交点座標の計算 */
			posIntersect.x = pos[0].x + (vecLine.x * fRate);
			posIntersect.z = pos[0].z + (vecLine.z * fRate);

			/** 失った移動ベクトルの計算 **/
			vecLostMove = vecMove - (posIntersect - *pPosOld);

			/** 交点からの残りの境界線ベクトルの計算 **/
			vecCutLine = vecLine - (vecLine * fRate);

			/** 各ベクトルの長さの計算 **/
			fLengthLine = sqrtf(powf(vecCutLine.x, 2.0f) + powf(vecCutLine.z, 2.0f));
			fLengthMove = sqrtf(powf(vecLostMove.x, 2.0f) + powf(vecLostMove.z, 2.0f));

			fRateLength = fLengthLine - fLengthMove;

			/** 二つのベクトルの内積を求める **/
			fDotLost = (vecLostMove.x * vecCutLine.x) + (vecLostMove.z * vecCutLine.z);

			/** 二つのベクトル間の角度を求める **/
			fAngleMoveToLine = acosf((fDot) / (fLengthLine * fLengthMove));

			/** 失った移動ベクトルを本来の方向に変更 **/
			vecLostMoveRef.x = vecLostMove.x * cosf((D3DX_PI - fAngleMoveToLine) * 2.0f) - vecLostMove.z * sinf((D3DX_PI - fAngleMoveToLine) * 2.0f);
			vecLostMoveRef.z = vecLostMove.x * sinf((D3DX_PI - fAngleMoveToLine) * 2.0f) + vecLostMove.z * cosf((D3DX_PI - fAngleMoveToLine) * 2.0f);

			/** 内積の計算 **/
			fDot = (-vecMove.x * vecNor.x) + (-vecMove.z * vecNor.z);

			/** 法線ベクトルに内積の結果を適用 **/
			vecDotNor = (vecNor * fDot) * 2;

			/** 移動ベクトルの結果を計算 **/
			vecMoveRef = (vecMove + vecDotNor);

			/*** プレイヤーの壁のめり込み判定 ***/
			if (fVecPos <= 0 && fVecPosOld >= 0)
			{
				/*** もしも比率が範囲内であれば,衝突 ***/
				if (fRate >= 0.0f && fRate <= 1.0f)
				{
					pPos->x = posIntersect.x + vecLostMoveRef.x;
					pPos->z = posIntersect.z + vecLostMoveRef.z;

					pMove->x = vecMoveRef.x;
					pMove->z = vecMoveRef.z;
				}
			}
		}
	}
}

void ReflectLine(D3DXVECTOR3 pPosStart, D3DXVECTOR3 pPosEnd, D3DXVECTOR3* pPosIntersect, D3DXVECTOR3* pMove)
{
	D3DXVECTOR3 pos[2];
	D3DXVECTOR3 vecMove = D3DXVECTOR3_NULL;		// 移動ベクトル
	D3DXVECTOR3 vecLine = D3DXVECTOR3_NULL;		// 境界線ベクトル
	D3DXVECTOR3 vecToPos = D3DXVECTOR3_NULL;	// 位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecToPosOld = D3DXVECTOR3_NULL;	// 過去位置と境界線のはじめを結んだベクトル
	D3DXVECTOR3 vecNor = D3DXVECTOR3_NULL;		// 境界線ベクトルと垂直な法線ベクトル
	D3DXVECTOR3 vecDotNor = D3DXVECTOR3_NULL;	// 大きさを考慮した法線ベクトル
	D3DXVECTOR3 vecMoveRef = D3DXVECTOR3_NULL;	// 最終的な移動ベクトル
	D3DXVECTOR3 posIntersect = D3DXVECTOR3_NULL;// 交点の座標
	D3DXVECTOR3 vecLostMove = D3DXVECTOR3_NULL;	// 壁にぶつかったことで失った、本来は壁の先にある残りのベクトル
	D3DXVECTOR3 vecLostMoveRef = D3DXVECTOR3_NULL;	// 角度を直した残りのベクトル
	D3DXVECTOR3 vecCutLine = D3DXVECTOR3_NULL;	// 交点からの境界線ベクトル
	float fRate = 0.0f;			// 面積比率
	float fDot = 0.0f;			// 内積の結果
	float fVecPos = 0.0f;
	float fVecPosOld = 0.0f;
	float fPosToMove = 0.0f;	// vecToPosとの外積
	float fLineToMove = 0.0f;	// vecLineとの外積
	float fLengthVec = 0.0f;	// 境界線ベクトルの長さ
	float fDotLost = 0.0f;
	float fLengthLine = 0.0f;
	float fLengthMove = 0.0f;					// 移動量
	float fRateLength = 0.0f;
	float fAngleMoveToLine = 0.0f;
	float fLengthMoveRef = 0.0f;	// 反射時のベクトルの大きさ(moveから計算)

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		if (g_aWall[nCntWall].bUse == true)
		{
			/*** 移動ベクトルの取得 ***/
			vecMove = pPosStart - pPosEnd;

			/*** 境界線ベクトルの取得の為、頂点を計算 ***/
			pos[0].x = g_aWall[nCntWall].pos.x - ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[0].y = g_aWall[nCntWall].pos.y;
			pos[0].z = g_aWall[nCntWall].pos.z + ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			pos[1].x = g_aWall[nCntWall].pos.x + ((g_aWall[nCntWall].fWidth * 0.5f) * cosf(g_aWall[nCntWall].rot.y));
			pos[1].y = g_aWall[nCntWall].pos.y;
			pos[1].z = g_aWall[nCntWall].pos.z - ((g_aWall[nCntWall].fWidth * 0.5f) * sinf(g_aWall[nCntWall].rot.y));

			/*** 境界線ベクトルを計算 ***/
			vecLine = pos[1] - pos[0];
			fLengthVec = sqrtf(powf(vecLine.x, 2.0f) + powf(vecLine.z, 2.0f));

			/*** 法線ベクトルを計算 ***/
			vecNor.x = vecLine.x * cosf(-(D3DX_PI * 0.5f)) - vecLine.z * sinf(-(D3DX_PI * 0.5f));
			vecNor.y = 0.0f;
			vecNor.z = vecLine.x * sinf(-(D3DX_PI * 0.5f)) + vecLine.z * cosf(-(D3DX_PI * 0.5f));

			/** 法線ベクトルを正規化 **/
			D3DXVec3Normalize(&vecNor, &vecNor);

			/*** 現在位置との関係を外積を使い求める ***/
			vecToPos = pPosStart - pos[0];
			fVecPos = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);

			/*** 過去位置との関係を外積を使い求める ***/
			vecToPosOld = pPosEnd - pos[0];
			fVecPosOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);

			RepairFloat(&fVecPosOld);

			/*** 強制位置の判定 ***/

			/** 現在位置との外積 **/
			fPosToMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);

			/** 最大値との外積 **/
			fLineToMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);

			/** 面積比率の計算 **/
			fRate = fPosToMove / fLineToMove;

			/* 交点座標の計算 */
			posIntersect.x = pos[0].x + (vecLine.x * fRate);
			posIntersect.z = pos[0].z + (vecLine.z * fRate);

			/** 失った移動ベクトルの計算 **/
			vecLostMove = vecMove - (posIntersect - pPosEnd);

			/** 交点からの残りの境界線ベクトルの計算 **/
			vecCutLine = vecLine - (vecLine * fRate);

			/** 各ベクトルの長さの計算 **/
			fLengthLine = sqrtf(powf(vecCutLine.x, 2.0f) + powf(vecCutLine.z, 2.0f));
			fLengthMove = sqrtf(powf(vecLostMove.x, 2.0f) + powf(vecLostMove.z, 2.0f));

			fRateLength = fLengthLine - fLengthMove;

			/** 二つのベクトルの内積を求める **/
			fDotLost = (vecLostMove.x * vecCutLine.x) + (vecLostMove.z * vecCutLine.z);

			/** 二つのベクトル間の角度を求める **/
			fAngleMoveToLine = acosf((fDot) / (fLengthLine * fLengthMove));

			/** 失った移動ベクトルを本来の方向に変更 **/
			vecLostMoveRef.x = vecLostMove.x * cosf((D3DX_PI - fAngleMoveToLine) * 2.0f) - vecLostMove.z * sinf((D3DX_PI - fAngleMoveToLine) * 2.0f);
			vecLostMoveRef.z = vecLostMove.x * sinf((D3DX_PI - fAngleMoveToLine) * 2.0f) + vecLostMove.z * cosf((D3DX_PI - fAngleMoveToLine) * 2.0f);

			/** 内積の計算 **/
			fDot = (-vecMove.x * vecNor.x) + (-vecMove.z * vecNor.z);

			/** 法線ベクトルに内積の結果を適用 **/
			vecDotNor = (vecNor * fDot) * 2;

			/** 移動ベクトルの結果を計算 **/
			vecMoveRef = (vecMove + vecDotNor);

			/*** プレイヤーの壁のめり込み判定 ***/
			if (fVecPos <= 0 && fVecPosOld >= 0)
			{
				/*** もしも比率が範囲内であれば,衝突 ***/
				if (fRate >= 0.0f && fRate <= 1.0f)
				{
					*pPosIntersect = posIntersect;

					pMove->x = vecMoveRef.x;
					pMove->z = vecMoveRef.z;
				}
			}
		}
	}
}
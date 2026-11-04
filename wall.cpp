//==================================================================================
//
// 壁(縦向きの板ポリゴン)の処理 [wall.cpp]
// Author : TENMA SAITO
//
// 当たり判定は、壁を真上から見た線分として外積で行う。
// (外積が 正 → 表側 / 負 → 裏側。表から裏へまたいだら押し戻す)
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "wall.h"
#include "loadxfile.h"
#include "player.h"
#include "debugproc.h"
#include "game.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_WALL		(128)		// 壁の最大数
#define WALL_SIZE_X		(100)		// 壁の初期の横幅
#define WALL_SIZE_Y		(150)		// 壁の初期の高さ

//**********************************************************************************
// *** 壁の構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;			// 位置(下辺の中心)
	D3DXVECTOR3 rot;			// 角度
	D3DXCOLOR col;				// 色
	D3DXMATRIX mtxWorld;		// ワールドマトリックス
	float fWidth;				// 横幅
	float fHeight;				// 高さ
	int nIndexTexture;			// テクスチャ番号
	bool bUse;					// 使用状況
	D3DCULL type;				// カリングタイプ
} Wall;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void GetWallLine(const Wall *pWall, D3DXVECTOR3 *pPosStart, D3DXVECTOR3 *pVecLine);
static bool IsInWallHeight(const Wall *pWall, const D3DXVECTOR3 *pPos);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffWall = NULL;		// 頂点バッファのポインタ(全壁分)
static Wall g_aWall[MAX_WALL];								// 壁の情報

//==================================================================================
// --- 壁の初期化処理 ---
//==================================================================================
void InitWall(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	VERTEX_3D *pVtx = NULL;							// 頂点情報へのポインタ

	/*** 各変数の初期化 ***/
	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		Wall *pWall = &g_aWall[nCntWall];		// 初期化する壁

		pWall->pos = D3DXVECTOR3_NULL;
		pWall->rot = D3DXVECTOR3_NULL;
		pWall->col = D3DXCOLOR_NULL;
		pWall->fWidth = WALL_SIZE_X;
		pWall->fHeight = WALL_SIZE_Y;
		pWall->nIndexTexture = 0;
		pWall->bUse = false;
	}

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4 * MAX_WALL,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffWall,
		NULL);

	/*** 頂点情報の初期設定 ***/
	g_pVtxBuffWall->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++, pVtx += 4)
	{
		float fHalfWidth = g_aWall[nCntWall].fWidth * 0.5f;		// 横幅の半分
		float fHeight = g_aWall[nCntWall].fHeight;				// 高さ

		pVtx[0].pos = D3DXVECTOR3(-fHalfWidth, fHeight, 0.0f);		// 左上
		pVtx[1].pos = D3DXVECTOR3(fHalfWidth, fHeight, 0.0f);		// 右上
		pVtx[2].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, 0.0f);			// 左下
		pVtx[3].pos = D3DXVECTOR3(fHalfWidth, 0.0f, 0.0f);			// 右下

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);		// 法線(手前向き)
			pVtx[nCntVtx].col = D3DXCOLOR_NULL;
		}

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
	}

	g_pVtxBuffWall->Unlock();
}

//==================================================================================
// --- 壁の終了処理 ---
//==================================================================================
void UninitWall(void)
{
	/*** 頂点バッファの破棄 ***/
	RELEASE(g_pVtxBuffWall);
}

//==================================================================================
// --- 壁の更新処理 ---
//==================================================================================
void UpdateWall(void)
{

}

//==================================================================================
// --- 壁の描画処理 ---
//==================================================================================
void DrawWall(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	DWORD dwCullDef = 0;							// 描画前のカリングタイプ

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		Wall *pWall = &g_aWall[nCntWall];		// 描画する壁

		if (pWall->bUse == false)
		{ // 未使用
			continue;
		}

		/*** この壁のカリングタイプに変更 ***/
		pDevice->GetRenderState(D3DRS_CULLMODE, &dwCullDef);
		pDevice->SetRenderState(D3DRS_CULLMODE, pWall->type);

		/*** ワールドマトリックスの計算 (回転 → 平行移動の順) ***/
		D3DXMatrixIdentity(&pWall->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pWall->rot.y, pWall->rot.x, pWall->rot.z);
		D3DXMatrixMultiply(&pWall->mtxWorld, &pWall->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pWall->pos.x, pWall->pos.y, pWall->pos.z);
		D3DXMatrixMultiply(&pWall->mtxWorld, &pWall->mtxWorld, &mtxTrans);

		pDevice->SetTransform(D3DTS_WORLD, &pWall->mtxWorld);

		/*** 頂点バッファ・テクスチャの設定 ***/
		pDevice->SetStreamSource(0, g_pVtxBuffWall, 0, sizeof(VERTEX_3D));
		pDevice->SetFVF(FVF_VERTEX_3D);
		pDevice->SetTexture(0, GetTexture(pWall->nIndexTexture));

		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntWall, 2);

		/*** カリングタイプを戻す ***/
		pDevice->SetRenderState(D3DRS_CULLMODE, dwCullDef);
	}
}

//==================================================================================
// --- 壁の設置処理 ---
// fWidth / fHeight : テクスチャ1枚分の大きさ (実際の大きさは繰り返し数倍になる)
//==================================================================================
void SetWall(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fHeight, int nIndexTexture, int nXblock, int nZblock, D3DXCOLOR col, D3DCULL type)
{
	VERTEX_3D *pVtx = NULL;		// 頂点情報へのポインタ

	/*** 角度を -π～π に収める ***/
	rot.x = RepairRot(rot.x);
	rot.y = RepairRot(rot.y);
	rot.z = RepairRot(rot.z);

	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		Wall *pWall = &g_aWall[nCntWall];		// 設置先の壁

		if (pWall->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 情報の設定 ***/
		pWall->pos = pos;
		pWall->rot = rot;
		pWall->col = col;
		pWall->fWidth = fWidth * nXblock;
		pWall->fHeight = fHeight * nZblock;
		pWall->nIndexTexture = nIndexTexture;
		pWall->type = type;
		pWall->bUse = true;

		/*** この壁の頂点情報の設定 ***/
		g_pVtxBuffWall->Lock(0, 0, (void**)&pVtx, 0);
		pVtx += 4 * nCntWall;

		float fHalfWidth = pWall->fWidth * 0.5f;		// 横幅の半分

		pVtx[0].pos = D3DXVECTOR3(-fHalfWidth, pWall->fHeight, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(fHalfWidth, pWall->fHeight, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(-fHalfWidth, 0.0f, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(fHalfWidth, 0.0f, 0.0f);

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f * nXblock, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f * nZblock);
		pVtx[3].tex = D3DXVECTOR2(1.0f * nXblock, 1.0f * nZblock);

		for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
		{
			pVtx[nCntVtx].col = col;
		}

		g_pVtxBuffWall->Unlock();

		return;
	}
}

//==================================================================================
// --- 壁との当たり判定 (表側から裏側へまたいだら壁の手前に押し戻し、壁に沿って滑らせる) ---
//==================================================================================
void CollisionWall(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove)
{
	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		const Wall *pWall = &g_aWall[nCntWall];		// 判定する壁

		if (pWall->bUse == false || IsInWallHeight(pWall, pPos) == false)
		{ // 未使用 or 高さが合わない
			continue;
		}

		D3DXVECTOR3 posStart, vecLine;				// 壁の線分(始点と向き)
		D3DXVECTOR3 vecMove = *pPos - *pPosOld;		// 移動ベクトル

		GetWallLine(pWall, &posStart, &vecLine);
		RepairFloat(&vecLine.x);
		RepairFloat(&vecLine.z);

		/*** 現在・前フレームの位置が壁のどちら側か ***/
		D3DXVECTOR3 vecToPos = *pPos - posStart;			// 始点 → 現在位置
		D3DXVECTOR3 vecToPosOld = *pPosOld - posStart;		// 始点 → 前フレームの位置

		float fCross = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);				// 現在位置との外積
		float fCrossOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);	// 前フレームの位置との外積
		RepairFloat(&fCross);
		RepairFloat(&fCrossOld);

		/*** 移動ベクトルと壁の交点の位置 (始点からの割合) ***/
		float fCrossPosMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);		// 始点→現在位置 と 移動ベクトルの外積
		float fCrossLineMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);		// 壁 と 移動ベクトルの外積
		float fRate = fCrossPosMove / fCrossLineMove;									// 交点の割合

		/*** 押し戻し後に壁に沿って滑らせる量 ***/
		D3DXVECTOR3 vecNor = D3DXVECTOR3(vecLine.z, 0.0f, -vecLine.x);		// 壁の法線
		D3DXVec3Normalize(&vecNor, &vecNor);

		float fCrossNorMove = (vecNor.z * vecMove.x) - (vecNor.x * vecMove.z);		// 法線と移動ベクトルの外積
		D3DXVECTOR3 vecLineDir;														// 壁の向き(単位ベクトル)
		D3DXVec3Normalize(&vecLineDir, &vecLine);

		D3DXVECTOR3 vecSlide = (fCrossNorMove > 0)
			? -vecLineDir * fabsf(fCrossNorMove)
			: vecLineDir * fabsf(fCrossNorMove);									// 滑らせる量

		if (fCross < 0 && fCrossOld >= 0 && fRate >= 0.0f && fRate <= 1.0f)
		{ // 表から裏へ壁をまたいだ → 交点まで押し戻して滑らせる
			pPos->x = posStart.x + (vecLine.x * fRate) + vecSlide.x;
			pPos->z = posStart.z + (vecLine.z * fRate) + vecSlide.z;
		}
	}
}

//==================================================================================
// --- 壁での反射 (表側から裏側へまたいだら、交点で跳ね返す) ---
//==================================================================================
void ReflectWall(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove)
{
	for (int nCntWall = 0; nCntWall < MAX_WALL; nCntWall++)
	{
		const Wall *pWall = &g_aWall[nCntWall];		// 判定する壁

		if (pWall->bUse == false || IsInWallHeight(pWall, pPos) == false)
		{ // 未使用 or 高さが合わない
			continue;
		}

		D3DXVECTOR3 posStart, vecLine;				// 壁の線分(始点と向き)
		D3DXVECTOR3 vecMove = *pPos - *pPosOld;		// 移動ベクトル

		GetWallLine(pWall, &posStart, &vecLine);

		/*** 壁の法線 (線分を -90度回転) ***/
		D3DXVECTOR3 vecNor = D3DXVECTOR3(
			vecLine.x * cosf(-D3DX_HALFPI) - vecLine.z * sinf(-D3DX_HALFPI),
			0.0f,
			vecLine.x * sinf(-D3DX_HALFPI) + vecLine.z * cosf(-D3DX_HALFPI));		// 壁の法線
		D3DXVec3Normalize(&vecNor, &vecNor);

		/*** 現在・前フレームの位置が壁のどちら側か ***/
		D3DXVECTOR3 vecToPos = *pPos - posStart;			// 始点 → 現在位置
		D3DXVECTOR3 vecToPosOld = *pPosOld - posStart;		// 始点 → 前フレームの位置

		float fCross = (vecLine.z * vecToPos.x) - (vecLine.x * vecToPos.z);				// 現在位置との外積
		float fCrossOld = (vecLine.z * vecToPosOld.x) - (vecLine.x * vecToPosOld.z);	// 前フレームの位置との外積

		if ((fCross <= 0 && fCrossOld >= 0) == false)
		{ // 壁をまたいでいない
			continue;
		}

		/*** 移動ベクトルと壁の交点 ***/
		float fCrossPosMove = (vecToPos.z * vecMove.x) - (vecToPos.x * vecMove.z);		// 始点→現在位置 と 移動ベクトルの外積
		float fCrossLineMove = (vecLine.z * vecMove.x) - (vecLine.x * vecMove.z);		// 壁 と 移動ベクトルの外積
		float fRate = fCrossPosMove / fCrossLineMove;									// 交点の割合

		if (fRate < 0.0f || fRate > 1.0f)
		{ // 交点が壁の範囲外
			continue;
		}

		D3DXVECTOR3 posIntersect = posStart + (vecLine * fRate);		// 交点
		D3DXVECTOR3 vecLostMove = vecMove - (posIntersect - *pPosOld);	// 壁の先に進むはずだった分の移動

		/*** 移動量を壁の法線で反射させる (反射ベクトル = 移動 + 法線 × 2 × 内積) ***/
		float fDot = (-vecMove.x * vecNor.x) + (-vecMove.z * vecNor.z);		// 逆向きの移動と法線の内積
		D3DXVECTOR3 vecMoveRef = vecMove + (vecNor * fDot) * 2.0f;			// 反射後の移動量

		/*** 交点から、壁の先に進むはずだった分を逆向きに戻した位置へ ***/
		pPos->x = posIntersect.x - vecLostMove.x;
		pPos->z = posIntersect.z - vecLostMove.z;

		pMove->x = vecMoveRef.x;
		pMove->z = vecMoveRef.z;
	}
}

//==================================================================================
// --- 壁を真上から見た線分 (左端 → 右端) ---
//==================================================================================
static void GetWallLine(const Wall *pWall, D3DXVECTOR3 *pPosStart, D3DXVECTOR3 *pVecLine)
{
	float fHalfWidth = pWall->fWidth * 0.5f;		// 横幅の半分

	D3DXVECTOR3 posStart = D3DXVECTOR3(pWall->pos.x - (fHalfWidth * cosf(pWall->rot.y)),
		pWall->pos.y,
		pWall->pos.z + (fHalfWidth * sinf(pWall->rot.y)));		// 左端

	D3DXVECTOR3 posEnd = D3DXVECTOR3(pWall->pos.x + (fHalfWidth * cosf(pWall->rot.y)),
		pWall->pos.y,
		pWall->pos.z - (fHalfWidth * sinf(pWall->rot.y)));		// 右端

	*pPosStart = posStart;
	*pVecLine = posEnd - posStart;
}

//==================================================================================
// --- 位置が壁の高さの範囲内か ---
//==================================================================================
static bool IsInWallHeight(const Wall *pWall, const D3DXVECTOR3 *pPos)
{
	return (pPos->y >= pWall->pos.y && pPos->y <= pWall->pos.y + pWall->fHeight);
}

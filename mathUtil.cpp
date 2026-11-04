//==================================================================================
//
// 計算関連cppファイル [mathUtil.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "mathUtil.h"
#include "debugproc.h"
#include "effect.h"

//**********************************************************************************
// *** レンダリング状態情報構造体 ***
//**********************************************************************************
typedef struct
{
	D3DRENDERSTATETYPE type;		// レンダリング種類
	DWORD dwValue;					// 値
} RenderStateInfo;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static RenderStateInfo g_aRStateInfo[300] = {};		// 情報

//==================================================================================
// --- 対象の位置の範囲確認処理 ---
//==================================================================================
bool CollisionBox(RECT rect, D3DXVECTOR3 pos)
{
	/*** 対象の位置が指定された四角形の範囲内か確認 ***/
	if (pos.x >= rect.left
		&& pos.x <= rect.right
		&& pos.y >= rect.top
		&& pos.y <= rect.bottom)
	{
		return true;
	}

	return false;
}

//==================================================================================
// --- 対象の位置の範囲確認処理(XZ平面) ---
//==================================================================================
bool CollisionBoxZ(D3DXVECTOR4 rect, D3DXVECTOR3 pos)
{
	/*** 対象の位置が指定された四角形の範囲内か確認 ***/
	if (pos.x >= rect.x
		&& pos.x <= rect.y
		&& pos.z >= rect.z
		&& pos.z <= rect.w)
	{
		return true;
	}

	return false;
}

//==================================================================================
// --- 列挙型の範囲確認処理 ---
//==================================================================================
HRESULT CheckIndex(int nIndexMax, int nIndex, int nIndexMin)
{
	if (nIndex < nIndexMin || nIndex >= nIndexMax)
	{
		return E_FAIL;
	}

	return S_OK;
}

//==================================================================================
// --- 四点指定処理 ---
//==================================================================================
POINT_RECT SetPointRect(D3DXVECTOR3 p1, D3DXVECTOR3 p2, D3DXVECTOR3 p3, D3DXVECTOR3 p4)
{
	POINT_RECT pointrect = {};

	pointrect.point1 = p1;
	pointrect.point2 = p2;
	pointrect.point3 = p3;
	pointrect.point4 = p4;

	return pointrect;
}

//==================================================================================
// --- RECTからPointRectへの変換処理 ---
//==================================================================================
POINT_RECT SetRectToPointRect(RECT rect)
{
	POINT_RECT prRect = {};

	prRect.point1 = D3DXVECTOR3((float)rect.left, (float)rect.top, 0.0f);
	prRect.point2 = D3DXVECTOR3((float)rect.right, (float)rect.top, 0.0f);
	prRect.point3 = D3DXVECTOR3((float)rect.left, (float)rect.bottom, 0.0f);
	prRect.point4 = D3DXVECTOR3((float)rect.right, (float)rect.bottom, 0.0f);

	return prRect;
}

//==================================================================================
// --- 対象から対象へホーミングする処理 ---
//==================================================================================
void HomingPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3 *posMover, float fSpeed)
{
	float fAngle = 0.0f;

	fAngle = atan2f(posTarget.x - posMover->x, posTarget.z - posMover->z);

	posMover->x += sinf(fAngle) * fSpeed;
	posMover->z += cosf(fAngle) * fSpeed;
}

//==================================================================================
// --- 対象から対象への角度を求める処理 ---
//==================================================================================
float GetPosToPos(D3DXVECTOR3 posTarget, D3DXVECTOR3 posMover)
{
	float fAngle = 0.0f;

	fAngle = atan2f(posTarget.x - posMover.x, posTarget.z - posMover.z);

	/** 角度の修正! **/
	if (fAngle < -D3DX_PI)
	{
		fAngle += D3DX_PI * 2.0f;
	}
	else if (fAngle >= D3DX_PI)
	{
		fAngle -= D3DX_PI * 2.0f;
	}

	return fAngle;
}

//==================================================================================
// --- 角度を -π～π (-π は含まない) に収める処理 ---
// ※ 1周分(2π)だけずらすので、2周以上ずれた角度は収まりきらない
//==================================================================================
float RepairRot(float fRot)
{
	if (fRot > D3DX_PI)
	{
		fRot -= D3DX_PI * 2.0f;
	}
	else if (fRot <= -D3DX_PI)
	{
		fRot += D3DX_PI * 2.0f;
	}

	return fRot;
}

//==================================================================================
// --- 角度を -π～π に収める処理 (ポインタ版) ---
//==================================================================================
void RepairRot(float *pOut, const float *pIn)
{
	*pOut = RepairRot(*pIn);
}

//==================================================================================
// --- 角度(XYZ)を -π～π に収める処理 (ポインタ版) ---
//==================================================================================
void RepairRot(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pIn)
{
	D3DXVECTOR3 rotOut = D3DXVECTOR3(RepairRot(pIn->x), RepairRot(pIn->y), RepairRot(pIn->z));		// 修正後の角度

	*pOut = rotOut;
}

//==================================================================================
// --- 角度を反転(+π)する処理 ---
//==================================================================================
float InverseRot(float fRot)
{
	return RepairRot(fRot + D3DX_PI);
}

//==================================================================================
// --- 角度(XYZ)を反転する処理 ※ 未実装 (常に0を返す) ---
//==================================================================================
D3DXVECTOR3 InverseRot(D3DXVECTOR3 rot)
{
	return VECNULL;
}

//==================================================================================
// --- 角度(XYZ)を -π～π に収めた値を返す処理 ---
//==================================================================================
D3DXVECTOR3 RepairedRot(const D3DXVECTOR3 rotIn)
{
	D3DXVECTOR3 rotOut;		// 修正後の角度

	RepairRot(&rotOut, &rotIn);

	return rotOut;
}

//==================================================================================
// --- 位置と位置の間の座標を求める処理 ---
//==================================================================================
D3DXVECTOR3 GetPosBetweenPos(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2)
{
	D3DXVECTOR3 posResult = D3DXVECTOR3_NULL;

	/*** X座標を比較し大きい方から差分を求める ***/
	if (pos1.x >= pos2.x)
	{
		posResult.x = (pos1.x + pos2.x) * 0.5f;
	}
	else
	{
		posResult.x = (pos2.x + pos1.x) * 0.5f;
	}

	/*** Y座標を比較し大きい方から差分を求める ***/
	if (pos1.y >= pos2.y)
	{
		posResult.y = (pos1.y + pos2.y) * 0.5f;
	}
	else
	{
		posResult.y = (pos2.y + pos1.y) * 0.5f;
	}

	/*** Z座標を比較し大きい方から差分を求める ***/
	if (pos1.z >= pos2.z)
	{
		posResult.z = (pos1.z + pos2.z) * 0.5f;
	}
	else
	{
		posResult.z = (pos2.z + pos1.z) * 0.5f;
	}

	/*** 結果 ***/
	return posResult;
}

//==================================================================================
// --- ランダムな色を求める処理 ---
//==================================================================================
D3DXCOLOR GetRandomColor(bool bUseAlphaRand)
{
	D3DXCOLOR col;

	col.r = (float)((rand() % 100) * 0.01f);
	col.g = (float)((rand() % 100) * 0.01f);
	col.b = (float)((rand() % 100) * 0.01f);

	if (bUseAlphaRand)
	{
		col.a = (float)((rand() % 100) * 0.01f);
	}
	else
	{
		col.a = 1.0f;
	}

	return col;
}

//==================================================================================
// --- 2点間の距離を求める処理 ---
//==================================================================================
float GetPTPLength(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2)
{
	float fLength = 0.0f;

	if (pos1.x >= pos2.x
		&& pos1.y >= pos2.y)
	{
		fLength = sqrtf(powf((pos1.x - pos2.x), 2) + powf((pos1.y - pos2.y), 2));
	}
	else if (pos1.x >= pos2.x
		&& pos1.y <= pos2.y)
	{
		fLength = sqrtf(powf((pos1.x - pos2.x), 2) + powf((pos2.y - pos1.y), 2));
	}
	else if (pos1.x <= pos2.x
		&& pos1.y >= pos2.y)
	{
		fLength = sqrtf(powf((pos2.x - pos1.x), 2) + powf((pos1.y - pos2.y), 2));
	}
	else
	{
		fLength = sqrtf(powf((pos2.x - pos1.x), 2) + powf((pos2.y - pos1.y), 2));
	}

	return fLength;
}

//==================================================================================
// --- ポリゴンを回転させる処理 ---
//==================================================================================
void RollPolygon(VERTEX_2D* pVtx, D3DXVECTOR3 pos, float fWidth, float fHeight, float fRot, int nSpeed)
{
	float fLength = sqrtf(powf(fWidth, 2.0f) + powf(fHeight, 2.0f));
	float fAngle = atan2f(fWidth, fHeight);

	/*** 頂点バッファのポインタがNULLの場合、処理を行わずにスキップ ***/
	if (pVtx == NULL)
	{
		return;
	}

	pVtx[0].pos.x = pos.x + sinf(D3DX_PI + fAngle + fRot) * fLength;
	pVtx[0].pos.y = pos.y + cosf(D3DX_PI + fAngle + fRot) * fLength;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = pos.x + sinf(D3DX_PI - fAngle + fRot) * fLength;
	pVtx[1].pos.y = pos.y + cosf(D3DX_PI - fAngle + fRot) * fLength;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = pos.x + sinf(fRot - fAngle) * fLength;
	pVtx[2].pos.y = pos.y + cosf(fRot - fAngle) * fLength;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = pos.x + sinf(fRot + fAngle) * fLength;
	pVtx[3].pos.y = pos.y + cosf(fRot + fAngle) * fLength;
	pVtx[3].pos.z = 0.0f;
}

//==================================================================================
// --- ポリゴンを画面全体に描画する処理 ---
//==================================================================================
void SetFullScreenPolygon(VERTEX_2D* pVtx)
{
	/*** 頂点バッファのポインタがNULLの場合、処理を行わずにスキップ ***/
	if (pVtx == NULL)
	{
		return;
	}

	pVtx[0].pos.x = 0.0f;
	pVtx[0].pos.y = 0.0f;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = SCREEN_WIDTH;
	pVtx[1].pos.y = 0.0f;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = 0.0f;
	pVtx[2].pos.y = SCREEN_HEIGHT;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = SCREEN_WIDTH;
	pVtx[3].pos.y = SCREEN_HEIGHT;
	pVtx[3].pos.z = 0.0f;
}

//==================================================================================
// --- 中心座標とサイズから2Dポリゴン(4頂点)の頂点座標を設定する処理 ---
//==================================================================================
void SetCenterPolygon2D(VERTEX_2D *pVtx, D3DXVECTOR3 pos, float fWidth, float fHeight)
{
	if (pVtx == NULL)
	{ // 頂点バッファのポインタがNULLの場合、処理を行わずにスキップ
		return;
	}

	float fHalfWidth = fWidth * 0.5f;		// 横幅の半分
	float fHalfHeight = fHeight * 0.5f;		// 高さの半分

	pVtx[0].pos = D3DXVECTOR3(pos.x - fHalfWidth, pos.y - fHalfHeight, 0.0f);	// 左上
	pVtx[1].pos = D3DXVECTOR3(pos.x + fHalfWidth, pos.y - fHalfHeight, 0.0f);	// 右上
	pVtx[2].pos = D3DXVECTOR3(pos.x - fHalfWidth, pos.y + fHalfHeight, 0.0f);	// 左下
	pVtx[3].pos = D3DXVECTOR3(pos.x + fHalfWidth, pos.y + fHalfHeight, 0.0f);	// 右下
}

//==================================================================================
// --- 2Dポリゴン(4頂点)の rhw・頂点カラー・テクスチャ座標を標準値で設定する処理 ---
//==================================================================================
void SetDefaultPolygon2D(VERTEX_2D *pVtx, D3DXCOLOR col)
{
	if (pVtx == NULL)
	{ // 頂点バッファのポインタがNULLの場合、処理を行わずにスキップ
		return;
	}

	for (int nCntVtx = 0; nCntVtx < 4; nCntVtx++)
	{
		pVtx[nCntVtx].rhw = 1.0f;		// 座標変換用係数
		pVtx[nCntVtx].col = col;		// 頂点カラー
	}

	/*** テクスチャ全体を貼る ***/
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
}

//==================================================================================
// --- 360°をD3DXVECTOR3のRadianに変換する処理 ---
//==================================================================================
D3DXVECTOR3 DegreeToRadian(D3DXVECTOR3 degree)
{
	D3DXVECTOR3 radian;

	radian.x = D3DXToRadian(degree.x);
	/** 角度の修正! **/
	if (radian.x < -D3DX_PI)
	{
		radian.x += D3DX_PI * 2.0f;
	}
	else if (radian.x >= D3DX_PI)
	{
		radian.x -= D3DX_PI * 2.0f;
	}

	radian.y = D3DXToRadian(degree.y);
	/** 角度の修正! **/
	if (radian.y < -D3DX_PI)
	{
		radian.y += D3DX_PI * 2.0f;
	}
	else if (radian.y >= D3DX_PI)
	{
		radian.y -= D3DX_PI * 2.0f;
	}

	radian.z = D3DXToRadian(degree.z);
	/** 角度の修正! **/
	if (radian.z < -D3DX_PI)
	{
		radian.z += D3DX_PI * 2.0f;
	}
	else if (radian.z >= D3DX_PI)
	{
		radian.z -= D3DX_PI * 2.0f;
	}

	return radian;
}

//==================================================================================
// --- RadianをD3DXVECTOR3のDegreeに変換する処理 ---
//==================================================================================
D3DXVECTOR3 RadianToDegree(D3DXVECTOR3 radian)
{
	D3DXVECTOR3 degree;

	degree.x = D3DXToDegree(radian.x);
	degree.y = D3DXToDegree(radian.y);
	degree.z = D3DXToDegree(radian.z);

	return degree;
}

//==================================================================================
// --- INT_VECTOR3をD3DXVECTOR3に変換する処理 ---
//==================================================================================
D3DXVECTOR3 INTToFloat(INT_VECTOR3 nVector3)
{
	D3DXVECTOR3 rot = D3DXVECTOR3_NULL;

	rot.x = (float)nVector3.x;
	rot.y = (float)nVector3.y;
	rot.z = (float)nVector3.z;

	return rot;
}

//==================================================================================
// --- INT_VECTOR2をD3DXVECTOR2に変換する処理 ---
//==================================================================================
D3DXVECTOR2 INTToFloat(INT_VECTOR2 nVector2)
{
	D3DXVECTOR2 tex = D3DXVECTOR2(0.0f, 0.0f);

	tex.x = (float)nVector2.x;
	tex.y = (float)nVector2.y;

	return tex;
}

//==================================================================================
// --- テクスチャ設定処理 ---
//==================================================================================
void SetVtxTexture(VERTEX_2D* pVtx, float x, float y)
{
	if (pVtx == NULL)
	{
		return;
	}

	pVtx[0].tex.x = 0.0f;
	pVtx[0].tex.y = 0.0f;

	pVtx[1].tex.x = 1.0f * x;
	pVtx[1].tex.y = 0.0f;

	pVtx[2].tex.x = 0.0f;
	pVtx[2].tex.y = 1.0f * y;

	pVtx[3].tex.x = 1.0f * x;
	pVtx[3].tex.y = 1.0f * y;
}

//==================================================================================
// --- テクスチャ設定処理 ---
//==================================================================================
void SetVtxTexture(VERTEX_3D* pVtx, float x, float y)
{
	if (pVtx == NULL)
	{
		return;
	}

	pVtx[0].tex.x = 0.0f;
	pVtx[0].tex.y = 0.0f;

	pVtx[1].tex.x = 1.0f * x;
	pVtx[1].tex.y = 0.0f;

	pVtx[2].tex.x = 0.0f;
	pVtx[2].tex.y = 1.0f * y;

	pVtx[3].tex.x = 1.0f * x;
	pVtx[3].tex.y = 1.0f * y;
}

//==================================================================================
// --- 位置ランダム設定処理 ---
//==================================================================================
D3DXVECTOR3 GetRandomVector3(int nRangeX, int nRangeY, int nRangeZ)
{
	int x, y, z = {};
	D3DXVECTOR3 vecRandom = {};

	x = rand() % nRangeX - (nRangeX / 2);
	y = rand() % nRangeY - (nRangeY / 2);
	z = rand() % nRangeZ - (nRangeZ / 2);

	vecRandom.x = (FLOAT)x;
	vecRandom.y = (FLOAT)y;
	vecRandom.z = (FLOAT)z;

	return vecRandom;
}

//==================================================================================
// --- インデックスを利用したメッシュ頂点座標設定処理 ---
//==================================================================================
void SetIndexedVertex(VERTEX_3D* pVtx, D3DXVECTOR3 pos, int nXBlock, int nZBlock, float fWidth, float fDepth)
{
	float fOnceWidth = fWidth / (nXBlock + 1);
	float fOnceDepth = fDepth / (nZBlock + 1);
	float fX = ((nXBlock - 1) / 2);

	/*** NULLチェック ***/
	if (pVtx == NULL)
	{
		return;
	}

	for (int nCntZBlock = 0; nCntZBlock < (nZBlock + 1); nCntZBlock++)
	{
		for (int nCntXBlock = 0; nCntXBlock < (nXBlock + 1); nCntXBlock++)
		{
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].pos.x = pos.x - (fWidth * 0.5f) + (fOnceWidth * nCntXBlock);
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].pos.y = pos.y;
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].pos.z = pos.z + (fDepth * 0.5f) - (fOnceDepth * nCntZBlock);
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].tex.x = (nCntXBlock * 1.0f);
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].tex.y = (nCntZBlock * 1.0f);
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			pVtx[nCntXBlock + (nCntZBlock * (nXBlock + 1))].col = D3DXCOLOR_NULL;
		}
	}
}

//==================================================================================
// --- インデックスの頂点番号設定処理 ---
//==================================================================================
int SetIndex(_Inout_ WORD *pIdx, _In_ int nXBlock, _In_ int nZBlock, _In_ int nOffSet, _Inout_opt_ WORD *pIdxSub)
{
	int nMaxIndex = (((nXBlock + 1) * 2) * nZBlock) + (2 * (nZBlock - 1));
	int nCount = 0;
	int nCounts = 0;
	int nLine = 0;
	int nCntIndex;
	if (pIdx == NULL)
	{
		return -1;
	}

	for (nCntIndex = 0; nCntIndex < nMaxIndex; nCntIndex++)
	{
		if (nCntIndex != 0 && nCounts == (nXBlock + 1))
		{ // 0ではなく、且つ上辺の点が右端に着いたとき
			nCounts--;
			pIdx[nCntIndex] = nCounts + ((nXBlock + 1) * nLine);

			if (pIdxSub != NULL)
			{
				pIdxSub[nCntIndex] = pIdx[nCntIndex];
			}

			pIdx[nCntIndex] += nOffSet;

			nCounts = 0;
		}
		else if (nCntIndex != 0 && nCount == (nXBlock + 1) && nCounts == 0)
		{ // 0ではなく、且つ右端の多重設定が済んでいる且つ、下辺の点が右端に着いたとき
			nCount = 0;
			nLine++;
			pIdx[nCntIndex] = nCount + ((nXBlock + 1) * (nLine + 1));

			if (pIdxSub != NULL)
			{
				pIdxSub[nCntIndex] = pIdx[nCntIndex];
			}

			pIdx[nCntIndex] += nOffSet;
		}
		else
		{
			if (nCntIndex == 0)
			{ // 初回限定0の場合
				pIdx[nCntIndex] = nCount + ((nXBlock + 1) * (nLine + 1));

				if (pIdxSub != NULL)
				{
					pIdxSub[nCntIndex] = pIdx[nCntIndex];
				}

				pIdx[nCntIndex] += nOffSet;

				nCount++;
			}
			else if (nCntIndex % 2 == 0)
			{ // 偶数の場合
				pIdx[nCntIndex] = nCount + ((nXBlock + 1) * (nLine + 1));

				if (pIdxSub != NULL)
				{
					pIdxSub[nCntIndex] = pIdx[nCntIndex];
				}

				pIdx[nCntIndex] += nOffSet;

				nCount++;
			}
			else
			{ // 奇数の場合
				pIdx[nCntIndex] = nCounts + ((nXBlock + 1) * nLine);

				if (pIdxSub != NULL)
				{
					pIdxSub[nCntIndex] = pIdx[nCntIndex];
				}

				pIdx[nCntIndex] += nOffSet;

				nCounts++;
			}
		}
	}

	return nCntIndex;
}

//==================================================================================
// --- float型の値の正規処理 ---
//==================================================================================
void RepairFloat(float *pTarget, int nCnt)
{
	int nTruncated;
	nTruncated = *pTarget * 10 * nCnt;
	nTruncated /= 10 * nCnt;
	*pTarget = nTruncated;
}

//==================================================================================
// --- Zテスト無効化設定処理 ---
//==================================================================================
void UnableZFunction(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** Zテストを無効にする ***/
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	g_aRStateInfo[D3DRS_ZWRITEENABLE].type = D3DRS_ZWRITEENABLE;
	g_aRStateInfo[D3DRS_ZWRITEENABLE].dwValue = FALSE;

	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

	g_aRStateInfo[D3DRS_ZFUNC].type = D3DRS_ZFUNC;
	g_aRStateInfo[D3DRS_ZFUNC].dwValue = D3DCMP_ALWAYS;
}

//==================================================================================
// --- Alphaテスト有効化設定処理 ---
//==================================================================================
void AbleAlphaFunction(D3DCMPFUNC alphaFunc, DWORD dwAlphaRef)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** アルファテストを有効にする ***/
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);		// アルファテストを有効

	g_aRStateInfo[D3DRS_ALPHATESTENABLE].type = D3DRS_ALPHATESTENABLE;
	g_aRStateInfo[D3DRS_ALPHATESTENABLE].dwValue = TRUE;

	pDevice->SetRenderState(D3DRS_ALPHAFUNC, alphaFunc);			// 基準値よりも大きい場合にZバッファに書き込み

	g_aRStateInfo[D3DRS_ALPHAFUNC].type = D3DRS_ALPHAFUNC;
	g_aRStateInfo[D3DRS_ALPHAFUNC].dwValue = alphaFunc;

	pDevice->SetRenderState(D3DRS_ALPHAREF, dwAlphaRef);				// 基準値

	g_aRStateInfo[D3DRS_ALPHAREF].type = D3DRS_ALPHAREF;
	g_aRStateInfo[D3DRS_ALPHAREF].dwValue = dwAlphaRef;
}

//==================================================================================
// --- レンダーステート設定終了処理 ---
//==================================================================================
void EndFunction(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	if (g_aRStateInfo[D3DRS_ALPHATESTENABLE].dwValue == TRUE)
	{
		/*** アルファテストを無効にする ***/
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);		// アルファテストを無効化

		g_aRStateInfo[D3DRS_ALPHATESTENABLE].dwValue = FALSE;

		pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);	// 無条件にZバッファに書き込み

		g_aRStateInfo[D3DRS_ALPHAFUNC].dwValue = D3DCMP_ALWAYS;

		pDevice->SetRenderState(D3DRS_ALPHAREF, 0);					// 基準値

		g_aRStateInfo[D3DRS_ALPHAREF].dwValue = 0;
	}

	if (g_aRStateInfo[D3DRS_ZWRITEENABLE].dwValue == FALSE)
	{
		/*** Zテストを有効にする ***/
		pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		g_aRStateInfo[D3DRS_ZFUNC].dwValue = D3DCMP_LESSEQUAL;

		pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

		g_aRStateInfo[D3DRS_ZWRITEENABLE].dwValue = TRUE;
	}
}

//==================================================================================
// --- パス指定の絶対アドレス確認処理 ---
//==================================================================================
bool CheckPath(_In_ const char *pFileName)
{
#ifndef _PATHCHECK_NO_WARNINGS
	if (strstr(pFileName, "C:") != NULL)
	{
		GenerateMessageBox(MB_ICONWARNING,
			"Warnings!",
			"アドレスが絶対パスの可能性があります。\n相対パスに切り替えてください。\n\n対象パス : %s",
			pFileName);

		return true;
	}
#endif

	return false;
}

//==================================================================================
// --- 書式付きメッセージボックス表示処理 (Debugビルドのみ表示) ---
// ※ 書式は printf と同じ (%d / %s など)
//==================================================================================
void GenerateMessageBox(_In_ UINT nType, _In_ const char *pCaption, _In_ const char *fmt, ...)
{
#ifdef _DEBUG
	HWND hWnd = GetHandleWindow();		// 親ウィンドウ
	if (hWnd == NULL)
	{
		return;
	}

	char aMessage[STRING_MAX] = {};		// 表示する文字列
	va_list args;						// 可変長引数

	va_start(args, fmt);
	_vsnprintf(aMessage, sizeof aMessage - 1, fmt, args);
	va_end(args);

	MessageBox(hWnd, aMessage, pCaption, nType);
#endif
}

//==================================================================================
// --- カリングタイプ設定処理 ---
//==================================================================================
void SetCullingType(D3DCULL type)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	pDevice->SetRenderState(D3DRS_CULLMODE, type);
}
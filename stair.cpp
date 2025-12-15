//================================================================================================================
//
// DirectXの階段設置ヘッダファイル [stair.h]
// Author : TENMA
//
//================================================================================================================

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "stair.h"
#include "wall.h"
#include "meshfield.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************

//**********************************************************************************
//*** グローバル変数 ***
//**********************************************************************************

//**********************************************************************************
//*** 階段設置処理 ***
//**********************************************************************************
void SetStair(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fDepth, int nIdxTexture)
{
	float fHeight;			// 階段のY座標
	InitedVec3(posMesh);	// メッシュフィールドの位置
	InitedVec3(posWall[3]);	// 階段を囲う壁の位置
	InitedVec3(rotWall[3]);	// 階段を囲う壁の角度

	posMesh = pos;
	posWall[0] = pos;
	posWall[1] = pos;
	posWall[2] = pos;

	rotWall[0] = rot;
	rotWall[1] = rot;
	rotWall[2] = rot;

	fHeight = sinf(RepairRot(rot.z)) * (fDepth * 0.5f);

	posMesh.y += fHeight;

	posWall[0].y += fHeight;
	posWall[0].x -= fWidth * 0.5f;
	rotWall[0].y -= D3DX_HALFPI;

	posWall[1].y += fHeight;
	posWall[1].z -= fDepth * 0.5f;

	posWall[2].y += fHeight;
	posWall[2].x += fWidth * 0.5f;
	rotWall[2].y += D3DX_HALFPI;

	SetMeshField(posMesh, RepairedRot(rot), fWidth, fDepth, 10, 10, nIdxTexture);
	
	SetWall(posWall[0], rotWall[0], fWidth, fHeight, nIdxTexture, 1, 1, D3DXCOLOR_NULL, D3DCULL_CW);
	SetWall(posWall[1], rotWall[1], fWidth, fHeight, nIdxTexture, 1, 1, D3DXCOLOR_NULL, D3DCULL_CW);
	SetWall(posWall[2], rotWall[2], fWidth, fHeight, nIdxTexture, 1, 1, D3DXCOLOR_NULL, D3DCULL_CW);
}
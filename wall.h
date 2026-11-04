//==================================================================================
//
// 壁表示用ヘッダファイル [wall.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _WALL_H_
#define _WALL_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitWall(void);
void UninitWall(void);
void UpdateWall(void);
void DrawWall(void);

void SetWall(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fWidth, float fHeight, int nIndexTexture, int nXblock, int nZblock, D3DXCOLOR col = D3DXCOLOR_NULL, D3DCULL type = D3DCULL_CCW);
void CollisionWall(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove);
void ReflectWall(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove);
#endif
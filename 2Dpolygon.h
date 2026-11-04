//==================================================================================
//
// ポリゴン表示用ヘッダファイル [2Dpolygon.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _2DPOLYGON_H_
#define _2DPOLYGON_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "tutorial.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void Init2DPolygon(void);
void Uninit2DPolygon(void);
void Update2DPolygon(void);
void Draw2DPolygon(void);

int Set2DPolygon(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR2 size, int nIdxTexture = -1);
void SetPosition2DPolygon(int nIdx2DPolygon, D3DXVECTOR3 pos);
void Destroy2DPolygon(int nIdx2DPolygon);
#endif
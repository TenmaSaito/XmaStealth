//==================================================================================
//
// ビルボード表示用ヘッダファイル [billboard.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _BILLBOARD_H_
#define _BILLBOARD_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitBillBoard(void);
void UninitBillBoard(void);
void UpdateBillBoard(void);
void DrawBillBoard(void);

int SetBillBoard(D3DXVECTOR3 pos, float fWidth, float fHeight, int nIdxTexture, D3DXCOLOR col = DEF_COL);
void KillBillboard(int nIdxBill);
#endif
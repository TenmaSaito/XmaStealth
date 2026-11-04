//==================================================================================
//
// PressEnter用ヘッダファイル [pressEnter.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _PRESSENTER_H_
#define _PRESSENTER_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "title.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitPressEnter(void);
void UninitPressEnter(void);
void UpdatePressEnter(void);
void DrawPressEnter(void);

void SetPressEnterEffect(D3DXVECTOR3 pos);
bool GetPressEnterEffect(void);
bool GetEndPressEnterEffect(void);

#endif
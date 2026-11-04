//==================================================================================
//
// エフェクト表示用ヘッダファイル [effect.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _EFFECT_H_
#define _EFFECT_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitEffect(void);
void UninitEffect(void);
void UpdateEffect(void);
void DrawEffect(void);

void SetEffect(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fSpd, float fWidth, float fHeight, int nLife = -1);
void SetEffectEx(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, float fSpd, float fWidth, float fHeight, int nLife);
void SetIndexTextureEffect(int nIndexTexture);
#endif
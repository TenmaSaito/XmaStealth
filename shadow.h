//================================================================================================================
//
// DirectXの影表示用ヘッダファイル [shadow.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _SHADOW_H_
#define _SHADOW_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

//**********************************************************************************
//*** サイズ変更の種類 ***
//**********************************************************************************
typedef enum
{
	SHADOW_SIZE_WIDTH = 0,			// 幅の増減
	SHADOW_SIZE_DEPTH,				// 奥行の増減
	SHADOW_SIZE_MAX
}SHADOW_SIZE;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitShadow(void);
void UninitShadow(void);
void UpdateShadow(void);
void DrawShadow(void);

int SetShadow(float fWidth, float fDepth);
void SetIndexTextureShadow(int nIndexTexture);
void SetPositionShadow(int nIdShadow, D3DXVECTOR3 pos, D3DXVECTOR3 rot);
void DestroyShadow(int nIdShadow);
float AddShadowSize(int nIdShadow, SHADOW_SIZE type, float fValue);
void InvisibleShadow(int nIdxShadow, float fAlpha);
#endif
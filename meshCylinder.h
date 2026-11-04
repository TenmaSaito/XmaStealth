//==================================================================================
//
// メッシュシリンダー用ヘッダファイル [meshCylinder.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _MESHCYLINDER_H_
#define _MESHCYLINDER_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitMeshCylinder(void);
void UninitMeshCylinder(void);
void UpdateMeshCylinder(void);
void DrawMeshCylinder(void);

int SetMeshCylinder(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fHeight, float fRadius, int nXBlock, int nYBlock, CULLINGTYPE type, int nIdxTexture = -1, bool bUseLight = true);
#endif
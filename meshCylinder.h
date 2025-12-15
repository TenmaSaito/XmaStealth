//================================================================================================================
//
// DirectXのメッシュシリンダー用ヘッダファイル [meshCylinder.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _MESHCYLINDER_H_
#define _MESHCYLINDER_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitMeshCylinder(void);
void UninitMeshCylinder(void);
void UpdateMeshCylinder(void);
void DrawMeshCylinder(void);

int SetMeshCylinder(D3DXVECTOR3 pos, D3DXVECTOR3 rot, float fHeight, float fRadius, int nXBlock, int nYBlock, CULLINGTYPE type, int nIdxTexture = -1, bool bUseLight = true);
//void CollisionMeshCylinder(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove);
#endif
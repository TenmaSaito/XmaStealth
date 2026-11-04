//==================================================================================
//
// タイトルカメラ用ヘッダファイル [cameraTitle.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _CAMERATITLE_H_
#define _CAMERATITLE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "title.h"
#include "camera.h"

//**********************************************************************************
// *** カメラ情報構造体 ***
//**********************************************************************************
typedef Camera CameraTitle;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitCameraTitle(void);
void UninitCameraTitle(void);
void UpdateCameraTitle(void);
void SetCameraTitle(void);

void SetCameraTitle(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd);
CameraTitle *GetCameraTitle(void);
#endif

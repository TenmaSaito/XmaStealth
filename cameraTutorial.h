//==================================================================================
//
// チュートリアルカメラ用ヘッダファイル [cameraTutorial.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _CAMERATUTORIAL_H_
#define _CAMERATUTORIAL_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "tutorial.h"
#include "camera.h"

//**********************************************************************************
// *** カメラ情報構造体 ***
//**********************************************************************************
typedef Camera CameraTutorial;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitCameraTutorial(void);
void UninitCameraTutorial(void);
void UpdateCameraTutorial(void);
void SetCameraTutorial(void);

void SetCameraTutorial(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd);
CameraTutorial *GetCameraTutorial(void);
void ResetCameraTutorial(void);
#endif

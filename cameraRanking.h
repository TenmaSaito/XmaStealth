//==================================================================================
//
// ランキングカメラ用ヘッダファイル [cameraRanking.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _CAMERARANKING_H_
#define _CAMERARANKING_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "ranking.h"
#include "camera.h"

//**********************************************************************************
// *** カメラ情報構造体 ***
//**********************************************************************************
typedef Camera CameraRanking;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitCameraRanking(void);
void UninitCameraRanking(void);
void UpdateCameraRanking(void);
void SetCameraRanking(void);

void SetCameraRanking(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd);
CameraRanking *GetCameraRanking(void);
#endif

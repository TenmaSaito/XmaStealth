//==================================================================================
//
// カメラ用ヘッダファイル [camera.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _CAMERA_H_
#define _CAMERA_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
// *** カメラ情報構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 posOffSet;			// 回転の中心(スクリプトで設定した注視点)
	D3DXVECTOR3 posV;				// 視点
	D3DXVECTOR3 posVOld;			// 前フレームの視点
	D3DXVECTOR3 posR;				// 注視点
	D3DXVECTOR3 posVDest;			// 目的の視点
	D3DXVECTOR3 posRDest;			// 目的の注視点
	D3DXVECTOR3 vecU;				// 上方向ベクトル
	D3DXVECTOR3 rot;				// 向き
	D3DXVECTOR3 move;				// 向きの移動量(スティック回転の慣性)
	D3DXVECTOR3 rotDest;			// 目的の向き
	D3DXMATRIX mtxProjection;		// プロジェクションマトリックス
	D3DXMATRIX mtxView;				// ビューマトリックス
	float fAngle;					// 視野角(度)
	float fZNear;					// 描画開始距離
	float fZFar;					// 描画限界距離
	float fZLength;					// 注視点との距離
	float fSpd;						// 移動速度
	float fRotSpd;					// 回転速度
	bool bUse;						// 使用状況
	bool bDisp;						// 映すか
} Camera;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
// --- ゲーム画面のカメラ ---
void InitCamera(void);
void UninitCamera(void);
void UpdateCamera(void);
void SetCamera(void);

void SetCamera(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd);
Camera *GetCamera(void);
void ResetCamera(void);

// --- 各画面のカメラ共通の処理 ---
void InitCameraInfo(Camera *pCamera);
void SetCameraInfo(Camera *pCamera, D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd);
void SetCameraTransform(Camera *pCamera);
void ResetCameraInfo(Camera *pCamera);
void UpdateFollowCamera(Camera *pCamera, bool bUseGameControl);

#endif

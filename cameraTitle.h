//================================================================================================================
//
// DirectXのタイトルカメラ用ヘッダファイル [cameraTitle.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _CAMERATITLE_H_
#define _CAMERATITLE_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "title.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

//**********************************************************************************
//*** カメラ情報構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 posOffSet;	// 回転軒天
	D3DXVECTOR3 posV;		// 視点
	D3DXVECTOR3 posVOld;	// 過去の始点
	D3DXVECTOR3 posR;		// 注視点
	D3DXVECTOR3 posVDest;	// 目的の視点
	D3DXVECTOR3 posRDest;	// 目的の注視点
	D3DXVECTOR3 vecU;		// 上方向ベクトル
	D3DXVECTOR3 rot;		// 向き
	D3DXVECTOR3 move;		// 向きの移動量
	D3DXVECTOR3 rotDest;	// 目的の向き
	D3DXMATRIX mtxProjection;		// プロジェクションマトリックス
	D3DXMATRIX mtxView;				// ビューマトリックス
	float fAngle;			// 視野角
	float zn;				// 描画開始距離
	float zf;				// 描画限界距離
	float fZlength;			// 注視点との距離
	float fSpd;				// 移動速度
	float fRotSpd;			// 回転速度
	bool bUse;				// 使用状況
	bool bDisp;				// 映すか
} CameraTitle;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitCameraTitle(void);
void UninitCameraTitle(void);
void UpdateCameraTitle(void);
void SetCameraTitle(void);

void SetCameraTitle(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float zn, float zf, float fZLength, float fSpd, float fRotSpd);
CameraTitle *GetCameraTitle(void);
#endif

//================================================================================================================
//
// DirectXのサブカメラ用ヘッダファイル [subSubCamera.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _SUBCAMERA_H_
#define _SUBCAMERA_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************

//**********************************************************************************
//*** カメラ情報構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 posV;		// 視点
	D3DXVECTOR3 posR;		// 注視点
	D3DXVECTOR3 posVDest;	// 目的の視点
	D3DXVECTOR3 posRDest;	// 目的の注視点
	D3DXVECTOR3 vecU;		// 上方向ベクトル
	D3DXMATRIX mtxProjection;		// プロジェクションマトリックス
	D3DXMATRIX mtxView;				// ビューマトリックス
	float fAngle;			// 視野角
	float zn;				// 描画開始距離
	float zf;				// 描画限界距離
	float fZlength;			// 注視点との距離
	float fSpd;				// 移動速度
	float fRotSpd;			// 回転速度
	float fViewPortX;		// ビューポートのX座標
	float fViewPortY;		// ビューポートのY座標
	float fViewPortWidth;	// ビューポートの幅
	float fViewPortHeight;	// ビューポートの高さ
	bool bDisp;				// 映すか
}SubCamera;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitSubCamera(void);
void UninitSubCamera(void);
void UpdateSubCamera(void);
void SetSubCamera(void);

void SetEnableSubCamera(bool bDisp);
bool GetEnableSubCamera(void);
void SetViewPort(void);
void EndViewPort(void);
SubCamera *GetSubCamera(void);
#endif

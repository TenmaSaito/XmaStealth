//================================================================================================================
//
// DirectXのランキングカメラ処理 [cameraRanking.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "cameraRanking.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define CAMERA_ZLENGTH		(300.0f)		// カメラの注視点との距離
#define CAMERA_SPD			(2.0f)			// カメラの移動速度
#define CAMERA_ROTSPD		(0.05f)			// カメラの回転速度
#define CAMERA_ZN			(10.0f)			// カメラの描画開始距離
#define CAMERA_ZF			(1000.0f)		// カメラの描画限界距離
#define CAMERA_VANGLE		(45.0f)			// カメラの視野角
#define CAMERA_MOUSEUD		(243.0f)		// マウスでの視点移動の上下の上限
#define ROTMOVE_RESIST		(0.05f)			// 回転減速係数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
CameraRanking g_cameraRanking;		// カメラの情報

//================================================================================================================
// --- カメラの初期化 ---
//================================================================================================================
void InitCameraRanking(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	CameraRanking *pCamera = &g_cameraRanking;

	/*** 構造体の初期化 ***/
	pCamera->posV = D3DXVECTOR3(0.0f, 75.0f, -CAMERA_ZLENGTH);
	pCamera->posR = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pCamera->vecU = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pCamera->posVDest = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pCamera->posRDest = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pCamera->rot = D3DXVECTOR3_NULL;
	pCamera->rotDest = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pCamera->zn = CAMERA_ZN;
	pCamera->zf = CAMERA_ZF;
	pCamera->fZlength = CAMERA_ZLENGTH;
	pCamera->fSpd = CAMERA_SPD;
	pCamera->fRotSpd = CAMERA_ROTSPD;
	pCamera->fAngle = CAMERA_VANGLE;
}

//================================================================================================================
// --- カメラの終了 ---
//================================================================================================================
void UninitCameraRanking(void)
{

}

//================================================================================================================
// --- カメラの更新 ---
//================================================================================================================
void UpdateCameraRanking(void)
{
	CameraRanking *pCamera = &g_cameraRanking;
	pCamera->rot.y -= pCamera->fRotSpd;

	if (pCamera->rot.y < -D3DX_PI)
	{
		pCamera->rot.y += D3DX_PI * 2.0f;
	}
	else if (pCamera->rot.y >= D3DX_PI)
	{
		pCamera->rot.y -= D3DX_PI * 2.0f;
	}

	pCamera->posV.x = pCamera->posR.x + sinf(pCamera->rot.y) * pCamera->fZlength;
	pCamera->posV.z = pCamera->posR.z + cosf(pCamera->rot.y) * pCamera->fZlength;
}

//================================================================================================================
// --- カメラの設置 ---
//================================================================================================================
void SetCameraRanking(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	CameraRanking *pCamera = &g_cameraRanking;
	RECT rect = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };			// 画面サイズ
	char aStr[MAX_PATH];

	/*** プロジェクションマトリックスの初期化 ***/
	D3DXMatrixIdentity(&pCamera->mtxProjection);

	/*** プロジェクションマトリックスの作成 ***/
	D3DXMatrixPerspectiveFovLH(&pCamera->mtxProjection,
		D3DXToRadian(pCamera->fAngle),					// 視野角
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,		// モニターのアスペクト比
		pCamera->zn,					// カメラの描画最小範囲
		pCamera->zf);					// カメラの描画最大範囲

	/*** プロジェクションマトリックスの設定 ***/
	pDevice->SetTransform(D3DTS_PROJECTION, &pCamera->mtxProjection);

	/*** ビューマトリックスの初期化 ***/
	D3DXMatrixIdentity(&pCamera->mtxView);

	/*** ビューマトリックスの作成 ***/
	D3DXMatrixLookAtLH(&pCamera->mtxView,
		&pCamera->posV,		// 視点
		&pCamera->posR,		// 注視点
		&pCamera->vecU);	// 上方向ベクトル

	/*** ビューマトリックスの設定 ***/
	pDevice->SetTransform(D3DTS_VIEW, &pCamera->mtxView);
}

//================================================================================================================
// --- カメラの設置 (スクリプト) ---
//================================================================================================================
void SetCameraRanking(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float zn, float zf, float fZLength, float fSpd, float fRotSpd)
{
	CameraRanking *pCamera = &g_cameraRanking;

	/*** カメラの情報セット ***/
	pCamera->posV = posV;
	pCamera->posR = posR;
	pCamera->posOffSet = posR;
	pCamera->fAngle = fAngle;
	pCamera->zn = zn;
	pCamera->zf = zf;
	pCamera->fZlength = fZLength;
	pCamera->fSpd = fSpd;
	pCamera->fRotSpd = fRotSpd;
}

//================================================================================================================
// --- カメラの取得 ---
//================================================================================================================
CameraRanking* GetCameraRanking(void)
{
	return &g_cameraRanking;
}
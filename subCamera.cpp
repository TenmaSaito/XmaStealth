//================================================================================================================
//
// DirectXのカメラ処理 [subSubCamera.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "subCamera.h"
#include "mathUtil.h"
#include "player.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define VIEWPORT_X				(800.0f)			// ビューポートの開始位置 X
#define VIEWPORT_Y				(480.0f)			// ビューポートの開始位置 Y
#define VIEWPORT_WIDTH			(400.0f)		// ビューポートのサイズ WIDTH
#define VIEWPORT_HEIGHT			(200.0f)		// ビューポートのサイズ HEIGHT
#define SUBCAMERA_ZLENGTH		(300.0f)		// カメラの注視点との距離
#define SUBCAMERA_SPD			(2.0f)			// カメラの移動速度
#define SUBCAMERA_ROTSPD		(0.05f)			// カメラの回転速度
#define SUBCAMERA_ZN			(10.0f)			// カメラの描画開始距離
#define SUBCAMERA_ZF			(10000.0f)		// カメラの描画限界距離
#define SUBCAMERA_VANGLE		(45.0f)			// カメラの視野角
#define SUBCAMERA_MOUSEUD		(243.0f)		// マウスでの視点移動の上下の上限

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
SubCamera g_subCamera;		// カメラの情報
D3DVIEWPORT9 g_viewPort;	// 保存用ビューポート変数
bool g_bUseMap;				// マップモードにするか

//================================================================================================================
// --- カメラの初期化 ---
//================================================================================================================
void InitSubCamera(void)
{
	/*** 構造体の初期化 ***/
	g_subCamera.posV = D3DXVECTOR3(0.0f, 500.0f, -0.1);
	g_subCamera.posR = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_subCamera.vecU = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	g_subCamera.posVDest = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_subCamera.posRDest = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	g_subCamera.zn = SUBCAMERA_ZN;
	g_subCamera.zf = SUBCAMERA_ZF;
	g_subCamera.fZlength = SUBCAMERA_ZLENGTH;
	g_subCamera.fSpd = SUBCAMERA_SPD;
	g_subCamera.fRotSpd = SUBCAMERA_ROTSPD;
	g_subCamera.fAngle = SUBCAMERA_VANGLE;
	g_subCamera.fViewPortX = VIEWPORT_X;
	g_subCamera.fViewPortY = VIEWPORT_Y;
	g_subCamera.fViewPortWidth = VIEWPORT_WIDTH;
	g_subCamera.fViewPortHeight = VIEWPORT_HEIGHT;
	g_subCamera.bDisp = false;
	g_bUseMap = false;
}

//================================================================================================================
// --- カメラの終了 ---
//================================================================================================================
void UninitSubCamera(void)
{

}

//================================================================================================================
// --- カメラの更新 ---
//================================================================================================================
void UpdateSubCamera(void)
{
	Player *pPlayer = GetPlayer();

	if (g_bUseMap != true)
	{
		g_subCamera.posV.x = pPlayer->pos.x;
		g_subCamera.posV.z = pPlayer->pos.z + 0.1f;
		g_subCamera.posR.x = pPlayer->pos.x;
		g_subCamera.posR.z = pPlayer->pos.z;
	}

	if (GetKeyboardTrigger(DIK_M) == true)
	{
		g_bUseMap = g_bUseMap ^ true;
		if (g_bUseMap == true)
		{
			g_subCamera.fViewPortX = 50.0f;
			g_subCamera.fViewPortY = 50.0f;
			g_subCamera.fViewPortWidth = 1180.0f;
			g_subCamera.fViewPortHeight = 620.0f;

			g_subCamera.posV = D3DXVECTOR3(-0.1f, 2000.0f, 0.0f);
			g_subCamera.posR = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			g_subCamera.posV = D3DXVECTOR3(0.0f, 500.0f, -0.1);
			g_subCamera.posV.x = pPlayer->pos.x;
			g_subCamera.posV.z = pPlayer->pos.z + 0.1f;
			g_subCamera.posR.x = pPlayer->pos.x;
			g_subCamera.posR.z = pPlayer->pos.z;
			g_subCamera.fViewPortX = VIEWPORT_X;
			g_subCamera.fViewPortY = VIEWPORT_Y;
			g_subCamera.fViewPortWidth = VIEWPORT_WIDTH;
			g_subCamera.fViewPortHeight = VIEWPORT_HEIGHT;
		}
	}
}

//================================================================================================================
// --- カメラの設置 ---
//================================================================================================================
void SetSubCamera(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	if (g_subCamera.bDisp == true)
	{
		/*** プロジェクションマトリックスの初期化 ***/
		D3DXMatrixIdentity(&g_subCamera.mtxProjection);

		/*** プロジェクションマトリックスの作成 ***/
		D3DXMatrixPerspectiveFovLH(&g_subCamera.mtxProjection,
			D3DXToRadian(g_subCamera.fAngle),					// 視野角
			(float)g_subCamera.fViewPortWidth / (float)g_subCamera.fViewPortHeight,		// モニターのアスペクト比
			g_subCamera.zn,					// カメラの描画最小範囲
			g_subCamera.zf);					// カメラの描画最大範囲

		/*** プロジェクションマトリックスの設定 ***/
		pDevice->SetTransform(D3DTS_PROJECTION, &g_subCamera.mtxProjection);

		/*** ビューマトリックスの初期化 ***/
		D3DXMatrixIdentity(&g_subCamera.mtxView);

		/*** ビューマトリックスの作成 ***/
		D3DXMatrixLookAtLH(&g_subCamera.mtxView,
			&g_subCamera.posV,		// 視点
			&g_subCamera.posR,		// 注視点
			&g_subCamera.vecU);	// 上方向ベクトル

		/*** ビューマトリックスの設定 ***/
		pDevice->SetTransform(D3DTS_VIEW, &g_subCamera.mtxView);
	}
}

//================================================================================================================
// --- カメラの設置 (スクリプト) ---
//================================================================================================================
void SetEnableSubCamera(bool bDisp) 
{
	/*** カメラの情報セット ***/
	g_subCamera.bDisp = bDisp;
}

//================================================================================================================
// --- サブカメラの使用状態の取得 ---
//================================================================================================================
bool GetEnableSubCamera(void)
{
	return g_subCamera.bDisp;
}

//================================================================================================================
// --- サブカメラのビューポートの設定 ---
//================================================================================================================
void SetViewPort(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DVIEWPORT9 view_port;

	/*** 現在のビューポートを取得し、保存 ***/
	pDevice->GetViewport(&g_viewPort);

	/*** ビューポートの左上座標 ***/
	view_port.X = g_subCamera.fViewPortX;
	view_port.Y = g_subCamera.fViewPortY;

	/*** ビューポートのサイズ ***/
	view_port.Width = g_subCamera.fViewPortWidth;	// 幅
	view_port.Height = g_subCamera.fViewPortHeight;	// 高さ

	/*** ビューポート深度設定 ***/
	view_port.MinZ = 0.0f;
	view_port.MaxZ = 1.0f;

	/*** ビューポート設定 ***/
	pDevice->SetViewport(&view_port);
}

//================================================================================================================
// --- ビューポートの設定のリセット ---
//================================================================================================================
void EndViewPort(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** ビューポート設定 ***/
	pDevice->SetViewport(&g_viewPort);
}

//================================================================================================================
// --- カメラの取得 ---
//================================================================================================================
SubCamera* GetSubCamera(void)
{
	return &g_subCamera;
}
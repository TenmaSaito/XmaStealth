//================================================================================================================
//
// DirectXのカメラ処理 [camera.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "camera.h"
#include "mathUtil.h"
#include "player.h"
#include "spotlight.h"
#include "wall.h"

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
Camera g_camera;		// カメラの情報
LPD3DXFONT g_pFontCamera = NULL;		// フォントへのポインタ
bool g_bControl;		// 視点移動中
bool g_bThirdPerson;	// 三人称視点の有効状態
float g_fRotCamera;
float g_fYCamera;

//================================================================================================================
// --- カメラの初期化 ---
//================================================================================================================
void InitCamera(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	Camera *pCamera = &g_camera;

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
	g_bControl = false;
	g_fYCamera = 0.0f;
}

//================================================================================================================
// --- カメラの終了 ---
//================================================================================================================
void UninitCamera(void)
{

}

//================================================================================================================
// --- カメラの更新 ---
//================================================================================================================
void UpdateCamera(void)
{
	/*** プレイヤー情報の取得 ***/
	Player* pPlayer = GetPlayer();
	Camera *pCamera = &g_camera;
	float fPlayerRot = 0.0f;
	float fRot = 0.0f;
	float fRotValue;
	float fRRot = 0.075f;
	float fVRot = 0.085f;
	float fStickSpd = 0.0f;
	float fMaxStickSpd = 0.05f;
	float fZLength = 0.0f;
	float fMinZLength = 100.0f;
	float fMaxZLength = 500.0f;
	bool bControlStick = false;

	pCamera->posVOld = pCamera->posV;

	if (pCamera->rot.y < -D3DX_PI)
	{
		pCamera->rot.y += D3DX_PI * 2.0f;
	}

	if (pCamera->rot.y >= D3DX_PI)
	{
		pCamera->rot.y -= D3DX_PI * 2.0f;
	}

	/*** 注視点の設定 (プレイヤー追従) ***/
	pCamera->posRDest.x = pPlayer->pos.x + (sinf(pCamera->rot.y) * 105.0f) + sinf(pCamera->rot.y) * 5.0f;
	pCamera->posRDest.y = pPlayer->pos.y + 5.0f + g_fYCamera;
	pCamera->posRDest.z = pPlayer->pos.z + (cosf(pCamera->rot.y) * 105.0f) + cosf(pCamera->rot.y) * 5.0f;

	g_fRotCamera = pCamera->rot.y + D3DX_PI;
	if (g_fRotCamera < -D3DX_PI)
	{
		g_fRotCamera += D3DX_PI * 2.0f;
	}

	if (g_fRotCamera >= D3DX_PI)
	{
		g_fRotCamera -= D3DX_PI * 2.0f;
	}

	/*** 目的の視点を設定 (プレイヤー追従) ***/
	pCamera->posVDest.x = pPlayer->pos.x + sinf(g_fRotCamera) * pCamera->fZlength;
	pCamera->posVDest.y = pPlayer->pos.y + 50.0f;
	pCamera->posVDest.z = pPlayer->pos.z + cosf(g_fRotCamera) * pCamera->fZlength;

	/*** 注視点の補正 ***/
	pCamera->posR.x += (pCamera->posRDest.x - pCamera->posR.x) * fRRot;
	pCamera->posR.y += (pCamera->posRDest.y - pCamera->posR.y) * fRRot;
	pCamera->posR.z += (pCamera->posRDest.z - pCamera->posR.z) * fRRot;

	/*** 視点の補正 ***/
	pCamera->posV.x += (pCamera->posVDest.x - pCamera->posV.x) * fVRot;
	pCamera->posV.y += (pCamera->posVDest.y - pCamera->posV.y) * fRRot;
	pCamera->posV.z += (pCamera->posVDest.z - pCamera->posV.z) * fVRot;

	/*** 視点の平行移動！ ***/
#if false
	if (GetKeyboardPress(DIK_UP) || GetJoypadPress(JOYKEY_UP))
	{ // Wを押したとき
		/** 追加入力 **/
		if (GetKeyboardPress(DIK_LEFT) || GetJoypadPress(JOYKEY_LEFT))
		{// Aを押したとき
			pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 1.75f)) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 1.75f)) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 1.75f)) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 1.75f)) * pCamera->fSpd;

		}
		else if (GetKeyboardPress(DIK_RIGHT) || GetJoypadPress(JOYKEY_RIGHT))
		{ // Dを押したとき
			pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pCamera->fSpd;
		}
		else
		{ // 純粋なW入力時
			pCamera->posV.x += sinf(pCamera->rot.y) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y) * pCamera->fSpd;
		}
	}
	else if (GetKeyboardPress(DIK_DOWN) || GetJoypadPress(JOYKEY_DOWN))
	{ // Sを押したとき
		/** 追加入力 **/
		if (GetKeyboardPress(DIK_LEFT) || GetJoypadPress(JOYKEY_LEFT))
		{// Aを押したとき
			pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pCamera->fSpd;

		}
		else if (GetKeyboardPress(DIK_RIGHT) || GetJoypadPress(JOYKEY_RIGHT))
		{ // Dを押したとき
			pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 0.75f)) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 0.75f)) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 0.75f)) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 0.75f)) * pCamera->fSpd;
		}
		else
		{ // 純粋なS入力時
			pCamera->posV.x += sinf(pCamera->rot.y + D3DX_PI) * pCamera->fSpd;
			pCamera->posV.z += cosf(pCamera->rot.y + D3DX_PI) * pCamera->fSpd;
			pCamera->posR.x += sinf(pCamera->rot.y + D3DX_PI) * pCamera->fSpd;
			pCamera->posR.z += cosf(pCamera->rot.y + D3DX_PI) * pCamera->fSpd;
		}
	}
	else if (GetKeyboardPress(DIK_LEFT) || GetJoypadPress(JOYKEY_LEFT))
	{ // Aを押したとき
		pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pCamera->fSpd;
		pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pCamera->fSpd;
		pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pCamera->fSpd;
		pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pCamera->fSpd;
	}
	else if (GetKeyboardPress(DIK_RIGHT) || GetJoypadPress(JOYKEY_RIGHT))
	{ // Dを押したとき
		pCamera->posV.x += sinf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pCamera->fSpd;
		pCamera->posV.z += cosf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pCamera->fSpd;
		pCamera->posR.x += sinf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pCamera->fSpd;
		pCamera->posR.z += cosf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pCamera->fSpd;
	}
#endif

#if false
	/** 注視点のY軸の移動 **/
	if (GetKeyboardPress(DIK_T))
	{
		pCamera->posR.y += pCamera->fSpd;
	}
	else if (GetKeyboardPress(DIK_B))
	{
		pCamera->posR.y -= pCamera->fSpd;
	}

	/** 視点のY軸の移動 **/
	if (GetKeyboardPress(DIK_Y))
	{
		pCamera->posV.y += pCamera->fSpd;
	}
	else if (GetKeyboardPress(DIK_N))
	{
		pCamera->posV.y -= pCamera->fSpd;
	}

	/*** カメラの回転！(視点中心) ***/
	if (GetKeyboardPress(DIK_Q))
	{
		pCamera->rot.y -= pCamera->fRotSpd;

		if (pCamera->rot.y < -D3DX_PI)
		{
			pCamera->rot.y += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.y >= D3DX_PI)
		{
			pCamera->rot.y -= D3DX_PI * 2.0f;
		}

		pCamera->posR.x = pCamera->posV.x + sinf(pCamera->rot.y) * pCamera->fZlength;
		pCamera->posR.z = pCamera->posV.z + cosf(pCamera->rot.y) * pCamera->fZlength;
	}
	else if (GetKeyboardPress(DIK_E))
	{
		pCamera->rot.y += pCamera->fRotSpd;

		if (pCamera->rot.y < -D3DX_PI)
		{
			pCamera->rot.y += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.y >= D3DX_PI)
		{
			pCamera->rot.y -= D3DX_PI * 2.0f;
		}

		pCamera->posR.x = pCamera->posV.x + sinf(pCamera->rot.y) * pCamera->fZlength;
		pCamera->posR.z = pCamera->posV.z + cosf(pCamera->rot.y) * pCamera->fZlength;
	}
#endif
	/*** カメラの回転！(注視点中心) ***/
	if (GetKeyboardPress(DIK_Q) || GetJoypadPress(JOYKEY_LEFT_SHOULDER))
	{
		pCamera->rot.y -= pCamera->fRotSpd;

		if (pCamera->rot.y < -D3DX_PI)
		{
			pCamera->rot.y += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.y >= D3DX_PI)
		{
			pCamera->rot.y -= D3DX_PI * 2.0f;
		}

		pCamera->posVDest.x = pCamera->posRDest.x + sinf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posVDest.z = pCamera->posRDest.z + cosf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	}
	else if (GetKeyboardPress(DIK_E) || GetJoypadPress(JOYKEY_RIGHT_SHOULDER))
	{
		pCamera->rot.y += pCamera->fRotSpd;

		if (pCamera->rot.y < -D3DX_PI)
		{
			pCamera->rot.y += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.y >= D3DX_PI)
		{
			pCamera->rot.y -= D3DX_PI * 2.0f;
		}

		pCamera->posVDest.x = pCamera->posRDest.x + sinf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posVDest.z = pCamera->posRDest.z + cosf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	}

	if (GetKeyboardPress(DIK_UP))
	{
		g_fYCamera += 1.0f;
	}
	else if (GetKeyboardPress(DIK_DOWN))
	{
		g_fYCamera -= 1.0f;
	}

	/*** マウスでの視点変更処理！ ***/
	if (GetMouseMove(MOUSESLOPE_LR) != 0 && FALSE)
	{
		pCamera->rot.y += (float)GetMouseMove(MOUSESLOPE_LR) * 0.008f;
		if (pCamera->rot.y < -D3DX_PI)
		{
			pCamera->rot.y += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.y >= D3DX_PI)
		{
			pCamera->rot.y -= D3DX_PI * 2.0f;
		}

		pCamera->posVDest.x = pCamera->posRDest.x + sinf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posVDest.z = pCamera->posRDest.z + cosf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	}

	/*** マウスでの注視点変更処理！ ***/
	if (GetMouseMove(MOUSESLOPE_FB) != 0 && GetMousePress(0) && FALSE)
	{
		pCamera->posR.y -= (float)GetMouseMove(MOUSESLOPE_FB);
		if (pCamera->posR.y < pCamera->posV.y - CAMERA_MOUSEUD)
		{
			pCamera->posR.y = pCamera->posV.y - CAMERA_MOUSEUD;
		}
		else if (pCamera->posR.y >= pCamera->posV.y + CAMERA_MOUSEUD)
		{
			pCamera->posR.y = pCamera->posV.y + CAMERA_MOUSEUD;
		}

		pCamera->posVDest.x = pCamera->posRDest.x + sinf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posVDest.z = pCamera->posRDest.z + cosf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	}

	if (GetKeyboardPress(DIK_V))
	{
		pCamera->rot.x -= pCamera->fRotSpd * sinf(pCamera->rot.y);
		pCamera->rot.z -= pCamera->fRotSpd * cosf(pCamera->rot.y);

		if (pCamera->rot.x < -D3DX_PI)
		{
			pCamera->rot.x += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.x >= D3DX_PI)
		{
			pCamera->rot.x -= D3DX_PI * 2.0f;
		}

		if (pCamera->rot.z < -D3DX_PI)
		{
			pCamera->rot.z += D3DX_PI * 2.0f;
		}
		else if (pCamera->rot.z >= D3DX_PI)
		{
			pCamera->rot.z -= D3DX_PI * 2.0f;
		}

		pCamera->posV.x = pCamera->posR.x + sinf(pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posV.z = pCamera->posR.z + cosf(pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	}

	if (GetJoyThumbWatch() == true && FALSE)
	{
		fStickSpd = GetJoyThumbPow(JOYTHUMB_RIGHT + JOYTHUMB_X);
		fZLength = GetJoyThumbPow(JOYTHUMB_RIGHT + JOYTHUMB_Y);

		if (fStickSpd < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || fStickSpd > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			fStickSpd *= 0.005f;
		}
		else
		{
			fStickSpd = 0.0f;
		}

		if (fZLength < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || fZLength > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			pCamera->fZlength += fZLength * 0.00005f;
		}

		if (pCamera->fZlength < fMinZLength)
		{
			pCamera->fZlength = fMinZLength;
		}
		else if (pCamera->fZlength > fMaxZLength)
		{
			pCamera->fZlength = fMaxZLength;
		}

		if (fStickSpd >= fMaxStickSpd)
		{
			fStickSpd = fMaxStickSpd;
		}
		else if (fStickSpd <= -fMaxStickSpd)
		{
			fStickSpd = -fMaxStickSpd;
		}

		/** カメラの角度に合わせて、平行移動！ **/
		pCamera->move.y += fStickSpd;

		pCamera->rot.y += pCamera->move.y * 1.0f;

		while (pCamera->rot.y < -D3DX_PI || pCamera->rot.y >= D3DX_PI)
		{
			if (pCamera->rot.y < -D3DX_PI)
			{
				pCamera->rot.y += D3DX_PI * 2.0f;
			}
			else if (pCamera->rot.y >= D3DX_PI)
			{
				pCamera->rot.y -= D3DX_PI * 2.0f;
			}
		}

		pCamera->posVDest.x = pCamera->posRDest.x + sinf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
		pCamera->posVDest.z = pCamera->posRDest.z + cosf(pPlayer->rot.y + pCamera->rot.y + D3DX_PI) * pCamera->fZlength;

		g_bControl = true;
	}
	else
	{
		g_bControl = false;
	}

	pCamera->move.y = (0.0f - pCamera->move.y) * 0.05f;

	InitedVec3(null);
	Vec3(posV = pCamera->posV);
	float fLengthBefore = 0.0f;
	float fLength = 0.0f;

	Vec3(length = pCamera->posR - pCamera->posV);

	fLengthBefore = D3DXVec3Length(&length);

	CollisionWall(&pCamera->posV, &pCamera->posVOld, &null);

	length = pCamera->posR - pCamera->posV;

	fLength = D3DXVec3Length(&length);
	if (fLength >= fLengthBefore)
	{
		pCamera->posV = posV;
	}

#ifdef SPOTLIGHT
	InitedVec3(vec);
	InitedVec3(vecOut);

	vec = pCamera->posR - pCamera->posV;

	D3DXVec3Normalize(&vecOut, &vec);

	SetSpotLight(pCamera->posV, vec);
#endif
}

//================================================================================================================
// --- カメラの設置 ---
//================================================================================================================
void SetCamera(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	Camera *pCamera = &g_camera;
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
void SetCamera(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float zn, float zf, float fZLength, float fSpd, float fRotSpd)
{
	Camera *pCamera = &g_camera;

	/*** カメラの情報セット ***/
	pCamera->posV = posV;
	pCamera->posR = posR;
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
Camera* GetCamera(void)
{
	return &g_camera;
}

//================================================================================================================
// --- カメラのリセット ---
//================================================================================================================
void ResetCamera(void)
{
	Camera *pCamera = &g_camera;

	pCamera->rot = D3DXVECTOR3_NULL;
	pCamera->fZlength = CAMERA_ZLENGTH;

	pCamera->posVDest.x = pCamera->posRDest.x + sinf(pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
	pCamera->posVDest.z = pCamera->posRDest.z + cosf(pCamera->rot.y + D3DX_PI) * pCamera->fZlength;
}
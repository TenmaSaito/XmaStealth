//==================================================================================
//
// カメラ処理 (ゲーム画面用 + 各画面共通の処理) [camera.cpp]
// Author : TENMA SAITO
//
// 各画面のカメラ(cameraTitle / cameraTutorial / cameraRanking)も同じ Camera 構造体を使い、
// 初期化・行列設定・プレイヤー追従はこのファイルの共通関数を呼び出す。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "camera.h"
#include "mathUtil.h"
#include "player.h"
#include "spotlight.h"
#include "wall.h"
#include "field.h"
#include "mesh.h"
#include "object.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
// --- 初期値 ---
#define CAMERA_POSV_Y			(75.0f)			// 初期の視点の高さ
#define CAMERA_ZLENGTH			(300.0f)		// カメラの注視点との距離
#define CAMERA_SPD				(2.0f)			// カメラの移動速度
#define CAMERA_ROTSPD			(0.05f)			// カメラの回転速度
#define CAMERA_ZN				(10.0f)			// カメラの描画開始距離
#define CAMERA_ZF				(1000.0f)		// カメラの描画限界距離
#define CAMERA_VANGLE			(45.0f)			// カメラの視野角

// --- プレイヤー追従 ---
#define FOLLOW_POSR_FORWARD		(110.0f)		// 注視点をプレイヤーのどれだけ前方に置くか
#define FOLLOW_POSR_HEIGHT		(5.0f)			// 注視点のプレイヤーからの高さ
#define FOLLOW_POSV_HEIGHT		(50.0f)			// 視点のプレイヤーからの高さ
#define FOLLOW_RATE_POSR		(0.075f)		// 注視点(と視点の高さ)が目標へ近づく割合
#define FOLLOW_RATE_POSV		(0.085f)		// 視点(水平方向)が目標へ近づく割合

// --- 入力による操作 ---
#define MOUSE_ROT_RATE			(0.0008f)		// マウス移動量1あたりの回転量
#define STICK_ROT_RATE			(0.005f)		// スティック入力値1あたりの回転量
#define STICK_ROT_MAX			(0.05f)			// スティックによる1フレームの最大回転量
#define STICK_ZOOM_RATE			(0.00005f)		// スティック入力値1あたりの距離変化
#define ZOOM_MIN				(100.0f)		// 注視点との最小距離
#define ZOOM_MAX				(500.0f)		// 注視点との最大距離
#define ROTMOVE_RESIST			(0.05f)			// スティック回転の減衰係数

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void SetFollowPosVDest(Camera *pCamera, const Player *pPlayer);
static void CollisionCameraPosV(Camera *pCamera);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static Camera g_camera;		// ゲーム画面のカメラの情報

//==================================================================================
// --- カメラの初期化 ---
//==================================================================================
void InitCamera(void)
{
	InitCameraInfo(&g_camera);
}

//==================================================================================
// --- カメラの終了 ---
//==================================================================================
void UninitCamera(void)
{

}

//==================================================================================
// --- カメラの更新 (プレイヤー追従 + マウス・ズーム操作 + 壁との当たり判定) ---
//==================================================================================
void UpdateCamera(void)
{
	UpdateFollowCamera(&g_camera, true);
}

//==================================================================================
// --- カメラの設置 (プロジェクション・ビューマトリックスの設定) ---
//==================================================================================
void SetCamera(void)
{
	SetCameraTransform(&g_camera);
}

//==================================================================================
// --- カメラの情報の設定 (スクリプトから) ---
//==================================================================================
void SetCamera(D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd)
{
	SetCameraInfo(&g_camera, posV, posR, fAngle, fZNear, fZFar, fZLength, fSpd, fRotSpd);
}

//==================================================================================
// --- カメラの取得 ---
//==================================================================================
Camera *GetCamera(void)
{
	return &g_camera;
}

//==================================================================================
// --- カメラの向き・距離のリセット ---
//==================================================================================
void ResetCamera(void)
{
	ResetCameraInfo(&g_camera);
}

//==================================================================================
//
// ここから各画面のカメラ共通の処理
//
//==================================================================================

//==================================================================================
// --- カメラ情報の初期化 ---
//==================================================================================
void InitCameraInfo(Camera *pCamera)
{
	pCamera->posV = D3DXVECTOR3(0.0f, CAMERA_POSV_Y, -CAMERA_ZLENGTH);
	pCamera->posR = D3DXVECTOR3_NULL;
	pCamera->vecU = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pCamera->posVDest = D3DXVECTOR3_NULL;
	pCamera->posRDest = D3DXVECTOR3_NULL;
	pCamera->rot = D3DXVECTOR3_NULL;
	pCamera->rotDest = D3DXVECTOR3_NULL;
	pCamera->fZNear = CAMERA_ZN;
	pCamera->fZFar = CAMERA_ZF;
	pCamera->fZLength = CAMERA_ZLENGTH;
	pCamera->fSpd = CAMERA_SPD;
	pCamera->fRotSpd = CAMERA_ROTSPD;
	pCamera->fAngle = CAMERA_VANGLE;
}

//==================================================================================
// --- カメラ情報の設定 ---
//==================================================================================
void SetCameraInfo(Camera *pCamera, D3DXVECTOR3 posV, D3DXVECTOR3 posR, float fAngle, float fZNear, float fZFar, float fZLength, float fSpd, float fRotSpd)
{
	pCamera->posV = posV;
	pCamera->posR = posR;
	pCamera->posOffSet = posR;
	pCamera->fAngle = fAngle;
	pCamera->fZNear = fZNear;
	pCamera->fZFar = fZFar;
	pCamera->fZLength = fZLength;
	pCamera->fSpd = fSpd;
	pCamera->fRotSpd = fRotSpd;
}

//==================================================================================
// --- プロジェクション・ビューマトリックスの作成と設定 ---
//==================================================================================
void SetCameraTransform(Camera *pCamera)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** プロジェクションマトリックスの作成・設定 ***/
	D3DXMatrixIdentity(&pCamera->mtxProjection);
	D3DXMatrixPerspectiveFovLH(&pCamera->mtxProjection,
		D3DXToRadian(pCamera->fAngle),					// 視野角
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,		// アスペクト比
		pCamera->fZNear,								// 描画開始距離
		pCamera->fZFar);								// 描画限界距離
	pDevice->SetTransform(D3DTS_PROJECTION, &pCamera->mtxProjection);

	/*** ビューマトリックスの作成・設定 ***/
	D3DXMatrixIdentity(&pCamera->mtxView);
	D3DXMatrixLookAtLH(&pCamera->mtxView,
		&pCamera->posV,		// 視点
		&pCamera->posR,		// 注視点
		&pCamera->vecU);	// 上方向ベクトル
	pDevice->SetTransform(D3DTS_VIEW, &pCamera->mtxView);
}

//==================================================================================
// --- カメラの向き・距離のリセット ---
//==================================================================================
void ResetCameraInfo(Camera *pCamera)
{
	pCamera->rot = D3DXVECTOR3_NULL;
	pCamera->fZLength = CAMERA_ZLENGTH;

	pCamera->posVDest.x = pCamera->posRDest.x + sinf(pCamera->rot.y + D3DX_PI) * pCamera->fZLength;
	pCamera->posVDest.z = pCamera->posRDest.z + cosf(pCamera->rot.y + D3DX_PI) * pCamera->fZLength;
}

//==================================================================================
// --- プレイヤー追従カメラの更新 ---
// bUseGameControl : true にするとマウス回転・スティックでのズーム・壁との当たり判定も行う
//==================================================================================
void UpdateFollowCamera(Camera *pCamera, bool bUseGameControl)
{
	Player *pPlayer = GetPlayer();		// 追従するプレイヤー
	bool bControlButton = false;		// ボタン(Q/E・LB/RB)で回転させたか

	pCamera->posVOld = pCamera->posV;
	pCamera->rot.y = RepairRot(pCamera->rot.y);

	/*** 目標の注視点 : プレイヤーの前方 ***/
	pCamera->posRDest.x = pPlayer->pos.x + sinf(pCamera->rot.y) * FOLLOW_POSR_FORWARD;
	pCamera->posRDest.y = pPlayer->pos.y + FOLLOW_POSR_HEIGHT;
	pCamera->posRDest.z = pPlayer->pos.z + cosf(pCamera->rot.y) * FOLLOW_POSR_FORWARD;

	/*** 目標の視点 : プレイヤーの後方 ***/
	float fRotBack = RepairRot(pCamera->rot.y + D3DX_PI);		// カメラの向きの反対方向

	pCamera->posVDest.x = pPlayer->pos.x + sinf(fRotBack) * pCamera->fZLength;
	pCamera->posVDest.y = pPlayer->pos.y + FOLLOW_POSV_HEIGHT;
	pCamera->posVDest.z = pPlayer->pos.z + cosf(fRotBack) * pCamera->fZLength;

	/*** 注視点・視点を目標へ少しずつ近づける ***/
	pCamera->posR += (pCamera->posRDest - pCamera->posR) * FOLLOW_RATE_POSR;

	pCamera->posV.x += (pCamera->posVDest.x - pCamera->posV.x) * FOLLOW_RATE_POSV;
	pCamera->posV.y += (pCamera->posVDest.y - pCamera->posV.y) * FOLLOW_RATE_POSR;
	pCamera->posV.z += (pCamera->posVDest.z - pCamera->posV.z) * FOLLOW_RATE_POSV;

	/*** ボタンでの回転 (注視点中心) ***/
	if (GetKeyboardPress(DIK_Q) || GetJoypadPress(JOYKEY_LEFT_SHOULDER))
	{ // 左回り
		pCamera->rot.y = RepairRot(pCamera->rot.y - pCamera->fRotSpd);
		SetFollowPosVDest(pCamera, pPlayer);
		bControlButton = true;
	}
	else if (GetKeyboardPress(DIK_E) || GetJoypadPress(JOYKEY_RIGHT_SHOULDER))
	{ // 右回り
		pCamera->rot.y = RepairRot(pCamera->rot.y + pCamera->fRotSpd);
		SetFollowPosVDest(pCamera, pPlayer);
		bControlButton = true;
	}

	/*** マウスでの回転 (ゲーム画面のみ) ***/
	if (bUseGameControl && GetMouseMove(MOUSESLOPE_LR) != 0 && bControlButton == false)
	{
		float fRotMouse = (float)GetMouseMove(MOUSESLOPE_LR) * MOUSE_ROT_RATE;		// マウスによる回転量
		if (fRotMouse >= pCamera->fRotSpd)
		{ // 右回りのみ上限あり (元の仕様)
			fRotMouse = pCamera->fRotSpd;
		}

		pCamera->rot.y = RepairRot(pCamera->rot.y + fRotMouse);
		SetFollowPosVDest(pCamera, pPlayer);
	}

	/*** 右スティックでの回転・ズーム ***/
	if (GetJoyThumbWatch() == true && bControlButton == false)
	{
		float fStickX = GetJoyThumbPow(JOYTHUMB_RIGHT + JOYTHUMB_X);		// 右スティック左右
		float fStickRot = 0.0f;												// スティックによる回転量

		if (fStickX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || fStickX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{ // デッドゾーン外
			fStickRot = fStickX * STICK_ROT_RATE;
		}

		if (bUseGameControl)
		{ // ゲーム画面のみ : 右スティック上下でズーム
			float fStickY = GetJoyThumbPow(JOYTHUMB_RIGHT + JOYTHUMB_Y);		// 右スティック上下

			if (fStickY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || fStickY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{ // デッドゾーン外
				pCamera->fZLength += fStickY * STICK_ZOOM_RATE;
			}

			if (pCamera->fZLength < ZOOM_MIN)		pCamera->fZLength = ZOOM_MIN;
			else if (pCamera->fZLength > ZOOM_MAX)	pCamera->fZLength = ZOOM_MAX;
		}

		/*** 回転量を制限して加算 ***/
		if (fStickRot >= STICK_ROT_MAX)			fStickRot = STICK_ROT_MAX;
		else if (fStickRot <= -STICK_ROT_MAX)	fStickRot = -STICK_ROT_MAX;

		pCamera->move.y += fStickRot;
		pCamera->rot.y += pCamera->move.y;

		while (pCamera->rot.y < -D3DX_PI || pCamera->rot.y >= D3DX_PI)
		{ // 何周分ずれていても範囲内に戻す
			pCamera->rot.y = RepairRot(pCamera->rot.y);
			if (pCamera->rot.y == D3DX_PI) pCamera->rot.y = -D3DX_PI;
		}

		SetFollowPosVDest(pCamera, pPlayer);
	}

	/*** スティック回転の減衰 ***/
	pCamera->move.y = (0.0f - pCamera->move.y) * ROTMOVE_RESIST;

	/*** 視点が壁・オブジェクトにめり込まないようにする (ゲーム画面のみ) ***/
	if (bUseGameControl)
	{
		CollisionCameraPosV(pCamera);
	}
}

//==================================================================================
// --- 現在の向きから目標の視点を再計算 (回転操作後に呼ぶ) ---
//==================================================================================
static void SetFollowPosVDest(Camera *pCamera, const Player *pPlayer)
{
	float fRot = pPlayer->rot.y + pCamera->rot.y + D3DX_PI;		// 視点を置く方向

	pCamera->posVDest.x = pCamera->posRDest.x + sinf(fRot) * pCamera->fZLength;
	pCamera->posVDest.z = pCamera->posRDest.z + cosf(fRot) * pCamera->fZLength;
}

//==================================================================================
// --- 視点と壁・オブジェクトの当たり判定 ---
// 押し戻した結果、注視点から遠ざかってしまう場合は押し戻しを取り消す
//==================================================================================
static void CollisionCameraPosV(Camera *pCamera)
{
	D3DXVECTOR3 moveNull = D3DXVECTOR3_NULL;		// 当たり判定に渡す移動量(使わない)
	D3DXVECTOR3 posVBefore = pCamera->posV;			// 当たり判定前の視点
	D3DXVECTOR3 vecToPosR = pCamera->posR - pCamera->posV;		// 視点 → 注視点
	float fLengthBefore = D3DXVec3Length(&vecToPosR);			// 当たり判定前の距離

	CollisionWall(&pCamera->posV, &pCamera->posVOld, &moveNull);
	CollisionObject(&pCamera->posV, &pCamera->posVOld, &moveNull, 0.0f);

	vecToPosR = pCamera->posR - pCamera->posV;
	if (D3DXVec3Length(&vecToPosR) >= fLengthBefore)
	{ // 遠ざかったら元に戻す
		pCamera->posV = posVBefore;
	}
}

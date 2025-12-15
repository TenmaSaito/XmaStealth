//================================================================================================================
//
// DirectXのプレイヤー表示処理 [player.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "player.h"
#include "mathUtil.h"
#include "shadow.h"
#include "wall.h"
#include "field.h"
#include "bullet.h"
#include "effect.h"
#include "explosion.h"
#include "object.h"
#include "debugproc.h"
#include "line.h"
#include "mesh.h"
#include "ornament.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define PLAYER_SIZE_X		(10)		// プレイヤーの基本サイズ - X
#define PLAYER_SIZE_Z		(10)		// プレイヤーの基本サイズ - Y
#define PLAYER_SPD			(2.0f)		// プレイヤーの移動スピード
#define PLAYER_ROTSPD		(0.1f)		// プレイヤーの回転スピード
#define PLAYER_WDSPD		(0.1f)		// プレイヤーの拡縮スピード
#define MAX_TEXTURE			(14)		// 読み込むテクスチャの最大数
#define MOVE_ACCELE			(5.0f)		// 加速係数
#define MOVE_RESIST			(0.5f)		// 減速係数
#define MAX_TAKECOPTER_SPD	(1.57f)		// タケコプターの最大回転速度

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************
void SetMotionType(MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend);
void UpdateMotion(void);

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
Player g_player;									// モデルの情報
POBJECTINFO g_pObjInfo = NULL;						// オブジェクトへの情報
float g_fAnglePlayer;								// モデルの向きの慣性	
float g_fTakecopterSpeed;							// タケコプターの回転速度
float g_fSinCurve;									// プレイヤーの縦移動時のサインカーブ用角度
float g_fMoveAccele;								// 前方へ進行した場合の前傾姿勢用角度
bool g_bUseSinCurve;								// プレイヤーの上下移動の有無
int g_nCounterPlayer;								// カウンター

//================================================================================================================
// --- Xファイル表示の初期化処理 ---
//================================================================================================================
void InitPlayer(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATERIAL* pMat = NULL;			// マテリアルへのポインタ
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	/*** 各変数の初期化 ***/
	pPlayer->pos = D3DXVECTOR3_NULL;
	pPlayer->posOld = D3DXVECTOR3_NULL;
	pPlayer->move = D3DXVECTOR3_NULL;
	pPlayer->rot = D3DXVECTOR3_NULL;
	pPlayer->fWidth = 0.0f;
	pPlayer->fHeight = 0.0f;
	pPlayer->fDepth = 0.0f;
	pPlayer->fSpd = 0.0f;
	g_fAnglePlayer = 0.0f;
	g_fSinCurve = 0.0f;
	g_fTakecopterSpeed = 0.0f;
	pPlayer->nIdShadow = -1;
	pPlayer->bJump = false;
	memset(&pPlayer->aModel[0], NULL, sizeof(PartsInfo) * 15);
	memset(&pPlayer->aMotionInfo[0], NULL, sizeof(MOTION_INFO) * MAX_MOTION);
	g_bUseSinCurve = false;

	/*** 影の生成 ***/
	pPlayer->nIdShadow = SetShadow(PLAYER_SIZE_X, PLAYER_SIZE_Z);

	pPlayer->motionType = MOTIONTYPE_NEUTRAL;
	pPlayer->nCounterMotion = 0;
	pPlayer->nKey = 0;
	g_nCounterPlayer = 0;
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitPlayer(void)
{

}

//================================================================================================================
// --- Xファイル表示の更新 ---
//================================================================================================================
void UpdatePlayer(void)
{
	/*** カメラ情報の取得 ***/
	Camera *pCamera = GetCamera();
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス
	D3DXVECTOR3 effectPos;				// エフェクトの位置
	D3DXVECTOR3 posStart;
	D3DXVECTOR3 posEnd;
	D3DXVECTOR3 posMove;
	D3DXVECTOR3 posReflect;
	D3DXVECTOR3 posReflectAnothor;
	D3DXVECTOR3 pos;
	float fAngle = 0.0f;		// 角度修正用変数
	bool bPushKey = false;		// 移動したか

	pPlayer->posOld = pPlayer->pos;
#if 1
	/*** ポリゴンの移動！ (※ 頂点バッファのロック・アンロックは必要なし！) ***/
	if (GetKeyboardPress(DIK_W))
	{ // Wを押したとき
		/** 追加入力確認！ **/
		if (GetKeyboardPress(DIK_A))
		{ // Aを押したとき
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.z += cosf(pCamera->rot.y - (D3DX_PI * 0.25f)) * pPlayer->fSpd;
			pPlayer->move.x += sinf(pCamera->rot.y - (D3DX_PI * 0.25f)) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = (pCamera->rot.y + (D3DX_PI * 0.75f));
		}
		else if (GetKeyboardPress(DIK_D))
		{ // Dを押したとき
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.z += cosf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pPlayer->fSpd;
			pPlayer->move.x += sinf(pCamera->rot.y + (D3DX_PI * 0.25f)) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = (pCamera->rot.y + (D3DX_PI * 1.25f));
		}
		else
		{ // 純粋なW入力時
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.x += sinf(pCamera->rot.y) * pPlayer->fSpd;
			pPlayer->move.z += cosf(pCamera->rot.y) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = (pCamera->rot.y + D3DX_PI);
		}

		bPushKey = true;
	}
	else if (GetKeyboardPress(DIK_S))
	{ // Sを押したとき
		/** 追加入力確認！ **/
		if (GetKeyboardPress(DIK_A))
		{ // Aを押したとき
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.z += cosf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pPlayer->fSpd;
			pPlayer->move.x += sinf(pCamera->rot.y + (D3DX_PI * 1.25f)) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = (pCamera->rot.y + (D3DX_PI * 0.25f));
		}
		else if (GetKeyboardPress(DIK_D))
		{ // Dを押したと
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.z += cosf(pCamera->rot.y - (D3DX_PI * 1.25f)) * pPlayer->fSpd;
			pPlayer->move.x += sinf(pCamera->rot.y - (D3DX_PI * 1.25f)) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = (pCamera->rot.y - (D3DX_PI * 0.25f));
		}
		else
		{ // 純粋なS入力時
			/* カメラの角度に合わせて、平行移動！ */
			pPlayer->move.x += sinf(pCamera->rot.y + D3DX_PI) * pPlayer->fSpd;
			pPlayer->move.z += cosf(pCamera->rot.y + D3DX_PI) * pPlayer->fSpd;

			/* カメラの角度に合わせて、モデルの目標角度を求める！ */
			g_fAnglePlayer = pCamera->rot.y;
		}

		bPushKey = true;
	}
	else if (GetKeyboardPress(DIK_A))
	{ // Aを押したとき
		/** カメラの角度に合わせて、平行移動！ **/
		pPlayer->move.z += cosf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pPlayer->fSpd;
		pPlayer->move.x += sinf(pCamera->rot.y + (D3DX_PI * 1.5f)) * pPlayer->fSpd;

		/** カメラの角度に合わせて、モデルの目標角度を求める！ **/
		g_fAnglePlayer = (pCamera->rot.y + (D3DX_PI * 0.5f));

		bPushKey = true;
	}
	else if (GetKeyboardPress(DIK_D))
	{ // Dを押したとき
		/** カメラの角度に合わせて、平行移動！ **/
		pPlayer->move.z += cosf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pPlayer->fSpd;
		pPlayer->move.x += sinf(pCamera->rot.y + (D3DX_PI * 0.5f)) * pPlayer->fSpd;

		/** カメラの角度に合わせて、モデルの目標角度を求める！ **/
		g_fAnglePlayer = (pCamera->rot.y + (D3DX_PI * 1.5f));

		bPushKey = true;
	}
	
	if (bPushKey == true 
		&& pPlayer->motionType != MOTIONTYPE_MOVE
		&& pPlayer->motionTypeBlend != MOTIONTYPE_MOVE
		&& pPlayer->motionType != MOTIONTYPE_JUMP
		&& pPlayer->motionTypeBlend != MOTIONTYPE_JUMP)
	{ // 移動するとMOVEに移行
		SetMotionType(MOTIONTYPE_MOVE, true, 10);
	}
	else if (bPushKey == false 
		&& pPlayer->motionType != MOTIONTYPE_NEUTRAL
		&& pPlayer->motionTypeBlend != MOTIONTYPE_NEUTRAL
		&& pPlayer->bJump == false
		&& pPlayer->motionType != MOTIONTYPE_LANDING
		&& pPlayer->motionTypeBlend != MOTIONTYPE_LANDING
		&& pPlayer->motionType != MOTIONTYPE_ACTION
		&& pPlayer->motionTypeBlend != MOTIONTYPE_ACTION)
	{ // 何もしていないとNEUTRALに移行
		SetMotionType(MOTIONTYPE_NEUTRAL, true, 10);
	}

	if (GetJoyThumbWASD() == true)
	{
		/** カメラの角度に合わせて、平行移動！ **/
		pPlayer->move.z += cosf(pCamera->rot.y + GetJoyThumbAngle(JOYTHUMB_LEFT)) * (sqrtf(powf((GetJoyThumbPow(JOYTHUMB_LEFT + JOYTHUMB_X) / 32767), 2.0f) + powf(GetJoyThumbPow(JOYTHUMB_LEFT + JOYTHUMB_Y) / 32767, 2.0f))) * pPlayer->fSpd;
		pPlayer->move.x += sinf(pCamera->rot.y + GetJoyThumbAngle(JOYTHUMB_LEFT)) * (sqrtf(powf((GetJoyThumbPow(JOYTHUMB_LEFT + JOYTHUMB_X) / 32767), 2.0f) + powf(GetJoyThumbPow(JOYTHUMB_LEFT + JOYTHUMB_Y) / 32767, 2.0f))) * pPlayer->fSpd;

		/** カメラの角度に合わせて、モデルの目標角度を求める！ **/
		g_fAnglePlayer = (pCamera->rot.y + (GetJoyThumbAngle(JOYTHUMB_LEFT) + D3DX_PI));
	}

#endif
	effectPos = pPlayer->pos;
	effectPos.y += 6.0f;

	/*** エンジンの煙 ***/
	if (bPushKey == true)
	{
		//SetEffect(effectPos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), 0.0f);
	}

	/*** モデルの高さ変更処理！ ***/
	if (GetKeyboardPress(DIK_7))
	{ // 7を押したとき
		pPlayer->move.y = 0.0f;
		pPlayer->pos.y += 1.0f;
	}
	else if (GetKeyboardPress(DIK_8))
	{ // 8を押したとき
		pPlayer->move.y = 0.0f;
		pPlayer->pos.y -= 1.0f;
	}

	if (GetKeyboardRepeat(DIK_4))
	{
		g_fTakecopterSpeed += 0.01f;
		if (g_fTakecopterSpeed >= MAX_TAKECOPTER_SPD)
		{
			g_fTakecopterSpeed = MAX_TAKECOPTER_SPD;
		}
	}
	else if (GetKeyboardRepeat(DIK_5))
	{
		g_fTakecopterSpeed -= 0.01f;
		if (g_fTakecopterSpeed <= -MAX_TAKECOPTER_SPD)
		{
			g_fTakecopterSpeed = -MAX_TAKECOPTER_SPD;
		}
	}

	if (GetKeyboardTrigger(DIK_F9))
	{
		g_bUseSinCurve = g_bUseSinCurve ^ true;
	}

	if (g_bUseSinCurve == true)
	{
		g_fSinCurve += 0.05f;
		/*** 角度の修正! ***/
		RepairRot(&g_fSinCurve, &g_fSinCurve);

		pPlayer->pos.y += 0.5f * sinf(g_fSinCurve);
	}

	pPlayer->aModel[1].rotLocal.y += g_fTakecopterSpeed;
	/*** 角度の修正! ***/
	RepairRot(&pPlayer->aModel[1].rotLocal.y, &pPlayer->aModel[1].rotLocal.y);

	/*** モデルの向きの慣性修正! ***/
	/*** 角度の修正! ***/
	RepairRot(&g_fAnglePlayer, &g_fAnglePlayer);

	/*** 角度の差を求める! ***/
	fAngle = (g_fAnglePlayer - pPlayer->rot.y);
	/** 角度の修正! **/
	RepairRot(&fAngle, &fAngle);

	/*** 角度の差分だけ角度を徐々に回転する！ ***/
	pPlayer->rot.y += fAngle * 0.05f;

	/*** 角度の修正! ***/
	RepairRot(&pPlayer->rot.y, &pPlayer->rot.y);

	/*** 角度の差分一定の範囲よりも小さくなった場合、矯正！ ***/
	if ((g_fAnglePlayer - pPlayer->rot.y) < 0.001f && (g_fAnglePlayer - pPlayer->rot.y) > -0.001f)
	{
		pPlayer->rot.y = g_fAnglePlayer;
	}

	/*** 落下防止処理 ***/

	/*** 弾発射処理 ***/
	if ((GetKeyboardTrigger(DIK_SPACE) || GetJoypadTrigger(JOYKEY_RIGHTTRIGGER))
		&& pPlayer->bJump == false)
	{
		/*** 各変数の初期化 ***/
		pPlayer->move.y = pPlayer->fJump;
		pPlayer->bJump = true;

		SetMotionType(MOTIONTYPE_JUMP, true, 10);
	}

	/*** アクション！ ***/
	if (GetKeyboardTrigger(DIK_RETURN)
		&& g_player.motionType != MOTIONTYPE_JUMP
		&& pPlayer->motionTypeBlend != MOTIONTYPE_JUMP)
	{
		/*** 各変数の初期化 ***/
		SetMotionType(MOTIONTYPE_ACTION, true, 10);
	}

	if (GetKeyboardTrigger(DIK_BACK))
	{
		if (pPlayer->nIdOrnament != -1)
		{
			ShotOrnament(pPlayer->nIdOrnament, pPlayer->rot, 5.0f);
		}
	}

	if (GetKeyboardTrigger(DIK_RETURN))
	{
		int nIdxCollision = CollisionOrnament(pPlayer->pos);
		if (nIdxCollision != -1)
		{
			SetParentOrnament(nIdxCollision, &pPlayer->mtxWorld);
			pPlayer->nIdOrnament = nIdxCollision;
		}
	}

	pPlayer->pos.x += pPlayer->move.x;
	PrintDebugProc("PMOVE %f\n", pPlayer->move.y);
	pPlayer->pos.y += pPlayer->move.y;
	pPlayer->pos.z += pPlayer->move.z;

	/*** 床下判定 ***/
	/*if ((pPlayer->pos.x > 50.0f && pPlayer->pos.z > -100.0f
		&& pPlayer->pos.x <= 100.0f && pPlayer->pos.z < 48.5f) != true)
	{
		if (pPlayer->pos.x >= -420.0f && pPlayer->pos.x <= 120.0f
			&& pPlayer->pos.z >= -420.0f && pPlayer->pos.z <= 195.0f)
		{
			if (pPlayer->posOld.y >= 187.0f && pPlayer->pos.y <= 187.0f)
			{
				pPlayer->pos.y = 187.0f;
				pPlayer->move.y = 0.0f;

				if (pPlayer->bJump == true)
				{
					pPlayer->bJump = false;
					SetMotionType(MOTIONTYPE_LANDING, true, 10);
				}
			}
		}
	}*/

	if (pPlayer->pos.y <= 0.0f)
	{
		pPlayer->pos.y = 0.0f;
		pPlayer->move.y = 0.0f;

		if (pPlayer->bJump == true)
		{
			pPlayer->bJump = false;
			SetMotionType(MOTIONTYPE_LANDING, true, 10);
		}
	}

	PrintDebugProc("PMOVE %f\n", pPlayer->move.y);

	if (CollisionObject(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move) == true)
	{
		if (pPlayer->bJump == true)
		{
			pPlayer->bJump = false;
			SetMotionType(MOTIONTYPE_LANDING, true, 10);
		}
	}

	CollisionWall(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move);
	
	if (CollisionFloor(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move, pPlayer->fHeight))
	{
		pPlayer->move.y = 0.0f;

		if (pPlayer->bJump == true)
		{
			pPlayer->bJump = false;
			SetMotionType(MOTIONTYPE_LANDING, true, 10);
		}
	}

	if (CollisionMeshField(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move))
	{
		if(pPlayer->bJump == true)
		{
			pPlayer->bJump = false;
			SetMotionType(MOTIONTYPE_LANDING, true, 10);
		}
	}


	/*** これを判定の上に書くと、オブジェクトの判定を食らわなくなるから注意(Y座標が範囲外に行くことが原因) ***/
	pPlayer->move.y += WORLD_GRAVITY;
	if (pPlayer->move.y < TERMINAL_VELOCITY)
	{
		pPlayer->move.y = TERMINAL_VELOCITY;
	}

	pPlayer->move.x += (0.0f - pPlayer->move.x) * MOVE_RESIST;
	pPlayer->move.z += (0.0f - pPlayer->move.z) * MOVE_RESIST;

	/*** 影の移動 ***/
	SetPositionShadow(pPlayer->nIdShadow, pPlayer->pos, pPlayer->rot);

	PrintDebugProc("PlayerPos / X : %f Y : %f Z : %f\n", pPlayer->pos.x, pPlayer->pos.y, pPlayer->pos.z);
	PrintDebugProc("TakecopterSpd / %f\n", g_fTakecopterSpeed);
	PrintDebugProc("Pos[0] / %~3f\n", pPlayer->aModel[0].pos.x, pPlayer->aModel[0].pos.y, pPlayer->aModel[0].pos.z);

	// 予測線 ======================
	posStart = pPlayer->pos;
	posStart.y = pPlayer->pos.y;
	posEnd.x = pPlayer->pos.x + sinf(pPlayer->rot.y + D3DX_PI) * 10000.0f;
	posEnd.y = pPlayer->pos.y;
	posEnd.z = pPlayer->pos.z + cosf(pPlayer->rot.y + D3DX_PI) * 10000.0f;

	posMove = D3DXVECTOR3_NULL;
	posReflect = D3DXVECTOR3_NULL;

	ReflectLine(posEnd, posStart, &posReflect, &posMove);

	pos.x = posMove.x;
	pos.y = pPlayer->pos.y;
	pos.z = posMove.z;

	SetLine(posStart, posReflect, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));

	ReflectLine(pos, posReflect, &posReflectAnothor, &posMove);

	SetLine(posReflect, pos, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f));

	pos.x = posMove.x;
	pos.y = pPlayer->pos.y;
	pos.z = posMove.z;

	SetLine(posReflectAnothor, pos, D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f));

	// 予測線終了 ======================

	/*** モーションの更新! ***/
	UpdateMotion();

	PrintDebugProc("MOTION : %d\n", pPlayer->motionType);
	PrintDebugProc("MOTIONBLEND : %d\n", pPlayer->motionTypeBlend);

	g_nCounterPlayer++;

	/*** 万が一に備えて、ステージの壁を設定 ***/
	if (pPlayer->pos.z > 500.0f)
	{ // ステージの絶対的な奥行き
		pPlayer->pos.z = 500.0f;
	}

	if (pPlayer->pos.z < -400.0f)
	{ // ステージの絶対的な奥行き(後ろ)
		pPlayer->pos.z = -400.0f;
	}

	if (pPlayer->pos.x < -400.0f)
	{ // ステージの絶対的な左端
		pPlayer->pos.x = -400.0f;
	}

	if (pPlayer->pos.x > 400.0f)
	{ // ステージの絶対的な右端
		pPlayer->pos.x = 400.0f;
	}

	if (pPlayer->pos.z < 300.0f && pPlayer->pos.x > 100.0f)
	{ // リビングの右端
		pPlayer->pos.x = 100.0f;
	}

	if (pPlayer->pos.x > 100.0f && pPlayer->pos.z < 300.0f)
	{
		pPlayer->pos.z = 300.0f;
	}
 }

//================================================================================================================
// --- モーションアップデート！ ---
//================================================================================================================
void UpdateMotion(void)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	/*** 現在のモーションのキー情報へのポインタ ***/
	KEY_INFO *pInfo = &pPlayer->aMotionInfo[pPlayer->motionType].aKeyInfo[pPlayer->nKey];
	KEY_INFO *pInfoBlend = &pPlayer->aMotionInfo[pPlayer->motionTypeBlend].aKeyInfo[pPlayer->nKeyBlend];

	/*** 全パーツの更新！ ***/
	for (int nCntModel = 0; nCntModel < pPlayer->nNumModel; nCntModel++)
	{
		int nNext = (pPlayer->nKey + 1) % pPlayer->aMotionInfo[pPlayer->motionType].nNumKey;		// 次のキーの値
		float fRateKey = (float)pPlayer->nCounterMotion / (float)pInfo->nFrame;			// モーションカウンター / 再生フレーム数
		D3DXVECTOR3 diffPos = {};	// 位置の差分
		D3DXVECTOR3 UpdatePos = {};	// 更新する位置
		D3DXVECTOR3 diffRot = {};	// 角度の差分
		D3DXVECTOR3 UpdateRot = {};	// 更新する角度
		KEY *pKey = &pInfo->aKey[nCntModel];		// 現在のキー
		KEY *pKeyNext = &pPlayer->aMotionInfo[pPlayer->motionType].aKeyInfo[nNext].aKey[nCntModel];			// 次のキー
		PartsInfo *pModel = &pPlayer->aModel[nCntModel];

		if (pPlayer->bBlendMotion == true)
		{ // モーションブレンドあり
			D3DXVECTOR3 diffKeyPosCurrent = {};	// 現在のモーションの位置の差分
			D3DXVECTOR3 diffKeyRotCurrent = {};	// 現在のモーションの角度の差分
			D3DXVECTOR3 diffKeyPosBlend = {};	// ブレンドモーションの位置の差分
			D3DXVECTOR3 diffKeyRotBlend = {};	// ブレンドモーションの角度の差分
			D3DXVECTOR3 diffPosBlend = {};		// 最終差分
			D3DXVECTOR3 diffRotBlend = {};		// 最終差分
			int nNextBlend = (pPlayer->nKeyBlend + 1) % pPlayer->aMotionInfo[pPlayer->motionTypeBlend].nNumKey;		// 次のキーの値
			KEY* pKeyBlend = &pInfoBlend->aKey[nCntModel];														// 現在のキー
			KEY* pKeyNextBlend = &pPlayer->aMotionInfo[pPlayer->motionTypeBlend].aKeyInfo[nNextBlend].aKey[nCntModel];		// 次のキー
			float fRateKeyBlend = (float)pPlayer->nCounterMotionBlend / (float)pInfoBlend->nFrame;				// モーションカウンター / 再生フレーム数
			float fRateBlend = (float) pPlayer->nCounterBlend / (float) pPlayer->nFrameBlend;					// ブレンドの相対量

			// --- X ---
			/* 現在のモーション */
			diffPos.x = pKeyNext->fPosX - pKey->fPosX;
			diffKeyPosCurrent.x = pKey->fPosX + diffPos.x * fRateKey;

			/* ブレンドモーション */
			diffPos.x = pKeyNextBlend->fPosX - pKeyBlend->fPosX;
			diffKeyPosBlend.x = pKeyBlend->fPosX + diffPos.x * fRateKeyBlend;

			/* 求める差分 */
			diffPosBlend.x = diffKeyPosBlend.x - diffKeyPosCurrent.x;
			UpdatePos.x = diffKeyPosCurrent.x + (diffPosBlend.x * fRateBlend);

			// --- Y ---
			/* 現在のモーション */
			diffPos.y = pKeyNext->fPosY - pKey->fPosY;
			diffKeyPosCurrent.y = pKey->fPosY + diffPos.y * fRateKey;

			/* ブレンドモーション */
			diffPos.y = pKeyNextBlend->fPosY - pKeyBlend->fPosY;
			diffKeyPosBlend.y = pKeyBlend->fPosY + diffPos.y * fRateKeyBlend;

			/* 求める差分 */
			diffPosBlend.y = diffKeyPosBlend.y - diffKeyPosCurrent.y;
			UpdatePos.y = diffKeyPosCurrent.y + (diffPosBlend.y * fRateBlend);

			// --- Z ---
			/* 現在のモーション */
			diffPos.z = pKeyNext->fPosZ - pKey->fPosZ;
			diffKeyPosCurrent.z = pKey->fPosZ + diffPos.z * fRateKey;

			/* ブレンドモーション */
			diffPos.z = pKeyNextBlend->fPosZ - pKeyBlend->fPosZ;
			diffKeyPosBlend.z = pKeyBlend->fPosZ + diffPos.z * fRateKeyBlend;

			/* 求める差分 */
			diffPosBlend.z = diffKeyPosBlend.z - diffKeyPosCurrent.z;
			UpdatePos.z = diffKeyPosCurrent.z + (diffPosBlend.z * fRateBlend);

			// === ROT ===
			// --- X ---
			/* 現在のモーション */
			diffRot.x = pKeyNext->fRotX - pKey->fRotX;
			RepairRot(&diffRot.x, &diffRot.x);

			diffKeyRotCurrent.x = pKey->fRotX + diffRot.x * fRateKey;
			RepairRot(&diffKeyRotCurrent.x, &diffKeyRotCurrent.x);

			/* ブレンドモーション */
			diffRot.x = pKeyNextBlend->fRotX - pKeyBlend->fRotX;
			RepairRot(&diffRot.x, &diffRot.x);

			diffKeyRotBlend.x = pKeyBlend->fRotX + diffRot.x * fRateKeyBlend;
			RepairRot(&diffKeyRotBlend.x, &diffKeyRotBlend.x);

			/* 求める差分 */
			diffRotBlend.x = diffKeyRotBlend.x - diffKeyRotCurrent.x;
			UpdateRot.x = diffKeyRotCurrent.x + (diffRotBlend.x * fRateBlend);

			// --- Y ---
			diffRot.y = pKeyNext->fRotY - pKey->fRotY;
			RepairRot(&diffRot.y, &diffRot.y);

			diffKeyRotCurrent.y = pKey->fRotY + diffRot.y * fRateKey;
			RepairRot(&diffKeyRotCurrent.y, &diffKeyRotCurrent.y);

			/* ブレンドモーション */
			diffRot.y = pKeyNextBlend->fRotY - pKeyBlend->fRotY;
			RepairRot(&diffRot.y, &diffRot.y);

			diffKeyRotBlend.y = pKeyBlend->fRotY + diffRot.y * fRateKeyBlend;
			RepairRot(&diffKeyRotBlend.y, &diffKeyRotBlend.y);

			/* 求める差分 */
			diffRotBlend.y = diffKeyRotBlend.y - diffKeyRotCurrent.y;
			UpdateRot.y = diffKeyRotCurrent.y + (diffRotBlend.y * fRateBlend);

			// --- Z ---
			diffRot.z = pKeyNext->fRotZ - pKey->fRotZ;
			RepairRot(&diffRot.z, &diffRot.z);

			diffKeyRotCurrent.z = pKey->fRotZ + diffRot.z * fRateKey;
			RepairRot(&diffKeyRotCurrent.z, &diffKeyRotCurrent.z);

			/* ブレンドモーション */
			diffRot.z = pKeyNextBlend->fRotZ - pKeyBlend->fRotZ;
			RepairRot(&diffRot.z, &diffRot.z);

			diffKeyRotBlend.z = pKeyBlend->fRotZ + diffRot.z * fRateKeyBlend;
			RepairRot(&diffKeyRotBlend.z, &diffKeyRotBlend.z);

			/* 求める差分 */
			diffRotBlend.z = diffKeyRotBlend.z - diffKeyRotCurrent.z;
			UpdateRot.z = diffKeyRotCurrent.z + (diffRotBlend.z * fRateBlend);
		}
		else
		{ // ブレンド無し
			/** キー情報から位置と向きを算出！ **/
			// === POS ===
			// --- X ---
			diffPos.x = pKeyNext->fPosX - pKey->fPosX;
			UpdatePos.x = pKey->fPosX + diffPos.x * fRateKey;

			// --- Y ---
			diffPos.y = pKeyNext->fPosY - pKey->fPosY;
			UpdatePos.y = pKey->fPosY + diffPos.y * fRateKey;

			// --- Z ---
			diffPos.z = pKeyNext->fPosZ - pKey->fPosZ;
			UpdatePos.z = pKey->fPosZ + diffPos.z * fRateKey;

			// === ROT ===
			// --- X ---
			diffRot.x = pKeyNext->fRotX - pKey->fRotX;
			RepairRot(&diffRot.x, &diffRot.x);

			UpdateRot.x = pKey->fRotX + diffRot.x * fRateKey;
			RepairRot(&UpdateRot.x, &UpdateRot.x);

			// --- Y ---
			diffRot.y = pKeyNext->fRotY - pKey->fRotY;
			RepairRot(&diffRot.y, &diffRot.y);

			UpdateRot.y = pKey->fRotY + diffRot.y * fRateKey;
			RepairRot(&UpdateRot.y, &UpdateRot.y);

			// --- Z ---
			diffRot.z = pKeyNext->fRotZ - pKey->fRotZ;
			RepairRot(&diffRot.z, &diffRot.z);

			UpdateRot.z = pKey->fRotZ + diffRot.z * fRateKey;
			RepairRot(&UpdateRot.z, &UpdateRot.z);
		}

		/** 位置、向きを更新！ **/
		pModel->pos = pModel->posLocal + UpdatePos;
		pModel->rot = pModel->rotLocal + UpdateRot;
	}

	if (pPlayer->bBlendMotion == false)
	{
		pPlayer->nCounterMotion++;
		if (pPlayer->nCounterMotion >= pInfo->nFrame)
		{ // モーションカウンターが現在のキー情報のフレーム数を超えた場合
			if (pPlayer->motionType < 0 || pPlayer->motionType >= MOTIONTYPE_MAX)
			{ // モーションインデックスの上下限確認
				return;
			}

			/** キーを一つ進める **/
			pPlayer->nKey = ((pPlayer->nKey + 1) % pPlayer->aMotionInfo[pPlayer->motionType].nNumKey);
			if (pPlayer->nKey == 0 && pPlayer->bLoop == false)
			{
				SetMotionType(MOTIONTYPE_NEUTRAL, true, 120);
			}

			pPlayer->nCounterMotion = 0;
		}
	}
	else
	{
		pPlayer->nCounterMotionBlend++;
		if (pPlayer->nCounterMotionBlend >= pInfoBlend->nFrame)
		{ // モーションカウンターがブレンドモーションの現在のキーのフレーム数を超えた場合
			/** ブレンドモーションのキーを一つ進める **/
			pPlayer->nKeyBlend = ((pPlayer->nKeyBlend + 1) % pPlayer->aMotionInfo[pPlayer->motionTypeBlend].nNumKey);

			pPlayer->nCounterMotionBlend = 0;
		}

		pPlayer->nCounterBlend++;
		if (pPlayer->nCounterBlend >= pPlayer->nFrameBlend)
		{ // ブレンドカウンターがブレンドフレーム数を超えた場合
			SetMotionType(pPlayer->motionTypeBlend, false, 0);
		}
	}
}

//================================================================================================================
// --- Xファイル表示の描画 ---
//================================================================================================================
void DrawPlayer(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL* pMat;					// マテリアルデータへのポインタ
	D3DMATERIAL9 mat;					// マテリアルデータのコピー
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	if (pPlayer->bDisp == true && pPlayer->bUse == true)
	{
		/*** ワールドマトリックスの初期化 ***/
		D3DXMatrixIdentity(&pPlayer->mtxWorld);

		/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
		D3DXMatrixRotationYawPitchRoll(&mtxRot,
			pPlayer->rot.y,			// Y軸回転
			pPlayer->rot.x,			// X軸回転
			pPlayer->rot.z);		// Z軸回転

		D3DXMatrixMultiply(&pPlayer->mtxWorld, &pPlayer->mtxWorld, &mtxRot);

		/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
		D3DXMatrixTranslation(&mtxTrans,
			pPlayer->pos.x,
			pPlayer->pos.y,
			pPlayer->pos.z);

		D3DXMatrixMultiply(&pPlayer->mtxWorld, &pPlayer->mtxWorld, &mtxTrans);

		/*** ワールドマトリックスの設定 ***/
		pDevice->SetTransform(D3DTS_WORLD, &pPlayer->mtxWorld);

		/*** 現在のマテリアルを保存 ***/
		pDevice->GetMaterial(&matDef);

		/*** 全パーツの描画 ***/
		for (int nCntModel = 0; nCntModel < pPlayer->nNumModel; nCntModel++)
		{
			D3DXMATRIX mtxRotModel, mtxTransModel;		// 計算用マトリックス
			D3DXMATRIX mtxParent;						// 親のマトリックス
			LPOBJECTINFO pObj = GetObjectInfo(pPlayer->aModel[nCntModel].nIdxObject);

			if (pObj != NULL)
			{
				/*** パーツのワールドマトリックスを初期化 ***/
				D3DXMatrixIdentity(&pPlayer->aModel[nCntModel].mtxWorld);

				/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
				D3DXMatrixRotationYawPitchRoll(&mtxRotModel,
					pPlayer->aModel[nCntModel].rot.y,		// Y軸回転
					pPlayer->aModel[nCntModel].rot.x,		// X軸回転
					pPlayer->aModel[nCntModel].rot.z);		// Z軸回転

				D3DXMatrixMultiply(&pPlayer->aModel[nCntModel].mtxWorld,
					&pPlayer->aModel[nCntModel].mtxWorld,
					&mtxRotModel);

				/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
				D3DXMatrixTranslation(&mtxTransModel,
					pPlayer->aModel[nCntModel].pos.x,
					pPlayer->aModel[nCntModel].pos.y,
					pPlayer->aModel[nCntModel].pos.z);

				D3DXMatrixMultiply(&pPlayer->aModel[nCntModel].mtxWorld,
					&pPlayer->aModel[nCntModel].mtxWorld,
					&mtxTransModel);

				/*** パーツの「親のマトリックス」を設定！ ***/
				if (pPlayer->aModel[nCntModel].nIdxModelParent != -1)
				{ // 親パーツが存在する場合
					mtxParent = pPlayer->aModel[pPlayer->aModel[nCntModel].nIdxModelParent].mtxWorld;
				}
				else
				{ // 自身が親パーツの場合
					mtxParent = pPlayer->mtxWorld;		// プレイヤーのマトリックスを指定
				}

				/*** 算出された「パーツのマトリックス」と「親のマトリックス」を掛け合わせる！ ***/
				D3DXMatrixMultiply(&pPlayer->aModel[nCntModel].mtxWorld,
					&pPlayer->aModel[nCntModel].mtxWorld,
					&mtxParent);

				/*** パーツのワールドマトリックスの設定！ ***/
				pDevice->SetTransform(D3DTS_WORLD, &pPlayer->aModel[nCntModel].mtxWorld);

				/*** マテリアルデータへのポインタを取得 ***/
				pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();

				for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
				{
					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

					/*** テクスチャの設定 ***/
					pDevice->SetTexture(0, pObj->apTexture[nCntMat]);

					/*** モデル(パーツ)の描画 ***/
					pObj->pMesh->DrawSubset(nCntMat);
				}
			}
		}

		/*** 保存していたマテリアルを戻す！ ***/
		pDevice->SetMaterial(&matDef);
	}
}

//================================================================================================================
// --- パーツ情報の読み込み！及び設定 ---
//================================================================================================================
void LoadPartsPlayer(PartsInfo pInfo)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	for (int nCntMotion = 0; nCntMotion < 20; nCntMotion++)
	{
		if (pPlayer->aModel[nCntMotion].bUse == false)
		{
			/*** 各パーツの階層構造設定 ***/
			pPlayer->aModel[nCntMotion] = pInfo;

			/** オフセットの位置をローカルで保存！ **/
			pPlayer->aModel[nCntMotion].posLocal = pInfo.pos;
			pPlayer->aModel[nCntMotion].rotLocal = pInfo.rot;
			pPlayer->aModel[nCntMotion].bUse = true;
			pPlayer->aModel[nCntMotion].nIdxCollision = -1;

			break;
		}
	}
}

//================================================================================================================
// --- モーション情報の読み込み！及び設定 ---
//================================================================================================================
void LoadMotionPlayer(MOTION_INFO *mInfo, MOTIONTYPE nIdxMotion)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	if (mInfo == NULL)
	{ // NULLチェック
		pPlayer->bUse = false;
		return;
	}

	/*** モーション情報を設定 ***/
	pPlayer->aMotionInfo[nIdxMotion] = *mInfo;
}

//================================================================================================================
// --- モーション総数の読み込み！及び設定 ---
//================================================================================================================
void SetMotionNum(int nMotionNum)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	/*** モーションの総数を設定 ***/
	pPlayer->nNumMotion = nMotionNum;
}

//================================================================================================================
// --- モーションの変更！ ---
//================================================================================================================
void SetMotionType(MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	if (motionTypeNext < 0 || motionTypeNext >= MOTIONTYPE_MAX)
	{ // モーションインデックスの上下限確認
		return;
	}

	pPlayer->bBlendMotion = bBlend;
	
	if (bBlend == false)
	{
		/*** 各変数を初期化し、次のモーションを設定 ***/
		pPlayer->nCounterMotion = pPlayer->nCounterMotionBlend;
		pPlayer->nKey = pPlayer->nKeyBlend;
		pPlayer->motionType = motionTypeNext;
		pPlayer->bLoop = pPlayer->aMotionInfo[motionTypeNext].bLoop;
		pPlayer->nNumKey = pPlayer->aMotionInfo[motionTypeNext].nNumKey;
		pPlayer->bFinishMotion = false;

		/*** 現在のモーションのキー情報へのポインタ ***/
		KEY_INFO* pInfo = &pPlayer->aMotionInfo[pPlayer->motionType].aKeyInfo[pPlayer->nKey];

		/*** 全パーツの初期設定！ ***/
		for (int nCntModel = 0; nCntModel < pPlayer->nNumModel; nCntModel++)
		{
			int nNext = (pPlayer->nKey + 1) % pPlayer->aMotionInfo[pPlayer->motionType].nNumKey;		// 次のキーの値
			float fRateKey = (float)pPlayer->nCounterMotion / (float)pInfo->nFrame;			// モーションカウンター / 再生フレーム数
			D3DXVECTOR3 diffPos = {};	// 位置の差分
			D3DXVECTOR3 UpdatePos = {};	// 更新する位置
			D3DXVECTOR3 diffRot = {};	// 角度の差分
			D3DXVECTOR3 UpdateRot = {};	// 更新する角度
			KEY *pKey = &pInfo->aKey[nCntModel];		// 現在のキー
			PartsInfo *pModel = &pPlayer->aModel[nCntModel];
			KEY *pKeyNext = &pPlayer->aMotionInfo[pPlayer->motionType].aKeyInfo[nNext].aKey[nCntModel];			// 次のキー

			/** キー情報から位置と向きを算出！ **/
			// === POS ===
			// --- X ---
			diffPos.x = pKeyNext->fPosX - pKey->fPosX;
			UpdatePos.x = pKey->fPosX + diffPos.x * fRateKey;

			// --- Y ---
			diffPos.y = pKeyNext->fPosY - pKey->fPosY;
			UpdatePos.y = pKey->fPosY + diffPos.y * fRateKey;

			// --- Z ---
			diffPos.z = pKeyNext->fPosZ - pKey->fPosZ;
			UpdatePos.z = pKey->fPosZ + diffPos.z * fRateKey;

			// === ROT ===
			// --- X ---
			diffRot.x = pKeyNext->fRotX - pKey->fRotX;
			RepairRot(&diffRot.x, &diffRot.x);

			UpdateRot.x = pKey->fRotX + diffRot.x * fRateKey;
			RepairRot(&UpdateRot.x, &UpdateRot.x);

			// --- Y ---
			diffRot.y = pKeyNext->fRotY - pKey->fRotY;
			RepairRot(&diffRot.y, &diffRot.y);

			UpdateRot.y = pKey->fRotY + diffRot.y * fRateKey;
			RepairRot(&UpdateRot.y, &UpdateRot.y);

			// --- Z ---
			diffRot.z = pKeyNext->fRotZ - pKey->fRotZ;
			RepairRot(&diffRot.z, &diffRot.z);

			UpdateRot.z = pKey->fRotZ + diffRot.z * fRateKey;
			RepairRot(&UpdateRot.z, &UpdateRot.z);

			/** 位置、向きを更新！ **/
			pModel->pos = pModel->posLocal + UpdatePos;
			pModel->rot = pModel->rotLocal + UpdateRot;
		}
	}
	else
	{
		/*** 各変数を初期化し、ブレンドモーションを設定 ***/
		pPlayer->bFinishMotion = false;
		pPlayer->motionTypeBlend = motionTypeNext;
		pPlayer->nCounterBlend = 0;
		pPlayer->nCounterMotionBlend = 0;
		pPlayer->nFrameBlend = nFrameBlend;
		pPlayer->nNumKeyBlend = pPlayer->aMotionInfo[motionTypeNext].nNumKey;
		pPlayer->nKeyBlend = 0;
	}
}

//================================================================================================================
// --- プレイターの設置 ---
//================================================================================================================
void SetPlayer(D3DXVECTOR3 pos, D3DXVECTOR3 rot, const char* pMotionName)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	/*** プレイヤーの情報を適用 ***/
	pPlayer->pos = pos;
	pPlayer->rot = rot;
	g_fAnglePlayer = 0.0f;
	pPlayer->bDisp = true;
	pPlayer->bUse = true;
}

//================================================================================================================
// --- モデル数の設定 ---
//================================================================================================================
void SetMotionPartsNum(int nNumParts)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	pPlayer->nNumModel = nNumParts;
}

//================================================================================================================
// --- 当たり判定の設定 ---
//================================================================================================================
void SetCollisionSize(float fRadius, float fHeight)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	pPlayer->fRadius = fRadius;
	pPlayer->fHeight = fHeight;
}

//================================================================================================================
// --- プレイヤーの設定 ---
//================================================================================================================
void SetPlayerSetting(float fMove, float fJump)
{
	Player *pPlayer = &g_player;		// プレイヤー情報へのアドレス

	pPlayer->fSpd = fMove;
	pPlayer->fJump = fJump;
}

//================================================================================================================
// --- モデルの取得 ---
//================================================================================================================
Player* GetPlayer(void)
{
	return &g_player;
}
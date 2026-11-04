//==================================================================================
//
// タイトル画面のプレイヤー(クマ)の処理 [playerTitle.cpp]
// Author : TENMA SAITO
//
// 決定入力(PressEnter 演出の表示後)までは着地モーションの2キー目で静止し、
// 決定されたらモーションを再生し始める。
// モーションの再生と描画は characterMotion.cpp の共通処理を使う。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "playerTitle.h"
#include "characterMotion.h"
#include "cameraTitle.h"
#include "pressEnter.h"
#include "fade.h"
#include "shadow.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TITLE_POSE_KEY		(1)			// 決定前に静止させる着地モーションのキー

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static PlayerTitle g_playerTitle;			// タイトル用プレイヤーの情報
static bool g_bUpdateMotionPlayer;			// モーションを再生するか(決定後 true)

//==================================================================================
// --- タイトル用プレイヤーの初期化処理 ---
//==================================================================================
void InitPlayerTitle(void)
{
	PlayerTitle *pPlayerTitle = &g_playerTitle;		// タイトル用プレイヤー

	ZeroMemory(pPlayerTitle, sizeof(PlayerTitle));

	pPlayerTitle->nIdShadow = -1;
	pPlayerTitle->motionType = MOTIONTYPE_NEUTRAL;
	pPlayerTitle->nCounterMotion = 0;
	pPlayerTitle->nKey = 0;
	pPlayerTitle->nIdx2DPolygon = -1;
	g_bUpdateMotionPlayer = false;
}

//==================================================================================
// --- タイトル用プレイヤーの終了処理 ---
//==================================================================================
void UninitPlayerTitle(void)
{

}

//==================================================================================
// --- タイトル用プレイヤーの更新処理 ---
//==================================================================================
void UpdatePlayerTitle(void)
{
	PlayerTitle *pPlayerTitle = &g_playerTitle;		// タイトル用プレイヤー

	pPlayerTitle->posOld = pPlayerTitle->pos;

	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE
		&& GetEndPressEnterEffect())
	{ // 決定されたらモーションの再生を始める
		g_bUpdateMotionPlayer = true;
	}

	if (g_bUpdateMotionPlayer == true)
	{
		UpdateCharacterMotion(pPlayerTitle);
	}

	if (pPlayerTitle->motionType != MOTIONTYPE_LANDING && g_bUpdateMotionPlayer == false)
	{ // 決定前 : 着地モーションの途中の姿勢で静止させる
		SetCharacterMotionType(pPlayerTitle, MOTIONTYPE_LANDING, false, 0);
		pPlayerTitle->nKey = TITLE_POSE_KEY;
		UpdateCharacterMotion(pPlayerTitle);
	}
}

//==================================================================================
// --- タイトル用プレイヤーの描画処理 ---
//==================================================================================
void DrawPlayerTitle(void)
{
	DrawCharacter(&g_playerTitle);
}

//==================================================================================
// --- パーツの登録 (スクリプトから) ---
//==================================================================================
void LoadPartsPlayerTitle(PartsInfo pInfo)
{
	LoadCharacterParts(&g_playerTitle, pInfo);
}

//==================================================================================
// --- モーションの登録 (スクリプトから) ---
//==================================================================================
void LoadMotionPlayerTitle(MOTION_INFO *mInfo, MOTIONTYPE nIdxMotion)
{
	LoadCharacterMotion(&g_playerTitle, mInfo, nIdxMotion);
}

//==================================================================================
// --- モーション総数の設定 ---
//==================================================================================
void SetMotionNumPlayerTitle(int nMotionNum)
{
	g_playerTitle.nNumMotion = nMotionNum;
}

//==================================================================================
// --- タイトル用プレイヤーの設置 ---
//==================================================================================
void SetPlayerTitle(D3DXVECTOR3 pos, D3DXVECTOR3 rot, const char *pMotionName)
{
	PlayerTitle *pPlayerTitle = &g_playerTitle;		// タイトル用プレイヤー

	pPlayerTitle->pos = pos;
	pPlayerTitle->rot = rot;
	pPlayerTitle->bDisp = true;
	pPlayerTitle->bUse = true;
}

//==================================================================================
// --- パーツ数の設定 ---
//==================================================================================
void SetMotionPartsNumPlayerTitle(int nNumParts)
{
	g_playerTitle.nNumModel = nNumParts;
}

//==================================================================================
// --- タイトル用プレイヤーの取得 ---
//==================================================================================
PlayerTitle *GetPlayerTitle(void)
{
	return &g_playerTitle;
}

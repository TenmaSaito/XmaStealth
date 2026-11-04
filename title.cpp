//==================================================================================
//
// タイトル画面の処理 [title.cpp]
// Author : TENMA SAITO
//
// 決定入力 1回目 : PressEnter 演出を表示
// 決定入力 2回目 : (演出のフェードイン後) 一定時間待ってチュートリアル画面へ
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "title.h"
#include "fade.h"
#include "titleLogo.h"
#include "pressEnter.h"
#include "object.h"
#include "loadxfile.h"
#include "effect.h"
#include "field.h"
#include "light.h"
#include "shadow.h"
#include "wall.h"
#include "billboard.h"
#include "mesh.h"
#include "cameraTitle.h"
#include "playerTitle.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TITLE_SCRIPT			"data\\SCRIPT\\modelTitle.txt"		// 配置スクリプト
#define TITLE_MOTION			"data\\SCRIPT\\motion_bear.txt"	// プレイヤー(クマ)のモーション
#define TITLE_AUTO_FRAME		(4000)		// 放置で自動的に遷移し始めるフレーム
#define TITLE_DECIDE_WAIT		(200)		// 決定してから遷移を始めるまでの待ち時間(フレーム)

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static int g_nCounterTitle;			// タイトル画面の経過フレーム(PressEnter表示でリセット)
static int g_nCounterFadeTitle;		// 決定してからの経過フレーム
static bool g_bCountFadeTitle;		// 決定済みで遷移待ちか

//==================================================================================
// --- タイトル画面の初期化 ---
//==================================================================================
void InitTitle(void)
{
	g_nCounterTitle = 0;
	g_nCounterFadeTitle = 0;
	g_bCountFadeTitle = false;

	/*** 各要素の初期化 ***/
	InitCameraTitle();
	InitTitleLogo();
	InitPressEnter();

#ifdef DIRECTIONAL_LIGHT
	InitLight();
#elif SPOT_LIGHT // DIRECTIONAL_LIGHT
	InitSpotLight();
#endif

	InitField();
	InitWall();
	InitShadow();
	InitObject();
	InitMeshfield();
	InitMeshCylinder();
	InitMeshDome();
	InitMeshSphere();
	InitPlayerTitle();

	/*** スクリプトの読み込み ***/
	if (FAILED(InitScript(TITLE_SCRIPT)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return;
	}

	if (FAILED(LoadMotion(TITLE_MOTION)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "LoadMotionに失敗しました。");
		return;
	}

	/*** プレイヤー(クマ)とカメラの配置 ***/
	SetPlayerTitle(D3DXVECTOR3(-25.0f, 0.0f, 275.0f), VEC_Y(45.0f), "");
	SetCameraTitle(D3DXVECTOR3(-60.0f, 10.0f, 200.0f), D3DXVECTOR3(-60.0f, 20.0f, 300.0f), 45.0f, 1, 10000, 100.0f, 0.0f, 0.0f);
}

//==================================================================================
// --- タイトル画面の終了 ---
//==================================================================================
void UninitTitle(void)
{
	UninitPressEnter();
	UninitTitleLogo();
	UninitField();
	UninitWall();
	UninitObject();
	UninitShadow();
	UninitScript();
	UninitMeshfield();
	UninitMeshCylinder();
	UninitMeshDome();
	UninitMeshSphere();
	UninitPlayerTitle();
	UninitLight();
	UninitCameraTitle();
}

//==================================================================================
// --- タイトル画面の更新 ---
//==================================================================================
void UpdateTitle(void)
{
	if (GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
	{ // 決定入力
		if (GetPressEnterEffect() == false)
		{ // 1回目 : PressEnter 演出を表示
			SetPressEnterEffect(PRESSENTER_POS);
			g_nCounterTitle = 0;
		}
		else if (GetFade() == FADE_NONE
			&& GetEndPressEnterEffect()
			&& g_bCountFadeTitle == false)
		{ // 2回目 : 決定音を鳴らして遷移待ちへ
			PlaySound(SOUND_LABEL_SE_ENTER);
			g_bCountFadeTitle = true;
		}
	}

	/*** 各要素の更新 ***/
	UpdateLight();
	UpdateCameraTitle();
	UpdatePlayerTitle();
	UpdateTitleLogo();
	UpdatePressEnter();
	UpdateField();
	UpdateWall();
	UpdateObject();
	UpdateShadow();
	UpdateMeshfield();
	UpdateMeshCylinder();
	UpdateMeshDome();
	UpdateMeshSphere();

	if (g_nCounterTitle >= TITLE_AUTO_FRAME && GetFade() == FADE_NONE)
	{ // 放置で自動的にチュートリアル画面へ
		SetFade(MODE_TUTORIAL, FADE_TYPE_NORMAL);
		FadeSound(SOUND_LABEL_BGM_RANKING);
	}

	g_nCounterTitle++;

	if (g_bCountFadeTitle == true)
	{ // 決定後、一定時間待ってチュートリアル画面へ
		g_nCounterFadeTitle++;
		if (g_nCounterFadeTitle >= TITLE_DECIDE_WAIT && GetFade() == FADE_NONE)
		{
			SetFade(MODE_TUTORIAL, FADE_TYPE_NORMAL);
			FadeSound(SOUND_LABEL_BGM_TUTORIAL);
			g_bCountFadeTitle = false;
		}
	}
}

//==================================================================================
// --- タイトル画面の描画 ---
//==================================================================================
void DrawTitle(void)
{
	SetCameraTitle();

	/*** 3Dの描画 ***/
	DrawField();
	DrawWall();
	DrawObject();
	DrawPlayerTitle();
	DrawMeshfield();
	DrawShadow();
	DrawMeshCylinder();
	DrawMeshDome();
	DrawMeshSphere();

	/*** 2Dの描画 (3Dより手前に表示するため最後に描画) ***/
	DrawTitleLogo();
	DrawPressEnter();
}

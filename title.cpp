//================================================================================================================
//
// DirectXのタイトル関連用のcppファイル [title.cpp]
// Author : TENMA
//
//================================================================================================================
//*************************************************************************************************
//*** インクルードファイル ***
//*************************************************************************************************
#include "title.h"
#include "fade.h"
#include "titleLogo.h"
#include "pressEnter.h"
#include "object.h"
#include "loadxfile.h"
#include "effect.h"
#include "field.h"
#include "light.h"
#include "object.h"
#include "loadxfile.h"
#include "shadow.h"
#include "wall.h"
#include "billboard.h"
#include "bullet.h"
#include "explosion.h"
#include "tree.h"
#include "mesh.h"
#include "cameraTitle.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
int g_nCounterTitle;

//=================================================================================================
// --- タイトル関連cppファイルの初期化 ---
//=================================================================================================
void InitTitle(void)
{
	g_nCounterTitle = 0;

	/*** Aの初期化 ***/
	InitCameraTitle();

	InitLight();

	/*** ロゴの初期化 ***/
	InitTitleLogo();

	/*** PressEnter演出の初期化 ***/
	InitPressEnter();

#ifdef DIRECTIONAL_LIGHT
	InitLight();
#elif SPOT_LIGHT // DIRECTIONAL_LIGHT
	InitSpotLight();
#elif POINT_LIGHT

#endif
	InitField();

	InitWall();

	InitShadow();

	InitObject();

	InitMeshfield();

	InitMeshCylinder();

	InitMeshDome();

	InitMeshSphere();

	if (FAILED(InitScript("data\\SCRIPT\\modelTitle.txt")))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return;
	}

	SetCameraTitle(D3DXVECTOR3(0.0f, 150.0f, 0.0f), D3DXVECTOR3(0.0f, 120.0f, 300.0f), 45.0f, 1, 10000, 300.0f, 1.0f, 0.02f);
}

//=================================================================================================
// --- タイトル関連cppファイルの終了 ---
//=================================================================================================
void UninitTitle(void)
{
	/*** Aの終了 ***/

	/*** PressEnter演出の終了 ***/
	UninitPressEnter();

	/*** ロゴの終了 ***/
	UninitTitleLogo();

	UninitField();

	UninitWall();

	UninitObject();

	UninitShadow();

	UninitScript();

	UninitLight();

	UninitMeshfield();

	UninitMeshCylinder();

	UninitMeshDome();

	UninitMeshSphere();

	UninitLight();

	UninitCameraTitle();
}

//=================================================================================================
// --- タイトル関連cppファイルの更新 ---
//=================================================================================================
void UpdateTitle(void)
{
	/*** Aの更新 ***/

	UpdateLight();

	UpdateCameraTitle();

	/*** ロゴの更新 ***/
	UpdateTitleLogo();

	/*** PressEnter演出の更新 ***/
	UpdatePressEnter();

	/*** フィールドの更新 ***/
	UpdateField();

	/*** 壁の更新 ***/
	UpdateWall();

	/*** オブジェクトの更新 ***/
	UpdateObject();

	/*** 影の更新 ***/
	UpdateShadow();

	/*** メッシュフィールドの更新 ***/
	UpdateMeshfield();

	UpdateMeshCylinder();

	UpdateMeshDome();

	UpdateMeshSphere();

	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() != FADE_NONE
		&& GetPressEnterEffect() == false)
	{
		SetPressEnterEffect(PRESSENTER_POS);
	}

	/*** 決定ボタンを押したとき ***/
	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE
		&& GetEndPressEnterEffect())
	{ // ゲーム画面へフェード
		/*** 決定音を鳴らす ***/
		PlaySound(SOUND_LABEL_SE_ENTER);
		SetFade(MODE_GAME, FADE_TYPE_NORMAL);

		/*** ゲームBGMをフェードイン ***/
		FadeSound(SOUND_LABEL_BGM_GAME);
	}

	if (g_nCounterTitle >= 1000 && GetFade() == FADE_NONE)
	{
		SetFade(MODE_RANKING, FADE_TYPE_NORMAL);

		/*** ゲームBGMをフェードイン ***/
		FadeSound(SOUND_LABEL_BGM_RESULT);
	}

	g_nCounterTitle++;
}

//=================================================================================================
// --- タイトル関連cppファイルの描画 ---
//=================================================================================================
void DrawTitle(void)
{
	/*** Aの描画 ***/
	SetCameraTitle();

	// VERTEX_3D ============================================
	/*** フィールドの描画 ***/
	DrawField();

	/*** 壁の描画 ***/
	DrawWall();

	/*** オブジェクトの描画 ***/
	DrawObject();

	/*** メッシュフィールドの描画 ***/
	DrawMeshfield();

	/*** 影の描画 ***/
	DrawShadow();

	DrawMeshCylinder();

	DrawMeshDome();

	DrawMeshSphere();

	// VERTEX_2D ============================================
	/*** ロゴの描画 ***/
	 DrawTitleLogo();

	/*** PressEnter演出の描画 ***/
	 DrawPressEnter();
}
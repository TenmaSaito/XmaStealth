//================================================================================================================
//
// DirectXのチュートリアル用のcppファイル [tutorial.cpp]
// Author : TENMA
//
//================================================================================================================
//*************************************************************************************************
//*** インクルードファイル ***
//*************************************************************************************************
#include "tutorial.h"
#include "light.h"
#include "fade.h"
#include "camera.h"
#include "field.h"
#include "object.h"
#include "mesh.h"
#include "ornament.h"
#include "XmasTree.h"
#include "shadow.h"
#include "effect.h"
#include "player.h"
#include "billboard.h"
#include "2Dpolygon.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
int g_nCounterTutorial;

//=================================================================================================
// --- チュートリアル関連cppファイルの初期化 ---
//=================================================================================================
void InitTutorial(void)
{
	g_nCounterTutorial = 0;

	/*** aの初期化 ***/

	/*** チュートリアルの初期化 ***/

	/*** 照明の初期化 ***/
	InitLight();

	/*** カメラの初期化 ***/
	InitCamera();

	InitField();

	InitObject();

	InitShadow();

	InitPlayer();

	InitEffect();

	InitXmasTree();

	InitMeshSphere();

	InitOrnament();

	InitMeshCylinder();

	InitMeshRing();

	InitBillBoard();

	Init2DPolygon();

	if (FAILED(InitScript("data//SCRIPT//modelTutorial.txt")))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return;
	}

	if (FAILED(LoadMotion("data//SCRIPT//motion_bear.txt")))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "LoadMotionに失敗しました。");
		return;
	}

	SetIndexTextureRing(5);

	SetOrnamentRand(200);

	SetBillBoard(D3DXVECTOR3(0.0f, 40.0f, 50.0f), 15.0f, 40.0f, 3);

	SetMeshCylinder(VEC_Y(-15000.0f), VECNULL, 30000.0f, 15000.0f, 32, 1, CULLINGTYPE_NONE, 1, false);

	SetPlayer(VECNULL, VECNULL, "data//SCRIPT//motion_bear.txt");
}

//=================================================================================================
// --- チュートリアル関連cppファイルの終了 ---
//=================================================================================================
void UninitTutorial(void)
{
	/*** aの終了 ***/

	UninitLight();

	UninitMeshCylinder();

	UninitField();

	UninitObject();

	UninitShadow();

	UninitPlayer();

	UninitEffect();

	UninitXmasTree();

	UninitMeshSphere();

	UninitOrnament();
	
	UninitMeshRing();
	
	UninitBillBoard();

	Uninit2DPolygon();
}

//=================================================================================================
// --- チュートリアル関連cppファイルの更新 ---
//=================================================================================================
void UpdateTutorial(void)
{
	/*** aの更新 ***/
	UpdateCamera();

	UpdateLight();

	UpdateField();

	UpdateMeshCylinder();

	UpdateObject();

	UpdatePlayer();

	UpdateShadow();

	UpdateEffect();

	UpdateXmasTree();

	UpdateMeshSphere();

	UpdateOrnament();
	
	UpdateMeshRing();
	
	UpdateBillBoard();

	Update2DPolygon();

	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE)
	{
		SetFade(MODE_GAME, FADE_TYPE_NORMAL, 120);

		/*** タイトルBGMをフェードイン ***/
		FadeSound(SOUND_LABEL_BGM_TITLE);
	}

	g_nCounterTutorial++;
}

//=================================================================================================
// --- チュートリアル関連cppファイルの描画 ---
//=================================================================================================
void DrawTutorial(void)
{
	/*** aの描画 ***/
	SetCamera();

	// VERTEX_3D ============================================
	DrawMeshCylinder();

	DrawField();

	DrawObject();

	DrawPlayer();

	DrawXmasTree();

	DrawMeshSphere();
	
	DrawMeshRing();

	DrawOrnament();
	
	DrawBillBoard();

	DrawShadow();

	DrawEffect();

	// VERTEX_2D ============================================
	Draw2DPolygon();
}
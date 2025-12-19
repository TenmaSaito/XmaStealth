//================================================================================================================
//
// DirectXのゲーム関連用のcppファイル [game.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "field.h"
#include "light.h"
#include "object.h"
#include "player.h"
#include "enemy.h"
#include "loadxfile.h"
#include "shadow.h"
#include "wall.h"
#include "billboard.h"
#include "bullet.h"
#include "effect.h"
#include "explosion.h"
#include "tree.h"
#include "line.h"
#include "fade.h"
#include "mesh.h"
#include "ornament.h"
#include "stair.h"
#include "spotlight.h"
#include "pause.h"
#include "XmasTree.h"
#include "timer.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
GAMESTATE g_gameState;			// 現在のゲームの状態
bool g_bPause;					// ポーズ状態
int g_nCounterStateGame;		// ゲーム状態カウンター
int g_nCounterGame;				// タイマー減少用カウンター

//================================================================================================================
// --- ゲーム関連cppファイルの初期化 ---
//================================================================================================================
HRESULT InitGame(void)
{
	/*** 変数の初期化 ***/
	g_bPause = false;

	InitCamera();

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

	InitPlayer();

	InitEnemy();

	InitEffect();

	InitExplosion();

	InitMeshfield();

	InitMeshCylinder();

	InitMeshDome();

	InitMeshSphere();

	InitMeshRing();

	InitOrnament();

	InitPause();

	InitXmasTree();

	InitTimer();

	if (FAILED(InitScript("data\\SCRIPT\\model.txt")))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return E_FAIL;
	}

	if (FAILED(LoadMotion("data\\SCRIPT\\motion_bear.txt")))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "LoadMotionに失敗しました。");
		return E_FAIL;
	}

	SetIndexTextureRing(8);

	g_nCounterStateGame = 0;
	g_nCounterGame = 0;
	g_gameState = GAMESTATE_NORMAL;
	SetTimer(120);

	SetOrnamentRand(200);

	SetPlayer(VECNULL, VECNULL, "");

	return S_OK;
}

//================================================================================================================
// --- ゲーム関連cppファイルの終了 ---
//================================================================================================================
void UninitGame(void)
{
	UninitCamera();

	ResetCamera();

	UninitField();

	UninitWall();

	UninitObject();

	UninitPlayer();

	UninitEnemy();

	UninitEffect();

	UninitExplosion();

	UninitShadow();

	UninitScript();

	UninitLight();

	UninitMeshfield();

	UninitMeshCylinder();

	UninitMeshDome();

	UninitMeshSphere();

	UninitMeshRing();

	UninitOrnament();

	UninitPause();

	UninitXmasTree();

	UninitTimer();

	SetVibration(0, 0, 0);
}

//================================================================================================================
// --- ゲーム関連cppファイルの更新 ---
//================================================================================================================
void UpdateGame(void)
{
	/*** ゲームの状態により処理を変更 ***/
	switch (g_gameState)
	{
		 // 何もなし
	case GAMESTATE_NONE:

		break;

		 // 通常状態
	case GAMESTATE_NORMAL:

		break;

		// ゲームクリア
	case GAMESTATE_GOODEND:

		g_nCounterStateGame--;
		if (g_nCounterStateGame <= 0)
		{
			if (GetFade() == FADE_NONE)
			{ // リザルト画面へ移行
				SetFade(MODE_GOODEND, FADE_TYPE_NORMAL);
				FadeSound(SOUND_LABEL_BGM_RESULT);
				g_gameState = GAMESTATE_NONE;
			}
		}

		break;

		// ゲームオーバー
	case GAMESTATE_BADEND:

		g_nCounterStateGame--;
		if (g_nCounterStateGame <= 0)
		{
			if (GetFade() == FADE_NONE)
			{ // リザルト画面へ移行
				SetFade(MODE_BADEND, FADE_TYPE_NORMAL);
				FadeSound(SOUND_LABEL_BGM_RESULT);
				g_gameState = GAMESTATE_NONE;
			}
		}

		break;
	}

	/*** ゲーム状態が通常なら ***/
	if (g_gameState == GAMESTATE_NORMAL)
	{
		/*** ポーズボタンを押したとき ***/
		if (GetKeyboardTrigger(DIK_P)
			|| GetJoypadTrigger(JOYKEY_START))
		{
			g_bPause = g_bPause ^ true;
		}
	}
	
	if (GetKeyboardTrigger(DIK_DELETE) && GetFade() == FADE_NONE)
	{
		SetFade(MODE_GAME, FADE_TYPE_NORMAL);
	}

	/*** ポーズ状態でなければ更新 ***/
	if (g_bPause == false)
	{
		UpdateCamera();

		g_nCounterGame++;
		if (g_nCounterGame % 60 == 0)
		{
			/*** タイマーの減少 ***/
			AddTimer(-1);
		}

		/*** タイマーの更新 ***/
		UpdateTimer();

		/*** フィールドの更新 ***/
		UpdateField();

		/*** 壁の更新 ***/
		UpdateWall();

		/*** プレイヤーの更新 ***/
		UpdatePlayer();

		/*** オブジェクトの更新 ***/
		UpdateObject();

		/*** 敵の更新 ***/
		UpdateEnemy();

		/*** エフェクトの更新 ***/
		UpdateEffect();

		/*** 爆発の更新 ***/
		UpdateExplosion();

		/*** 影の更新 ***/
		UpdateShadow();

		UpdateOrnament();

		UpdateMeshSphere();

		UpdateXmasTree();

		/*** メッシュフィールドの更新 ***/
		UpdateMeshfield();

		UpdateMeshCylinder();

		UpdateMeshDome();

		UpdateMeshRing();
	}
	
	UpdateLight();

	UpdatePause();

#if ENABLE_LOOP == true
	if (GetKeyboardTrigger(DIK_RETURN)
		&& GetFade() == FADE_NONE)
	{
		SetFade(MODE_RESULT, FADE_TYPE_NORMAL);
	}
#endif
}

//================================================================================================================
// --- ゲーム関連cppファイルの描画 ---
//================================================================================================================
void DrawGame(void)
{
	SetCamera();

	// VERTEX_3D ============================================
	/*** フィールドの描画 ***/
	DrawField();

	/*** 壁の描画 ***/
	DrawWall();

	/*** プレイヤーの描画 ***/
	DrawPlayer();

	/*** 敵の描画 ***/
	DrawEnemy();

	/*** オブジェクトの描画 ***/
	DrawObject();

	/*** 爆発の描画 ***/
	DrawExplosion();

	/*** メッシュフィールドの描画 ***/
	DrawMeshfield();

	/*** 影の描画 ***/
	DrawShadow();

	DrawOrnament();

	DrawXmasTree();

	DrawMeshCylinder();

	DrawMeshDome();

	DrawMeshSphere();

	DrawMeshRing();

	/*** エフェクトの描画 ***/
	DrawEffect();

	// VERTEX_2D ============================================
	/*** タイマーの描画 ***/
	DrawTimer();

	if (g_bPause == true)
	{
		/*** ポーズ画面の描画 ***/
		DrawPause();
	}
}

//================================================================================================================
// --- ゲーム状態の設定処理 ---
//================================================================================================================
void SetEnablePause(bool bPause)
{
	g_bPause = bPause;
}

//================================================================================================================
// --- ゲーム状態の取得処理 ---
//================================================================================================================
bool GetEnablePause(void)
{
	return g_bPause;
}

//================================================================================================================
// --- ゲーム状態の設定処理 ---
//================================================================================================================
void SetGameState(GAMESTATE state, int nCounter)
{
	if (g_gameState == GAMESTATE_NORMAL)
	{
		/*** ゲーム状態を設定 ***/
		g_gameState = state;

		/*** その状態の維持時間を設定 ***/
		g_nCounterStateGame = nCounter;
	}
}
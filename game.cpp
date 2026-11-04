//==================================================================================
//
// ゲーム画面の処理 [game.cpp]
// Author : TENMA SAITO
//
// 制限時間内にオーナメントをツリーに付けて、倒さずに飾り切るとクリア。
// ゲームの状態 : NORMAL(プレイ中) → GOODEND / BADEND(一定時間後にリザルトへ) → NONE
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "camera.h"
#include "field.h"
#include "light.h"
#include "object.h"
#include "player.h"
#include "loadxfile.h"
#include "shadow.h"
#include "wall.h"
#include "billboard.h"
#include "effect.h"
#include "fade.h"
#include "mesh.h"
#include "ornament.h"
#include "spotlight.h"
#include "pause.h"
#include "XmasTree.h"
#include "timer.h"
#include "2Dpolygon.h"
#include "score.h"
#include "bearState.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define GAME_SCRIPT				"data\\SCRIPT\\model.txt"			// 配置スクリプト
#define GAME_MOTION				"data\\SCRIPT\\motion_bear.txt"	// プレイヤー(クマ)のモーション
#define GAME_TIME_LIMIT			(120)		// 制限時間(秒)
#define GAME_FRAME_PER_SECOND	(60)		// 1秒あたりのフレーム数
#define GAME_ORNAMENT_NUM		(200)		// ランダムに配置するオーナメントの数

// --- スコアの桁数 ---
#define DIGIT_TIMER				(3)			// 制限時間の桁数
#define DIGIT_ORNAMENT			(3)			// オーナメント数の桁数
#define DIGIT_SCORE				(6)			// スコアの桁数

// --- model.txt 内のテクスチャ番号 ---
#define TEXIDX_SKY				(7)			// 背景の円柱(空)
#define TEXIDX_RING				(8)			// ツリーの周りのリング
#define TEXIDX_LABEL_ORNAMENT	(9)			// 「オーナメント数」見出し
#define TEXIDX_SCORE_NUMBER		(12)		// スコアの数字
#define TEXIDX_LABEL_SCORE		(13)		// 「スコア」見出し
#define TEXIDX_LABEL_TIMER		(14)		// 制限時間の枠
#define TEXIDX_BEARSTATE		(15)		// クマの状態アイコン

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static GAMESTATE g_gameState;			// 現在のゲームの状態
static bool g_bPause;					// ポーズ中か
static int g_nCounterStateGame;			// 終了状態になってからリザルトへ移るまでの残りフレーム
static int g_nCounterGame;				// 制限時間を減らすためのフレームカウンター
static int g_nIdxOrnament;				// オーナメント数を表示するスコアの番号
static int g_nIdxScore;					// 現在のスコアを表示するスコアの番号
static int g_nIdxTimer;					// 制限時間を表示するスコアの番号
static int g_nFinallyScore;				// 最終スコア(リザルト画面で表示)

//==================================================================================
// --- ゲーム画面の初期化 ---
//==================================================================================
HRESULT InitGame(void)
{
	g_bPause = false;
	g_nFinallyScore = 0;

	/*** 各要素の初期化 ***/
	InitCamera();

#ifdef DIRECTIONAL_LIGHT
	InitLight();
#elif SPOT_LIGHT // DIRECTIONAL_LIGHT
	InitSpotLight();
#endif

	InitField();
	InitWall();
	InitShadow();
	InitObject();
	InitPlayer();
	InitEffect();
	InitMeshfield();
	InitMeshCylinder();
	InitMeshSphere();
	InitMeshRing();
	InitOrnament();
	InitPause();
	InitXmasTree();
	InitTimer();
	Init2DPolygon();
	InitScore();
	InitBearState();

	/*** スクリプトの読み込み ***/
	if (FAILED(InitScript(GAME_SCRIPT)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return E_FAIL;
	}

	if (FAILED(LoadMotion(GAME_MOTION)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "LoadMotionに失敗しました。");
		return E_FAIL;
	}

	SetIndexTextureRing(TEXIDX_RING);
	SetTextureScore(TEXIDX_SCORE_NUMBER);

	/*** ゲーム状態の初期化 ***/
	g_nCounterStateGame = 0;
	g_nCounterGame = 0;
	g_gameState = GAMESTATE_NORMAL;

	/*** ステージの設定 ***/
	SetOrnamentRand(GAME_ORNAMENT_NUM);
	SetPlayer(VECNULL, VECNULL, "");

	/*** UIの設定 ***/
	// 制限時間
	Set2DPolygon(D3DXVECTOR3(640.0f, 70.0f, 0.0f), VECNULL, D3DXVECTOR2(200.0f, 110.0f), TEXIDX_LABEL_TIMER);
	g_nIdxTimer = SetScore(GAME_TIME_LIMIT, D3DXVECTOR3(600.0f, 90.0f, 0.0f), DIGIT_TIMER);

	// オーナメント数
	Set2DPolygon(D3DXVECTOR3(135.0f, 60.0f, 0.0f), VECNULL, D3DXVECTOR2(200.0f, 60.0f), TEXIDX_LABEL_ORNAMENT);
	g_nIdxOrnament = SetScore(0, D3DXVECTOR3(265.0f, 60.0f, 0.0f), DIGIT_ORNAMENT);

	// スコア
	Set2DPolygon(D3DXVECTOR3(850.0f, 60.0f, 0.0f), VECNULL, D3DXVECTOR2(105.0f, 30.0f), TEXIDX_LABEL_SCORE);
	g_nIdxScore = SetScore(0, D3DXVECTOR3(950.0f, 60.0f, 0.0f), DIGIT_SCORE);

	SetBearState(D3DXVECTOR3(640.0f, 640.0f, 0.0f), D3DXVECTOR2(128.0f, 128.0f), TEXIDX_BEARSTATE);

	/*** 背景の円柱(空)の設置 ***/
	SetMeshCylinder(VEC_Y(-15000.0f), VECNULL, 30000.0f, 15000.0f, 32, 3, CULLINGTYPE_NONE, TEXIDX_SKY, false);

	return S_OK;
}

//==================================================================================
// --- ゲーム画面の終了 ---
//==================================================================================
void UninitGame(void)
{
	UninitCamera();
	ResetCamera();
	UninitField();
	UninitWall();
	UninitObject();
	UninitPlayer();
	UninitEffect();
	UninitShadow();
	UninitScript();
	UninitLight();
	UninitMeshfield();
	UninitMeshCylinder();
	UninitMeshSphere();
	UninitMeshRing();
	UninitOrnament();
	UninitPause();
	UninitXmasTree();
	UninitTimer();
	Uninit2DPolygon();
	UninitScore();
	UninitBearState();

	/*** コントローラーの振動を止める ***/
	SetVibration(0, 0, 0);
}

//==================================================================================
// --- ゲーム画面の更新 ---
//==================================================================================
void UpdateGame(void)
{
	/*** 終了状態ならカウントダウン後にリザルト画面へ ***/
	switch (g_gameState)
	{
	case GAMESTATE_GOODEND:		// クリア
		g_nCounterStateGame--;
		if (g_nCounterStateGame <= 0 && GetFade() == FADE_NONE)
		{
			g_nFinallyScore = GetScore(g_nIdxScore);
			SetFade(MODE_GOODEND, FADE_TYPE_NORMAL);
			FadeSound(SOUND_LABEL_BGM_GAMECLEAR);
			g_gameState = GAMESTATE_NONE;
		}
		break;

	case GAMESTATE_BADEND:		// ゲームオーバー
		g_nCounterStateGame--;
		if (g_nCounterStateGame <= 0 && GetFade() == FADE_NONE)
		{
			SetFade(MODE_BADEND, FADE_TYPE_NORMAL);
			FadeSound(SOUND_LABEL_BGM_GAMEOVER);
			g_gameState = GAMESTATE_NONE;
		}
		break;

	default:					// プレイ中・何もなし
		break;
	}

	/*** ポーズの切り替え (プレイ中のみ) ***/
	if (g_gameState == GAMESTATE_NORMAL && GetFade() == FADE_NONE)
	{
		if (GetKeyboardTrigger(DIK_P) || GetJoypadTrigger(JOYKEY_START))
		{
			g_bPause = !g_bPause;
			ShowCursor(g_bPause);
		}
	}

	UpdateCamera();

	/*** プレイ中でポーズしていなければ、ゲームを進める ***/
	if (g_bPause == false && g_gameState == GAMESTATE_NORMAL)
	{
		/*** 1秒ごとに制限時間を減らす ***/
		g_nCounterGame++;
		if (g_nCounterGame % GAME_FRAME_PER_SECOND == 0)
		{
			AddScore(-1, g_nIdxTimer);
		}

		UpdateTimer();
		UpdateField();
		UpdateWall();
		UpdatePlayer();
		UpdateObject();
		UpdateEffect();
		UpdateShadow();
		UpdateOrnament();
		UpdateMeshSphere();
		UpdateXmasTree();
		UpdateMeshfield();
		UpdateMeshCylinder();
		UpdateMeshRing();
		Update2DPolygon();
		UpdateScore();
		UpdateBearState();
	}

	UpdateLight();
	UpdatePause();
}

//==================================================================================
// --- ゲーム画面の描画 ---
//==================================================================================
void DrawGame(void)
{
	SetCamera();

	/*** 3Dの描画 ***/
	DrawField();
	DrawWall();
	DrawPlayer();
	DrawObject();
	DrawMeshfield();
	DrawShadow();
	DrawOrnament();
	DrawXmasTree();
	DrawMeshCylinder();
	DrawMeshSphere();
	DrawMeshRing();
	DrawEffect();

	/*** 2Dの描画 (3Dより手前に表示するため最後に描画) ***/
	Draw2DPolygon();
	DrawBearState();
	DrawScore();

	if (g_bPause == true)
	{ // ポーズ画面
		DrawPause();
	}
}

//==================================================================================
// --- ポーズ状態の設定 ---
//==================================================================================
void SetEnablePause(bool bPause)
{
	g_bPause = bPause;
}

//==================================================================================
// --- ポーズ中か ---
//==================================================================================
bool GetEnablePause(void)
{
	return g_bPause;
}

//==================================================================================
// --- ゲーム状態の設定 (プレイ中の時のみ変更できる) ---
// nCounter : その状態を維持するフレーム数 (終了状態ならリザルトへ移るまでの時間)
//==================================================================================
void SetGameState(GAMESTATE state, int nCounter)
{
	if (g_gameState == GAMESTATE_NORMAL)
	{
		g_gameState = state;
		g_nCounterStateGame = nCounter;
	}
}

//==================================================================================
// --- オーナメント数を表示するスコアの番号の取得 (ゲーム画面のみ) ---
//==================================================================================
bool GetOrnamentNumIndex(int *pOrnament)
{
	if (pOrnament == NULL || GetMode() != MODE_GAME)
	{
		return false;
	}

	*pOrnament = g_nIdxOrnament;
	return true;
}

//==================================================================================
// --- 現在のスコアを表示するスコアの番号の取得 ---
// ゲーム画面と、ゲーム画面からクリア・ランキング画面へ遷移中の間のみ取得できる
//==================================================================================
bool GetScoreIndex(int *pScore)
{
	if (pScore == NULL)
	{
		return false;
	}

	if (GetMode() != MODE_GAME && GetModeNext() != MODE_RANKING && GetModeNext() != MODE_GOODEND)
	{
		return false;
	}

	*pScore = g_nIdxScore;
	return true;
}

//==================================================================================
// --- 制限時間を表示するスコアの番号の取得 (ゲーム画面のみ) ---
//==================================================================================
bool GetTimerIndex(int *pTimer)
{
	if (pTimer == NULL || GetMode() != MODE_GAME)
	{
		return false;
	}

	*pTimer = g_nIdxTimer;
	return true;
}

//==================================================================================
// --- 最終スコアの取得 ---
//==================================================================================
int GetFinallyScore(void)
{
	return g_nFinallyScore;
}

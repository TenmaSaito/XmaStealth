//==================================================================================
//
// ゲームクリア(リザルト)画面の処理 [gameclear.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "gameclear.h"
#include "gameclearBg.h"
#include "fade.h"
#include "2Dpolygon.h"
#include "score.h"
#include "loadxfile.h"
#include "ornament.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define GAMECLEAR_SCRIPT		"data/SCRIPT/Texture.txt"	// 使用テクスチャを記述したスクリプト
#define GAMECLEAR_FADE_TIME		(120)		// ランキング画面へのフェード時間(フレーム)
#define GAMECLEAR_AUTO_FRAME	(4000)		// 自動でランキング画面へ遷移し始めるフレーム

// --- Texture.txt 内のテクスチャ番号 ---
#define TEXIDX_LABEL_ORNAMENT	(1)			// 「オーナメント数」見出し
#define TEXIDX_LABEL_TRASH		(2)			// 「ゴミ数」見出し
#define TEXIDX_LABEL_SECRET		(3)			// 「シークレットスコア」見出し
#define TEXIDX_LABEL_FINALLY	(4)			// 「最終スコア」見出し
#define TEXIDX_SCORE_NUMBER		(5)			// スコアの数字

// --- スコアの桁数 ---
#define DIGIT_COUNT				(3)			// 個数表示の桁数
#define DIGIT_SCORE				(6)			// スコア表示の桁数

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static int g_nCounterGameClear;			// ゲームクリア画面の経過フレーム
static int g_nIdxNumOrnament;			// オーナメント数のスコア番号
static int g_nIdxNumTrash;				// ゴミ数のスコア番号
static int g_nIdxSecretScore;			// シークレットスコアのスコア番号
static int g_nIdxFinallyScore;			// 最終スコアのスコア番号

//==================================================================================
// --- ゲームクリア画面の初期化 ---
//==================================================================================
void InitGameClear(void)
{
	g_nCounterGameClear = 0;
	g_nIdxNumOrnament = -1;
	g_nIdxNumTrash = -1;
	g_nIdxSecretScore = -1;
	g_nIdxFinallyScore = -1;

	/*** 各要素の初期化 ***/
	InitGameClearBg();
	Init2DPolygon();
	InitScore();

	if (FAILED(InitScript(GAMECLEAR_SCRIPT)))
	{
		GenerateMessageBox(MB_ICONERROR, "Failed", "スクリプト読み込みに失敗しました。");
		return;
	}

	SetTextureScore(TEXIDX_SCORE_NUMBER);

	/*** 各スコアの見出しポリゴンを設置 ***/
	Set2DPolygon(D3DXVECTOR3(115.0f, 345.0f, 0.0f), VECNULL, D3DXVECTOR2(215.0f, 75.0f), TEXIDX_LABEL_ORNAMENT);
	Set2DPolygon(D3DXVECTOR3(580.0f, 345.0f, 0.0f), VECNULL, D3DXVECTOR2(215.0f, 75.0f), TEXIDX_LABEL_TRASH);
	Set2DPolygon(D3DXVECTOR3(115.0f, 490.0f, 0.0f), VECNULL, D3DXVECTOR2(235.0f, 75.0f), TEXIDX_LABEL_SECRET);
	Set2DPolygon(D3DXVECTOR3(115.0f, 645.0f, 0.0f), VECNULL, D3DXVECTOR2(235.0f, 75.0f), TEXIDX_LABEL_FINALLY);

	/*** 各スコアの数字を設置 ***/
	g_nIdxNumOrnament = SetScore(GetOrnamentNum(), D3DXVECTOR3(265.0f, 345.0f, 0.0f), DIGIT_COUNT);
	g_nIdxNumTrash = SetScore(GetTrashNum(), D3DXVECTOR3(730.0f, 345.0f, 0.0f), DIGIT_COUNT);
	g_nIdxSecretScore = SetScore(GetBonusScore(), D3DXVECTOR3(295.0f, 490.0f, 0.0f), DIGIT_SCORE);
	g_nIdxFinallyScore = SetScore(GetFinallyScore(), D3DXVECTOR3(295.0f, 645.0f, 0.0f), DIGIT_SCORE);
}

//==================================================================================
// --- ゲームクリア画面の終了 ---
//==================================================================================
void UninitGameClear(void)
{
	UninitGameClearBg();
	Uninit2DPolygon();
	UninitScore();
	UninitScript();
}

//==================================================================================
// --- ゲームクリア画面の更新 ---
//==================================================================================
void UpdateGameClear(void)
{
	UpdateGameClearBg();
	Update2DPolygon();
	UpdateScore();

	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE)
	{ // 決定入力でランキング画面へ
		PlaySound(SOUND_LABEL_SE_ENTER);

		SetFade(MODE_RANKING, FADE_TYPE_NORMAL, GAMECLEAR_FADE_TIME);
	}

	if (g_nCounterGameClear >= GAMECLEAR_AUTO_FRAME && GetFade() == FADE_NONE)
	{ // 一定時間経過で自動的にランキング画面へ
		SetFade(MODE_RANKING, FADE_TYPE_NORMAL, GAMECLEAR_FADE_TIME);
	}

	/*** デバッグ表示 : マウス座標 ***/
	PrintDebugProc("MOUSE x/y %~2f\n", (float)GetMousePos().x, (float)GetMousePos().y);

	g_nCounterGameClear++;
}

//==================================================================================
// --- ゲームクリア画面の描画 ---
//==================================================================================
void DrawGameClear(void)
{
	DrawGameClearBg();
	Draw2DPolygon();
	DrawScore();
}

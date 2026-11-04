//==================================================================================
//
// ランキング画面の処理 [ranking.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "ranking.h"
#include "light.h"
#include "fade.h"
#include "rankingScore.h"
#include "cameraRanking.h"
#include "field.h"
#include "object.h"
#include "mesh.h"
#include "2Dpolygon.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define RANKING_SCRIPT			"data//SCRIPT//modelRanking.txt"	// 配置スクリプト
#define RANKING_FADE_TIME		(120)		// タイトル画面へのフェード時間(フレーム)
#define RANKING_AUTO_FRAME		(4000)		// 自動でタイトル画面へ遷移し始めるフレーム
#define RANKING_RANK_NUM		(5)			// 表示する順位の数

// --- modelRanking.txt 内のテクスチャ番号 ---
#define TEXIDX_SKY				(1)			// 背景の円柱(空)
#define TEXIDX_TITLE			(2)			// 「ランキング」見出し
#define TEXIDX_RANK_FIRST		(3)			// 「1位」の見出し (以降、順位ごとに連番)

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static int g_nCounterRanking;		// ランキング画面の経過フレーム

//==================================================================================
// --- ランキング画面の初期化 ---
//==================================================================================
void InitRanking(void)
{
	g_nCounterRanking = 0;

	/*** 各要素の初期化 ***/
	InitRankingScore();
	InitLight();
	InitCameraRanking();
	InitField();
	InitObject();
	InitMeshCylinder();
	Init2DPolygon();

	/*** ランキングスコア・カメラの設定 ***/
	SetRankingScore();
	SetCameraRanking(D3DXVECTOR3(0.0f, 150.0f, 100.0f), D3DXVECTOR3(0.0f, 50.0f, 0.0f), 45.0f, 1.0f, 30000.0f, 300.0f, 1.0f, 0.02f);

	if (FAILED(InitScript(RANKING_SCRIPT)))
	{
		return;
	}

	/*** 見出しポリゴンの設置 ***/
	Set2DPolygon(D3DXVECTOR3(640.0f, 100.0f, 0.0f), VECNULL, D3DXVECTOR2(800.0f, 150.0f), TEXIDX_TITLE);

	for (int nCntRanking = 0; nCntRanking < RANKING_RANK_NUM; nCntRanking++)
	{ // 順位の見出しを縦に並べる
		D3DXVECTOR3 pos = D3DXVECTOR3(450.0f, 150.0f + (103.0f * (nCntRanking + 1)), 0.0f);		// 表示位置

		Set2DPolygon(pos, VECNULL, D3DXVECTOR2(150.0f, 70.0f), TEXIDX_RANK_FIRST + nCntRanking);
	}

	/*** 背景の円柱(空)の設置 ***/
	SetMeshCylinder(VEC_Y(-15000.0f), VECNULL, 30000.0f, 20000.0f, 32, 1, CULLINGTYPE_NONE, TEXIDX_SKY, false);
}

//==================================================================================
// --- ランキング画面の終了 ---
//==================================================================================
void UninitRanking(void)
{
	UninitRankingScore();
	UninitCameraRanking();
	UninitLight();
	UninitMeshCylinder();
	UninitField();
	UninitObject();
	Uninit2DPolygon();
	UninitScript();
}

//==================================================================================
// --- ランキング画面の更新 ---
//==================================================================================
void UpdateRanking(void)
{
	UpdateCameraRanking();
	UpdateLight();
	UpdateRankingScore();
	UpdateField();
	UpdateMeshCylinder();
	UpdateObject();
	Update2DPolygon();

	if ((GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_A)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE)
	{ // 決定入力でタイトル画面へ
		SetFade(MODE_TITLE, FADE_TYPE_NORMAL, RANKING_FADE_TIME);

		/*** タイトルBGMをフェードイン ***/
		FadeSound(SOUND_LABEL_BGM_TITLE);
	}

	if (g_nCounterRanking >= RANKING_AUTO_FRAME && GetFade() == FADE_NONE)
	{ // 一定時間経過で自動的にタイトル画面へ
		PlaySound(SOUND_LABEL_SE_ENTER);

		SetFade(MODE_TITLE, FADE_TYPE_NORMAL);

		/*** タイトルBGMをフェードイン ***/
		FadeSound(SOUND_LABEL_BGM_TITLE);
	}

	g_nCounterRanking++;
}

//==================================================================================
// --- ランキング画面の描画 ---
//==================================================================================
void DrawRanking(void)
{
	SetCameraRanking();

	/*** 3Dの描画 ***/
	DrawMeshCylinder();
	DrawField();
	DrawObject();

	/*** 2Dの描画 (3Dより手前に表示するため最後に描画) ***/
	DrawRankingScore();
	Draw2DPolygon();
}

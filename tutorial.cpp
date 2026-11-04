//==================================================================================
//
// チュートリアル画面の処理 [tutorial.cpp]
// Author : TENMA SAITO
//
// ゲームと同じ操作を練習できる画面。BackSpace / STARTボタンでゲーム画面へ進む。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "tutorial.h"
#include "light.h"
#include "fade.h"
#include "cameraTutorial.h"
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
#include "bearState.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define TUTORIAL_SCRIPT			"data//SCRIPT//modelTutorial.txt"	// 配置スクリプト
#define TUTORIAL_MOTION			"data//SCRIPT//motion_bear.txt"	// プレイヤー(クマ)のモーション
#define TUTORIAL_FADE_TIME		(120)		// ゲーム画面へのフェード時間(フレーム)
#define TUTORIAL_ORNAMENT_NUM	(40)		// ランダムに配置するオーナメントの数

// --- スキップ表示の揺れ ---
#define SKIP_POS				D3DXVECTOR3(1100.0f, 650.0f, 0.0f)	// スキップ表示の初期位置
#define SKIP_SPEED				(0.5f)		// スキップ表示の横移動速度
#define SKIP_TURN_FRAME			(60)		// スキップ表示の移動方向を反転する間隔(フレーム)

// --- modelTutorial.txt 内のテクスチャ番号 ---
#define TEXIDX_SKY				(1)			// 背景の円柱(空)
#define TEXIDX_RING				(5)			// ツリーの周りのリング
#define TEXIDX_SKIP				(6)			// 「スキップ」表示
#define TEXIDX_BEARSTATE		(7)			// クマの状態アイコン
#define TEXIDX_TITLE			(9)			// 「チュートリアル」見出し

//**********************************************************************************
// *** 揺れる2Dポリゴン構造体の定義 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;			// 位置
	float fSpd;					// 移動速度
	int nIdx2DPolygon;			// 2Dポリゴンの番号
	int nCounter;				// 経過フレーム
	int nTerminalMove;			// 移動方向を反転する間隔
} _2DPOLYGON_MOVE;

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static int g_nCounterTutorial;				// チュートリアル画面の経過フレーム
static _2DPOLYGON_MOVE g_skipPolygon;		// 「スキップ」表示

//==================================================================================
// --- チュートリアル画面の初期化 ---
//==================================================================================
void InitTutorial(void)
{
	g_nCounterTutorial = 0;
	ZeroMemory(&g_skipPolygon, sizeof(_2DPOLYGON_MOVE));

	/*** 各要素の初期化 ***/
	InitLight();
	InitCameraTutorial();
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
	InitBearState();

	/*** スクリプトの読み込み ***/
	if (FAILED(InitScript(TUTORIAL_SCRIPT)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "InitScriptに失敗しました。");
		return;
	}

	if (FAILED(LoadMotion(TUTORIAL_MOTION)))
	{
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "LoadMotionに失敗しました。");
		return;
	}

	/*** ステージの設定 ***/
	SetIndexTextureRing(TEXIDX_RING);
	SetOrnamentRand(TUTORIAL_ORNAMENT_NUM);
	SetCameraTutorial(D3DXVECTOR3(0.0f, 55.0f, -300.0f), D3DXVECTOR3(0.0f, 55.0f, 0.0f), 45.0f, 1.0f, 30000.0f, 300.0f, 5.0f, 0.07f);
	SetMeshCylinder(VEC_Y(-15000.0f), VECNULL, 30000.0f, 15000.0f, 32, 3, CULLINGTYPE_NONE, TEXIDX_SKY, false);

	/*** UIの設定 ***/
	g_skipPolygon.pos = SKIP_POS;
	g_skipPolygon.nIdx2DPolygon = Set2DPolygon(g_skipPolygon.pos, VECNULL, D3DXVECTOR2(300.0f, 75.0f), TEXIDX_SKIP);
	g_skipPolygon.fSpd = SKIP_SPEED;
	g_skipPolygon.nTerminalMove = SKIP_TURN_FRAME;

	SetBearState(D3DXVECTOR3(640.0f, 640.0f, 0.0f), D3DXVECTOR2(128.0f, 128.0f), TEXIDX_BEARSTATE);
	Set2DPolygon(D3DXVECTOR3(640.0f, 70.0f, 0.0f), VECNULL, D3DXVECTOR2(300.0f, 110.0f), TEXIDX_TITLE);

	/*** プレイヤーの設置とBGMの再生 ***/
	SetPlayer(VEC_Z(-100.0f), VECNULL, TUTORIAL_MOTION);
	Play3DSound(SOUND_LABEL_BGM_GAME, D3DXVECTOR3(0, 300, 0), VECNULL, D3DXVECTOR3(0, 1, 0));
}

//==================================================================================
// --- チュートリアル画面の終了 ---
//==================================================================================
void UninitTutorial(void)
{
	UninitLight();
	UninitMeshCylinder();
	UninitField();
	UninitObject();
	UninitScript();		// ※ InitScript で読み込んだテクスチャの破棄 (呼び忘れるとリプレイの度にリークする)
	UninitShadow();
	UninitPlayer();
	UninitEffect();
	UninitXmasTree();
	UninitMeshSphere();
	UninitOrnament();
	UninitMeshRing();
	UninitBillBoard();
	Uninit2DPolygon();
	UninitBearState();
	UninitCameraTutorial();
}

//==================================================================================
// --- チュートリアル画面の更新 ---
//==================================================================================
void UpdateTutorial(void)
{
	/*** 各要素の更新 ***/
	UpdateCameraTutorial();
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
	UpdateBearState();

	if ((GetKeyboardTrigger(DIK_BACK)
		|| GetJoypadTrigger(JOYKEY_START))
		&& GetFade() == FADE_NONE)
	{ // スキップ入力でゲーム画面へ
		PlaySound(SOUND_LABEL_SE_ENTER);

		SetFade(MODE_GAME, FADE_TYPE_NORMAL, TUTORIAL_FADE_TIME);
		FadeSound(SOUND_LABEL_BGM_GAME);
	}

	/*** 「スキップ」表示を左右に揺らす ***/
	g_skipPolygon.nCounter++;
	if (g_skipPolygon.nCounter % g_skipPolygon.nTerminalMove == 0)
	{ // 一定間隔で移動方向を反転
		g_skipPolygon.fSpd *= -1;
	}
	else
	{
		g_skipPolygon.pos.x += g_skipPolygon.fSpd;
		SetPosition2DPolygon(g_skipPolygon.nIdx2DPolygon, g_skipPolygon.pos);
	}

	g_nCounterTutorial++;
}

//==================================================================================
// --- チュートリアル画面の描画 ---
//==================================================================================
void DrawTutorial(void)
{
	SetCameraTutorial();

	/*** 3Dの描画 ***/
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

	/*** 2Dの描画 (3Dより手前に表示するため最後に描画) ***/
	DrawBearState();
	Draw2DPolygon();
}

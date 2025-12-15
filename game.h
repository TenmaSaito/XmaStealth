//================================================================================================================
//
// DirectXのゲーム用ヘッダファイル [game.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _GAME_H_
#define _GAME_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"
#include "sound.h"
#include "mathUtil.h"
#include "debugproc.h"
#include "loadxfile.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define SOUND_SPD			D3DXVECTOR3(1.0f, 1.0f, 1.0f);		// ベロシティ値
#define WORLD_GRAVITY		(-0.5f)								// 重力
#define TERMINAL_VELOCITY	(-10.0f)							// 終端速度
#define DIRECTIONAL_LIGHT
// #define POINT_LIGHT
// #define SPOT_LIGHT

//**********************************************************************************
//*** ゲームの状態 ***
//**********************************************************************************
typedef enum
{
	GAMESTATE_NONE = 0,
	GAMESTATE_NORMAL,
	GAMESTATE_GOODEND,
	GAMESTATE_BADEND,
	GAMESTATE_MAX
}GAMESTATE;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
HRESULT InitGame(void);
void UninitGame(void);
void UpdateGame(void);
void DrawGame(void);

void SetEnablePause(bool bPause);
bool GetEnablePause(void);
void SetGameState(GAMESTATE state, int nCounter);
#endif
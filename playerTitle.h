//==================================================================================
//
// タイトル画面のプレイヤー(クマ)用ヘッダファイル [playerTitle.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _PLAYERTITLE_H_
#define _PLAYERTITLE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "object.h"
#include "player.h"

//**********************************************************************************
// *** タイトル用プレイヤー構造体 (ゲーム用プレイヤーと同じ構造) ***
//**********************************************************************************
typedef Player PlayerTitle;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitPlayerTitle(void);
void UninitPlayerTitle(void);
void UpdatePlayerTitle(void);
void DrawPlayerTitle(void);

void LoadPartsPlayerTitle(PartsInfo pInfo);
void SetMotionPartsNumPlayerTitle(int nNumParts);
void LoadMotionPlayerTitle(MOTION_INFO *mInfo, MOTIONTYPE nIdxMotion);
void SetMotionNumPlayerTitle(int nMotionNum);

void SetPlayerTitle(D3DXVECTOR3 pos, D3DXVECTOR3 rot, const char *pMotionName);
PlayerTitle *GetPlayerTitle(void);

#endif

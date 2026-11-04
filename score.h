//==================================================================================
//
// スコアヘッダーファイル [score.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _SCORE_H_
#define _SCORE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitScore(void);
void UninitScore(void);
void UpdateScore(void);
void DrawScore(void);

void SetTextureScore(int nIdxTexture);
int SetScore(int nScore, D3DXVECTOR3 posCenter, int nScoreNum = 3);
int GetScore(int nIdxScore);
void AddScore(int nValue, int nIdxScore);
void SetScore(int nScore, int nIdxScore);
void MoveScore(D3DXVECTOR3 pos, int nIdxScore);
void DestroyScore(int nIdxScore);

#endif

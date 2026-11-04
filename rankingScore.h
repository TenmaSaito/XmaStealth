//==================================================================================
//
// ランキング表示のヘッダーファイル [rankingScore.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _RANKING_SCORE_H_
#define _RANKING_SCORE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "ranking.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitRankingScore(void);
void UninitRankingScore(void);
void UpdateRankingScore(void);
void DrawRankingScore(void);

void SetRankingScore(void);
int GetRankingScore(void);
void AddRankingScore(int nScore);

#endif

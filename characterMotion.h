//==================================================================================
//
// 階層モデル(キャラクター)のモーション・描画の共通処理用ヘッダファイル [characterMotion.h]
// Author : TENMA SAITO
//
// ゲーム用プレイヤー(player.cpp)とタイトル用プレイヤー(playerTitle.cpp)で共通の処理。
//
//==================================================================================
#ifndef _CHARACTERMOTION_H_
#define _CHARACTERMOTION_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "player.h"

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
// --- モーションの再生を1フレーム進め、各パーツの位置・向きを更新 ---
void UpdateCharacterMotion(Player *pChara);

// --- モーションの変更 (bBlend : 現在のモーションから nFrameBlend フレームかけて切り替える) ---
void SetCharacterMotionType(Player *pChara, MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend);

// --- 全パーツの描画 ---
void DrawCharacter(Player *pChara);

// --- スクリプトから読み込んだパーツ・モーションの登録 ---
void LoadCharacterParts(Player *pChara, PartsInfo parts);
void LoadCharacterMotion(Player *pChara, MOTION_INFO *pMotionInfo, MOTIONTYPE motionType);

#endif

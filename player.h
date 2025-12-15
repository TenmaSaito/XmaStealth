//================================================================================================================
//
// DirectXのプレイヤー表示用ヘッダファイル [player.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _PLAYER_H_
#define _PLAYER_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"
#include "camera.h"
#include "object.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define MAX_PLAYER		(1)				// プレイヤーの最大数

//*************************************************************************************************
//*** プレイヤー構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// モデルの位置
	D3DXVECTOR3 posOld;					// 過去の位置
	D3DXVECTOR3 move;					// モデルの移動量
	D3DXVECTOR3 rot;					// モデルの向き
	float fSpd;							// 移動速度
	float fJump;						// 跳躍力
	float fWidth;						// モデルの幅
	float fHeight;						// モデルの高さ
	float fDepth;						// モデルの奥行
	float fRadius;						// 半径
	int nIdShadow;						// 影の番号
	int nIdOrnament;					// 現在つかんでいるオーナメントのインデックス
	bool bDisp;							// 描画の有無
	bool bUse;							// 読み込みに成功しているか
	bool bJump;							// 跳んだか
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	PartsInfo aModel[15];				// モデルアニメーション情報(パーツ)
	int nNumModel;						// モデル数

	MOTION_INFO aMotionInfo[MAX_MOTION];// モーションの情報
	int nNumMotion;						// モーションの総数
	MOTIONTYPE motionType;				// 現在のモーションの種類
	bool bLoop;							// 現在のループするかどうか
	int nNumKey;						// 現在のキーの総数
	int nKey;							// 現在の現在のキーNo (基本始まりは0)
	int nCounterMotion;					// 現在のモーションのカウンター
	bool bFinishMotion;					// モーションが終了しているか

	bool bBlendMotion;					// ブレンドモーションがあるか
	MOTIONTYPE motionTypeBlend;			// ブレンドモーションのモーションの種類
	bool bLoopBlend;					// ブレンドモーションがループするかどうか
	int nNumKeyBlend;					// ブレンドモーションのキーの総数
	int nKeyBlend;						// ブレンドモーションの現在のキーNo (基本始まりは0)
	int nCounterMotionBlend;			// ブレンドモーションのカウンター

	int nFrameBlend;					// ブレンドフレーム数
	int nCounterBlend;					// ブレンドカウンター数
}Player;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

void LoadPartsPlayer(PartsInfo pInfo);
void SetMotionPartsNum(int nNumParts);
void LoadMotionPlayer(MOTION_INFO *mInfo, MOTIONTYPE nIdxMotion);
void SetMotionNum(int nMotionNum);

void SetCollisionSize(float fRadius, float fHeight);
void SetPlayerSetting(float fMove, float fJump);
void SetPlayer(D3DXVECTOR3 pos, D3DXVECTOR3 rot, const char *pMotionName);
// void SetMotionPlayer(MOTIONTYPE motionType);
Player *GetPlayer(void);
#endif
//================================================================================================================
//
// DirectXの投擲物表示用ヘッダファイル [ornament.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _ORNAMENT_H_
#define _ORNAMENT_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "mesh.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define MAX_ORNAMENT			(512)		// 読み込める投擲物の総数

//*************************************************************************************************
//*** 投擲物情報の定義 ***
//*************************************************************************************************
typedef struct
{
	int nOrnamentType;		// オーナメントのタイプ
	int nType;		// オブジェクト番号
	float fRadius;	// 判定半径
	float fWeight;	// オーナメントの重さ
	bool bUse;		// 情報が格納されているか
	bool bSafe;		// 取り扱っても問題ないか
}ORNAMENT_INFO, *PORNAMENT_INFO;

//*************************************************************************************************
//*** 投擲物の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// モデルの位置
	D3DXVECTOR3 posOld;					// モデルの過去位置
	D3DXVECTOR3 move;					// モデルの移動量
	D3DXVECTOR3 rot;					// モデルの向き
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	D3DXMATRIX *pMtxParent;				// つかんでいるモデルのマトリックスへのポインタ
	float fSpd;							// 移動速度
	int nIdShadow;						// 影の番号
	int nType;							// オーナメント番号
	int nIdxCollision;					// 球形当たり判定のインデックス
	int nIdxBill;						// ビルボードのインデックス
	int nCounter;						// カウンター
	bool bDisp;							// 描画するか
	bool bUse;							// 使用しているかどうか
	bool bCatched;						// つかまれているか
	bool bReady;						// つかむことが出来るか
	bool bCatchedTree;					// 木に当たったか
	bool bLand;							// 床に落ちたか
}Ornament, *PORNAMENT;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitOrnament(void);
void UninitOrnament(void);
void UpdateOrnament(void);
void DrawOrnament(void);

void SettingOrnamentInfo(int nOrnamentType, int nType, float fRadius, float fWeight);
int CollisionOrnament(LPMESHSPHERE pCollisionSphere);
int CollisionOrnament(D3DXVECTOR3 pos);
void SetParentOrnament(int nIdxOrnament, LPD3DXMATRIX pMtxParent);
void RemoveParentOrnament(int nIdxOrnament);
void ShotOrnament(int nIdxOrnament, D3DXVECTOR3 vec, float fSpeed);
PORNAMENT GetOrnament(int nIdxOrnament);
void SetOrnament(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType, float fSpd, float fRadius);
void CollisionTree(LPMESHSPHERE pCollisionSphere, LPD3DXMATRIX pMtxXmasTree, D3DXVECTOR3 posTree);
void SetOrnamentRand(int nNumOrnament);
#endif
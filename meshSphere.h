//================================================================================================================
//
// DirectXのメッシュフィールド用ヘッダファイル [meshSphere.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _MESHSPHERE_H_
#define _MESHSPHERE_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define _OffSet_

//**********************************************************************************
//*** メッシュスフィア構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// 位置(オフセット)
	D3DXVECTOR3 posLocal;	// 位置のローカル
	D3DXVECTOR3 rot;		// ポリゴンの角度
	D3DXVECTOR3 rotLocal;	// ポリゴンの角度
	D3DXCOLOR col;			// 色
	D3DXMATRIX mtxWorld;	// ワールドマトリックス
	D3DXMATRIX *pMtxParent;	// 親のワールドマトリックス
	float fRadius;			// 半径
	int nXBlock;			// Xの分割数
	int nYBlock;			// Yの分割数
	int nIdxParent;			// 親子インデックス
	LPDIRECT3DTEXTURE9		pTexture;	// テクスチャへのポインタ
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ
	bool bUse;				// 使われているか
	bool bDisp;				// 描画するか
	bool bCollision;		// 球形当たり半径として扱うか
}MeshSphere, *LPMESHSPHERE;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitMeshSphere(void);
void UninitMeshSphere(void);
void UpdateMeshSphere(void);
void DrawMeshSphere(void);

int SetMeshSphere(_OffSet_ D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp);
int SetParentSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp, D3DXMATRIX *pMtxParent);
LPMESHSPHERE GetMeshSphere(int nIdxSphere);
void CollisionSphere(D3DXVECTOR3 pos, float fRadius);
bool CollisionSphereToSphere(LPMESHSPHERE pSphere);
bool CollisionSphereToSphere(LPMESHSPHERE pSphere, LPMESHSPHERE pSphereAnother);
bool CollisionIndexSphere(int nIndexSphere, D3DXVECTOR3 pos, float fRadius);
bool CollisionLPSphere(LPMESHSPHERE pSphere, D3DXVECTOR3 pos, float fMag = 1.0f);
#endif
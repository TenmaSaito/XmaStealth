//==================================================================================
//
// メッシュスフィア(球・球形の当たり判定)用ヘッダファイル [meshSphere.h]
// Author : TENMA SAITO
//
//==================================================================================
#ifndef _MESHSPHERE_H_
#define _MESHSPHERE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
// *** メッシュスフィア構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// 位置(親がある場合は親からのオフセット)
	D3DXVECTOR3 rot;					// 角度
	D3DXCOLOR col;						// 色
	D3DXMATRIX mtxWorld;				// ワールドマトリックス (描画時に更新)
	D3DXMATRIX *pMtxParent;				// 親のワールドマトリックス (無ければ NULL)
	float fRadius;						// 半径
	int nXBlock;						// 横(円周)の分割数
	int nYBlock;						// 縦の分割数
	LPDIRECT3DVERTEXBUFFER9 pVtxBuff;	// 頂点バッファのポインタ (描画する球のみ)
	LPDIRECT3DINDEXBUFFER9 pIdxBuff;	// インデックスバッファへのポインタ (描画する球のみ)
	bool bUse;							// 使われているか
	bool bDisp;							// 描画するか
	bool bCollision;					// 球形の当たり判定として扱うか
} MeshSphere, *LPMESHSPHERE;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
void InitMeshSphere(void);
void UninitMeshSphere(void);
void UpdateMeshSphere(void);
void DrawMeshSphere(void);

// --- 球の設置 (bDisp が false なら当たり判定用で、頂点バッファは作らない) ---
int SetMeshSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp);

// --- 親マトリックスに追従する当たり判定用の球の設置 ---
int SetParentSphere(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, int nXBlock, int nYBlock, float fRadius, bool bDisp, D3DXMATRIX *pMtxParent);

LPMESHSPHERE GetMeshSphere(int nIdxSphere);
bool CollisionSphereToSphere(LPMESHSPHERE pSphere, LPMESHSPHERE pSphereAnother);
bool CollisionLPSphere(LPMESHSPHERE pSphere, D3DXVECTOR3 pos, float fMag = 1.0f);

#endif

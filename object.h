//================================================================================================================
//
// DirectXのXファイル表示用ヘッダファイル [model.h]
// Author : TENMA
//
//================================================================================================================
#ifndef _OBJECT_H_
#define _OBJECT_H_

//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "game.h"

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define MAX_OBJECT			(1024)			// 読み込めるオブジェクト数
#define MAX_OBJTEXTURE		(14)			// 読み込むテクスチャの最大数
#define MAX_MOTION			(10)			// モーションの最大数
#define MAX_KEY_INFO		(30)			// キー情報の最大数
#define MAX_KEY				(20)			// キーの最大数

//*************************************************************************************************
//*** モーションの種類 ***
//*************************************************************************************************
typedef enum
{
	MOTIONTYPE_NEUTRAL = 0,			// 待機
	MOTIONTYPE_MOVE,				// 移動
	MOTIONTYPE_ACTION,				// 行動
	MOTIONTYPE_JUMP,				// ジャンプ
	MOTIONTYPE_LANDING,				// 着地
	MOTIONTYPE_MAX
}MOTIONTYPE;

//*************************************************************************************************
//*** 当たり判定の種類 ***
//*************************************************************************************************
typedef enum
{
	COLLISIONTYPE_NONE = 0,			// 当たり判定無し
	COLLISIONTYPE_RECT,				// 矩形
	COLLISIONTYPE_CROSS,			// 外積
	COLLISIONTYPE_SPHERE,			// 球形
	COLLISIONTYPE_MAX
}COLLISIONTYPE;

//*************************************************************************************************
//*** キー要素構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	float fPosX;		// 位置X
	float fPosY;		// 位置Y
	float fPosZ;		// 位置Z
	float fRotX;		// 向きX
	float fRotY;		// 向きY
	float fRotZ;		// 向きZ
} KEY;

//*************************************************************************************************
//*** キー情報構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	int nFrame;					// 再生フレーム
	KEY aKey[MAX_KEY];			// 各パーツのキー要素(パーツ数分だけ用意)
} KEY_INFO;

//*************************************************************************************************
//*** モーション情報構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	bool bLoop;							// ループするかどうか
	int nNumKey;						// キーの総数
	KEY_INFO aKeyInfo[MAX_KEY_INFO];	// キー情報
}MOTION_INFO;

//*************************************************************************************************
//*** モデル情報構造体の定義 ***
//*************************************************************************************************
typedef struct ObjectInfo
{
	LPD3DXMESH pMesh;								// メッシュ(頂点情報)へのポインタ
	LPD3DXBUFFER pBuffMat;							// マテリアルへのポインタ
	LPDIRECT3DTEXTURE9 apTexture[MAX_OBJTEXTURE];	// テクスチャへのポインタ
	DWORD dwNumMat;									// マテリアルの数
	D3DXVECTOR3 mtxMin;								// オブジェクトの最小値
	D3DXVECTOR3 mtxMax;								// オブジェクトの最大値
	D3DXMATRIX mtxWorld;							// ワールドマトリックス
	bool bUse;										// 一度格納しようとしたか
	bool bSafe;										// 情報が格納されているか(参照可能か)
}ObjectInfo;

typedef struct ObjectInfo *POBJECTINFO, *LPOBJECTINFO;

//*************************************************************************************************
//*** 階層構造情報構造体の定義 ***
//*************************************************************************************************
typedef struct PartsInfo
{
	int nIdxObject;				// オブジェクト管理用インデックス
	int nIdxModel;				// モデルインデックス
	int nIdxModelParent;		// 親モデルのインデックス
	int nIdxCollision;			// 当たり判定球用インデックス
	D3DXVECTOR3 pos;			// 位置(オフセット)
	D3DXVECTOR3 posLocal;		// ローカル座標(オフセット)
	D3DXVECTOR3 rot;			// 向き
	D3DXVECTOR3 rotLocal;		// ローカル角度
	D3DXMATRIX mtxWorld;		// ワールドマトリックス
	bool bUse;					// 使用状況
}PartsInfo;

typedef struct PartsInfo* PPARTSINFO, *LPPARTSINFO;

//*************************************************************************************************
//*** モデル構造体の定義 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;					// モデルの位置
	D3DXVECTOR3 move;					// モデルの移動量
	D3DXVECTOR3 rot;					// モデルの向き
	D3DXMATRIX mtxWorld;				// ワールドマトリックス
	float fWidth;						// モデルの幅
	float fHeight;						// モデルの高さ
	float fDepth;						// モデルの奥行
	int nType;							// 設置するモデルの種類
	char aCode[MAX_PATH];				// 判別用特殊文字列
	bool bUse;							// 使われているか
	COLLISIONTYPE collisionType;		// 当たり判定の種類(0 : NULL, 1 : 矩形, 2 : 外積, 3 : 球形)
}Object, *LPOBJECT;

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
void InitObject(void);
void UninitObject(void);
void UpdateObject(void);
void DrawObject(void);

_Check_return_ HRESULT LoadObject(_In_ const char *pXFileName);
void SetObject(_In_ D3DXVECTOR3 pos, _In_ D3DXVECTOR3 rot, _In_ int nType, _In_ COLLISIONTYPE nTypeCollision = COLLISIONTYPE_RECT, _In_opt_ const char *pCode = NULL);
bool CollisionObject(D3DXVECTOR3 *pPos, D3DXVECTOR3 *pPosOld, D3DXVECTOR3 *pMove, float fHeight, bool* pColl = NULL);
Object *GetObject(void);
ObjectInfo *GetObjectInfo(_In_ int nType);
D3DXVECTOR3 GetObjectMVector(int nType, int nXYZ);
#endif
//==================================================================================
//
// オーナメント(拾って投げて、ツリーに飾る物)の処理 [ornament.cpp]
// Author : TENMA SAITO
//
// 状態の流れ :
//   床に置かれている(bReady) → プレイヤーが拾う(bCatched) → 投げる → 床に落ちる / ツリーに付く(bCatchedTree)
// ツリーに付くとスコアが入り、ツリーが重さの分だけ傾く。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "ornament.h"
#include "player.h"
#include "shadow.h"
#include "object.h"
#include "effect.h"
#include "wall.h"
#include "XmasTree.h"
#include "mesh.h"
#include "field.h"
#include "billboard.h"
#include "score.h"
#include "fade.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define ORNAMENT_GRAVITY_RATE	(0.05f)		// 重力の倍率
#define ORNAMENT_BOUND_SPEED	(0.5f)		// 床に落ちた時に跳ねる速さ
#define ORNAMENT_CATCH_RANGE	(6.0f)		// 拾える距離 (当たり判定の半径に対する倍率)
#define ORNAMENT_SPHERE_BLOCK	(12)		// 当たり判定球の分割数
#define ORNAMENT_TOP_HEIGHT		(0.85f)		// てっぺんに付けた時の高さ (球の半径に対する倍率)
#define ORNAMENT_END_FRAME		(120)		// 全て飾り終えてからリザルトへ移るまでのフレーム
#define MAX_STAR				(5)			// 1ステージに置く星の数

// --- 目印の演出 ---
#define RING_INTERVAL			(120)		// 足元のリングを出す間隔(フレーム)
#define RING_HEIGHT				(0.5f)		// リングの高さ
#define RING_SPD_IN				(0.75f)		// リングの内周の広がる速さ
#define RING_SPD_OUT			(1.35f)		// リングの外周の広がる速さ
#define RING_LIFE				(75)		// リングの寿命
#define ARROW_HEIGHT_SET		(50.0f)		// 置いた時の矢印の高さ (チュートリアル)
#define ARROW_HEIGHT_LAND		(25.0f)		// 落ちた時の矢印の高さ (チュートリアル)
#define ARROW_WIDTH				(10.0f)		// 矢印の幅
#define ARROW_SIZE_Y			(25.0f)		// 矢印の高さ
#define ARROW_TEXIDX			(3)			// 矢印のテクスチャ番号

// --- ランダム配置の範囲 ---
#define RAND_GAME_RANGE_X		(800)		// ゲーム画面 : X方向の乱数の幅
#define RAND_GAME_RANGE_Y		(500)		// ゲーム画面 : Y方向の乱数の幅
#define RAND_GAME_RANGE_Z		(900)		// ゲーム画面 : Z方向の乱数の幅
#define RAND_TUTORIAL_RANGE_X	(1000)		// チュートリアル画面 : X方向の乱数の幅
#define RAND_TUTORIAL_RANGE_Y	(2)			// チュートリアル画面 : Y方向の乱数の幅
#define RAND_TUTORIAL_RANGE_Z	(1000)		// チュートリアル画面 : Z方向の乱数の幅
#define STAIR_MIN_Y				(187.0f)	// 階段付近の高さ(置かない範囲)の下限
#define STAIR_MAX_Y				(199.0f)	// 階段付近の高さ(置かない範囲)の上限
#define RAND_RETRY_MAX			(100000)	// ランダム配置のやり直しの上限 (無限ループ防止)

// --- 移動可能範囲 ---
#define STAGE_MAX_Z				(500.0f)	// ゲーム画面 : 奥の端
#define STAGE_MIN_Z				(-400.0f)	// ゲーム画面 : 手前の端
#define STAGE_MIN_X				(-400.0f)	// ゲーム画面 : 左端
#define STAGE_MAX_X				(400.0f)	// ゲーム画面 : 右端
#define LIVING_MAX_X			(100.0f)	// ゲーム画面 : リビングの右端
#define LIVING_MAX_Z			(295.0f)	// ゲーム画面 : リビングの右端の壁が始まる奥行
#define KITCHEN_MIN_Z			(300.0f)	// ゲーム画面 : 子供部屋の下の奥行
#define TUTORIAL_STAGE_RECT		D3DXVECTOR4(-500.0f, 500.0f, -200.0f, 700.0f)	// チュートリアル画面 (x:左 y:右 z:手前 w:奥)

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void UpdateOrnamentFree(PORNAMENT pOrnament, MODE mode);
static void DrawOrnamentModel(PORNAMENT pOrnament, LPOBJECTINFO pObj);
static void AddTreeScore(const ORNAMENT_INFO *pInfo, bool bTop);
static D3DXCOLOR GetOrnamentTypeColor(int nOrnamentType);
static bool GetRandomOrnamentPos(MODE mode, bool bCheckStair, D3DXVECTOR3 *pPos);
static int GetRandomOrnamentType(bool bStarOnly, int nCntStar);
static bool IsIndexValidOrnament(int nIdxOrnament);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static Ornament g_aOrnament[MAX_ORNAMENT];				// オーナメントの情報
static ORNAMENT_INFO g_aOrnamentInfo[MAX_ORNAMENT];		// オーナメントの種類の情報 (スクリプトの ORNAMENT)
static int g_nCounterOrnament;							// 配置したオーナメントの数
static int g_nCounterOrnamentInfo;						// 登録したオーナメントの種類の数
static int g_nCounterCatchedOrnament;					// ツリーに付いたオーナメントの数
static int g_nCounterCatchedTrash;						// ツリーに付いたゴミの数
static int g_nSecretScore;								// シークレットスコア(てっぺんボーナス)の合計

//==================================================================================
// --- オーナメントの初期化処理 ---
//==================================================================================
void InitOrnament(void)
{
	ZeroMemory(&g_aOrnament[0], sizeof g_aOrnament);
	ZeroMemory(&g_aOrnamentInfo[0], sizeof g_aOrnamentInfo);

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		g_aOrnament[nCntOrnament].nIdxBill = -1;
	}

	g_nCounterOrnament = 0;
	g_nCounterOrnamentInfo = 0;
	g_nCounterCatchedOrnament = 0;
	g_nCounterCatchedTrash = 0;
	g_nSecretScore = 0;
}

//==================================================================================
// --- オーナメントの終了処理 ---
//==================================================================================
void UninitOrnament(void)
{

}

//==================================================================================
// --- オーナメントの更新処理 ---
//==================================================================================
void UpdateOrnament(void)
{
	MODE mode = GetMode();		// 現在の画面
	int nCounterPutted = 0;		// ツリーに付いているオーナメントの数

	if (mode != MODE_GAME && mode != MODE_TUTORIAL)
	{ // ゲーム・チュートリアル画面以外では動かさない
		return;
	}

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 更新するオーナメント

		if (pOrnament->bUse == false)
		{ // 未使用
			continue;
		}

		pOrnament->posOld = pOrnament->pos;

		if (pOrnament->pMtxParent == NULL)
		{ // 誰にも持たれていない(床にある・飛んでいる)
			UpdateOrnamentFree(pOrnament, mode);
		}

		if (pOrnament->bCatchedTree == true)
		{
			nCounterPutted++;
		}

		pOrnament->nCounter++;
	}

	if (nCounterPutted >= g_nCounterOrnament && mode == MODE_GAME)
	{ // 全てのオーナメントをツリーに付けたらクリア
		SetGameState(GAMESTATE_GOODEND, ORNAMENT_END_FRAME);
	}
}

//==================================================================================
// --- オーナメントの描画処理 ---
//==================================================================================
void DrawOrnament(void)
{
	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 描画するオーナメント

		if (pOrnament->bUse == false || pOrnament->bDisp == false || g_aOrnamentInfo[pOrnament->nType].bSafe == false)
		{ // 未使用 or 非表示 or 種類の情報が無い
			continue;
		}

		LPOBJECTINFO pObj = GetObjectInfo(g_aOrnamentInfo[pOrnament->nType].nType);		// 使用するモデル
		if (pObj == NULL)
		{ // モデルが無い
			continue;
		}

		DrawOrnamentModel(pOrnament, pObj);
	}
}

//==================================================================================
// --- オーナメントの種類の登録 (スクリプトから) ---
// nOrnamentType : ORNAMENTTYPE (範囲外なら通常) / nType : 使用するモデル番号
//==================================================================================
void SettingOrnamentInfo(int nOrnamentType, int nType, float fRadius, float fWeight)
{
	for (int nCntInfo = 0; nCntInfo < MAX_ORNAMENT; nCntInfo++)
	{
		PORNAMENT_INFO pInfo = &g_aOrnamentInfo[nCntInfo];		// 登録先

		if (pInfo->bUse == true)
		{ // 使用中
			continue;
		}

		if (GetObjectInfo(nType) != NULL)
		{ // モデルが読み込めている
			pInfo->nOrnamentType = (nOrnamentType < 0 || nOrnamentType >= ORNAMENTTYPE_MAX) ? ORNAMENTTYPE_NORMAL : nOrnamentType;
			pInfo->nType = nType;
			pInfo->fRadius = fRadius;
			pInfo->fWeight = fWeight;
			pInfo->bSafe = true;
		}
		else
		{ // モデルが無い → 使わない種類として登録 (番号をずらさないため)
			pInfo->bSafe = false;
		}

		pInfo->bUse = true;
		g_nCounterOrnamentInfo++;

		return;
	}
}

//==================================================================================
// --- オーナメントの配置 ---
//==================================================================================
void SetOrnament(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType, float fSpd, float fRadius)
{
	LPOBJECTINFO pObj = GetObjectInfo(g_aOrnamentInfo[nType].nType);		// 使用するモデル

	if (pObj == NULL)
	{ // モデルが無い
		return;
	}

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 配置先

		if (pOrnament->bUse == true)
		{ // 使用中
			continue;
		}

		/*** 影の生成と情報の設定 ***/
		pOrnament->nIdShadow = SetShadow((pObj->mtxMax.x - pObj->mtxMin.x), (pObj->mtxMax.z - pObj->mtxMin.z), pos);

		pOrnament->pos = pos;
		pOrnament->rot = rot;
		pOrnament->fSpd = fSpd;
		pOrnament->nType = nType;
		pOrnament->pMtxParent = NULL;
		pOrnament->bDisp = true;
		pOrnament->bUse = true;
		pOrnament->bCatched = false;
		pOrnament->bCatchedTree = false;
		pOrnament->bReady = false;			// 床に落ちるまでは拾えない

		/*** 当たり判定の球 (オーナメントの行列に追従) ***/
		pOrnament->nIdxCollision = SetParentSphere(VECNULL,
			VECNULL,
			D3DXCOLOR(1.0f, 0.0f, 0.0f, 0.3f),
			ORNAMENT_SPHERE_BLOCK,
			ORNAMENT_SPHERE_BLOCK,
			g_aOrnamentInfo[nType].fRadius,
			false,
			&pOrnament->mtxWorld);

		if (GetModeNext() == MODE_TUTORIAL)
		{ // チュートリアルでは、種類の色の矢印を上に出す
			D3DXVECTOR3 posArrow = pOrnament->pos;		// 矢印の位置
			posArrow.y += ARROW_HEIGHT_SET;

			pOrnament->nIdxBill = SetBillBoard(posArrow, ARROW_WIDTH, ARROW_SIZE_Y, ARROW_TEXIDX, GetOrnamentTypeColor(g_aOrnamentInfo[nType].nOrnamentType));
		}

		g_nCounterOrnament++;

		return;
	}
}

//==================================================================================
// --- 球とオーナメントの当たり判定 ---
// 戻り値 : 最初に当たったオーナメントの番号 (無ければ -1)
//==================================================================================
int CollisionOrnament(LPMESHSPHERE pCollisionSphere)
{
	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 判定するオーナメント

		if (pOrnament->bUse == true && CollisionSphereToSphere(pCollisionSphere, GetMeshSphere(pOrnament->nIdxCollision)))
		{
			return nCntOrnament;
		}
	}

	return -1;
}

//==================================================================================
// --- 点と拾えるオーナメントの当たり判定 ---
// 戻り値 : 最初に当たったオーナメントの番号 (無ければ -1)
//==================================================================================
int CollisionOrnament(D3DXVECTOR3 pos)
{
	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 判定するオーナメント

		if (pOrnament->bUse == true && pOrnament->bReady == true
			&& CollisionLPSphere(GetMeshSphere(pOrnament->nIdxCollision), pos, ORNAMENT_CATCH_RANGE))
		{
			return nCntOrnament;
		}
	}

	return -1;
}

//==================================================================================
// --- ツリーの当たり判定球とオーナメントの判定 (当たったらツリーに付ける) ---
//==================================================================================
void CollisionTree(LPMESHSPHERE pCollisionSphere, LPD3DXMATRIX pMtxXmasTree, D3DXVECTOR3 posTree)
{
	PXMASTREE pXmas = GetXmasTree();		// クリスマスツリー

	if (pCollisionSphere == NULL || pMtxXmasTree == NULL || GetFade() != FADE_NONE)
	{ // 判定できない or 画面遷移中
		return;
	}

	for (int nCntOrnament = 0; nCntOrnament < MAX_ORNAMENT; nCntOrnament++)
	{
		PORNAMENT pOrnament = &g_aOrnament[nCntOrnament];		// 判定するオーナメント
		PORNAMENT_INFO pInfo = &g_aOrnamentInfo[pOrnament->nType];	// オーナメントの種類

		if (pOrnament->bUse == false || pOrnament->bCatchedTree == true || pOrnament->bCatched == true)
		{ // 未使用 or 既にツリーに付いている or 持たれている
			continue;
		}

		if (CollisionSphereToSphere(pCollisionSphere, GetMeshSphere(pOrnament->nIdxCollision)) == false)
		{ // 当たっていない
			continue;
		}

		D3DXVECTOR3 posTreeSphere = D3DXVECTOR3(pCollisionSphere->mtxWorld._41, pCollisionSphere->mtxWorld._42, pCollisionSphere->mtxWorld._43);	// ツリーの球の中心
		float fAngle = GetPosToPos(pOrnament->pos, posTreeSphere);		// 球の中心から見たオーナメントの方向
		D3DXVECTOR3 posOrnament;										// ツリーの球から見た付ける位置
		bool bTop = false;												// てっぺんに付けたか

		if (pCollisionSphere == GetMeshSphere(pXmas->aIdxCollision[0]) && pXmas->bTop == false)
		{ // まだ何も付いていないてっぺん → 真上に付ける
			posOrnament = D3DXVECTOR3(0.0f, pCollisionSphere->fRadius * ORNAMENT_TOP_HEIGHT, 0.0f);
			fAngle = 0.0f;
			pXmas->bTop = true;
			bTop = true;
		}
		else
		{ // それ以外 → 当たった方向の球の表面に付けて、重さの分ツリーを傾ける
			posOrnament.x = sinf(fAngle) * pCollisionSphere->fRadius;
			posOrnament.y = pOrnament->pos.y - posTreeSphere.y;
			posOrnament.z = cosf(fAngle) * pCollisionSphere->fRadius;

			PutOrnament(fAngle, pInfo->fWeight);
		}

		/*** オーナメント数とスコアの加算 ***/
		int nIdxOrnamentScore = -1;		// オーナメント数のスコア番号
		if (GetOrnamentNumIndex(&nIdxOrnamentScore))
		{
			AddScore(1, nIdxOrnamentScore);
			g_nCounterCatchedOrnament++;
		}

		AddTreeScore(pInfo, bTop);

		/*** ツリーの球に固定する ***/
		pOrnament->pos = posOrnament;
		pOrnament->move = VECNULL;
		pOrnament->rot.y = InverseRot(fAngle);
		pOrnament->bReady = false;
		pOrnament->bCatchedTree = true;
		pOrnament->pMtxParent = &pCollisionSphere->mtxWorld;
		InvisibleShadow(pOrnament->nIdShadow, 0.0f);

		PlaySound(SOUND_LABEL_SE_CATCH);
	}
}

//==================================================================================
// --- オーナメントを持たせる (拾える状態の時のみ) ---
//==================================================================================
void SetParentOrnament(int nIdxOrnament, LPD3DXMATRIX pMtxParent)
{
	if (IsIndexValidOrnament(nIdxOrnament) == false || g_aOrnament[nIdxOrnament].bReady == false)
	{ // 番号が範囲外 or 拾えない状態
		return;
	}

	PORNAMENT pOrnament = &g_aOrnament[nIdxOrnament];		// 持たせるオーナメント

	pOrnament->pos = VECNULL;
	pOrnament->rot = VECNULL;
	pOrnament->move = VECNULL;
	pOrnament->bCatched = true;
	pOrnament->bReady = false;
	pOrnament->pMtxParent = pMtxParent;

	/*** 影と矢印を消す ***/
	InvisibleShadow(pOrnament->nIdShadow, 0.0f);
	KillBillboard(pOrnament->nIdxBill);
	pOrnament->nIdxBill = -1;
}

//==================================================================================
// --- オーナメントを手放す ---
//==================================================================================
void RemoveParentOrnament(int nIdxOrnament)
{
	if (IsIndexValidOrnament(nIdxOrnament) == false)
	{ // 番号が範囲外
		return;
	}

	g_aOrnament[nIdxOrnament].bCatched = false;
	g_aOrnament[nIdxOrnament].pMtxParent = NULL;
	InvisibleShadow(g_aOrnament[nIdxOrnament].nIdShadow, 1.0f);		// 影を出す
}

//==================================================================================
// --- 持っているオーナメントを投げる ---
// vec.y : 投げる方向(Y軸の角度)
//==================================================================================
void ShotOrnament(int nIdxOrnament, D3DXVECTOR3 vec, float fSpeed)
{
	if (IsIndexValidOrnament(nIdxOrnament) == false || g_aOrnament[nIdxOrnament].bCatched == false)
	{ // 番号が範囲外 or 持たれていない
		return;
	}

	PORNAMENT pOrnament = &g_aOrnament[nIdxOrnament];		// 投げるオーナメント

	pOrnament->bReady = false;
	pOrnament->move.x = sinf(vec.y + D3DX_PI) * fSpeed;
	pOrnament->move.z = cosf(vec.y + D3DX_PI) * fSpeed;

	/*** 持っていた位置(ワールド座標)から投げる ***/
	D3DXVec3TransformCoord(&pOrnament->pos, &pOrnament->pos, &pOrnament->mtxWorld);
	pOrnament->rot = GetPlayer()->rot;

	RemoveParentOrnament(nIdxOrnament);
}

//==================================================================================
// --- オーナメントの取得 (範囲外は NULL) ---
//==================================================================================
PORNAMENT GetOrnament(int nIdxOrnament)
{
	if (IsIndexValidOrnament(nIdxOrnament) == false)
	{
		return NULL;
	}

	return &g_aOrnament[nIdxOrnament];
}

//==================================================================================
// --- オーナメントの種類の情報の取得 (範囲外・使えない種類は NULL) ---
//==================================================================================
PORNAMENT_INFO GetOrnamentInfo(int nIdxOrnamentInfo)
{
	if (nIdxOrnamentInfo < 0 || nIdxOrnamentInfo >= g_nCounterOrnamentInfo || g_aOrnamentInfo[nIdxOrnamentInfo].bSafe == false)
	{
		return NULL;
	}

	return &g_aOrnamentInfo[nIdxOrnamentInfo];
}

//==================================================================================
// --- オーナメントをランダムに配置 (星は必ず MAX_STAR 個にする) ---
//==================================================================================
void SetOrnamentRand(int nNumOrnament)
{
	MODE modeNext = GetModeNext();		// これから始まる画面
	int nMaxOrnament = nNumOrnament;	// 配置する数
	int nCntOrnament = 0;				// 配置した数
	int nCntStar = 0;					// 配置した星の数
	int nCntRetry = 0;					// やり直した回数

	if (g_nCounterOrnamentInfo <= 0 || (modeNext != MODE_GAME && modeNext != MODE_TUTORIAL))
	{ // 種類が登録されていない or 配置しない画面
		return;
	}

	if (nMaxOrnament >= MAX_ORNAMENT) nMaxOrnament = MAX_ORNAMENT;

	/*** ランダムな種類を配置 (星は MAX_STAR 個まで) ***/
	while (nCntOrnament < nMaxOrnament && nCntRetry < RAND_RETRY_MAX)
	{
		D3DXVECTOR3 pos;		// 配置する位置
		if (GetRandomOrnamentPos(modeNext, true, &pos) == false)
		{ // 置けない位置
			nCntRetry++;
			continue;
		}

		int nType = GetRandomOrnamentType(false, nCntStar);		// 配置する種類
		if (nType == -1)
		{ // 星以外の種類が無い
			nCntRetry++;
			continue;
		}

		if (g_aOrnamentInfo[nType].nOrnamentType == ORNAMENTTYPE_STAR) nCntStar++;

		SetOrnament(pos, VECNULL, nType, 0.0f, g_aOrnamentInfo[nType].fRadius);
		nCntOrnament++;
	}

	/*** 星が足りなければ追加で配置 ***/
	while (nCntStar < MAX_STAR && nCntRetry < RAND_RETRY_MAX)
	{
		D3DXVECTOR3 pos;		// 配置する位置
		if (GetRandomOrnamentPos(modeNext, false, &pos) == false || pos == VECNULL)
		{ // 置けない位置
			nCntRetry++;
			continue;
		}

		int nType = GetRandomOrnamentType(true, nCntStar);		// 配置する種類(星)
		if (nType == -1)
		{ // 星の種類が無い
			break;
		}

		SetOrnament(pos, VECNULL, nType, 0.0f, g_aOrnamentInfo[nType].fRadius);
		nCntOrnament++;
		nCntStar++;
	}
}

//==================================================================================
// --- ツリーに付いたオーナメントの数 ---
//==================================================================================
int GetOrnamentNum(void)
{
	return g_nCounterCatchedOrnament;
}

//==================================================================================
// --- ツリーに付いたゴミの数 ---
//==================================================================================
int GetTrashNum(void)
{
	return g_nCounterCatchedTrash;
}

//==================================================================================
// --- シークレットスコア(てっぺんボーナス)の合計 ---
//==================================================================================
int GetBonusScore(void)
{
	return g_nSecretScore;
}

//==================================================================================
//
// ここから内部処理
//
//==================================================================================

//==================================================================================
// --- 誰にも持たれていないオーナメントの更新 (移動・当たり判定・目印の演出) ---
//==================================================================================
static void UpdateOrnamentFree(PORNAMENT pOrnament, MODE mode)
{
	float fRadius = g_aOrnamentInfo[pOrnament->nType].fRadius;		// 当たり判定の半径

	/*** 移動 ***/
	pOrnament->pos += pOrnament->move;

	/*** オブジェクトに当たったら止まる ***/
	if (CollisionObject(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move, fRadius))
	{
		pOrnament->move = VECNULL;
		pOrnament->bReady = true;
	}

	/*** 壁で跳ね返り、床に落ちたら止まる (ゲーム画面のみ) ***/
	if (mode == MODE_GAME)
	{
		ReflectWall(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move);

		if (CollisionFloor(&pOrnament->pos, &pOrnament->posOld, &pOrnament->move, fRadius))
		{
			pOrnament->pos.y += fRadius;
			pOrnament->move = VECNULL;
			pOrnament->bReady = true;
			pOrnament->bLand = true;
		}
	}

	/*** 重力・地面 ***/
	pOrnament->move.y += WORLD_GRAVITY * ORNAMENT_GRAVITY_RATE;

	if (pOrnament->pos.y < fRadius)
	{ // 地面に着いたら止まって拾えるようにする
		pOrnament->pos.y = fRadius;
		pOrnament->move = VECNULL;
		pOrnament->bReady = true;
		pOrnament->bLand = true;
	}

	SetPositionShadow(pOrnament->nIdShadow, pOrnament->pos, pOrnament->rot);

	/*** 一定間隔で足元に種類の色のリングを出す ***/
	if (pOrnament->nCounter % RING_INTERVAL == 0)
	{
		D3DXVECTOR3 posRing = pOrnament->pos;		// リングの位置
		posRing.y += RING_HEIGHT;

		SetMeshRing(posRing, VECNULL, 0.0f, 1.0f, 0, RING_SPD_IN, RING_SPD_OUT, RING_LIFE, GetOrnamentTypeColor(g_aOrnamentInfo[pOrnament->nType].nOrnamentType));
	}

	/*** 床に落ちた瞬間 ***/
	if (pOrnament->bLand == true)
	{
		if (pOrnament->nIdxBill == -1 && mode == MODE_TUTORIAL)
		{ // チュートリアルでは矢印を出し直す
			D3DXVECTOR3 posArrow = pOrnament->pos;		// 矢印の位置
			posArrow.y += ARROW_HEIGHT_LAND;
			pOrnament->nIdxBill = SetBillBoard(posArrow, ARROW_WIDTH, ARROW_SIZE_Y, ARROW_TEXIDX);
		}

		pOrnament->move.y = ORNAMENT_BOUND_SPEED;		// 少し跳ねる
		pOrnament->bReady = true;
		pOrnament->bLand = false;
	}

	/*** チュートリアルでは範囲外に出ないようにする ***/
	if (mode == MODE_TUTORIAL)
	{
		D3DXVECTOR4 rect = TUTORIAL_STAGE_RECT;		// 移動可能範囲

		if (CollisionBoxZ(rect, pOrnament->pos) == false)
		{
			if (pOrnament->pos.x <= rect.x)			pOrnament->pos.x = rect.x;
			else if (pOrnament->pos.x >= rect.y)	pOrnament->pos.x = rect.y;

			if (pOrnament->pos.z <= rect.z)			pOrnament->pos.z = rect.z;
			else if (pOrnament->pos.z >= rect.w)	pOrnament->pos.z = rect.w;
		}
	}
}

//==================================================================================
// --- オーナメントのモデルの描画 ---
//==================================================================================
static void DrawOrnamentModel(PORNAMENT pOrnament, LPOBJECTINFO pObj)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	D3DMATERIAL9 matDef;							// 現在のマテリアル保存用

	/*** ワールドマトリックスの計算 (回転 → 平行移動 → 親の順) ***/
	D3DXMatrixIdentity(&pOrnament->mtxWorld);

	D3DXMatrixRotationYawPitchRoll(&mtxRot, pOrnament->rot.y, pOrnament->rot.x, pOrnament->rot.z);
	D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, &mtxRot);

	D3DXMatrixTranslation(&mtxTrans, pOrnament->pos.x, pOrnament->pos.y, pOrnament->pos.z);
	D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, &mtxTrans);

	if (pOrnament->pMtxParent != NULL)
	{ // 持たれている・ツリーに付いている
		D3DXMatrixMultiply(&pOrnament->mtxWorld, &pOrnament->mtxWorld, pOrnament->pMtxParent);
	}

	pDevice->SetTransform(D3DTS_WORLD, &pOrnament->mtxWorld);

	/*** マテリアルごとに描画 ***/
	pDevice->GetMaterial(&matDef);

	D3DXMATERIAL *pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();		// マテリアルデータ

	for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
	{
		pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
		pDevice->SetTexture(0, pObj->apTexture[nCntMat]);
		pObj->pMesh->DrawSubset(nCntMat);
	}

	pDevice->SetMaterial(&matDef);
}

//==================================================================================
// --- ツリーに付けた時のスコア加算 ---
// bTop : てっぺんに付けたか (付けると種類に関係なくボーナス)
//==================================================================================
static void AddTreeScore(const ORNAMENT_INFO *pInfo, bool bTop)
{
	int nIdxScore = -1;		// スコアの番号

	if (GetScoreIndex(&nIdxScore) == false)
	{ // スコアが無い画面
		return;
	}

	switch (pInfo->nOrnamentType)
	{
	case ORNAMENTTYPE_NORMAL:		// 通常
		AddScore(NORMAL_SCORE, nIdxScore);
		break;

	case ORNAMENTTYPE_STAR:			// 星
		AddScore(STAR_SCORE, nIdxScore);
		break;

	case ORNAMENTTYPE_TRASH:		// ゴミ (てっぺんでもボーナス無し)
		AddScore(TRASH_SCORE, nIdxScore);
		g_nCounterCatchedTrash++;
		return;
	}

	if (bTop == true)
	{ // てっぺんボーナス
		AddScore(SPECIAL_SCORE, nIdxScore);
		g_nSecretScore += SPECIAL_SCORE;
	}
}

//==================================================================================
// --- オーナメントの種類の色 (通常 : 緑 / 星 : 黄 / ゴミ : 赤) ---
//==================================================================================
static D3DXCOLOR GetOrnamentTypeColor(int nOrnamentType)
{
	switch (nOrnamentType)
	{
	case ORNAMENTTYPE_NORMAL:	return D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
	case ORNAMENTTYPE_STAR:		return D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
	case ORNAMENTTYPE_TRASH:	return D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	default:					return DEF_COL;
	}
}

//==================================================================================
// --- オーナメントを置けるランダムな位置を1つ取得 ---
// bCheckStair : 階段付近の高さを避けるか
// 戻り値 : 置ける位置だったか (false なら呼び出し側でやり直す)
//==================================================================================
static bool GetRandomOrnamentPos(MODE mode, bool bCheckStair, D3DXVECTOR3 *pPos)
{
	if (mode == MODE_TUTORIAL)
	{ // チュートリアル : 四角い範囲の中ならどこでも
		*pPos = GetRandomVector3(RAND_TUTORIAL_RANGE_X, RAND_TUTORIAL_RANGE_Y, RAND_TUTORIAL_RANGE_Z);

		return CollisionBoxZ(TUTORIAL_STAGE_RECT, *pPos);
	}

	/*** ゲーム : 家の中で、壁の中・オブジェクトの中でない位置 ***/
	D3DXVECTOR3 pos = GetRandomVector3(RAND_GAME_RANGE_X, RAND_GAME_RANGE_Y, RAND_GAME_RANGE_Z);		// 候補の位置
	D3DXVECTOR3 move;																					// 当たり判定用(未使用)

	*pPos = pos;

	if ((pos.z < STAGE_MAX_Z && pos.z > STAGE_MIN_Z && pos.x > STAGE_MIN_X && pos.x < STAGE_MAX_X) == false)
	{ // 家の外
		return false;
	}

	if ((pos.z < LIVING_MAX_Z && pos.x > LIVING_MAX_X) || (pos.x > LIVING_MAX_X && pos.z < KITCHEN_MIN_Z))
	{ // リビングの右端・子供部屋の下 (壁の中)
		return false;
	}

	if (bCheckStair && pos.y >= STAIR_MIN_Y && pos.y <= STAIR_MAX_Y)
	{ // 階段付近
		return false;
	}

	/*** 床の上に乗せ、オブジェクトに埋まっていないか確認 ***/
	CollisionFloor(&pos, &pos, &move, 0.0f);
	*pPos = pos;

	return (CollisionObject(&pos, &pos, &move, 0.0f) == false);
}

//==================================================================================
// --- ランダムなオーナメントの種類を1つ取得 ---
// bStarOnly : 星だけから選ぶか / nCntStar : 配置済みの星の数 (MAX_STAR 以上なら星を選ばない)
// 戻り値 : 種類の番号 (選べる種類が無ければ -1)
//==================================================================================
static int GetRandomOrnamentType(bool bStarOnly, int nCntStar)
{
	bool bExist = false;		// 条件に合う種類があるか

	for (int nCntInfo = 0; nCntInfo < g_nCounterOrnamentInfo; nCntInfo++)
	{ // 無限ループ防止のため、先に条件に合う種類があるか確認
		bool bStar = (g_aOrnamentInfo[nCntInfo].nOrnamentType == ORNAMENTTYPE_STAR);		// 星か

		if ((bStarOnly && bStar) || (bStarOnly == false && (bStar == false || nCntStar < MAX_STAR)))
		{
			bExist = true;
			break;
		}
	}

	if (bExist == false)
	{
		return -1;
	}

	while (1)
	{
		int nType = rand() % g_nCounterOrnamentInfo;		// 候補の種類
		bool bStar = (g_aOrnamentInfo[nType].nOrnamentType == ORNAMENTTYPE_STAR);		// 星か

		if (bStarOnly && bStar) return nType;
		if (bStarOnly == false && (bStar == false || nCntStar < MAX_STAR)) return nType;
	}
}

//==================================================================================
// --- オーナメントの番号が配置済みの範囲か ---
//==================================================================================
static bool IsIndexValidOrnament(int nIdxOrnament)
{
	return (nIdxOrnament >= 0 && nIdxOrnament < g_nCounterOrnament);
}

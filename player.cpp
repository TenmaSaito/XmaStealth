//==================================================================================
//
// プレイヤー(クマ)の処理 [player.cpp]
// Author : TENMA SAITO
//
// ・カメラの向きを基準に8方向 + 左スティックで移動
// ・オーナメントに触れると拾う (1つ目は手、2つ目以降は頭の上に積む)
// ・持っているオーナメントの重さで移動・ジャンプが遅くなる
// ・モーションの再生と描画は characterMotion.cpp の共通処理を使う
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "player.h"
#include "characterMotion.h"
#include "camera.h"
#include "cameraTutorial.h"
#include "mathUtil.h"
#include "shadow.h"
#include "wall.h"
#include "field.h"
#include "effect.h"
#include "debugproc.h"
#include "mesh.h"
#include "ornament.h"
#include "2Dpolygon.h"
#include "bearState.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define PLAYER_SHADOW_SIZE		(10)		// 影の大きさ
#define MOVE_RESIST				(0.5f)		// 移動量の減衰係数
#define MAX_CATCH_ORNAMENT		(10)		// 一度に持てるオーナメントの最大数
#define WEIGHT_RESIST			(0.003f)	// オーナメントの重さ1あたりの移動抵抗
#define WEIGHT_COLOR_RATE		(1.25f)		// 重さによるクマの状態アイコンの赤み(抵抗に対する倍率)
#define ROT_FOLLOW_RATE			(0.05f)		// 目標の向きへ近づく割合
#define ROT_SNAP_RANGE			(0.001f)	// 目標の向きとの差がこれ未満なら一致させる
#define PLAYER_GRAVITY_RATE		(0.75f)		// 重力の倍率
#define MOTION_BLEND_FRAME		(10)		// モーション切り替えのブレンドフレーム
#define WALK_SE_INTERVAL		(20)		// 足音の間隔(フレーム)
#define ORNAMENT_STACK_HEIGHT	(15.0f)		// 頭の上に積むオーナメントの間隔
#define SHOT_SPEED				(5.0f)		// オーナメントを投げる速さ
#define SHOT_VIBRATION_POWER	(20000)		// 投げた時のコントローラーの振動の強さ
#define SHOT_VIBRATION_TIME		(30)		// 投げた時のコントローラーの振動の時間
#define STICK_MAX_VALUE			(32767.0f)	// スティックの最大入力値

// --- 着地時のリング演出 ---
#define LANDING_RING_HEIGHT		(0.5f)		// 足元からの高さ
#define LANDING_RING_SPD_IN		(2.15f)		// 内周の広がる速さ
#define LANDING_RING_SPD_OUT	(2.5f)		// 外周の広がる速さ
#define LANDING_RING_LIFE		(25)		// 寿命

// --- ゲーム画面の移動可能範囲 ---
#define STAGE_MAX_Z				(500.0f)	// 奥の端
#define STAGE_MIN_Z				(-400.0f)	// 手前の端
#define STAGE_MIN_X				(-400.0f)	// 左端
#define STAGE_MAX_X				(400.0f)	// 右端
#define LIVING_MAX_X			(100.0f)	// リビングの右端
#define LIVING_MAX_Z			(295.0f)	// リビングの右端の壁が始まる奥行
#define KITCHEN_MIN_Z			(300.0f)	// リビングの右側(キッチン)へ入れる奥行

// --- チュートリアル画面の移動可能範囲 (x:左 y:右 z:手前 w:奥) ---
#define TUTORIAL_STAGE_RECT		D3DXVECTOR4(-500.0f, 500.0f, -200.0f, 700.0f)

//**********************************************************************************
// *** 移動入力の方向 ***
//**********************************************************************************
typedef struct
{
	float fMoveAngle;		// カメラの向きに対する移動方向
	float fFaceAngle;		// カメラの向きに対する向く方向
} MOVE_DIRECTION;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static bool UpdatePlayerMoveInput(Player *pPlayer, const Camera *pCamera);
static void UpdatePlayerMotionState(Player *pPlayer, bool bMove);
static float UpdatePlayerRotation(Player *pPlayer);
static void UpdatePlayerAction(Player *pPlayer, const Camera *pCamera, float fAngleDiff);
static void UpdatePlayerCollision(Player *pPlayer);
static void ClampPlayerStage(Player *pPlayer);
static void LandPlayer(Player *pPlayer);
static void SetMotionType(MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend);
static bool CatchOrnament(int nIdOrnament);
static int ThrowOrnament(void);
static float GetWeightResistRate(const Player *pPlayer);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static Player g_player;										// プレイヤーの情報
static float g_fAnglePlayer;								// 向こうとしている向き(目標の向き)
static float g_fTakecopterSpeed;							// 頭のパーツの回転速度(現在は常に0)
static int g_nCounterPlayer;								// 経過フレーム
static int g_nCntPlayerOrnament;							// 持っているオーナメントの数
static int g_aOrnamentShot[MAX_CATCH_ORNAMENT];				// 持っているオーナメントの番号(拾った順 / 空きは-1)
static int g_nCounterWalk;									// 歩き続けているフレーム数(足音用)

// --- 方向キーの組み合わせごとの移動方向・向く方向 (×π) ---
static const MOVE_DIRECTION g_moveForwardLeft = { -0.25f, 0.75f };		// W + A
static const MOVE_DIRECTION g_moveForwardRight = { 0.25f, 1.25f };		// W + D
static const MOVE_DIRECTION g_moveForward = { 0.0f, 1.0f };				// W
static const MOVE_DIRECTION g_moveBackLeft = { 1.25f, 0.25f };			// S + A
static const MOVE_DIRECTION g_moveBackRight = { -1.25f, -0.25f };		// S + D
static const MOVE_DIRECTION g_moveBack = { 1.0f, 0.0f };				// S
static const MOVE_DIRECTION g_moveLeft = { 1.5f, 0.5f };				// A
static const MOVE_DIRECTION g_moveRight = { 0.5f, 1.5f };				// D

//==================================================================================
// --- プレイヤーの初期化処理 ---
//==================================================================================
void InitPlayer(void)
{
	Player *pPlayer = &g_player;		// プレイヤー

	/*** 各変数の初期化 ***/
	ZeroMemory(pPlayer, sizeof(Player));
	g_fAnglePlayer = 0.0f;
	g_fTakecopterSpeed = 0.0f;
	g_nCounterWalk = 0;
	g_nCounterPlayer = 0;
	g_nCntPlayerOrnament = 0;
	memset(&g_aOrnamentShot[0], -1, sizeof g_aOrnamentShot);

	/*** 影の生成 ***/
	pPlayer->nIdShadow = SetShadow(PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);

	pPlayer->motionType = MOTIONTYPE_NEUTRAL;
	pPlayer->nCounterMotion = 0;
	pPlayer->nKey = 0;
	pPlayer->nIdx2DPolygon = -1;
}

//==================================================================================
// --- プレイヤーの終了処理 ---
//==================================================================================
void UninitPlayer(void)
{

}

//==================================================================================
// --- プレイヤーの更新処理 ---
//==================================================================================
void UpdatePlayer(void)
{
	Player *pPlayer = &g_player;		// プレイヤー
	Camera *pCamera = (GetMode() == MODE_TUTORIAL) ? (Camera*)GetCameraTutorial() : GetCamera();		// 移動の基準にするカメラ

	pPlayer->posOld = pPlayer->pos;

	/*** 移動入力とモーションの切り替え ***/
	bool bMove = UpdatePlayerMoveInput(pPlayer, pCamera);		// 移動入力があったか

	g_nCounterWalk = bMove ? (g_nCounterWalk + 1) : 0;
	UpdatePlayerMotionState(pPlayer, bMove);

	/*** 向き・ジャンプ・オーナメントの操作 ***/
	float fAngleDiff = UpdatePlayerRotation(pPlayer);		// 目標の向きとの差
	UpdatePlayerAction(pPlayer, pCamera, fAngleDiff);

	/*** 重さによる減速と、クマの状態アイコンの色 ***/
	float fResistRate = GetWeightResistRate(pPlayer);		// 重さによる移動の倍率
	pPlayer->move.x *= fResistRate;
	pPlayer->move.z *= fResistRate;

	D3DXCOLOR colState = DEF_COL;		// アイコンの色 (重いほど赤く)
	colState.g = 1.0f / ((pPlayer->fResist * (WEIGHT_RESIST * WEIGHT_COLOR_RATE)) + 1.0f);
	colState.b = colState.g;
	SetBearStateColor(colState);

	/*** 位置の反映と当たり判定 ***/
	pPlayer->pos += pPlayer->move;
	UpdatePlayerCollision(pPlayer);

	/*** 重力・移動量の減衰 (※ 当たり判定より後に行うこと : 先に行うとオブジェクトの上に乗れなくなる) ***/
	pPlayer->move.y += WORLD_GRAVITY * PLAYER_GRAVITY_RATE;
	if (pPlayer->move.y < TERMINAL_VELOCITY)
	{
		pPlayer->move.y = TERMINAL_VELOCITY;
	}

	pPlayer->move.x += (0.0f - pPlayer->move.x) * MOVE_RESIST;
	pPlayer->move.z += (0.0f - pPlayer->move.z) * MOVE_RESIST;

	/*** 影の移動 ***/
	SetPositionShadow(pPlayer->nIdShadow, pPlayer->pos, pPlayer->rot);

	/*** モーションの更新と足音 ***/
	UpdateCharacterMotion(pPlayer);

	if (g_nCounterWalk != 0 && pPlayer->bJump == false)
	{
		if (g_nCounterWalk == 1 || g_nCounterWalk % WALK_SE_INTERVAL == 0)
		{
			PlaySound(SOUND_LABEL_SE_WALK);
		}
	}

	g_nCounterPlayer++;

	/*** ステージの外に出ないようにする ***/
	ClampPlayerStage(pPlayer);
}

//==================================================================================
// --- プレイヤーの描画処理 ---
//==================================================================================
void DrawPlayer(void)
{
	DrawCharacter(&g_player);
}

//==================================================================================
// --- パーツの登録 (スクリプトから) ---
//==================================================================================
void LoadPartsPlayer(PartsInfo pInfo)
{
	LoadCharacterParts(&g_player, pInfo);
}

//==================================================================================
// --- モーションの登録 (スクリプトから) ---
//==================================================================================
void LoadMotionPlayer(MOTION_INFO *mInfo, MOTIONTYPE nIdxMotion)
{
	LoadCharacterMotion(&g_player, mInfo, nIdxMotion);
}

//==================================================================================
// --- モーション総数の設定 ---
//==================================================================================
void SetMotionNum(int nMotionNum)
{
	g_player.nNumMotion = nMotionNum;
}

//==================================================================================
// --- プレイヤーの設置 ---
//==================================================================================
void SetPlayer(D3DXVECTOR3 pos, D3DXVECTOR3 rot, const char *pMotionName)
{
	Player *pPlayer = &g_player;		// プレイヤー

	pPlayer->pos = pos;
	pPlayer->rot = rot;
	g_fAnglePlayer = 0.0f;
	pPlayer->bDisp = true;
	pPlayer->bUse = true;
}

//==================================================================================
// --- パーツ数の設定 ---
//==================================================================================
void SetMotionPartsNum(int nNumParts)
{
	g_player.nNumModel = nNumParts;
}

//==================================================================================
// --- 当たり判定の大きさの設定 ---
//==================================================================================
void SetCollisionSize(float fRadius, float fHeight)
{
	g_player.fRadius = fRadius;
	g_player.fHeight = fHeight;
}

//==================================================================================
// --- 移動速度・跳躍力の設定 ---
//==================================================================================
void SetPlayerSetting(float fMove, float fJump)
{
	g_player.fSpd = fMove;
	g_player.fJump = fJump;
}

//==================================================================================
// --- 頭・手のパーツ番号の設定 ---
//==================================================================================
void SetPlayerPartsIndices(int nIdxHead, int nIdxHand)
{
	g_player.nModelHead = nIdxHead;
	g_player.nModelHand = nIdxHand;
}

//==================================================================================
// --- プレイヤーの取得 ---
//==================================================================================
Player *GetPlayer(void)
{
	return &g_player;
}

//==================================================================================
//
// ここから内部処理
//
//==================================================================================

//==================================================================================
// --- 移動入力 (キーボード8方向 → 無ければ左スティック) ---
// 戻り値 : 移動入力があったか
//==================================================================================
static bool UpdatePlayerMoveInput(Player *pPlayer, const Camera *pCamera)
{
	bool bUp = GetKeyboardPress(DIK_W) || GetJoypadPress(JOYKEY_UP);			// 前
	bool bDown = GetKeyboardPress(DIK_S) || GetJoypadPress(JOYKEY_DOWN);		// 後ろ
	bool bLeft = GetKeyboardPress(DIK_A) || GetJoypadPress(JOYKEY_LEFT);		// 左
	bool bRight = GetKeyboardPress(DIK_D) || GetJoypadPress(JOYKEY_RIGHT);		// 右
	const MOVE_DIRECTION *pDir = NULL;											// 入力された方向

	/*** 方向キーの組み合わせから方向を決める (前後を優先) ***/
	if (bUp)
	{
		pDir = bLeft ? &g_moveForwardLeft : (bRight ? &g_moveForwardRight : &g_moveForward);
	}
	else if (bDown)
	{
		pDir = bLeft ? &g_moveBackLeft : (bRight ? &g_moveBackRight : &g_moveBack);
	}
	else if (bLeft)
	{
		pDir = &g_moveLeft;
	}
	else if (bRight)
	{
		pDir = &g_moveRight;
	}

	if (pDir != NULL)
	{ // カメラの向きに合わせて移動し、目標の向きを決める
		float fMoveAngle = pCamera->rot.y + (D3DX_PI * pDir->fMoveAngle);		// 移動する方向

		pPlayer->move.x += sinf(fMoveAngle) * pPlayer->fSpd;
		pPlayer->move.z += cosf(fMoveAngle) * pPlayer->fSpd;
		g_fAnglePlayer = pCamera->rot.y + (D3DX_PI * pDir->fFaceAngle);

		return true;
	}

	/*** 左スティック (傾けた量に応じた速さ) ***/
	int nValueH = 0, nValueV = 0;		// スティックの横・縦の入力値
	if (GetJoyThumbValue(&nValueH, &nValueV, JOYTHUMB_LEFT))
	{
		float fStickAngle = atan2f((float)nValueH, (float)nValueV);		// スティックの方向
		float fStickPower = sqrtf(powf((float)nValueH / STICK_MAX_VALUE, 2.0f) + powf((float)nValueV / STICK_MAX_VALUE, 2.0f));	// 傾けた量(0～1)

		pPlayer->move.z += cosf(pCamera->rot.y + fStickAngle) * fStickPower * pPlayer->fSpd;
		pPlayer->move.x += sinf(pCamera->rot.y + fStickAngle) * fStickPower * pPlayer->fSpd;
		g_fAnglePlayer = pCamera->rot.y + (fStickAngle + D3DX_PI);

		return true;
	}

	return false;
}

//==================================================================================
// --- 移動入力に応じて 移動 / 待機 モーションへ切り替える ---
//==================================================================================
static void UpdatePlayerMotionState(Player *pPlayer, bool bMove)
{
	if (bMove == true
		&& pPlayer->motionType != MOTIONTYPE_MOVE
		&& pPlayer->motionTypeBlend != MOTIONTYPE_MOVE
		&& pPlayer->motionType != MOTIONTYPE_JUMP
		&& pPlayer->motionTypeBlend != MOTIONTYPE_JUMP)
	{ // 移動し始めた (ジャンプ中は除く)
		SetMotionType(MOTIONTYPE_MOVE, true, MOTION_BLEND_FRAME);
	}
	else if (bMove == false
		&& pPlayer->motionType != MOTIONTYPE_NEUTRAL
		&& pPlayer->motionTypeBlend != MOTIONTYPE_NEUTRAL
		&& pPlayer->bJump == false
		&& pPlayer->motionType != MOTIONTYPE_LANDING
		&& pPlayer->motionTypeBlend != MOTIONTYPE_LANDING
		&& pPlayer->motionType != MOTIONTYPE_ACTION
		&& pPlayer->motionTypeBlend != MOTIONTYPE_ACTION)
	{ // 止まった (ジャンプ・着地・アクション中は除く)
		SetMotionType(MOTIONTYPE_NEUTRAL, true, MOTION_BLEND_FRAME);
	}
}

//==================================================================================
// --- 目標の向きへ少しずつ回転する ---
// 戻り値 : 回転前の目標の向きとの差
//==================================================================================
static float UpdatePlayerRotation(Player *pPlayer)
{
	/*** 頭のパーツの回転 (現在は速度0のため回らない) ***/
	pPlayer->aModel[1].rotLocal.y += g_fTakecopterSpeed;
	RepairRot(&pPlayer->aModel[1].rotLocal.y, &pPlayer->aModel[1].rotLocal.y);

	/*** 目標の向きとの差の分だけ回転 ***/
	RepairRot(&g_fAnglePlayer, &g_fAnglePlayer);

	float fAngleDiff = g_fAnglePlayer - pPlayer->rot.y;		// 目標の向きとの差
	RepairRot(&fAngleDiff, &fAngleDiff);

	pPlayer->rot.y += fAngleDiff * ROT_FOLLOW_RATE;
	RepairRot(&pPlayer->rot.y, &pPlayer->rot.y);

	/*** 差が十分小さくなったら一致させる ***/
	float fAngleRemain = g_fAnglePlayer - pPlayer->rot.y;		// 回転後の差
	if (fAngleRemain < ROT_SNAP_RANGE && fAngleRemain > -ROT_SNAP_RANGE)
	{
		pPlayer->rot.y = g_fAnglePlayer;
	}

	return fAngleDiff;
}

//==================================================================================
// --- ジャンプ・アクション・オーナメントの投擲と取得 ---
//==================================================================================
static void UpdatePlayerAction(Player *pPlayer, const Camera *pCamera, float fAngleDiff)
{
	/*** ジャンプ (重いほど低くなる) ***/
	if ((GetKeyboardTrigger(DIK_SPACE) || GetJoypadTrigger(JOYKEY_A)) && pPlayer->bJump == false)
	{
		pPlayer->move.y = pPlayer->fJump * GetWeightResistRate(pPlayer);
		pPlayer->bJump = true;

		SetMotionType(MOTIONTYPE_JUMP, true, MOTION_BLEND_FRAME);
		PlaySound(SOUND_LABEL_SE_JUMP);
	}

	/*** アクション(投げる動き) ***/
	if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_RIGHTTRIGGER))
	{
		SetMotionType(MOTIONTYPE_ACTION, true, MOTION_BLEND_FRAME);
	}

	/*** オーナメントを投げる (カメラの向きへ) ***/
	if (GetKeyboardTrigger(DIK_RETURN)
		|| GetJoypadTrigger(JOYKEY_RIGHTTRIGGER)
		|| GetMouseTrigger(MOUSEKEY_LEFT))
	{
		D3DXVECTOR3 vecShot = pCamera->rot;		// 投げる方向
		vecShot.y = RepairRot(vecShot.y + D3DX_PI);
		g_fAnglePlayer = pCamera->rot.y + (fAngleDiff + D3DX_PI);

		ShotOrnament(ThrowOrnament(), vecShot, SHOT_SPEED);
		SetVibration(0, SHOT_VIBRATION_POWER, SHOT_VIBRATION_TIME);
		PlaySound(SOUND_LABEL_SE_SHOT);
	}

	/*** 触れたオーナメントを拾う ***/
	if (g_nCntPlayerOrnament < MAX_CATCH_ORNAMENT)
	{
		int nIdxOrnament = CollisionOrnament(pPlayer->pos);		// 触れたオーナメントの番号
		if (nIdxOrnament != -1)
		{
			CatchOrnament(nIdxOrnament);
		}
	}
}

//==================================================================================
// --- 床・オブジェクト・壁・メッシュフィールドとの当たり判定 ---
//==================================================================================
static void UpdatePlayerCollision(Player *pPlayer)
{
	/*** 地面(高さ0)より下に行かない ***/
	if (pPlayer->pos.y <= 0.0f)
	{
		pPlayer->pos.y = 0.0f;
		pPlayer->move.y = 0.0f;
		LandPlayer(pPlayer);
	}

	/*** オブジェクト ***/
	bool bCollision = false;		// オブジェクトに当たったか
	if (CollisionObject(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move, pPlayer->fHeight, &bCollision) == true)
	{ // オブジェクトの上に乗った
		LandPlayer(pPlayer);
	}

	if (GetMode() != MODE_GAME)
	{ // 壁・床・坂はゲーム画面のみ
		return;
	}

	/*** 壁 ***/
	CollisionWall(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move);

	/*** 床 ***/
	if (CollisionFloor(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move, pPlayer->fHeight) && pPlayer->pos.y > 0.0f)
	{
		pPlayer->move.y = 0.0f;
		LandPlayer(pPlayer);
	}

	/*** メッシュフィールド(坂) ***/
	if (CollisionMeshField(&pPlayer->pos, &pPlayer->posOld, &pPlayer->move))
	{
		LandPlayer(pPlayer);
	}
}

//==================================================================================
// --- ステージの外に出ないように位置を制限する ---
//==================================================================================
static void ClampPlayerStage(Player *pPlayer)
{
	if (GetMode() == MODE_GAME)
	{ // ゲーム画面 : 家の中
		if (pPlayer->pos.z > STAGE_MAX_Z) pPlayer->pos.z = STAGE_MAX_Z;
		if (pPlayer->pos.z < STAGE_MIN_Z) pPlayer->pos.z = STAGE_MIN_Z;
		if (pPlayer->pos.x < STAGE_MIN_X) pPlayer->pos.x = STAGE_MIN_X;
		if (pPlayer->pos.x > STAGE_MAX_X) pPlayer->pos.x = STAGE_MAX_X;

		// リビングの右端の壁
		if (pPlayer->pos.z < LIVING_MAX_Z && pPlayer->pos.x > LIVING_MAX_X) pPlayer->pos.x = LIVING_MAX_X;
		if (pPlayer->pos.x > LIVING_MAX_X && pPlayer->pos.z < KITCHEN_MIN_Z) pPlayer->pos.z = KITCHEN_MIN_Z;
	}
	else if (GetMode() == MODE_TUTORIAL)
	{ // チュートリアル画面 : 四角い範囲
		D3DXVECTOR4 rect = TUTORIAL_STAGE_RECT;		// 移動可能範囲

		if (CollisionBoxZ(rect, pPlayer->pos) == false)
		{ // 範囲外に出たら端に戻す
			if (pPlayer->pos.x <= rect.x)		pPlayer->pos.x = rect.x;
			else if (pPlayer->pos.x >= rect.y)	pPlayer->pos.x = rect.y;

			if (pPlayer->pos.z <= rect.z)		pPlayer->pos.z = rect.z;
			else if (pPlayer->pos.z >= rect.w)	pPlayer->pos.z = rect.w;
		}
	}
}

//==================================================================================
// --- ジャンプ中なら着地させる ---
//==================================================================================
static void LandPlayer(Player *pPlayer)
{
	if (pPlayer->bJump == true)
	{
		pPlayer->bJump = false;
		SetMotionType(MOTIONTYPE_LANDING, true, MOTION_BLEND_FRAME);
	}
}

//==================================================================================
// --- モーションの変更 (着地した瞬間は足元にリングを出す) ---
//==================================================================================
static void SetMotionType(MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend)
{
	Player *pPlayer = &g_player;		// プレイヤー

	if (motionTypeNext < 0 || motionTypeNext >= MOTIONTYPE_MAX)
	{ // モーションの種類が範囲外
		return;
	}

	if (motionTypeNext == MOTIONTYPE_LANDING
		&& pPlayer->motionType != MOTIONTYPE_LANDING
		&& pPlayer->motionTypeBlend != MOTIONTYPE_LANDING)
	{ // 着地した瞬間
		D3DXVECTOR3 posRing = pPlayer->pos;		// リングの位置
		posRing.y += LANDING_RING_HEIGHT;

		SetMeshRing(posRing, VECNULL, 0.0f, 1.0f, 0, LANDING_RING_SPD_IN, LANDING_RING_SPD_OUT, LANDING_RING_LIFE);
	}

	SetCharacterMotionType(pPlayer, motionTypeNext, bBlend, nFrameBlend);
}

//==================================================================================
// --- オーナメントを拾う ---
// 戻り値 : 拾えたか
//==================================================================================
static bool CatchOrnament(int nIdOrnament)
{
	Player *pPlayer = &g_player;		// プレイヤー

	if (g_nCntPlayerOrnament >= MAX_CATCH_ORNAMENT || nIdOrnament == -1)
	{ // これ以上持てない or 番号が無効
		return false;
	}

	for (int nCntCheck = 0; nCntCheck < g_nCntPlayerOrnament; nCntCheck++)
	{
		if (g_aOrnamentShot[nCntCheck] == nIdOrnament)
		{ // 既に持っている
			return false;
		}
	}

	PORNAMENT pOrnament = GetOrnament(nIdOrnament);		// 拾うオーナメント

	g_aOrnamentShot[g_nCntPlayerOrnament] = nIdOrnament;
	if (pOrnament == NULL)
	{
		return false;
	}

	if (g_nCntPlayerOrnament == 0)
	{ // 1つ目 : 手に持つ
		pOrnament->pos = VECNULL;
		SetParentOrnament(nIdOrnament, &pPlayer->aModel[pPlayer->nModelHand].mtxWorld);
	}
	else
	{ // 2つ目以降 : 頭の上に積む
		SetParentOrnament(nIdOrnament, &pPlayer->aModel[pPlayer->nModelHead].mtxWorld);
		pOrnament->pos.y = ORNAMENT_STACK_HEIGHT * g_nCntPlayerOrnament;
	}

	g_nCntPlayerOrnament++;

	/*** 重さを加える ***/
	PORNAMENT_INFO pInfo = GetOrnamentInfo(pOrnament->nType);		// オーナメントの種類の情報
	if (pInfo != NULL)
	{
		pPlayer->fResist += pInfo->fWeight;
	}

	PlaySound(SOUND_LABEL_SE_FIND);
	return true;
}

//==================================================================================
// --- 手に持っているオーナメントを手放し、頭の上の1つ目を手に移す ---
// 戻り値 : 手放したオーナメントの番号 (持っていなければ -1)
//==================================================================================
static int ThrowOrnament(void)
{
	Player *pPlayer = &g_player;					// プレイヤー
	int nIdShotOrnament = g_aOrnamentShot[0];		// 手放すオーナメント

	if (g_nCntPlayerOrnament == 0)
	{ // 何も持っていない
		return -1;
	}

	/*** 持っている順を1つずつ前に詰める ***/
	for (int nCntOrnament = 0; nCntOrnament < MAX_CATCH_ORNAMENT; nCntOrnament++)
	{
		if (nCntOrnament == (MAX_CATCH_ORNAMENT - 1))
		{ // 最後の枠は空きにする
			g_aOrnamentShot[nCntOrnament] = -1;
			continue;
		}

		g_aOrnamentShot[nCntOrnament] = g_aOrnamentShot[nCntOrnament + 1];

		PORNAMENT pOrnament = GetOrnament(g_aOrnamentShot[nCntOrnament]);		// 詰めたオーナメント
		if (pOrnament == NULL)
		{
			continue;
		}

		if (nCntOrnament == 0)
		{ // 新しく手に持つオーナメント : 親を頭から手に変更
			pOrnament->pos = VECNULL;
			pOrnament->bReady = true;
			RemoveParentOrnament(g_aOrnamentShot[nCntOrnament]);
			SetParentOrnament(g_aOrnamentShot[nCntOrnament], &pPlayer->aModel[pPlayer->nModelHand].mtxWorld);
		}
		else
		{ // 頭の上 : 1段下げる
			pOrnament->pos.y = ORNAMENT_STACK_HEIGHT * nCntOrnament;
		}
	}

	/*** 手放した分の重さを減らす ***/
	PORNAMENT pOrnament = GetOrnament(nIdShotOrnament);		// 手放したオーナメント
	if (pOrnament != NULL)
	{
		PORNAMENT_INFO pInfo = GetOrnamentInfo(pOrnament->nType);		// オーナメントの種類の情報
		if (pInfo != NULL)
		{
			pPlayer->fResist -= pInfo->fWeight;
		}
	}

	g_nCntPlayerOrnament--;
	return nIdShotOrnament;
}

//==================================================================================
// --- 持っているオーナメントの重さによる移動の倍率 (重いほど小さい) ---
//==================================================================================
static float GetWeightResistRate(const Player *pPlayer)
{
	return 1.0f / ((pPlayer->fResist * WEIGHT_RESIST) + 1.0f);
}

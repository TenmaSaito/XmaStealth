//==================================================================================
//
// ランキングスコアの処理 [rankingScore.cpp]
// Author : TENMA SAITO
//
// ・ランキングはバイナリファイル(int × 順位数)に保存する
// ・表示時は下の順位から1つずつ、回転しながら画面右から入ってくる
// ・今回のスコアがランクインしていれば点滅させる
// ・R + 右Shift (またはパッドの A + B + X + Y) でランキングをリセット
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "rankingScore.h"
#include "game.h"
#include "input.h"
#include "score.h"
#include "fade.h"
#include "2Dpolygon.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define NUM_PLACE				(6)				// スコアの桁数
#define NUM_WIDTH				(25.0f)			// 数字1つの横幅の半分
#define NUM_HEIGHT				(35.0f)			// 数字1つの縦幅の半分
#define NUM_TEX_WIDTH			(0.1f)			// 数字1つ分のテクスチャ幅 (0～9 の10個並び)
#define MAX_RANKING				(5)				// 表示する順位の数
#define FIRSTMOVE_RANKING		(MAX_RANKING - 1)	// 最初に右から入ってくる順位(一番下)
#define RANKING_START_X			(1380.0f)		// 入ってくる前の位置(画面外の右)
#define RANKING_GOAL_X			(600.0f)		// 入ってきた後の位置
#define RANKING_TOP_Y			(150.0f)		// 1位の1つ上の高さ
#define RANKING_SPACE_Y			(103.0f)		// 順位ごとの間隔
#define RANKING_MOVE_SPEED		(15.0f)			// 入ってくる速さ
#define RANKING_ROT_SPEED		(0.04f)			// 入ってくる間の回転速度
#define BLINK_INTERVAL			(15)			// 点滅の周期(フレーム)
#define BLINK_SPEED				(0.08f)			// 点滅の速さ
#define BLINK_ALPHA_MIN			(0.5f)			// 点滅の最も薄いα値
#define DEFAULT_RANKING_SCORE	(155000)		// ファイルが無い時の既定スコアの単位(1位 = この値 × 順位数)

#define RANKING_TEXTURE			"data/TEXTURE/number003.png"			// 数字テクスチャのパス
#define FILENAME_RANKING_EASY	"data\\RANKING\\Ranking.bin"			// ランキング保存用ファイル名(EASY)
#define FILENAME_RANKING_NORMAL	"data\\RANKING\\Ranking.bin"			// ランキング保存用ファイル名(NORMAL)
#define FILENAME_RANKING_HARD	"data\\RANKING\\Ranking_H.bin"			// ランキング保存用ファイル名(HARD)
#define FILENAME_RESET			"data\\RANKING\\RankingScore_Reset.bin"	// リセット用バイナリファイル名

//**********************************************************************************
// *** ランキング構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;						// 表示位置(最上位の桁の中心)
	D3DXCOLOR col;							// 色
	int nScore;								// スコアの値
	int nRankingScore;						// 順位(0始まり)
	float fRot;								// 回転角度
	bool bAdd;								// 今回追加されたスコアか(点滅させる)
	bool bUse;								// 使われているか
} RANKING, *PRANKING;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static void LoadRankingScore(PRANKING pRanking);
static int CompareScoreDescending(const void *pA, const void *pB);
static void ResetRankingScore(void);
static const char *GetRankingFileName(GAMEDIFFICULTY difficulty);
static D3DXVECTOR3 GetGoalPosRanking(int nRank);
static void SetVertexPosRanking(VERTEX_2D *pVtx, const RANKING *pRanking);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_pTextureRankingScore = NULL;			// テクスチャへのポインタ
static LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffRankingScore = NULL;		// 頂点バッファのポインタ(順位数 × 桁数)
static RANKING g_aRankingScore[GAMEDIFFICULTY_MAX][MAX_RANKING];	// 難易度ごとのランキング
static int g_nScoreCurrent;											// 今回のスコア
static int g_nCounterRankingScore;									// 点滅用カウンター
static int g_nCurrentMoveRankingScore;								// 現在入ってきている順位 (-1 なら全員到着)
static bool g_bScoreReseted;										// この画面でリセット済みか

//==================================================================================
// --- ランキングの初期化処理 ---
//==================================================================================
void InitRankingScore(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();								// デバイス
	PRANKING pRankingTop = &g_aRankingScore[GetGameDifficulty()][0];		// 現在の難易度のランキング
	VERTEX_2D *pVtx = NULL;													// 頂点情報へのポインタ

	/*** 各順位の初期化 (画面外の右で待機) ***/
	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		PRANKING pRanking = &pRankingTop[nCntRank];		// 初期化する順位

		pRanking->pos = D3DXVECTOR3(RANKING_START_X, GetGoalPosRanking(nCntRank).y, 0.0f);
		pRanking->col = D3DXCOLOR_NULL;
		pRanking->nScore = 0;
		pRanking->nRankingScore = 1;
		pRanking->fRot = 0.0f;
		pRanking->bAdd = false;
		pRanking->bUse = false;
	}

	g_bScoreReseted = false;
	g_nCounterRankingScore = 0;
	g_nCurrentMoveRankingScore = FIRSTMOVE_RANKING;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice, RANKING_TEXTURE, &g_pTextureRankingScore);

	/*** 頂点バッファの生成 (順位数 × 桁数) ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * (NUM_PLACE * MAX_RANKING),
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffRankingScore,
		NULL);

	/*** 頂点情報の初期設定 (全桁「0」) ***/
	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++, pVtx += 4 * NUM_PLACE)
	{
		SetVertexPosRanking(pVtx, &pRankingTop[nCntRank]);

		for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++)
		{
			SetDefaultPolygon2D(&pVtx[nCntPlace * 4]);
			pVtx[nCntPlace * 4 + 1].tex.x = NUM_TEX_WIDTH;
			pVtx[nCntPlace * 4 + 3].tex.x = NUM_TEX_WIDTH;
		}
	}

	g_pVtxBuffRankingScore->Unlock();

	/*** 前の画面に応じてランキングを準備 ***/
	if (GetModeExac() == MODE_GOODEND)
	{ // クリア後 : 今回のスコアを追加
		AddRankingScore(GetFinallyScore());
	}
	else if (GetModeExac() == MODE_TITLE || GetModeExac() == MODE_BADEND)
	{ // それ以外 : 読み込むだけ
		LoadRankingScore(pRankingTop);
	}
}

//==================================================================================
// --- ランキングの終了処理 ---
//==================================================================================
void UninitRankingScore(void)
{
	/*** テクスチャ・頂点バッファの破棄 ***/
	RELEASE(g_pTextureRankingScore);
	RELEASE(g_pVtxBuffRankingScore);
}

//==================================================================================
// --- ランキングの更新処理 ---
//==================================================================================
void UpdateRankingScore(void)
{
	PRANKING pRankingTop = &g_aRankingScore[GetGameDifficulty()][0];		// 現在の難易度のランキング
	VERTEX_2D *pVtx = NULL;													// 頂点情報へのポインタ

	/*** ランキングのリセット (1画面につき1回) ***/
	bool bResetKeyboard = (GetKeyboardPress(DIK_R) && GetKeyboardPress(DIK_RSHIFT));		// キーボードのリセット入力
	bool bResetJoypad = (GetJoypadPress(JOYKEY_A) && GetJoypadPress(JOYKEY_B)
		&& GetJoypadPress(JOYKEY_X) && GetJoypadPress(JOYKEY_Y));							// パッドのリセット入力

	if ((bResetKeyboard || bResetJoypad) && g_bScoreReseted == false)
	{
		ResetRankingScore();
	}

	/*** 決定入力で、入ってくる演出を飛ばす ***/
	if (GetKeyboardTrigger(DIK_RETURN) || GetJoypadTrigger(JOYKEY_A) || GetJoypadTrigger(JOYKEY_START))
	{
		for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
		{
			pRankingTop[nCntRank].pos = GetGoalPosRanking(nCntRank);
			pRankingTop[nCntRank].fRot = 0.0f;
		}

		g_nCurrentMoveRankingScore = -1;
	}

	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++, pVtx += 4 * NUM_PLACE)
	{
		PRANKING pRanking = &pRankingTop[nCntRank];		// 更新する順位

		if (g_nCurrentMoveRankingScore == nCntRank)
		{ // 入ってくる途中 : 左へ移動しながら回転
			pRanking->pos.x -= RANKING_MOVE_SPEED;

			if (pRanking->pos.x <= RANKING_GOAL_X)
			{ // 到着したら次の(1つ上の)順位へ
				pRanking->pos.x = RANKING_GOAL_X;
				g_nCurrentMoveRankingScore--;
			}
			else
			{
				pRanking->fRot += RANKING_ROT_SPEED;
			}
		}
		else if ((int)(pRanking->fRot * 10.0f) % (int)(D3DX_PI * 20.0f) != 0)
		{ // 到着後 : ちょうど1周するまで回転を続ける
			pRanking->fRot += RANKING_ROT_SPEED;
		}

		if (pRanking->bAdd == true)
		{ // 今回のスコア : 点滅
			if ((g_nCounterRankingScore % BLINK_INTERVAL) <= BLINK_INTERVAL / 2)
			{
				pRanking->col.a -= BLINK_SPEED;
				if (pRanking->col.a < BLINK_ALPHA_MIN) pRanking->col.a = BLINK_ALPHA_MIN;
			}
			else
			{
				pRanking->col.a += BLINK_SPEED;
				if (pRanking->col.a > 1.0f) pRanking->col.a = 1.0f;
			}
		}

		/*** 頂点座標・頂点カラーの更新 ***/
		SetVertexPosRanking(pVtx, pRanking);

		for (int nCntVtx = 0; nCntVtx < 4 * NUM_PLACE; nCntVtx++)
		{
			pVtx[nCntVtx].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, pRanking->col.a);
		}
	}

	g_pVtxBuffRankingScore->Unlock();

	g_nCounterRankingScore++;
}

//==================================================================================
// --- ランキングの描画処理 ---
//==================================================================================
void DrawRankingScore(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** 頂点バッファ・頂点フォーマット・テクスチャの設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffRankingScore, 0, sizeof(VERTEX_2D));
	pDevice->SetFVF(FVF_VERTEX_2D);
	pDevice->SetTexture(0, g_pTextureRankingScore);

	/*** 全順位・全桁の描画 ***/
	for (int nCntDigit = 0; nCntDigit < (NUM_PLACE * MAX_RANKING); nCntDigit++)
	{
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4 * nCntDigit, 2);
	}
}

//==================================================================================
// --- 各順位のスコアを数字のテクスチャに反映 ---
//==================================================================================
void SetRankingScore(void)
{
	PRANKING pRankingTop = &g_aRankingScore[GetGameDifficulty()][0];		// 現在の難易度のランキング
	VERTEX_2D *pVtx = NULL;													// 頂点情報へのポインタ

	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		int nDivisor = 1;		// 桁を取り出すための除数

		for (int nCntPlace = 1; nCntPlace < NUM_PLACE; nCntPlace++)
		{ // 最上位の桁の除数を求める
			nDivisor *= 10;
		}

		for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++, pVtx += 4, nDivisor /= 10)
		{ // 上の桁から順に数字を設定
			int nDigit = (pRankingTop[nCntRank].nScore / nDivisor) % 10;		// この桁の数字
			float fTexU = NUM_TEX_WIDTH * nDigit;								// この数字のテクスチャ左端

			pVtx[0].tex = D3DXVECTOR2(fTexU, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(fTexU, 1.0f);
			pVtx[3].tex = D3DXVECTOR2(fTexU + NUM_TEX_WIDTH, 1.0f);
		}
	}

	g_pVtxBuffRankingScore->Unlock();
}

//==================================================================================
// --- 今回のスコアの取得 ---
//==================================================================================
int GetRankingScore(void)
{
	return g_nScoreCurrent;
}

//==================================================================================
// --- 今回のスコアをランキングに追加してファイルに保存 ---
//==================================================================================
void AddRankingScore(int nScore)
{
	PRANKING pRankingTop = &g_aRankingScore[GetGameDifficulty()][0];		// 現在の難易度のランキング
	int aScore[MAX_RANKING + 1] = {};										// 既存のランキング + 今回のスコア

	g_nScoreCurrent = nScore;

	/*** 既存のランキングと今回のスコアを並べて、降順に並び替え ***/
	LoadRankingScore(pRankingTop);

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		aScore[nCntRank] = pRankingTop[nCntRank].nScore;
	}

	aScore[MAX_RANKING] = g_nScoreCurrent;

	qsort(&aScore[0], MAX_RANKING + 1, sizeof(int), CompareScoreDescending);

	/*** 上位だけを新しいランキングにする ***/
	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		pRankingTop[nCntRank].nRankingScore = nCntRank;
		pRankingTop[nCntRank].nScore = aScore[nCntRank];
	}

	/*** 今回のスコアがランクインしていれば点滅させる ***/
	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		if (pRankingTop[nCntRank].nScore == g_nScoreCurrent)
		{
			pRankingTop[nCntRank].bAdd = true;
			break;
		}
	}

	/*** ファイルに保存 ***/
	FILE *pFile = fopen(GetRankingFileName(GetGameDifficulty()), "wb");		// ランキングファイル
	if (pFile == NULL)
	{
		MessageBox(GetActiveWindow(), "ランキングデータの書き出しに失敗！", "警告", MB_ICONWARNING);
		return;
	}

	fwrite(&aScore[0], sizeof(int), MAX_RANKING, pFile);
	fclose(pFile);
}

//==================================================================================
// --- ランキングをファイルから読み込む (無ければ既定のランキング) ---
//==================================================================================
static void LoadRankingScore(PRANKING pRanking)
{
	int aBuffer[MAX_RANKING] = {};		// 読み込んだスコア

	FILE *pFile = fopen(GetRankingFileName(GetGameDifficulty()), "rb");		// ランキングファイル
	if (pFile == NULL)
	{ // 読み込みに失敗した場合、既定のランキングを使う
		for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
		{
			aBuffer[nCntRank] = DEFAULT_RANKING_SCORE * (MAX_RANKING - nCntRank);
		}
	}
	else
	{
		fread(aBuffer, sizeof(int), MAX_RANKING, pFile);
		fclose(pFile);
	}

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		pRanking[nCntRank].nRankingScore = nCntRank;
		pRanking[nCntRank].nScore = aBuffer[nCntRank];
		if (pRanking[nCntRank].nScore == 0) pRanking[nCntRank].bUse = false;
	}
}

//==================================================================================
// --- qsort 用の比較関数 (降順) ---
//==================================================================================
static int CompareScoreDescending(const void *pA, const void *pB)
{
	int nScoreA = *(const int*)pA;		// 比較するスコアA
	int nScoreB = *(const int*)pB;		// 比較するスコアB

	if (nScoreA < nScoreB) return 1;
	if (nScoreA > nScoreB) return -1;
	return 0;
}

//==================================================================================
// --- ランキングのリセット (リセット用ファイルで上書き) ---
//==================================================================================
static void ResetRankingScore(void)
{
	BOOL bSucceed = CopyFileA(FILENAME_RESET, GetRankingFileName(GetGameDifficulty()), FALSE);		// 上書きに成功したか

	if (bSucceed == FALSE)
	{
		GenerateMessageBox(MB_ICONERROR, "FAILED", "Reset has Failed.");
	}
	else
	{
		GenerateMessageBox(MB_ICONINFORMATION, "SUCCEEDED", "Score has Reseted.");
	}

	g_bScoreReseted = true;
}

//==================================================================================
// --- 難易度に対応するランキングファイル名の取得 ---
//==================================================================================
static const char *GetRankingFileName(GAMEDIFFICULTY difficulty)
{
	switch (difficulty)
	{
	case GAMEDIFFICULTY_EASY:	return FILENAME_RANKING_EASY;
	case GAMEDIFFICULTY_HARD:	return FILENAME_RANKING_HARD;
	default:					return FILENAME_RANKING_NORMAL;
	}
}

//==================================================================================
// --- 指定した順位の最終的な表示位置 ---
//==================================================================================
static D3DXVECTOR3 GetGoalPosRanking(int nRank)
{
	return D3DXVECTOR3(RANKING_GOAL_X, RANKING_TOP_Y + (RANKING_SPACE_Y * (nRank + 1)), 0.0f);
}

//==================================================================================
// --- 1順位分(全桁)の頂点座標を、現在の位置・回転角度で設定 ---
//==================================================================================
static void SetVertexPosRanking(VERTEX_2D *pVtx, const RANKING *pRanking)
{
	for (int nCntPlace = 0; nCntPlace < NUM_PLACE; nCntPlace++, pVtx += 4)
	{
		D3DXVECTOR3 posPlace = D3DXVECTOR3(pRanking->pos.x + ((NUM_WIDTH * 2.0f) * nCntPlace), pRanking->pos.y, 0.0f);	// この桁の中心

		RollPolygon(pVtx, posPlace, NUM_WIDTH, NUM_HEIGHT, pRanking->fRot, 1);
	}
}

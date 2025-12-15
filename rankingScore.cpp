//================================================================================================================
//
// DirectXのランキング処理 [ranking.cpp]
// Author : TENMA
//
//================================================================================================================
//*************************************************************************************************
//*** インクルードファイル ***
//*************************************************************************************************
#include "rankingScore.h"
#include "game.h"
#include "input.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define NUM_PLACE			(2)				// スコアの桁数
#define NUM_WIDTH			(25)			// 一桁の横幅
#define NUM_HEIGHT			(35)			// 一桁の縦幅
#define MAX_RANKING			(5)				// 表示する順位の数
#define FIRSTMOVE_RANKING	(4)				// 最初に右から出て来る順位
#define MOVE_SPD			D3DXVECTOR3(15.0f,0.0f,0.0f)								// 移動する速さ
#define GOAL_POS			D3DXVECTOR3(600.0f, 150.0f + (103.0f * (nCntRankingScore + 1)), 0.0f)		// 最終的なスコアの位置
#define FILENAME_RANKING_EASY	"data\\bin\\RANKING\\Ranking.bin"					// ランキング保存用ファイル名
#define FILENAME_RANKING_NORMAL	"data\\bin\\RANKING\\Ranking.bin"					// ランキング保存用ファイル名(NORMAL)
#define FILENAME_RANKING_HARD	"data\\bin\\RANKING\\Ranking_H.bin"					// ランキング保存用ファイル名(HARD)
#define FILENAME_RESET			"data\\bin\\RANKING\\RankingScore_Reset.bin"		// リセット用バイナリファイル名
#define RESET_COMMAND_NUM	(7)				// スコアリセット時に入力するキーの合計数

//**********************************************************************************
//*** ランキング構造体 ***
//**********************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;						// スコア表示の位置
	D3DXCOLOR col;							// 色
	int nScore;								// スコアの値
	int nRankingScore;						// 順位
	float fRot;								// 回転角度
	bool bAdd;								// 追加されたスコアか
	bool bUse;								// 使われているか
	bool bMove;								// 画面内に入ってくるか
}RANKING, *PRANKING;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************
void CheckRankingScore(PRANKING pRankingScore);
int Compare(const void* a, const void* b);
void ResetRankingScore(void);

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_pTextureRankingScore = NULL;						// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffRankingScore = NULL;						// 頂点バッファのポインタ
RANKING g_aRankingScore[GAMEDIFFICULTY_MAX][MAX_RANKING];					// ランキング構造体の宣言
int g_nScoreCurrent;													// 今回のスコア
int g_nCounterRankingScore;													// 汎用カウンター
int g_nCurrentMoveRankingScore;												// 現在動いている順位
char g_aRankingScoreFileName[STRING_MAX];									// ランキングのファイル名
bool g_aTrigger[RESET_COMMAND_NUM];										// 指定のキーが入力されたか

//================================================================================================================
// ランキングの初期化処理
//================================================================================================================
void InitRankingScore(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int nCntRankingScore;
	PRANKING pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	// 構造体の初期化
	for (nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		pRankingScore->pos = D3DXVECTOR3(1380.0f, 150.0f + (103.0f * (nCntRankingScore + 1)), 0.0f);
		pRankingScore->col = D3DXCOLOR_NULL;
		pRankingScore->nScore = 0;
		pRankingScore->nRankingScore = 1;
		pRankingScore->fRot = 0.0f;
		pRankingScore->bAdd = false;
		pRankingScore->bUse = false;
	}

	switch (GetGameDifficulty())
	{
	case GAMEDIFFICULTY_EASY:

		strcpy(g_aRankingScoreFileName, FILENAME_RANKING_EASY);

		break;

	case GAMEDIFFICULTY_NORMAL:

		strcpy(g_aRankingScoreFileName, FILENAME_RANKING_NORMAL);

		break;

	case GAMEDIFFICULTY_HARD:

		strcpy(g_aRankingScoreFileName, FILENAME_RANKING_HARD);

		break;

	default:

		strcpy(g_aRankingScoreFileName, FILENAME_RANKING_NORMAL);

		break;
	}

	g_nCounterRankingScore = 0;
	g_nCurrentMoveRankingScore = FIRSTMOVE_RANKING;

	/*** テクスチャの読み込み ***/
	D3DXCreateTextureFromFile(pDevice,
		"data\\TEXTURE\\UI\\UI_STOCKNUM.png",
		&g_pTextureRankingScore);

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * (NUM_PLACE * MAX_RANKING),
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffRankingScore,
		NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	for (nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		for (int nCntScore = 0; nCntScore < NUM_PLACE; nCntScore++)
		{
			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntScore) - NUM_WIDTH;
			pVtx[0].pos.y = pRankingScore->pos.y - NUM_HEIGHT;
			pVtx[0].pos.z = 0.0f;
	
			pVtx[1].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntScore) + NUM_WIDTH;
			pVtx[1].pos.y = pRankingScore->pos.y - NUM_HEIGHT;
			pVtx[1].pos.z = 0.0f;
	
			pVtx[2].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntScore) - NUM_WIDTH;
			pVtx[2].pos.y = pRankingScore->pos.y + NUM_HEIGHT;
			pVtx[2].pos.z = 0.0f;
	
			pVtx[3].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntScore) + NUM_WIDTH;
			pVtx[3].pos.y = pRankingScore->pos.y + NUM_HEIGHT;
			pVtx[3].pos.z = 0.0f;
	
			/*** rhwの設定 ***/
			pVtx[0].rhw = 1.0f;
			pVtx[1].rhw = 1.0f;
			pVtx[2].rhw = 1.0f;
			pVtx[3].rhw = 1.0f;
	
			/*** 頂点カラー設定 ***/
			pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	
			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(0.1f, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
			pVtx[3].tex = D3DXVECTOR2(0.1f, 1.0f);
	
			pVtx += 4;
		}
	}
	
	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffRankingScore->Unlock();
	
	if (GetModeExac() == MODE_GOODEND)
	{
		AddRankingScore(0);
	}
	else if (GetModeExac() == MODE_TITLE)
	{
		CheckRankingScore(&g_aRankingScore[GetGameDifficulty()][0]);
	}
}

//================================================================================================================
// ランキングの終了処理
//================================================================================================================
void UninitRankingScore(void)
{
	/*** テクスチャの破棄 ***/
	if (g_pTextureRankingScore != NULL)
	{
		g_pTextureRankingScore->Release();
		g_pTextureRankingScore = NULL;
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffRankingScore != NULL)
	{
		g_pVtxBuffRankingScore->Release();
		g_pVtxBuffRankingScore = NULL;
	}
}

//================================================================================================================
// ランキングの更新処理
//================================================================================================================
void UpdateRankingScore(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	PRANKING pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	/*** ランキングをリセット ***/
	if ((GetKeyboardPress(DIK_R) == true
		&& GetKeyboardPress(DIK_RSHIFT) == true)
		||(GetJoypadPress(JOYKEY_A) == true
			&& GetJoypadPress(JOYKEY_B) == true
			&& GetJoypadPress(JOYKEY_X) == true
			&& GetJoypadPress(JOYKEY_Y) == true))
	{
		ResetRankingScore();
	}

	/*** ランキングを即時表示 ***/
	if (GetKeyboardTrigger(DIK_RETURN) == true
		|| GetJoypadTrigger(JOYKEY_A) == true
		|| GetJoypadTrigger(JOYKEY_START) == true)
	{
		for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
		{
			pRankingScore->pos = GOAL_POS;
			pRankingScore->fRot = 0.0f;
		}

		g_nCurrentMoveRankingScore = -1;
		pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		if (g_nCurrentMoveRankingScore == nCntRankingScore)
		{
			pRankingScore->pos -= MOVE_SPD;
			if (pRankingScore->pos.x <= GOAL_POS.x)
			{
				pRankingScore->pos.x = GOAL_POS.x;
				g_nCurrentMoveRankingScore--;
			}
			else
			{
				pRankingScore->fRot += 0.04f;
			}
		}
		else
		{
			if ((int)(pRankingScore->fRot * 10.0f) % (int)(D3DX_PI * 20.0f) != 0)
			{
				pRankingScore->fRot += 0.04f;
			}
		}

		if (pRankingScore->bAdd == true)
		{
			if ((g_nCounterRankingScore % 15) <= 7)
			{
				pRankingScore->col.a -= 0.08f;
				if (pRankingScore->col.a < 0.5f)
				{
					pRankingScore->col.a = 0.5f;
				}
			}
			else
			{
				pRankingScore->col.a += 0.08f;
				if (pRankingScore->col.a > 1.0f)
				{
					pRankingScore->col.a = 1.0f;
				}
			}
		}

		for (int nCntCol = 0; nCntCol < NUM_PLACE; nCntCol++)
		{
			/*** 頂点座標の設定の設定 ***/
			pVtx[0].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntCol) - NUM_WIDTH;
			pVtx[0].pos.y = pRankingScore->pos.y - NUM_HEIGHT;
			pVtx[0].pos.z = 0.0f;

			pVtx[1].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntCol) + NUM_WIDTH;
			pVtx[1].pos.y = pRankingScore->pos.y - NUM_HEIGHT;
			pVtx[1].pos.z = 0.0f;

			pVtx[2].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntCol) - NUM_WIDTH;
			pVtx[2].pos.y = pRankingScore->pos.y + NUM_HEIGHT;
			pVtx[2].pos.z = 0.0f;

			pVtx[3].pos.x = pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntCol) + NUM_WIDTH;
			pVtx[3].pos.y = pRankingScore->pos.y + NUM_HEIGHT;
			pVtx[3].pos.z = 0.0f;

			RollPolygon(pVtx, D3DXVECTOR3(pRankingScore->pos.x + ((NUM_WIDTH * 2) * nCntCol), pRankingScore->pos.y, 0.0f), NUM_WIDTH, NUM_HEIGHT, pRankingScore->fRot, 1.0f);

			/*** 頂点カラー設定 ***/
			pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, pRankingScore->col.a);
			pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, pRankingScore->col.a);
			pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, pRankingScore->col.a);
			pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, pRankingScore->col.a);

			pVtx += 4;
		}

		/*** 頂点バッファの設定を終了 ***/
		g_pVtxBuffRankingScore->Unlock();
	}

	g_nCounterRankingScore++;
}

//================================================================================================================
// ランキングの描画処理
//================================================================================================================
void DrawRankingScore(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	int nCntRankingScore;

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffRankingScore, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	/*** テクスチャの設定 ***/
	pDevice->SetTexture(0, g_pTextureRankingScore);

	for (nCntRankingScore = 0; nCntRankingScore < (NUM_PLACE * MAX_RANKING); nCntRankingScore++)
	{
		/*** ポリゴンの描画 ***/
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
			4 * nCntRankingScore,							// 描画する最初の頂点インデックス
			2);											// 描画するプリミティブの数
	}
}

//================================================================================================================
// ランキングの表示処理
//================================================================================================================
void SetRankingScore(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	PRANKING pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];
	int aTexU[MAX_RANKING][NUM_PLACE];				//各桁の数字を収納

	/*** 値を各桁に分解 ***/
	for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++,pRankingScore++)
	{
		for (int nCntScore = 0; nCntScore < NUM_PLACE; nCntScore++)
		{
			aTexU[nCntRankingScore][nCntScore] = pRankingScore->nScore % (int)powf(10.0f, (float)(NUM_PLACE - nCntScore)) / (int)powf(10.0f, (float)(NUM_PLACE - nCntScore) - 1.0f);
		}
	}

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffRankingScore->Lock(0, 0, (void**)&pVtx, 0);

	pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	/*** 分解した値を各桁で描画 ***/
	for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		for (int nCntScore = 0; nCntScore < NUM_PLACE; nCntScore++)
		{
			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2((0.1f * aTexU[nCntRankingScore][nCntScore]), 0.0f);
			pVtx[1].tex = D3DXVECTOR2((0.1f * aTexU[nCntRankingScore][nCntScore]) + 0.1f, 0.0f);
			pVtx[2].tex = D3DXVECTOR2((0.1f * aTexU[nCntRankingScore][nCntScore]), 1.0f);
			pVtx[3].tex = D3DXVECTOR2((0.1f * aTexU[nCntRankingScore][nCntScore]) + 0.1f, 1.0f);

			pVtx += 4;
		}
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffRankingScore->Unlock();
}

//================================================================================================================
// ランキングの取得
//================================================================================================================
int GetRankingScore(void)
{
	return g_nScoreCurrent;
}

//================================================================================================================
// ランキングの追加処理
//================================================================================================================
void AddRankingScore(int nScore)
{
	PRANKING pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];
	FILE* pFile;
	HWND hWnd = GetActiveWindow();
	int aScore[MAX_RANKING + 1] = {};
	int nCntRankingScore;

	g_nScoreCurrent = nScore;

	CheckRankingScore(pRankingScore);
	for (nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		aScore[nCntRankingScore] = pRankingScore->nScore;
	}

	aScore[nCntRankingScore] = g_nScoreCurrent;

	qsort(&aScore[0], MAX_RANKING + 1, sizeof(int), Compare);

	pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	for (nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		pRankingScore->nRankingScore = nCntRankingScore;
		pRankingScore->nScore = aScore[nCntRankingScore];
	}

	pRankingScore = &g_aRankingScore[GetGameDifficulty()][0];

	for (nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		if (pRankingScore->nScore == g_nScoreCurrent)
		{
			pRankingScore->bAdd = true;
			break;
		}
	}

	pFile = fopen(&g_aRankingScoreFileName[0], "wb");
	if (pFile == NULL) 
	{
		MessageBox(hWnd, "ランキングデータの書き出しに失敗！", "警告", MB_ICONWARNING);
		return;
	}

	fwrite(&aScore[0], sizeof(int), MAX_RANKING, pFile);

	fclose(pFile);
}

//================================================================================================================
// ランキングの読み込み処理
//================================================================================================================
void CheckRankingScore(PRANKING pRankingScore)
{
	FILE* pFile;
	HWND hWnd = NULL;
	int aBuffer[MAX_RANKING];		// スコアの一時保管場所

	pFile = fopen(&g_aRankingScoreFileName[0], "rb");
	if (pFile == NULL) 
	{ 
		// ファイルの読み込みに失敗した場合、既定のランキングデータを代入
		for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
		{
			pRankingScore->nRankingScore = nCntRankingScore;
			pRankingScore->nScore = 155000 * (MAX_RANKING - nCntRankingScore);
			if (pRankingScore->nScore == 0) pRankingScore->bUse = false;
		}

		return;
	}

	fread(aBuffer, sizeof(int), MAX_RANKING, pFile);
	fclose(pFile);

	// 取得したランキングデータを代入
	for (int nCntRankingScore = 0; nCntRankingScore < MAX_RANKING; nCntRankingScore++, pRankingScore++)
	{
		pRankingScore->nRankingScore = nCntRankingScore;
		pRankingScore->nScore = aBuffer[nCntRankingScore];
		if (pRankingScore->nScore == 0) pRankingScore->bUse = false;
	}
}

//================================================================================================================
// 比較関数
//================================================================================================================
int Compare(const void* a, const void* b)
{
	int ret;
	const int* p_int1 = (int *)a;
	const int* p_int2 = (int *)b;

	if (*p_int1 < *p_int2) {
		ret = 1;
	}
	else if (*p_int1 > *p_int2) {
		ret = -1;
	}
	else {
		ret = 0;
	}

	return ret;
}

//================================================================================================================
// ランキングのリセット処理
//================================================================================================================
void ResetRankingScore(void)
{
	HWND hWnd; 
	BOOL BSUCCEED;

	switch (GetGameDifficulty())
	{
	case GAMEDIFFICULTY_EASY:

		BSUCCEED = CopyFileA(FILENAME_RESET, FILENAME_RANKING_EASY, FALSE);		// リセット用のバイナリファイルで現在のスコアファイルを上書き

		break;

	case GAMEDIFFICULTY_NORMAL:

		BSUCCEED = CopyFileA(FILENAME_RESET, FILENAME_RANKING_EASY, FALSE);		// リセット用のバイナリファイルで現在のスコアファイルを上書き

		break;

	case GAMEDIFFICULTY_HARD:

		BSUCCEED = CopyFileA(FILENAME_RESET, FILENAME_RANKING_EASY, FALSE);		// リセット用のバイナリファイルで現在のスコアファイルを上書き

		break;

	default:

		BSUCCEED = FALSE;

		break;
	}

#ifdef _DEBUG
	if (BSUCCEED == FALSE)
	{
		MessageBox(GetHandleWindow(), "FAILED", "ERORR", MB_ICONERROR);
	}
#endif
}
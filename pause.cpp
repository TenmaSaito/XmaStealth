//================================================================================================================
//
// DirectXのポーズ画面表示用処理 [pause.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "pause.h"
#include "fade.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define PAUSE_ICON_WIDTH		(200.0f)		// ポーズアイコンの幅
#define PAUSE_ICON_HEIGHT		(200.0f)		// ポーズアイコンの高さ

//*************************************************************************************************
//*** ポーズアイコンの種類 ***
//*************************************************************************************************
typedef enum
{
	PAUSE_TYPE_BLACKBG = 0,			// ポーズ画面の黒い半透明の背景
	PAUSE_TYPE_CONTINUE,			// 再開
	PAUSE_TYPE_REPLAY,				// リプレイ
	PAUSE_TYPE_EXIT,				// 終了
	PAUSE_TYPE_MAX
}PAUSE_TYPE;

//*************************************************************************************************
//*** スター数情報構造体 ***
//*************************************************************************************************
typedef struct
{
	D3DXVECTOR3 pos;		// ストック情報の位置
	float fWidth;			// 横幅
	float fHeight;			// 縦幅
	bool bUse;				// 使われているか
}PAUSE_ICON;

//*************************************************************************************************
//*** プロトタイプ宣言 ***
//*************************************************************************************************
void DoPauseSwitch(PAUSE_TYPE type);

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
LPDIRECT3DTEXTURE9		g_apTexturePause[PAUSE_TYPE_MAX] = {};		// テクスチャへのポインタ
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffPause = NULL;						// 頂点バッファのポインタ
PAUSE_ICON g_aPauseIcon[PAUSE_TYPE_MAX];		// ポーズアイコン
PAUSE_TYPE g_CurrentPauseType;					// 現在のポーズの種類
bool g_bShowPause;								// 描画状態
bool g_bSkipStart;								// STARTボタンの持続入力によるポーズスキップ阻止変数
const D3DXVECTOR3 g_aIconPos[] =				// 各ポーズアイコンの位置
{
	D3DXVECTOR3(0.0f,0.0f,0.0f),				// 位置は原点
	D3DXVECTOR3(250.0f - (PAUSE_ICON_WIDTH * 0.5f), 300.0f, 0.0f),
	D3DXVECTOR3(640.0f - (PAUSE_ICON_WIDTH * 0.5f), 300.0f, 0.0f),
	D3DXVECTOR3(1030.0f - (PAUSE_ICON_WIDTH * 0.5f), 300.0f, 0.0f)
};

const char* g_aPauseIconTexture[] =				// 各ポーズアイコンのテクスチャ
{
	"",											// DO NOT WRITE PAUSE TEXTURE PATH IN THIS CHAR*
	"data\\TEXTURE\\PAUSE\\CONTINUE.png",
	"data\\TEXTURE\\PAUSE\\REPLAY.png",
	"data\\TEXTURE\\PAUSE\\EXIT.png"
};

//================================================================================================================
// --- ポーズ画面表示の初期化 ---
//================================================================================================================
void InitPause(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ

	/*** ポーズアイコンの初期化 ***/
	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		g_aPauseIcon[nCntPause].pos = g_aIconPos[nCntPause];
		g_aPauseIcon[nCntPause].fWidth = PAUSE_ICON_WIDTH;
		g_aPauseIcon[nCntPause].fHeight = PAUSE_ICON_HEIGHT;
		g_aPauseIcon[nCntPause].bUse = false;

		if (g_aPauseIconTexture[nCntPause] != NULL)
		{
			/*** テクスチャの読み込み ***/
			D3DXCreateTextureFromFile(pDevice,
									g_aPauseIconTexture[nCntPause],
									&g_apTexturePause[nCntPause]);
		}
	}

	/*** 最初のポーズアイコンの種類を指定 ***/
	g_CurrentPauseType = PAUSE_TYPE_CONTINUE;
	g_aPauseIcon[g_CurrentPauseType].bUse = true;
	g_bShowPause = true;
	g_bSkipStart = false;

	/*** 頂点バッファの生成 ***/
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * (PAUSE_TYPE_MAX),		// ポーズアイコンの種類 + 黒い背景
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffPause,
		NULL);

	/*** 頂点バッファの設定 ***/
	g_pVtxBuffPause->Lock(0, 0, (void**)&pVtx, 0);

	/*** --- 半透明の黒い背景を設定 --- ***/

	/*** 頂点座標の設定の設定 ***/
	pVtx[0].pos.x = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.x;
	pVtx[0].pos.y = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.y;
	pVtx[0].pos.z = 0.0f;

	pVtx[1].pos.x = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.x + SCREEN_WIDTH;
	pVtx[1].pos.y = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.y;
	pVtx[1].pos.z = 0.0f;

	pVtx[2].pos.x = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.x;
	pVtx[2].pos.y = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.y + SCREEN_HEIGHT;
	pVtx[2].pos.z = 0.0f;

	pVtx[3].pos.x = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.x + SCREEN_WIDTH;
	pVtx[3].pos.y = g_aPauseIcon[PAUSE_TYPE_BLACKBG].pos.y + SCREEN_HEIGHT;
	pVtx[3].pos.z = 0.0f;

	/*** rhwの設定 ***/
	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	/*** 頂点カラー設定 ***/
	pVtx[1].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.7f);
	pVtx[2].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.7f);
	pVtx[3].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.7f);
	pVtx[0].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.7f);

	pVtx += 4;

	/*** --- ポーズアイコンを設定 --- ***/
	for (int nCntPause = PAUSE_TYPE_CONTINUE; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		/*** 頂点座標の設定の設定 ***/
		pVtx[0].pos.x = g_aPauseIcon[nCntPause].pos.x;
		pVtx[0].pos.y = g_aPauseIcon[nCntPause].pos.y;
		pVtx[0].pos.z = 0.0f;

		pVtx[1].pos.x = g_aPauseIcon[nCntPause].pos.x + g_aPauseIcon[nCntPause].fWidth;
		pVtx[1].pos.y = g_aPauseIcon[nCntPause].pos.y;
		pVtx[1].pos.z = 0.0f;

		pVtx[2].pos.x = g_aPauseIcon[nCntPause].pos.x;
		pVtx[2].pos.y = g_aPauseIcon[nCntPause].pos.y + g_aPauseIcon[nCntPause].fHeight;
		pVtx[2].pos.z = 0.0f;

		pVtx[3].pos.x = g_aPauseIcon[nCntPause].pos.x + g_aPauseIcon[nCntPause].fWidth;
		pVtx[3].pos.y = g_aPauseIcon[nCntPause].pos.y + g_aPauseIcon[nCntPause].fHeight;
		pVtx[3].pos.z = 0.0f;

		/*** rhwの設定 ***/
		pVtx[0].rhw = 1.0f;
		pVtx[1].rhw = 1.0f;
		pVtx[2].rhw = 1.0f;
		pVtx[3].rhw = 1.0f;

		/*** 頂点カラー設定 ***/
		pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
		pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
		pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
		pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);

		/*** テクスチャ座標の設定 ***/
		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pVtx += 4;
	}

	/*** 頂点バッファの設定を終了 ***/
	g_pVtxBuffPause->Unlock();
}

//================================================================================================================
// --- ポーズ画面表示の終了 ---
//================================================================================================================
void UninitPause(void)
{
	/*** テクスチャの破棄 ***/
	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		if (g_apTexturePause[nCntPause] != NULL)
		{
			g_apTexturePause[nCntPause]->Release();
			g_apTexturePause[nCntPause] = NULL;
		}
	}

	/*** 頂点バッファの破棄 ***/
	if (g_pVtxBuffPause != NULL)
	{
		g_pVtxBuffPause->Release();
		g_pVtxBuffPause = NULL;
	}
}

//================================================================================================================
// --- ポーズ画面表示の更新 ---
//================================================================================================================
void UpdatePause(void)
{
	VERTEX_2D* pVtx;					// 頂点情報へのポインタ
	int nPauseType = 0;

	/*** F10を押したら ***/
	if (GetKeyboardTrigger(DIK_F10))
	{
		/*** 描画状態を変更 ***/
		g_bShowPause = g_bShowPause ^ true;
	}

	if (GetEnablePause() && g_bShowPause)
	{
		/*** フェード時は操作不能にする ***/
		if (GetFade() == FADE_NONE)
		{
			/*** 右入力をしたとき ***/
			if (GetKeyboardRepeat(DIK_D)
				|| GetJoypadRepeat(JOYKEY_RIGHT, 20)
				|| GetJoyThumbRepeat(JOYTHUMB_LX_UP, 20))
			{
				/*** 選択音 ***/
				PlaySound(SOUND_LABEL_SE_ENTER);

				nPauseType = g_CurrentPauseType;			// 現在のポーズアイコンを取得しint変更し値の変更を許可
				nPauseType++;								// 値をインクリメント
				/*** インデックスの範囲を確認 ***/
				if (FAILED(CheckIndex(PAUSE_TYPE_MAX, nPauseType, PAUSE_TYPE_CONTINUE)))
				{ // 範囲外(最大値よりも大きい)の場合
					nPauseType = PAUSE_TYPE_CONTINUE;			// 値を最初の選択肢に設定
				}

				g_CurrentPauseType = (PAUSE_TYPE)nPauseType;	// 変更後の値をインデックスの型にキャストし代入
			}
			/*** 左入力をしたとき ***/
			else if (GetKeyboardRepeat(DIK_A)
				|| GetJoypadRepeat(JOYKEY_LEFT, 20)
				|| GetJoyThumbRepeat(JOYTHUMB_LX_DOWN, 20))
			{
				/*** 選択音 ***/
				PlaySound(SOUND_LABEL_SE_ENTER);

				nPauseType = g_CurrentPauseType;				// 現在のポーズアイコンを取得しint変更し値の変更を許可
				nPauseType--;									// 値をデクリメント
				/*** インデックスの範囲を確認 ***/
				if (FAILED(CheckIndex(PAUSE_TYPE_MAX, nPauseType, PAUSE_TYPE_CONTINUE)))
				{ // 範囲外(最大値よりも大きい)の場合
					nPauseType = PAUSE_TYPE_EXIT;				// 値を最後の選択肢に設定
				}

				g_CurrentPauseType = (PAUSE_TYPE)nPauseType;	// 変更後の値をインデックスの型にキャストし代入
			}

			/*** 決定ボタンを押したとき ***/
			if (GetKeyboardTrigger(DIK_RETURN)
				|| GetJoypadTrigger(JOYKEY_A))
			{
				/*** 決定音 ***/
				PlaySound(SOUND_LABEL_SE_ENTER);

				/*** 現在のポーズタイプによる処理を実行 ***/
				DoPauseSwitch(g_CurrentPauseType);
			}
		}

		/*** 頂点バッファの設定 ***/
		g_pVtxBuffPause->Lock(0, 0, (void**)&pVtx, 0);

		pVtx += 4;			// 黒い背景は更新しないため、ポインタをずらす

		/*** 各選択肢の更新 ***/
		for (int nCntPause = PAUSE_TYPE_CONTINUE; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
		{
			/*** ポーズアイコンが選ばれているか確認 ***/
			if (nCntPause == g_CurrentPauseType)
			{ // 選ばれていれば
				/*** 頂点カラー設定 ***/
				pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else
			{
				/*** 頂点カラー設定 ***/
				pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
				pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
				pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
				pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
			}

			/*** テクスチャ座標の設定 ***/
			pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
			pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
			pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
			pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

			pVtx += 4;
		}

		/*** 頂点バッファの設定を終了 ***/
		g_pVtxBuffPause->Unlock();
	}
	else
	{
		g_CurrentPauseType = PAUSE_TYPE_CONTINUE;
	}
}

//================================================================================================================
// --- ポーズ画面表示の描画 ---
//================================================================================================================
void DrawPause(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	/*** 頂点バッファをデータストリームに設定 ***/
	pDevice->SetStreamSource(0, g_pVtxBuffPause, 0, sizeof(VERTEX_2D));

	/*** 頂点フォーマットの設定 ***/
	pDevice->SetFVF(FVF_VERTEX_2D);

	for (int nCntPause = 0; nCntPause < PAUSE_TYPE_MAX; nCntPause++)
	{
		/*** テクスチャの設定 ***/
		if (nCntPause == PAUSE_TYPE_BLACKBG)
		{ 
			pDevice->SetTexture(0, NULL);
		}
		else
		{ 
			pDevice->SetTexture(0, g_apTexturePause[nCntPause]);
		}

		/*** ポーズ画面の描画が有効なら ***/
		if (GetEnablePause() && g_bShowPause)
		{
			/*** ポリゴンの描画 ***/
			pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,		// プリミティブの種類
				4 * nCntPause,								// 描画する最初の頂点インデックス
				2);											// 描画するプリミティブの数
		}
	}
}

//================================================================================================================
// --- 各ポーズアイコンの選択時処理 ---
//================================================================================================================
void DoPauseSwitch(PAUSE_TYPE type)
{
	g_bSkipStart = false;

	/*** ポーズアイコンの種類によって処理を変更 ***/
	switch (type)
	{
	// CONTINUEの場合
	case PAUSE_TYPE_CONTINUE:
		
		/*** ポーズを解除し、ゲームを再開 ***/
		SetEnablePause(false);
		// g_CurrentPauseType = PAUSE_TYPE_CONTINUE;

		break;

	// RETRYの場合
	case PAUSE_TYPE_REPLAY:

		/*** 現在のステージを保存する ***/
		SetStageExac(0);

		/*** ポーズを解除し、ゲームを再開 ***/
		SetFade(MODE_GAME, FADE_TYPE_NORMAL, 60);
		SetEnablePause(false);
		// g_CurrentPauseType = PAUSE_TYPE_CONTINUE;

		break;

	// EXITの場合
	case PAUSE_TYPE_EXIT:
		
		/*** 現在のステージ番号をリセットする ***/
		SetStageExac(0);

		/*** タイトル画面へ戻る ***/
		SetFade(MODE_TITLE, FADE_TYPE_NORMAL);
		FadeSound(SOUND_LABEL_BGM_TITLE);
		// g_CurrentPauseType = PAUSE_TYPE_CONTINUE;

		break;

	default:
		break;
	}
}

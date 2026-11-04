//==================================================================================
//
// デバッグ文字表示の処理 [debugproc.cpp]
// Author : TENMA SAITO
//
// PrintDebugProc で1フレーム分の文字列を溜め、DrawDebugProc で画面左上に表示する。
// 書式指定 (独自仕様) :
//   %d / %f / %c / %s / %b(bool)  … 値を1つ表示
//   %~N<型>                       … 同じ型の値を N 個、空白区切りで表示 (例 : %~3f)
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "debugproc.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define DEBUG_FONT_HEIGHT		(23)				// フォントの高さ
#define DEBUG_FONT_NAME			"PixelMplus12"		// フォント名
#define DEBUG_STRING_MAX		(4096)				// 1フレームに溜められる文字数
#define DEBUG_VALUE_MAX			(256)				// 値1つ分の文字数

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
#ifdef _DEBUG
static void AppendDebugValue(char *pDest, size_t nDestSize, char cType, va_list *pArgs, const char *pSeparator);
#endif

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPD3DXFONT g_pFont = NULL;				// フォントへのポインタ
static char g_aStrDebug[DEBUG_STRING_MAX];		// 1フレーム分のデバッグ表示用文字列

//==================================================================================
// --- デバッグ表示の初期化 ---
//==================================================================================
void InitDebugProc(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス

	/*** フォントの作成 ***/
	D3DXCreateFont(pDevice,
		DEBUG_FONT_HEIGHT,		// 高さ
		0,						// 幅(自動)
		0,						// 太さ(既定)
		0,						// ミップマップレベル
		FALSE,					// 斜体
		SHIFTJIS_CHARSET,		// 文字セット
		OUT_DEFAULT_PRECIS,		// 出力精度
		DEFAULT_QUALITY,		// 品質
		DEFAULT_PITCH,			// ピッチ
		DEBUG_FONT_NAME,		// フォント名
		&g_pFont);

	/*** 文字列バッファのクリア ***/
	memset(g_aStrDebug, 0, sizeof g_aStrDebug);
}

//==================================================================================
// --- デバッグ表示の終了 ---
//==================================================================================
void UninitDebugProc(void)
{
	/*** フォントの破棄 ***/
	RELEASE(g_pFont);
}

//==================================================================================
// --- デバッグ表示の更新 ---
//==================================================================================
void UpdateDebugProc(void)
{

}

//==================================================================================
// --- デバッグ表示の描画 (Debugビルドのみ表示し、毎フレーム文字列をクリア) ---
//==================================================================================
void DrawDebugProc(void)
{
#ifdef _DEBUG
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };		// 表示範囲(画面全体)

	g_pFont->DrawText(NULL, g_aStrDebug, -1, &rect, DT_LEFT, D3DCOLOR_RGBA(255, 255, 255, 255));
#endif

	/*** 文字列バッファのクリア ***/
	memset(g_aStrDebug, 0, sizeof g_aStrDebug);
}

//==================================================================================
// --- デバッグ表示の追加 (Debugビルドのみ) ---
//==================================================================================
void PrintDebugProc(const char *fmt, ...)
{
#ifdef _DEBUG
	va_list args;		// 可変長引数

	va_start(args, fmt);

	for (const char *p = fmt; *p != '\0'; p++)
	{
		if (*p != '%')
		{ // 通常の文字はそのまま追加
			char aChar[2] = { *p, '\0' };		// 1文字分の文字列
			strncat(g_aStrDebug, aChar, sizeof g_aStrDebug - strlen(g_aStrDebug) - 1);
			continue;
		}

		p++;		// % の次の文字へ
		if (*p == '\0') break;

		if (*p == '~')
		{ // %~N<型> : N 個まとめて表示
			p++;
			if (*p == '\0') break;
			int nCount = *p - '0';		// 表示する個数(1桁)

			p++;
			if (*p == '\0') break;

			for (int nCntValue = 0; nCntValue < nCount; nCntValue++)
			{
				AppendDebugValue(g_aStrDebug, sizeof g_aStrDebug, *p, &args, " ");
			}
		}
		else
		{ // %<型> : 1つ表示
			AppendDebugValue(g_aStrDebug, sizeof g_aStrDebug, *p, &args, "");
		}
	}

	va_end(args);
#endif
}

#ifdef _DEBUG
//==================================================================================
// --- 型指定文字に応じて可変長引数を1つ取り出し、文字列にして追加 ---
// ※ 未対応の型指定文字の場合は何も追加しない(引数も取り出さない)
//==================================================================================
static void AppendDebugValue(char *pDest, size_t nDestSize, char cType, va_list *pArgs, const char *pSeparator)
{
	char aValue[DEBUG_VALUE_MAX] = {};		// 値を文字列にしたもの

	switch (cType)
	{
	case 'd':		// 整数
		_snprintf(aValue, sizeof aValue - 1, "%d%s", va_arg(*pArgs, int), pSeparator);
		break;

	case 'f':		// 小数 (float は double に昇格して渡される)
		_snprintf(aValue, sizeof aValue - 1, "%f%s", va_arg(*pArgs, double), pSeparator);
		break;

	case 'c':		// 文字 (char は int に昇格して渡される)
		_snprintf(aValue, sizeof aValue - 1, "%c%s", (char)va_arg(*pArgs, int), pSeparator);
		break;

	case 's':		// 文字列
		_snprintf(aValue, sizeof aValue - 1, "%s%s", va_arg(*pArgs, const char*), pSeparator);
		break;

	case 'b':		// bool (int に昇格して渡される)
		_snprintf(aValue, sizeof aValue - 1, "%d%s", va_arg(*pArgs, int), pSeparator);
		break;

	default:		// 未対応
		return;
	}

	/*** バッファの残りに収まる分だけ追加 ***/
	strncat(pDest, aValue, nDestSize - strlen(pDest) - 1);
}
#endif

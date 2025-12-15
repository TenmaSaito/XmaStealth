//=============================================================================
//
// サウンド処理 [sound.h]
// Author : TENMA SAITO (SPACIAL THANKS : AKIRA TANAKA)
//
//=============================================================================
#ifndef _SOUND_H_
#define _SOUND_H_

#include "main.h"

// マクロ定義
#define SOUND_BGMSTART		(SOUND_LABEL_BGM_TITLE)		// BGMの開始位置
#define SOUND_SESTART		(SOUND_LABEL_SE_ENTER)		// SEの開始位置
#define SOUND_STANDARD		(0.75f)						// 初期音量

#ifndef NDEBUG
#undef SOUND_STANDARD
#define SOUND_STANDARD		(0.05f)						// 初期音量
#endif

//#define _3DAUDIO

//*****************************************************************************
// サウンド一覧 (sound.cppのsoundinfoにも追加する！)
//*****************************************************************************
typedef enum
{
	SOUND_LABEL_BGM_TITLE = 0,		// タイトル画面のBGM
	SOUND_LABEL_BGM_GAME,			// ゲーム画面のBGM
	SOUND_LABEL_BGM_RESULT,			// 結果画面のBGM
	SOUND_LABEL_BGM_RANKING,		// ランキング画面のBGM
	SOUND_LABEL_BGM_CHILL,			// チルBGM(将来置き換え)
	SOUND_LABEL_SE_ENTER,			// 選択音
	SOUND_LABEL_MAX,
} SOUND_LABEL;

//*****************************************************************************
// サウンドの種類
//*****************************************************************************
typedef enum
{
	SETSOUND_GAME = 0,			// ゲーム内音声
	SETSOUND_MENU_SE,			// システム音声
	SETSOUND_MAX
}SETSOUND;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitSound(HWND hWnd);
void UninitSound(void);
HRESULT PlaySound(SOUND_LABEL label);
void StopSound(SOUND_LABEL label);
void StopSound(void);

// 追加プロトタイプ宣言
void SetVolume(float volume, SETSOUND sound);
float GetVolume(SETSOUND sound);
void FadeSound(SOUND_LABEL label);
SOUND_LABEL GetPlaySound(void);
void UpdateSound(void);
void Play3DSound(SOUND_LABEL label, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 posU);

#endif

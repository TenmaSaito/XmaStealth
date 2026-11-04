//==================================================================================
//
// サウンド処理 [sound.h]
// Author : TENMA SAITO (SPACIAL THANKS : AKIRA TANAKA)
//
//==================================================================================
#ifndef _SOUND_H_
#define _SOUND_H_

#include "main.h"

// マクロ定義
#define SOUND_BGMSTART		(SOUND_LABEL_BGM_TITLE)		// BGMの開始位置
#define SOUND_SESTART		(SOUND_LABEL_SE_ENTER)		// SEの開始位置
#define SOUND_STANDARD		(0.8f)						// 初期音量

#ifndef NDEBUG
#undef SOUND_STANDARD
#define SOUND_STANDARD		(0.6f)						// 初期音量
#endif

//#define _3DAUDIO										// 有効にすると立体音響(X3DAudio)を使う (現在は無効)

//**********************************************************************************
// *** サウンド一覧 (sound.cppのsoundinfoにも追加する！) ***
//**********************************************************************************
typedef enum
{
	SOUND_LABEL_BGM_TITLE = 0,		// タイトル画面のBGM
	SOUND_LABEL_BGM_TUTORIAL,		// チュートリアル画面のBGM
	SOUND_LABEL_BGM_GAME,			// ゲーム画面のBGM
	SOUND_LABEL_BGM_GAMECLEAR,		// クリア画面のBGM
	SOUND_LABEL_BGM_GAMEOVER,		// ゲームオーバー画面のBGM
	SOUND_LABEL_BGM_RANKING,		// ランキング画面のBGM
	SOUND_LABEL_SE_ENTER,			// 選択音
	SOUND_LABEL_SE_SELECT,			// セレクト音
	SOUND_LABEL_SE_WALK,			// 歩行音
	SOUND_LABEL_SE_JUMP,			// ジャンプ音
	SOUND_LABEL_SE_SHOT,			// 投擲音
	SOUND_LABEL_SE_FIND,			// 取得音
	SOUND_LABEL_SE_CATCH,			// 衝突音
	SOUND_LABEL_MAX,
} SOUND_LABEL;

//**********************************************************************************
// *** サウンドチェック構造体 ***
//**********************************************************************************
typedef struct SoundChecker : public IXAudio2VoiceCallback
{
private:
	HRESULT m_LastError;
	bool m_bEnd;
	bool m_bSubmit;

public:

	SoundChecker() : m_LastError(S_OK), m_bEnd(false), m_bSubmit(false){}

	// バッファ―処理終了時
	void STDMETHODCALLTYPE OnBufferEnd(void *pBufferContext) override
	{
		OutputDebugString(TEXT("バッファー処理が終了したよ！\n"));

		// 必要に応じてvoid*をキャストして使用
		if (pBufferContext != nullptr)
		{ // ポインタを使用した処理

		}

		m_bSubmit = false;
	}

	// バッファ―処理開始時
	void STDMETHODCALLTYPE OnBufferStart(void *pBufferContext) override
	{
		OutputDebugString(TEXT("バッファー処理スタート！\n"));

		// 必要に応じてvoid*をキャストして使用
		if (pBufferContext != nullptr)
		{ // ポインタを使用した処理

		}

		m_bSubmit = true;
	}

	// ループ終了時
	void STDMETHODCALLTYPE OnLoopEnd(void *pBufferContext) override
	{
		// 必要に応じてvoid*をキャストして使用
		if (pBufferContext != nullptr)
		{ // ポインタを使用した処理

		}
	}

	// 再生終了時
	void STDMETHODCALLTYPE OnStreamEnd(void) override
	{
		OutputDebugString(TEXT("音源の再生が終了したよ！\n"));

		m_bEnd = true;
	}

	// エラー発生時
	void STDMETHODCALLTYPE OnVoiceError(
		void *pBufferContext,
		HRESULT Error
	) override
	{
		OutputDebugString(TEXT("ひぃん......エラー発生......\n"));

		// 必要に応じてvoid*をキャストして使用
		if (pBufferContext != nullptr)
		{ // ポインタを使用した処理

		}

		m_LastError = Error;
	}

	// 音声の処理パス終了直後
	void STDMETHODCALLTYPE OnVoiceProcessingPassEnd(void) override
	{

	}

	// データを読み取る直前
	void STDMETHODCALLTYPE OnVoiceProcessingPassStart(
		UINT32 BytesRequired
	) override
	{

	}

	// 再生中か
	bool IsPlayingSound(void)
	{
		return !m_bEnd;
	}

	// バッファーの登録がされているか
	bool IsSubmitBuffer(void)
	{
		return m_bSubmit;
	}

	// 最後に発生したエラー取得
	HRESULT GetLastError(void)
	{
		return m_LastError;
	}
} SoundChecker;

typedef SoundChecker *LPSOUNDCHECKER, *PSOUNDCHECKER;

//**********************************************************************************
// *** サウンドの種類 ***
//**********************************************************************************
typedef enum
{
	SETSOUND_GAME = 0,			// ゲーム内音声
	SETSOUND_MENU_SE,			// システム音声
	SETSOUND_MAX
}SETSOUND;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
HRESULT InitSound(HWND hWnd);
void UninitSound(void);
HRESULT PlaySound(SOUND_LABEL label);
void StopSound(SOUND_LABEL label);
void StopSound(void);

// 追加プロトタイプ宣言
bool GetSound(SOUND_LABEL label);
void SetVolume(float fVolume, SETSOUND sound);
float GetVolume(SETSOUND sound);
void FadeSound(SOUND_LABEL label);
SOUND_LABEL GetPlaySound(void);
void UpdateSound(void);
void Play3DSound(SOUND_LABEL label, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 posU);

#endif

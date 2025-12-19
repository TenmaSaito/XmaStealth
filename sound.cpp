//=============================================================================
//
// サウンド処理 [sound.cpp]
// Author : TENMA SAITO (SPACIAL THANKS : AKIRA TANAKA)
//
//=============================================================================
#include "sound.h"
#include "camera.h"
#include "fade.h"
#include "game.h"

//#define _3DAUDIO

//*****************************************************************************
// サウンド情報の構造体定義
//*****************************************************************************
typedef struct
{
	const char *pFilename;		// ファイル名
	int nCntLoop;				// ループカウント
	D3DXVECTOR3 posEmitter;		// 発信源位置
	D3DXVECTOR3 rotEmitter;		// 発信向き
	D3DXVECTOR3 posUEmitter;	// 発信源の上方向ベクトル
	bool b3D;					// 立体音響を適用するか
} SOUNDINFO;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2オブジェクトへのインターフェイス
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// マスターボイス
IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// ソースボイス
XAUDIO2_VOICE_DETAILS g_MasterVoiceDetails = {};						// マスターボイスの音声情報
XAUDIO2_VOICE_DETAILS g_aSourceVoiceDetails[SOUND_LABEL_MAX] = {};		// ソースボイスの音声情報
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// オーディオデータ
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// オーディオデータサイズ
bool g_aPlayAudio[SOUND_LABEL_MAX] = {};					// 再生状況
float g_fVolumeAudio;										// 現在の音量
SOUND_LABEL g_PlaySound;									// 再生中の音源
SOUND_LABEL	g_FadelabelExac;								// フェードアウト時に再生されている音源
SOUND_LABEL g_Fadelabel;									// フェードイン時に再生する音源
FADE g_CurrentFadeSound;									// 現在のフェード状態
int g_nCounterSound;										// 汎用カウンター

XAUDIO2_DEVICE_DETAILS g_xa2_details;						// デバイスのオーディオ取得
X3DAUDIO_HANDLE g_X3DInstance = {};							// X3DAudioのハンドル
X3DAUDIO_DSP_SETTINGS g_DSPSettings = {};					// DSP設定
X3DAUDIO_LISTENER g_Listener;								// リスナー
X3DAUDIO_EMITTER g_aEmitter[SOUND_LABEL_MAX];				// エミッター
XAUDIO2_BUFFER g_aSubmixBuffer[SOUND_LABEL_MAX] = {};
WAVEFORMATEX g_wfxSource[SOUND_LABEL_MAX] = {};

// サウンドの情報(sound.hのLABELに追加したらここにも追加する！)
SOUNDINFO g_aSoundInfo[SOUND_LABEL_MAX] =
{
	{"data/BGM/BGM_TITLE_TEST.wav", -1, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// タイトル画面のBGM
	{"data/BGM/BGM_TITLE_TEST.wav", -1, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// タイトル画面のBGM
	{"data/BGM/BGM_TITLE_TEST.wav", -1, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// タイトル画面のBGM
	{"data/BGM/BGM_TITLE_TEST.wav", -1, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// タイトル画面のBGM
	{"data/BGM/BGM_CHILL.wav", -1, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// タイトル画面のBGM
	{"data/SE/SE_ENTER.wav", 0, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, D3DXVECTOR3_NULL, false},		// 決定音
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitSound(HWND hWnd)
{
	HRESULT hr;

	// COMライブラリの初期化
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "COMライブラリの初期化に失敗！", "警告！", MB_ICONWARNING);

		return E_FAIL;
	}

	// XAudio2オブジェクトの作成
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2オブジェクトの作成に失敗！", "警告！", MB_ICONWARNING);

		// COMライブラリの終了処理
		CoUninitialize();

		return E_FAIL;
	}
	
	// マスターボイスの生成
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "マスターボイスの生成に失敗！", "警告！", MB_ICONWARNING);

		if(g_pXAudio2 != NULL)
		{
			// XAudio2オブジェクトの開放
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COMライブラリの終了処理
		CoUninitialize();

		return E_FAIL;
	}

	// サウンドデータの初期化
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// バッファのクリア
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// サウンドデータファイルの生成
		hFile = CreateFile(g_aSoundInfo[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(1)", "警告！", MB_ICONWARNING);

			return HRESULT_FROM_WIN32(GetLastError());
		}

		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// ファイルポインタを先頭に移動
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(2)", "警告！", MB_ICONWARNING);

			return HRESULT_FROM_WIN32(GetLastError());
		}
	
		// WAVEファイルのチェック
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(1)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		// チャンクデータの読み込み
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(2)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(3)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}
	
		// フォーマットチェック
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(1)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		// チャンクデータの読み込み
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(2)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		// データチェック
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(1)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		// チャンクデータの読み込み
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(2)", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}
	
		// ソースボイスの生成
		hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format));
		if(FAILED(hr))
		{
			MessageBox(hWnd, "ソースボイスの生成に失敗！", "警告！", MB_ICONWARNING);

			return S_FALSE;
		}

		// バッファの値設定
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		memset(g_aSourceVoiceDetails, 0, sizeof(XAUDIO2_VOICE_DETAILS));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aSoundInfo[nCntSound].nCntLoop;

		// オーディオバッファの登録
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);

		g_apSourceVoice[nCntSound]->GetVoiceDetails(&g_aSourceVoiceDetails[nCntSound]);
		g_wfxSource[nCntSound] = wfx.Format;

		// ファイルをクローズ
		CloseHandle(hFile);
	}

	g_pMasteringVoice->GetVoiceDetails(&g_MasterVoiceDetails);
	g_pXAudio2->GetDeviceDetails(0, &g_xa2_details);

#ifdef _3DAUDIO
	X3DAudioInitialize(g_xa2_details.OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, g_X3DInstance);
	if (g_X3DInstance == NULL)
	{
		return E_FAIL;
	}

	/*** リスナーの初期化 ***/
	g_Listener.OrientFront = D3DXVECTOR3_NULL;
	g_Listener.OrientTop = D3DXVECTOR3_NULL;
	g_Listener.Position = D3DXVECTOR3_NULL;
	g_Listener.Velocity = SOUND_SPD;
	g_Listener.pCone = NULL;

	/*** エミッターの初期化 ***/
	for (int nCntEmmiter = 0; nCntEmmiter < SOUND_LABEL_MAX; nCntEmmiter++)
	{
		g_aEmitter[nCntEmmiter].OrientFront = g_aSoundInfo[nCntEmmiter].rotEmitter;
		g_aEmitter[nCntEmmiter].OrientTop = g_aSoundInfo[nCntEmmiter].posUEmitter;
		g_aEmitter[nCntEmmiter].Position = g_aSoundInfo[nCntEmmiter].posEmitter;
		g_aEmitter[nCntEmmiter].Velocity = SOUND_SPD;
		g_aEmitter[nCntEmmiter].ChannelCount = 1;
		g_aEmitter[nCntEmmiter].CurveDistanceScaler = g_aEmitter[nCntEmmiter].DopplerScaler = 50.0f;
		g_aEmitter[nCntEmmiter].ChannelRadius = 0.0f;
		g_aEmitter[nCntEmmiter].CurveDistanceScaler = 0.0f;
		g_aEmitter[nCntEmmiter].DopplerScaler = 0.0f;
		g_aEmitter[nCntEmmiter].InnerRadius = 0.0f;
		g_aEmitter[nCntEmmiter].InnerRadiusAngle = 0.0f;
		g_aEmitter[nCntEmmiter].pChannelAzimuths = NULL;
		g_aEmitter[nCntEmmiter].pCone = NULL;
		g_aEmitter[nCntEmmiter].pLFECurve = NULL;
		g_aEmitter[nCntEmmiter].pLPFDirectCurve = NULL;
		g_aEmitter[nCntEmmiter].pReverbCurve = NULL;
		g_aEmitter[nCntEmmiter].pVolumeCurve = NULL;
	}

#endif

	for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		g_aPlayAudio[nCntSound] = false;
	}

	for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if (g_apSourceVoice[nCntSound] != NULL)
		{
			// 音量を設定
			g_apSourceVoice[nCntSound]->SetVolume(0.0f, XAUDIO2_COMMIT_NOW);
		}
	}

	// 初期音量を代入
	g_fVolumeAudio = 0.0f;

	// フェードイン時の音源を設定(初期)
	g_Fadelabel = SOUND_LABEL_BGM_TITLE;

	// フェードアウト時の音源を設定(初期)
	g_FadelabelExac = SOUND_LABEL_BGM_TITLE;

	// 再生中の音源の初期化
	g_PlaySound = SOUND_LABEL_BGM_TITLE;

	// フェードインに設定
	g_CurrentFadeSound = FADE_IN;

	// カウンターを初期化
	g_nCounterSound = 0;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound] != NULL)
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// ソースボイスの破棄
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
			g_aSourceVoiceDetails[nCntSound] = {};
	
			// オーディオデータの開放
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// マスターボイスの破棄
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	g_MasterVoiceDetails = {};
	
	if(g_pXAudio2 != NULL)
	{
		// XAudio2オブジェクトの開放
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}

	// COMライブラリの終了処理
	CoUninitialize();
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateSound(void)
{
	Camera *pCamera = GetCamera();
	X3DAUDIO_CONE ListenerCone = {};
	
#ifdef _3DAUDIO
	/*** 3Dオーディオ(立体音響の設定！) ***/
	for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if (g_aSoundInfo[nCntSound].b3D == true)
		{
			/*** 変数宣言 ***/
			FLOAT32 matrix[16] = {};		// 8(or 16)にすると立体に聞こえる！
			D3DXVECTOR3 vecFront;			// カメラの視点位置からの方向ベクトル
			D3DXVECTOR3 vecFrontEmitter;	// 音源からの方向ベクトル

			/** コーンの設定 **/
			ListenerCone = X3DAudioDefault_DirectionalCone;

			/*** カメラの向いている方向ベクトルを取得 ***/
			vecFront.x = pCamera->posV.x + (sinf(pCamera->rot.y) * 1000.0f);
			vecFront.y = pCamera->posV.y;
			vecFront.z = pCamera->posV.z + (cosf(pCamera->rot.y) * 1000.0f);

			/** 方向ベクトルを正規化 **/
			D3DXVec3Normalize(&vecFront, &vecFront);

			/*** 音源の向いている方向ベクトルを取得 ***/
			vecFrontEmitter.x = g_aSoundInfo[nCntSound].posEmitter.x + (sinf(g_aSoundInfo[nCntSound].rotEmitter.y) * 1000.0f);
			vecFrontEmitter.y = g_aSoundInfo[nCntSound].posEmitter.y;
			vecFrontEmitter.z = g_aSoundInfo[nCntSound].posEmitter.z + (cosf(g_aSoundInfo[nCntSound].rotEmitter.y) * 1000.0f);

			/** 方向ベクトルを正規化 **/
			D3DXVec3Normalize(&vecFrontEmitter, &vecFrontEmitter);

			/*** 各変数を初期化 ***/
			g_DSPSettings = {};
			g_Listener = {};
			g_aEmitter[nCntSound] = {};

			/*** (※)発信源の音源はモノラルに！且つChannelCountを1にする！(ステレオだと聞こえない) ***/
			g_aEmitter[nCntSound].ChannelCount = 1;

			/** 音量の距離減衰を設定(大きければ大きいほど減衰しない) **/
			g_aEmitter[nCntSound].CurveDistanceScaler = g_aEmitter[nCntSound].DopplerScaler = 50.0f;

			/*** 聞き手の位置、方向、上方向ベクトルを設定 ***/
			g_Listener.OrientFront = vecFront;
			g_Listener.OrientTop = pCamera->vecU;
			g_Listener.Position = pCamera->posV;
			g_Listener.Velocity = SOUND_SPD;
			g_Listener.pCone = &ListenerCone;

			/*** 発信源の位置、方向、上方向ベクトルを設定 ***/
			g_aEmitter[nCntSound].OrientFront = vecFrontEmitter;
			g_aEmitter[nCntSound].OrientTop = g_aSoundInfo[nCntSound].posUEmitter;
			g_aEmitter[nCntSound].Position = g_aSoundInfo[nCntSound].posEmitter;
			g_aEmitter[nCntSound].Velocity = SOUND_SPD;

			/*** 音量計算に使用する初期値を設定 ***/
			g_DSPSettings.SrcChannelCount = 1;				// Srcはモノラルの為、1に設定
			g_DSPSettings.DstChannelCount = 16;				// Dstは立体に近づけるため、16に設定
			g_DSPSettings.pMatrixCoefficients = &matrix[0];	// マトリックス用配列のアドレスを渡す

			/*** 音量計算！DSPSettingsが計算結果 ***/
			X3DAudioCalculate(g_X3DInstance, &g_Listener, &g_aEmitter[nCntSound],
				X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DELAY | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
				&g_DSPSettings);

			/*** 音量に計算結果を反映 ***/
			/** マトリックスの結果を反映！(第1引数 : MasteringVoiceのポインタ,
										   第2引数 : 音源のチャンネル数(モノラルの為1)
										   第3引数 : 聞き手のチャンネル数(マトリックスの配列の数と同期)
										   第4引数 : 計算結果のDSPSettingsのマトリックスへのポインタ) **/
			g_apSourceVoice[nCntSound]->SetOutputMatrix(g_pMasteringVoice, 1, 16, g_DSPSettings.pMatrixCoefficients);

			/** ドップラー効果の反映...?(よくわかってない) **/
			g_apSourceVoice[nCntSound]->SetFrequencyRatio(g_DSPSettings.DopplerFactor);
		}
	}
#endif // _3DAUDIO

	/*** 音源のフェード効果設定！ ***/
	if (g_CurrentFadeSound != FADE_NONE)
	{
		if (g_CurrentFadeSound == FADE_IN && (GetFade() == FADE_IN))
		{
			g_fVolumeAudio += 0.01f;
			if (g_nCounterSound == 0)
			{
				PlaySound(g_Fadelabel);
				g_nCounterSound--;
			}
			else
			{
				g_nCounterSound--;
			}

			for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
			{
				if (g_apSourceVoice[nCntSound] != NULL)
				{
					// 音量を設定
					g_apSourceVoice[nCntSound]->SetVolume(g_fVolumeAudio, XAUDIO2_COMMIT_NOW);
				}
			}

			if (g_fVolumeAudio >= SOUND_STANDARD)
			{
				g_CurrentFadeSound = FADE_NONE;
				g_fVolumeAudio = SOUND_STANDARD;
				for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
				{
					if (g_apSourceVoice[nCntSound] != NULL)
					{
						// 音量を設定
						g_apSourceVoice[nCntSound]->SetVolume(g_fVolumeAudio, XAUDIO2_COMMIT_NOW);
					}
				}
			}
		}
		else if (g_CurrentFadeSound == FADE_OUT)
		{
			g_fVolumeAudio -= 0.01f;
			for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
			{
				if (g_apSourceVoice[nCntSound] != NULL)
				{
					// 音量を設定
					g_apSourceVoice[nCntSound]->SetVolume(g_fVolumeAudio, XAUDIO2_COMMIT_NOW);
				}
			}

			if (g_fVolumeAudio <= 0.0f)
			{
				g_CurrentFadeSound = FADE_IN;
				g_fVolumeAudio = 0.0f;
				for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
				{
					if (g_apSourceVoice[nCntSound] != NULL)
					{
						// 音量を設定
						g_apSourceVoice[nCntSound]->SetVolume(g_fVolumeAudio, XAUDIO2_COMMIT_NOW);
					}
				}

				g_nCounterSound = 6;

				StopSound();
			}
		}
	}
}

//=============================================================================
// セグメント再生(再生中なら停止)
//=============================================================================
HRESULT PlaySound(SOUND_LABEL label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	if (label < 0 || label >= SOUND_LABEL_MAX)
	{
		return E_FAIL;
	}

	// バッファの値設定
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aSoundInfo[label].nCntLoop;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	// オーディオバッファの登録
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// 再生
	g_apSourceVoice[label]->Start(0);

	// 再生状況をtrueに
	g_aPlayAudio[label] = true;

	if (label >= SOUND_BGMSTART && label < SOUND_SESTART)
	{// 再生するのがBGMだった場合
		g_PlaySound = label;
	}

	/*** 立体音響モードを解除 ***/
	g_aSoundInfo[label].b3D = false;

	return S_OK;
}

//=============================================================================
// セグメント再生(再生中なら停止)
//=============================================================================
void Play3DSound(SOUND_LABEL label, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 posU)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	if (label < 0 || label >= SOUND_LABEL_MAX)
	{
		return;
	}

	// バッファの値設定
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = g_aSoundInfo[label].nCntLoop;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if (xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	// オーディオバッファの登録
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// 再生
	g_apSourceVoice[label]->Start(0);

	// 再生状況をtrueに
	g_aPlayAudio[label] = true;

	if (label >= SOUND_BGMSTART && label < SOUND_SESTART)
	{// 再生するのがBGMだった場合
		g_PlaySound = label;
	}

	/*** 立体音響モードを設定 ***/
	g_aSoundInfo[label].posEmitter = pos;
	g_aSoundInfo[label].rotEmitter = rot;
	g_aSoundInfo[label].posUEmitter = posU;
	g_aSoundInfo[label].b3D = true;
}

//=============================================================================
// セグメント停止(ラベル指定)
//=============================================================================
void StopSound(SOUND_LABEL label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();

		// 再生状況をfalseに
		g_aPlayAudio[label] = false;
	}
}

//=============================================================================
// セグメント停止(全て)
//=============================================================================
void StopSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound] != NULL)
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
			// 再生状況をfalseに
			g_aPlayAudio[nCntSound] = false;
		}
	}
}

//=============================================================================
// チャンクのチェック
//=============================================================================
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD dwRead;
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD dwBytesRead = 0;
	DWORD dwOffset = 0;
	
	if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを先頭に移動
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクデータの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// ファイルタイプの読み込み
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// ファイルポインタをチャンクデータ分移動
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		dwOffset += sizeof(DWORD) * 2;
		if(dwChunkType == format)
		{
			*pChunkSize         = dwChunkDataSize;
			*pChunkDataPosition = dwOffset;

			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		if(dwBytesRead >= dwRIFFDataSize)
		{
			return S_FALSE;
		}
	}
	
	return S_OK;
}

//=============================================================================
// チャンクデータの読み込み
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを指定位置まで移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// データの読み込み
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}

//=============================================================================
// 音量の設定
//=============================================================================
void SetVolume(float volume, SETSOUND sound)
{
	switch (sound)
	{

	case SETSOUND_MAX:

		for (int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
		{
			if (g_apSourceVoice[nCntSound] != NULL)
			{
				// 音量を設定
				g_apSourceVoice[nCntSound]->SetVolume(volume, XAUDIO2_COMMIT_NOW);
			}
		}

		break;

	default:
		break;
	}

	g_fVolumeAudio = volume;
}

//=============================================================================
// 音量の取得
//=============================================================================
float GetVolume(SETSOUND sound)
{
	float fVolume = 0.0f;

	if (sound == SETSOUND_MAX)
	{
		g_apSourceVoice[SOUND_BGMSTART]->GetVolume(&fVolume);
	}
	
	return fVolume;
}

//=============================================================================
// 音楽のフェード処理
//=============================================================================
void FadeSound(SOUND_LABEL label)
{
	g_CurrentFadeSound = FADE_OUT;			// フェードアウト状態に
	g_Fadelabel = label;					// フェードイン時のBGMを指定
	g_FadelabelExac = g_PlaySound;			// フェードアウト時のBGMを取得
}

//=============================================================================
// 現在再生中のBGMの取得
//=============================================================================
SOUND_LABEL GetPlaySound(void)
{
	return g_PlaySound;
}
//================================================================================================================
//
// DirectXのメイン処理 [main.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include "input.h"
#include "camera.h"
#include "sound.h"
#include "fade.h"
#include "game.h"
#include "player.h"
#include "wall.h"
#include "debugproc.h"
#include "title.h"
#include "result.h"
#include "gameclear.h"
#include "gameover.h"
#include "ranking.h"
#include "xmodel.h"
#include "tutorial.h"
#include <crtdbg.h>
#include <assert.h>

//**********************************************************************************
//*** マクロ定義 ***
//**********************************************************************************
#define CLASS_NAME		"WindowClass"				// ウィンドウクラスの名前
#define WINDOW_NAME		"3Dゲームの名前どうしようかな..."			// キャプションに表示される名前(提出する前に変えること！！！)
#if 1
#define MODE_ON				// モード設定を有効化
#endif

//**********************************************************************************
//*** プロトタイプ宣言 ***
//**********************************************************************************
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);
void DrawDebug(void);
void ToggleFullscreen(HWND hWnd);
void onWireFrame(void);
void offWireFrame(void);

//**********************************************************************************
//*** グローバル変数 ***
//**********************************************************************************
LPDIRECT3D9				g_pD3D = NULL;					// Direct3Dオブジェクトへのポインタ
LPDIRECT3DDEVICE9		g_pD3DDevice = NULL;			// Direct3Dデバイスへのポインタ
MODE g_mode = MODE_TITLE;								// 現在の画面
MODE g_modeExac = MODE_TITLE;							// ひとつ前の過去の画面
MODE g_modeInit = MODE_TITLE;							// init中の画面
HWND g_hWnd = NULL;										// 獲得したウィンドウハンドル
int g_nCountFPS = 0;									// FPSカウンタ
bool g_isFullscreen = false;							// フルスクリーンの使用状況
RECT g_windowRect;										// ウィンドウサイズ
GAMEDIFFICULTY g_Difficulty = GAMEDIFFICULTY_EASY;		// ゲームの難易度
int g_nStageExac;										// ステージ番号

//================================================================================================================
// --- メイン関数 ---
//================================================================================================================
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hInstancePrev, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);			// メモリリーク検知用のフラグ
#endif // _DEBUG

	HWND hWnd;									// ウィンドウハンドル
	MSG msg;									// メッセージを格納する変数
	DWORD dwCurrentTime;						// 現在時刻
	DWORD dwExecLastTime;						// 最後に処理した時刻
	DWORD dwFrameCount;							// フレームカウント
	DWORD dwFPSLastTime;						// 最後にFPSを計測した時刻
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

	WNDCLASSEX wcex =
	{
		sizeof(WNDCLASSEX),						// ウィンドウクラスのメモリサイズ
		CS_CLASSDC,								// ウィンドウのスタイル
		WindowProc,								// ウィンドウプロシージャ
		0,										// 0
		0,										// 0
		hInstance,								// インスタンスハンドル
		NULL,									// タスクバーのアイコン
		LoadCursor(NULL,IDC_ARROW),				// マウスカーソル
		(HBRUSH)(COLOR_WINDOW + 1),				// クライアント領域の背景色
		NULL,									// メニューバー
		CLASS_NAME,								// ウィンドウクラスの名前
		NULL									// ファイルのアイコン
	}; 

	// ウィンドウクラスの登録
	RegisterClassEx(&wcex);

	// クライアント領域を指定のサイズに調整
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウの生成
	hWnd = CreateWindowEx(
		0,										// 拡張ウィンドウスタイル
		CLASS_NAME,								// ウィンドウクラスの名前
		WINDOW_NAME,							// ウィンドウの名前
		WS_OVERLAPPEDWINDOW,					// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上X座標
		CW_USEDEFAULT,							// ウィンドウの左上Y座標
		(rect.right - rect.left),				// ウィンドウの幅
		(rect.bottom - rect.top),				// ウィンドウの高さ
		NULL,									// 親ウィンドウのハンドル
		NULL,									// メニュー(もしくは子ウィンドウ)ハンドル
		hInstance,								// インスタンスハンドル
		NULL);									// ウィンドウ作成データ

	if(hWnd == NULL)
	{
		MessageBox(hWnd, "ウィンドウハンドルが確保されていません!", "警告！", MB_ICONWARNING);
		return -1;
	}

	g_hWnd = hWnd;

	// 初期化処理
	if (FAILED(Init(hInstance, hWnd, TRUE)))
	{// 初期化処理が失敗した場合
		MessageBox(hWnd, "Initに失敗しました。", "Error (0)", MB_ICONERROR);
		return -1;
	}

	//分解能を設定
	timeBeginPeriod(1);

	dwCurrentTime = 0;							// 初期化
	dwExecLastTime = timeGetTime();				// 現在時刻を取得
	dwFrameCount = 0;
	dwFPSLastTime = timeGetTime();

	// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);					// ウィンドウの表示状態を設定

	UpdateWindow(hWnd);							// クライアント領域を更新

	// メッセージループ
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE) != 0)
		{// windowsの処理
			if (msg.message == WM_QUIT)
			{// WM_QUITメッセージを受けると、メッセージループを抜ける
				break;
			}
			else
			{
				// メッセージの設定
				TranslateMessage(&msg);					// 仮想キーメッセージを文字メッセージへ変換

				DispatchMessage(&msg);					// ウィンドウプロシージャへメッセージを送出
			}
		}
		else
		{// DirectXの処理
			dwCurrentTime = timeGetTime();				// 現在時刻を取得

			if ((dwCurrentTime - dwFPSLastTime) >= 500)
			{// 0.5秒経過
				// FPSを計測
				g_nCountFPS = (dwFrameCount * 1000) / (dwCurrentTime - dwFPSLastTime);

				dwFPSLastTime = dwCurrentTime;			// FPSを計測した時刻を取得
				dwFrameCount = 0;						// フレームカウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{//60分の1秒経過

				dwExecLastTime = dwCurrentTime;			//処理開始時刻[現在時刻]を保存

				// 更新処理
				Update();

				// 描画処理
				Draw();

				dwFrameCount++;							// フレームカウントを加算
			}
		}
	}

	// 終了処理
	Uninit();

	// ウィンドウクラスの登録解除
	UnregisterClass(CLASS_NAME, wcex.hInstance); 

	return (int)msg.wParam;
}

//================================================================================================================
// --- ウィンドウプロシージャ ---
//================================================================================================================
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:		// ウィンドウ破棄のメッセージ
		// WM_QUITメッセージを送る
		PostQuitMessage(0);
		break;

	// キー押下のメッセージ
	case WM_KEYDOWN:
		// 押されたキーの判定
		switch (wParam)
		{
		case VK_ESCAPE:
			// ウィンドウを破棄する(WM_DESTROYメッセージを送る)
			DestroyWindow(hWnd);
			break;

		case VK_F11:
			// F11でフルスクリーン
			ToggleFullscreen(hWnd);
			break;
		}

		break;

	// 閉じるボタン押下のメッセージ
	case WM_CLOSE:		
		
		// ウィンドウを破棄する(WM_DESTROYメッセージを送る)
		DestroyWindow(hWnd);
		
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//================================================================================================================
// --- 初期化処理 ---
//================================================================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	D3DDISPLAYMODE d3ddm;			// ディスプレイモード
	D3DPRESENT_PARAMETERS d3dpp;	// プレゼンテーションパラメータ		

	// Direct3Dオブジェクトの作成
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3D == NULL)
	{
		MessageBox(hWnd, "Direct3Dオブジェクトの作成に失敗しました。", "Error (-1)", MB_ICONERROR);
		return E_FAIL;
	}

	// 現在のディスプレイモードを取得
	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
		&d3ddm)))
	{
		MessageBox(hWnd, "ディスプレイモードの取得に失敗しました。", "Error (-1)", MB_ICONERROR);
		return E_FAIL;
	}

	// デバイスのプレゼンテーションパラメータの設定
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// パラメータのゼロクリア

	d3dpp.BackBufferWidth = SCREEN_WIDTH;		// ゲームの画面サイズ(横)
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;		// ゲームの画面サイズ(高さ)
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// バックバッファの形式
	d3dpp.BackBufferCount = 1;					// バックバッファの数
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ダブルバッファの切り替え(映像信号と同期)
	d3dpp.EnableAutoDepthStencil = TRUE;		// デプスバッファとステンシルバッファを作成
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// デプスバッファとして16bitを使う
	d3dpp.Windowed = bWindow;					// ウィンドウモード
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;			// リフレッシュレート
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;			// インターバル

	// Direct3Dデバイスの作成
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&g_pD3DDevice)))
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&g_pD3DDevice)))
		{
			if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_REF,
				hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&d3dpp,
				&g_pD3DDevice)))
			{
				MessageBox(hWnd, "Direct3Dデバイスの作成に失敗しました。", "Error (-1)", MB_ICONERROR);
				return E_FAIL;
			}
		}
	}

	// レンダーステートの設定(消さないこと！ALPHA値の設定を適用する為の設定！)
	g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// サンプラーステートの設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);		// テクスチャの拡縮補間の設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);		// テクスチャの繰り返しの設定
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	// テクスチャステージステートの設定(テクスチャのアルファブレンドの設定[テクスチャとポリゴンのALPHA値を混ぜる！])
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	// キーボードの初期化処理
	if (FAILED(InitKeyboard(hInstance, hWnd)))
	{
		MessageBox(hWnd, "InitKeyboardに失敗しました。", "Error (0)", MB_ICONERROR);
		return E_FAIL;
	}

	// ジョイパッドの初期化処理
	if (FAILED(InitJoypad()))
	{
		MessageBox(hWnd, "InitJoypadに失敗しました。", "Error (1)", MB_ICONERROR);
		return E_FAIL;
	}

	// マウスの初期化処理
	if (FAILED(InitMouse(hWnd)))
	{
		MessageBox(hWnd, "InitMouseに失敗しました。", "Error (2)", MB_ICONERROR);
		return E_FAIL;
	}

	// サウンドの初期化処理
	if (FAILED(InitSound(hWnd)))
	{
		MessageBox(hWnd, "InitSoundに失敗しました。", "Error (3)", MB_ICONERROR);
		return E_FAIL;
	}

	InitDebugProc();

#ifdef MODE_ON
	
	g_nStageExac = 0;

	InitFade(g_mode);
	SetMode(g_mode);

	/*** タイトルBGMをフェードイン ***/
	//FadeSound(SOUND_LABEL_BGM_TITLE);

#endif // MODE_ON

	return S_OK;
}

//================================================================================================================
// --- 終了処理 ---
//================================================================================================================
void Uninit(void)
{
#ifdef MODE_ON
	// タイトル画面の終了処理
	UninitTitle();

	// ゲーム画面の終了処理
	UninitGame();

	// リザルト画面の終了処理
	UninitRanking();

	// クリア画面の終了処理
	UninitGameClear();

	// ゲームオーバー画面の終了処理
	UninitGameOver();

	// チュートリアル終了処理
	UninitTutorial();

	UninitScript();

	UninitObject();

	// フェードの終了処理
	UninitFade();
#endif

	UninitDebugProc();
	UninitCamera();

	// キーボードの終了処理
	UninitKeyboard();

	// ジョイパッドの終了処理
	UninitJoypad();

	// マウスの終了処理
	UninitMouse();

	// サウンドの終了処理
	UninitSound();

	// Direct3Dデバイスの破棄
	if (g_pD3DDevice != NULL)
	{
		g_pD3DDevice->Release();

		g_pD3DDevice = NULL;
	}

	// Direct3Dオブジェクトの破棄
	if (g_pD3D != NULL)
	{
		g_pD3D->Release();

		g_pD3D = NULL;
	}
}

//================================================================================================================
// --- 更新処理 ---
//================================================================================================================
void Update(void)
{
	PrintDebugProc("FPS : %d\n", g_nCountFPS);

	PrintDebugProc("BACKSPACE : 弾発射\nP : ポーズ\n");

	//キーボードの更新処理(これより上に書くな)
	UpdateKeyboard();

	//ジョイパッドの更新処理
	UpdateJoypad();

	// マウスの更新処理
	UpdateMouse();

	UpdateDebugProc();

#ifdef MODE_ON
#if ENABLE_CLICKUP
	if (GetKeyboardRepeat(DIK_U))
	{
		/*** 現在の画面(モード)の更新処理 ***/
		switch (g_mode)
		{
			// タイトル画面
			case MODE_TITLE:
				UpdateTitle();
				break;

			// ゲーム画面
		case MODE_GAME:
			UpdateGame();
			break;

			// リザルト画面
			case MODE_RESULT:
				UpdateResult();
				break;

			// ランキング画面
			case MODE_RANKING:
				UpdateRanking();
				break;
		}
	}
#else
	/*** 現在の画面(モード)の更新処理 ***/
	switch (g_mode)
	{
		// タイトル画面
		case MODE_TITLE:
			UpdateTitle();
			break;

			// チュートリアル画面
		case MODE_TUTORIAL:
			UpdateTutorial();
			break;

		// ゲーム画面
		case MODE_GAME:
			UpdateGame();
		break;

		// クリア画面
		case MODE_GOODEND:
			UpdateGameClear();
			break;

			// ゲームオーバー画面
		case MODE_BADEND:
			UpdateGameOver();
			break;

		// ランキング画面
		case MODE_RANKING:
			UpdateRanking();
			break;
	}
#endif

	// フェードの更新処理
	UpdateFade();

	// サウンドの更新処理
	UpdateSound();
#endif
}

//================================================================================================================
// --- 描画処理 ---
//================================================================================================================
void Draw(void)
{
	HRESULT hr;

	// 画面クリア(バックバッファとZバッファのクリア)
	g_pD3DDevice->Clear(0, NULL,
		(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER),
		D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	// 描画開始
	if (SUCCEEDED(g_pD3DDevice->BeginScene()))
	{// 描画開始が成功した場合
#ifdef MODE_ON
			/*** 現在の画面(モード)の描画処理 ***/
		switch (g_mode)
		{
			// タイトル画面
		case MODE_TITLE:
			DrawTitle();
			break;

			// チュートリアル画面
		case MODE_TUTORIAL:
			DrawTutorial();
			break;

			// ゲーム画面
		case MODE_GAME:
			DrawGame();
			break;

			// クリア画面
		case MODE_GOODEND:
			DrawGameClear();
			break;

			// ゲームオーバー画面
		case MODE_BADEND:
			DrawGameOver();
			break;

			// ランキング画面
		case MODE_RANKING:
			DrawRanking();
			break;
		}

		// フェードの描画処理
		DrawFade();
#endif

		// デバッグ表示
		DrawDebugProc();

		// 描画終了
		g_pD3DDevice->EndScene();
	}

	// バックバッファとフロントバッファの入れ替え
   	hr = g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
#ifdef _DEBUG
	if (FAILED(hr))
	{
		LPVOID* errorString;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER					// テキストのメモリ割り当てを要求する
			| FORMAT_MESSAGE_FROM_SYSTEM					// エラーメッセージはWindowsが用意しているものを使用
			| FORMAT_MESSAGE_IGNORE_INSERTS,				// 次の引数を無視してエラーコードに対するエラーメッセージを作成する
			NULL, 
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),		// 言語を指定
			(LPTSTR)&errorString,							// メッセージテキストが保存されるバッファへのポインタ
			0,
			NULL);

		GenerateMessageBox(MB_ICONERROR,
			"error",
			(LPCTSTR)errorString);

		LocalFree(errorString);
	}
#endif
}

//================================================================================================================
// --- デバイスの取得 ---
//================================================================================================================
LPDIRECT3DDEVICE9 GetDevice(void)
{
	return g_pD3DDevice;
}

#ifdef MODE_ON
//================================================================================================================
// --- 画面の設定 ---
//================================================================================================================
void SetMode(MODE mode)
{
	g_modeInit = mode;

	// 現在の画面(モード)の終了
	switch (g_mode)
	{
	// タイトル画面
	case MODE_TITLE:
		UninitTitle();
		break;

		// チュートリアル画面
	case MODE_TUTORIAL:
		UninitTutorial();
		break;

	// ゲーム画面
	case MODE_GAME:
		UninitGame();
		break;

		// クリア画面
	case MODE_GOODEND:
		UninitGameClear();
		break;

		// ゲームオーバー画面
	case MODE_BADEND:
		UninitGameOver();
		break;

	// ランキング画面
	case MODE_RANKING:
		UninitRanking();
		break;
	}

	// 現在のモードを保存
	g_modeExac = g_mode;

	// 指定の画面(モード)の初期化処理
	switch (mode)
	{
	// タイトル画面
	case MODE_TITLE:
		InitTitle();
		break;

		// チュートリアル画面
	case MODE_TUTORIAL:
		InitTutorial();
		break;

	// ゲーム画面
	case MODE_GAME:
		if (FAILED(InitGame()))
		{
			DestroyWindow(g_hWnd);
		}

		break;

	// クリア画面
	case MODE_GOODEND:
		InitGameClear();
		break;

	// ゲームオーバー画面
	case MODE_BADEND:
		InitGameOver();
		break;

	// ランキング画面
	case MODE_RANKING:
		InitRanking();
		break;
	}

	// モードを保存
	g_mode = mode;
}

//================================================================================================================
// --- 現在のモードを取得 ---
//================================================================================================================
MODE GetMode(void)
{
	return g_mode;
}

//================================================================================================================
// --- 直前のモードを取得 ---
//================================================================================================================
MODE GetModeExac(void)
{
	return g_modeExac;
}

//================================================================================================================
// --- Init後のモードを取得 ---
//================================================================================================================
MODE GetModeNext(void)
{
	return g_modeInit;
}

#endif

//================================================================================================================
// -- 獲得済みウィンドウの取得 ---
//================================================================================================================
HWND GetHandleWindow(void)
{
	if (g_hWnd != NULL)
	{
		return g_hWnd;
	}
	else
	{
		MessageBox(g_hWnd, "ウィンドウハンドルが確保されていません!", "警告！", MB_ICONWARNING);
		return NULL;
	}
}

//================================================================================================================
// --- デバッグ表示 ---
//================================================================================================================
void DrawDebug(void)
{

}

//===============================================================================================================
// --- ウィンドウフルスクリーン処理 ---
//===============================================================================================================
void ToggleFullscreen(HWND hWnd)
{
	// 現在のウィンドウスタイルを取得
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	if (g_isFullscreen)
	{
		// ウィンドウモードに切り替え
		SetWindowLong(hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(hWnd, HWND_TOP, g_windowRect.left, g_windowRect.top,
			g_windowRect.right - g_windowRect.left, g_windowRect.bottom - g_windowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_NORMAL);
	}
	else
	{
		// フルスクリーンモードに切り替え
		GetWindowRect(hWnd, &g_windowRect);
		SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_MAXIMIZE);
	}

	g_isFullscreen = !g_isFullscreen;
}

//================================================
// --- ゲームの難易度設定処理 ---
//================================================
void SetGameDifficulty(GAMEDIFFICULTY difficulty)
{
	g_Difficulty = difficulty;
}

//================================================
// --- ゲームの難易度設定処理 ---
//================================================
GAMEDIFFICULTY GetGameDifficulty(void)
{
	return g_Difficulty;
}

//================================================
// --- ゲームの直前のステージ番号保存処理 ---
//================================================
void SetStageExac(int nStage)
{
	g_nStageExac = nStage;
}

//================================================
// --- ゲームの直前のステージ番号取得処理 ---
//================================================
int GetStageExac(void)
{
	return g_nStageExac;
}

//=====================
// ワイヤーフレームON
//=====================
void onWireFrame()
{
	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
}

//=====================
// ワイヤーフレームOFF
//=====================
void offWireFrame()
{
	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}
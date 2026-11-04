//==================================================================================
//
// スクリプトファイル読み込み処理 [loadxfile.cpp]
// Author : TENMA SAITO
//
// テクスチャ / モデル / 配置情報を記述したスクリプト(model.txt等)と、
// 階層モデルのモーションスクリプト(motion_xxx.txt)を読み込む。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "loadxfile.h"
#include "camera.h"
#include "player.h"
#include "object.h"
#include "wall.h"
#include "field.h"
#include "effect.h"
#include "shadow.h"
#include "mesh.h"
#include "XmasTree.h"
#include "ornament.h"
#include "playerTitle.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MAX_MODEL				(MAX_PLAYER + MAX_MODEL_RESERVE + MAX_OBJECT)		// 読み込めるモデル最大数
#define MAX_MODEL_RESERVE		(32)					// モデル数の予備 (旧 MAX_ENEMY の分)
#define MAX_TEXTURE				(128)				// 読み込めるテクスチャの最大数
#define MAX_MOTION_MODEL		(30)				// モーションスクリプトで読み込めるモデルの最大数

// --- スクリプト共通 ---
#define READ_SCRIPT				"SCRIPT"			// スクリプトの開始位置
#define READ_ENDSCRIPT			"END_SCRIPT"		// スクリプトの終了位置
#define READ_TEXTURENUM			"NUM_TEXTURE"		// 読み込むテクスチャ数
#define READ_TEXTUREPATH		"TEXTURE_FILENAME"	// テクスチャのファイル名
#define READ_MODELNUM			"NUM_MODEL"			// 読み込むモデル数
#define READ_MODELPATH			"MODEL_FILENAME"	// モデルのファイル名

// --- 配置情報ブロック ---
#define READ_CAMERASET			"CAMERASET"			// カメラ情報の開始位置
#define READ_ENDCAMERASET		"END_CAMERASET"		// カメラ情報の終了位置
#define READ_MODELSET			"MODELSET"			// モデル情報の開始位置
#define READ_ENDMODELSET		"END_MODELSET"		// モデル情報の終了位置
#define READ_ORNAMENTSET		"ORNAMENTSET"		// オーナメント情報の開始位置
#define READ_ENDORNAMENTSET		"END_ORNAMENTSET"	// オーナメント情報の終了位置
#define READ_ORNAMENT			"ORNAMENT"			// オーナメント1種類分の開始位置
#define READ_ENDORNAMENT		"END_ORNAMENT"		// オーナメント1種類分の終了位置
#define READ_FIELDSET			"FIELDSET"			// 地面情報の開始位置
#define READ_ENDFIELDSET		"END_FIELDSET"		// 地面情報の終了位置
#define READ_WALLSET			"WALLSET"			// 壁情報の開始位置
#define READ_ENDWALLSET			"END_WALLSET"		// 壁情報の終了位置
#define READ_MESHFIELDSET		"MESHFIELDSET"		// メッシュフィールド情報の開始位置
#define READ_ENDMESHFIELDSET	"END_MESHFIELDSET"	// メッシュフィールド情報の終了位置
#define READ_TREESET			"TREESET"			// 木(ビルボード)情報の開始位置
#define READ_ENDTREESET			"END_TREESET"		// 木(ビルボード)情報の終了位置
#define READ_XMASTREESET		"XMASTREESET"		// クリスマスツリー情報の開始位置
#define READ_ENDXMASTREESET		"END_XMASTREESET"	// クリスマスツリー情報の終了位置
#define READ_EFFECTSET			"EFFECTSET"			// エフェクト情報の開始位置
#define READ_ENDEFFECTSET		"END_EFFECTSET"		// エフェクト情報の終了位置
#define READ_SHADOWSET			"SHADOWSET"			// 影情報の開始位置
#define READ_ENDSHADOWSET		"END_SHADOWSET"		// 影情報の終了位置
#define READ_BULLETSET			"BULLETSET"			// 弾情報の開始位置
#define READ_ENDBULLETSET		"END_BULLETSET"		// 弾情報の終了位置
#define READ_EXPLOSIONSET		"EXPLOSIONSET"		// 爆発情報の開始位置
#define READ_ENDEXPLOSIONSET	"END_EXPLOSIONSET"	// 爆発情報の終了位置

// --- 配置情報の項目 ---
#define READ_POSV				"POSV"				// 視点位置
#define READ_POSR				"POSR"				// 注視点位置
#define READ_ZLENGTH			"ZLEN"				// 注視点距離
#define READ_VANGLE				"VANGLE"			// 視野角
#define READ_ZN					"ZN"				// 描画開始距離
#define READ_ZF					"ZF"				// 描画限界距離
#define READ_ISSTAR				"ISSTAR"			// オーナメントの種類
#define READ_TYPE				"TYPE"				// 種類
#define READ_TEXTYPE			"TEXTYPE"			// テクスチャの種類
#define READ_INDEX				"INDEX"				// インデックス
#define READ_POS				"POS"				// 位置
#define READ_ROT				"ROT"				// 角度
#define READ_COL				"COL"				// 色
#define READ_MOVE				"MOVE"				// 移動量
#define READ_SPD				"SPD"				// 速度
#define READ_COLLISION			"COLLISION"			// 当たり判定の有無
#define READ_CULL				"CULL"				// カリングタイプ
#define READ_ROTSPD				"ROTSPEED"			// 回転速度
#define READ_BLOCK				"BLOCK"				// 分割数
#define READ_SIZE				"SIZE"				// サイズ
#define READ_NUMORNAMENT		"NUM_ORNAMENT"		// 設定するオーナメントの数
#define READ_WEIGHT				"WEIGHT"			// 重量

// --- MODELSET の INDEX に指定する設置タイプ ---
#define PLAYERTYPE				(-1)				// プレイヤーとして設置
#define OBJECTTYPE				(1)					// オブジェクトとして設置

// --- モーションスクリプトの項目 ---
#define READ_CHARACTER			"CHARACTERSET"		// 階層構造のキャラクター情報
#define READ_ENDCHARACTER		"END_CHARACTERSET"	// キャラクター情報設定の解除
#define READ_PARTS				"NUM_PARTS"			// パーツ数
#define READ_RADIUS				"RADIUS"			// 球の当たり判定の半径
#define READ_HEIGHT				"HEIGHT"			// 上下の当たり判定の大きさ
#define READ_JUMP				"JUMP"				// 跳躍力
#define READ_HEAD				"HEAD"				// 頭のインデックスの管理
#define READ_HAND				"HAND"				// 手のインデックス管理
#define READ_PARTSSET			"PARTSSET"			// パーツ情報の設定
#define READ_PARENT				"PARENT"			// パーツの親インデックス
#define READ_ENDPARTSSET		"END_PARTSSET"		// パーツ情報設定の解除
#define READ_MOTIONSET			"MOTIONSET"			// モーションデータの読み込み
#define READ_ENDMOTIONSET		"END_MOTIONSET"		// モーション設定の解除
#define READ_KEYSET				"KEYSET"			// キー情報の読み込み
#define READ_ENDKEYSET			"END_KEYSET"		// キー情報設定の解除
#define READ_KEY				"KEY"				// キーの読み込み
#define READ_ENDKEY				"END_KEY"			// キー設定の解除
#define READ_LOOP				"LOOP"				// ループ情報
#define READ_NUMKEY				"NUM_KEY"			// キー情報の数
#define READ_FRAME				"FRAME"				// フレーム数

//**********************************************************************************
// *** スクリプト読み込み状態構造体 ***
//**********************************************************************************
typedef struct
{
	FILE *pFile;				// 読み込み中のファイル
	char aLine[MAX_PATH];		// 直前に読み込んだ1行(コメント除去済み)
	bool bEof;					// 直前の読み込みでファイル終端に達したか
} SCRIPT_READER;

//**********************************************************************************
// *** モーション読み込み状態構造体 ***
//**********************************************************************************
typedef struct
{
	MODE mode;								// 読み込み中のモード(タイトルか否かで登録先が変わる)
	int aIdxObject[MAX_MOTION_MODEL];		// モデル番号 → オブジェクト番号の対応表
	int nNumModelPath;						// 読み込んだモデルパスの数
	MOTIONTYPE motionType;					// 次に登録するモーションの種類
} MOTION_LOADER;

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
// --- 共通 ---
static bool DeleteComments(char *pLine);
static bool ReadScriptLine(SCRIPT_READER *pReader);
static const char *GetValue(const char *pLine);
static HRESULT SeekScriptStart(SCRIPT_READER *pReader);
static void ReleaseScriptTexture(void);

// --- InitScript 用 ---
static void ReadTexturePath(SCRIPT_READER *pReader, int nCntTexture);
static void ReadModelPath(SCRIPT_READER *pReader, int nCntModel);
static void ReadCameraSet(SCRIPT_READER *pReader);
static void ReadOrnamentSet(SCRIPT_READER *pReader);
static void ReadModelSet(SCRIPT_READER *pReader, int nCntModel);
static void ReadXmasTreeSet(SCRIPT_READER *pReader, int nCntModel);
static void ReadFieldSet(SCRIPT_READER *pReader, int nCntField);
static void ReadMeshFieldSet(SCRIPT_READER *pReader, int nCntField);
static void ReadWallSet(SCRIPT_READER *pReader, int nCntWall);
static void SkipScriptBlock(SCRIPT_READER *pReader, const char *pEndText);
static void ReadTextureIndexSet(SCRIPT_READER *pReader, const char *pEndText, void (*pSetFunc)(int), const char *pErrorCaption, const char *pErrorText);

// --- LoadMotion 用 ---
static HRESULT ReadMotionLine(SCRIPT_READER *pReader);
static void ReadMotionModelPath(SCRIPT_READER *pReader, MOTION_LOADER *pLoader);
static HRESULT ReadCharacterSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader);
static HRESULT ReadPartsSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader, PartsInfo *pParts);
static HRESULT ReadMotionSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader);
static HRESULT ReadKeySet(SCRIPT_READER *pReader, int nKeySet);
static HRESULT ReadKey(SCRIPT_READER *pReader, int nKeySet, int nKey);

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static LPDIRECT3DTEXTURE9 g_apTextureModel[MAX_TEXTURE] = {};	// 読み込んだテクスチャ
static char g_aXFileName[MAX_MODEL][MAX_PATH];					// 読み込んだXファイルのパス
static char g_aTextureFileName[MAX_TEXTURE][MAX_PATH];			// 読み込んだテクスチャのパス
static int g_nTextureNum = 0;									// 読み込むテクスチャ数
static int g_nModelNum = 0;										// 読み込んだモデルの数
static MOTION_INFO g_motionInfo = {};							// 読み込み中のモーション情報

//==================================================================================
// --- スクリプトの読み込み ---
//==================================================================================
HRESULT InitScript(_In_ const char *pScriptFileName)
{
	if (GetHandleWindow() == NULL || pScriptFileName == NULL)
	{ // ウィンドウ未生成 or ファイル名未指定
		return E_FAIL;
	}

	SCRIPT_READER reader = {};		// スクリプト読み込み状態
	int nCntTexture = 0;			// 読み込んだテクスチャ数
	int nCntModel = 0;				// 読み込んだモデルパス数
	int nCntSetModel = 0;			// 設置したモデル数(MODELSET + XMASTREESET)
	int nCntField = 0;				// 設置した床の数
	int nCntWall = 0;				// 設置した壁の数

	/*** 前回読み込んだデータの破棄・初期化 ***/
	// 0クリアだけだと解放前のテクスチャが参照不能になりリークするため、必ずReleaseする
	ReleaseScriptTexture();
	g_nModelNum = 0;
	g_nTextureNum = 0;
	ZeroMemory(g_aTextureFileName, sizeof g_aTextureFileName);
	ZeroMemory(g_aXFileName, sizeof g_aXFileName);

	/*** ファイルオープン ***/
	reader.pFile = fopen(pScriptFileName, "r");
	if (reader.pFile == NULL)
	{ // ファイルオープン失敗時
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "Scriptファイルの読み込みに失敗しました。");
		return E_FAIL;
	}

	/*** SCRIPT の行まで読み飛ばす ***/
	if (FAILED(SeekScriptStart(&reader)))
	{
		fclose(reader.pFile);
		return E_FAIL;
	}

	/*** END_SCRIPT まで1行ずつ解釈 ***/
	while (1)
	{
		bool bValid = ReadScriptLine(&reader);		// 有効な行か
		if (reader.bEof) break;						// EOFの場合、読み込み終了
		if (bValid == false) continue;				// コメント行・空行は読み飛ばし

		const char *pLine = reader.aLine;			// 読み込んだ行

		// ※ strstr による部分一致判定のため、判定の順番を変えないこと
		if (strstr(pLine, READ_TEXTURENUM) != NULL)
		{ // 読み込むテクスチャ数 (上限を超えたら制限)
			(void)sscanf(GetValue(pLine), "%d", &g_nTextureNum);
			if (g_nTextureNum > MAX_TEXTURE) g_nTextureNum = MAX_TEXTURE;
		}
		else if (strstr(pLine, READ_TEXTUREPATH) != NULL && nCntTexture < g_nTextureNum)
		{ // テクスチャパス (読み込む数分のみ)
			ReadTexturePath(&reader, nCntTexture);
			nCntTexture++;
		}
		else if (strstr(pLine, READ_MODELNUM) != NULL)
		{ // 読み込むモデル数 (上限を超えたら制限)
			(void)sscanf(GetValue(pLine), "%d", &g_nModelNum);
			if (g_nModelNum > MAX_MODEL) g_nModelNum = MAX_MODEL;
		}
		else if (strstr(pLine, READ_MODELPATH) != NULL && nCntModel < g_nModelNum)
		{ // モデルパス (読み込む数分のみ)
			ReadModelPath(&reader, nCntModel);
			nCntModel++;
		}
		else if (strcmp(pLine, READ_CAMERASET) == 0)
		{ // カメラ
			ReadCameraSet(&reader);
		}
		else if (strcmp(pLine, READ_ORNAMENTSET) == 0)
		{ // オーナメントの種類情報
			ReadOrnamentSet(&reader);
		}
		else if (strcmp(pLine, READ_MODELSET) == 0 && nCntSetModel < MAX_MODEL)
		{ // モデルの設置
			ReadModelSet(&reader, nCntSetModel);
			nCntSetModel++;
		}
		else if (strcmp(pLine, READ_XMASTREESET) == 0 && nCntSetModel < MAX_MODEL)
		{ // クリスマスツリーの設置
			ReadXmasTreeSet(&reader, nCntSetModel);
			nCntSetModel++;
		}
		else if (strcmp(pLine, READ_FIELDSET) == 0)
		{ // 床の設置
			ReadFieldSet(&reader, nCntField);
			nCntField++;
		}
		else if (strcmp(pLine, READ_MESHFIELDSET) == 0)
		{ // メッシュフィールドの設置 (エラー表示用の番号は床と共有)
			ReadMeshFieldSet(&reader, nCntField);
		}
		else if (strcmp(pLine, READ_WALLSET) == 0)
		{ // 壁の設置
			ReadWallSet(&reader, nCntWall);
			nCntWall++;
		}
		else if (strcmp(pLine, READ_TREESET) == 0)
		{ // 木(ビルボード) : 現在は未使用のため読み飛ばす
			SkipScriptBlock(&reader, READ_ENDTREESET);
		}
		else if (strstr(pLine, READ_EFFECTSET) != NULL)
		{ // エフェクトのテクスチャ
			ReadTextureIndexSet(&reader, READ_ENDEFFECTSET, SetIndexTextureEffect,
				"Error (7)", "エフェクトのテクスチャ設定に失敗しました。");
		}
		else if (strstr(pLine, READ_SHADOWSET) != NULL)
		{ // 影のテクスチャ
			ReadTextureIndexSet(&reader, READ_ENDSHADOWSET, SetIndexTextureShadow,
				"Error (8)", "影のテクスチャ設定に失敗しました。");
		}
		else if (strstr(pLine, READ_BULLETSET) != NULL)
		{ // 弾 : 現在は未使用のため読み飛ばす
			SkipScriptBlock(&reader, READ_ENDBULLETSET);
		}
		else if (strstr(pLine, READ_EXPLOSIONSET) != NULL)
		{ // 爆発 : 現在は未使用のため読み飛ばす
			SkipScriptBlock(&reader, READ_ENDEXPLOSIONSET);
		}
		else if (strstr(pLine, READ_ENDSCRIPT) != NULL)
		{ // 読み込み終了の合図
			break;
		}
	}

	/*** 読み込み終了 ***/
	fclose(reader.pFile);

	return S_OK;
}

//==================================================================================
// --- モーションスクリプトの読み込み ---
//==================================================================================
HRESULT LoadMotion(_In_ const char *pMotionFileName)
{
	if (GetHandleWindow() == NULL)
	{ // ウィンドウ未生成
		return E_FAIL;
	}

	SCRIPT_READER reader = {};		// スクリプト読み込み状態
	MOTION_LOADER loader = {};		// モーション読み込み状態

	loader.mode = GetModeNext();				// Init中のモードを取得
	loader.motionType = MOTIONTYPE_NEUTRAL;		// 最初のモーションはニュートラル

	ZeroMemory(&g_motionInfo, sizeof(MOTION_INFO));

	/*** ファイルオープン ***/
	reader.pFile = fopen(pMotionFileName, "r");
	if (reader.pFile == NULL)
	{ // ファイルオープン失敗時
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "Scriptファイルの読み込みに失敗しました。");
		return E_FAIL;
	}

	/*** SCRIPT の行まで読み飛ばす ***/
	if (FAILED(SeekScriptStart(&reader)))
	{
		fclose(reader.pFile);
		return E_FAIL;
	}

	/*** END_SCRIPT まで1行ずつ解釈 ***/
	// ※ 各 Read 関数が E_FAIL を返した時点で、エラー表示とファイルクローズは済んでいる
	while (1)
	{
		HRESULT hr = ReadMotionLine(&reader);		// 1行読み込み
		if (FAILED(hr)) return E_FAIL;				// 終了位置の前にEOF
		if (hr == S_FALSE) continue;				// コメント行・空行は読み飛ばし

		const char *pLine = reader.aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDSCRIPT) != NULL)
		{ // 読み込み終了の合図
			break;
		}
		else if (strstr(pLine, READ_MODELNUM) != NULL)
		{ // モデル数 (READ_MODELPATH の数で判断するため、値は使用しない)
		}
		else if (strstr(pLine, READ_MODELPATH) != NULL)
		{ // パーツのモデルパス
			ReadMotionModelPath(&reader, &loader);
		}
		else if (strstr(pLine, READ_CHARACTER) != NULL)
		{ // キャラクター(パーツ構成)情報
			if (FAILED(ReadCharacterSet(&reader, &loader))) return E_FAIL;
		}
		else if (strstr(pLine, READ_MOTIONSET) != NULL)
		{ // モーション情報
			if (FAILED(ReadMotionSet(&reader, &loader))) return E_FAIL;
		}
	}

	/*** 読み込み終了 ***/
	fclose(reader.pFile);

	return S_OK;
}

//==================================================================================
// --- 読み込んだテクスチャの終了処理 ---
//==================================================================================
void UninitScript(void)
{
	ReleaseScriptTexture();
}

//==================================================================================
// --- 読み込んだテクスチャの取得 ---
//==================================================================================
LPDIRECT3DTEXTURE9 GetTexture(_In_ int nIndexTexture)
{
	if (FAILED(CheckIndex(g_nTextureNum, nIndexTexture)))
	{ // インデックス範囲外
		return NULL;
	}

	return g_apTextureModel[nIndexTexture];
}

//==================================================================================
//
// ここから共通の内部処理
//
//==================================================================================

//==================================================================================
// --- 読み込んだテクスチャの全破棄 ---
//==================================================================================
static void ReleaseScriptTexture(void)
{
	for (int nCntTexture = 0; nCntTexture < MAX_TEXTURE; nCntTexture++)
	{
		RELEASE(g_apTextureModel[nCntTexture]);
	}
}

//==================================================================================
// --- コメントアウト関連消去処理 ---
// 戻り値 : true = 読み込む行 / false = コメント行・空行
//==================================================================================
static bool DeleteComments(char *pLine)
{
	if (pLine[0] == '#' || pLine[0] == '\n')
	{ // 行頭が # (コメントアウト) or 空行の場合、読み込まない
		return false;
	}

	char *pFound = NULL;		// 検索した文字の位置

	/*** 改行を消去 ***/
	pFound = strchr(pLine, '\n');
	if (pFound != NULL) *pFound = '\0';

	/*** 行の途中の # 以降を消去 ***/
	pFound = strchr(pLine, '#');
	if (pFound != NULL) *pFound = '\0';

	/*** 行頭のタブを詰める ***/
	while (pLine[0] == '\t')
	{
		memmove(&pLine[0], &pLine[1], strlen(pLine));	// 終端文字ごと1文字詰める
	}

	/*** 行の途中のタブ以降を消去 ***/
	pFound = strchr(pLine, '\t');
	if (pFound != NULL) *pFound = '\0';

	return true;
}

//==================================================================================
// --- 1行読み込み (コメントを除去して pReader->aLine に格納) ---
// 戻り値 : true = 読み込む行 / false = コメント行・空行
//==================================================================================
static bool ReadScriptLine(SCRIPT_READER *pReader)
{
	memset(pReader->aLine, 0, sizeof pReader->aLine);
	(void)fgets(pReader->aLine, sizeof pReader->aLine, pReader->pFile);
	pReader->bEof = (feof(pReader->pFile) != 0);

	return DeleteComments(pReader->aLine);
}

//==================================================================================
// --- "項目 = 値" の値部分の先頭を取得 ( = が無ければ空文字列) ---
//==================================================================================
static const char *GetValue(const char *pLine)
{
	const char *pEqual = strchr(pLine, '=');		// = の位置
	if (pEqual == NULL)
	{
		return "";
	}

	return pEqual + 1;
}

//==================================================================================
// --- SCRIPT の行まで読み飛ばす ---
//==================================================================================
static HRESULT SeekScriptStart(SCRIPT_READER *pReader)
{
	while (1)
	{
		bool bValid = ReadScriptLine(pReader);		// 有効な行か

		if (bValid && strcmp(pReader->aLine, READ_SCRIPT) == 0)
		{ // SCRIPT 発見時、読み込み開始
			return S_OK;
		}

		if (pReader->bEof)
		{ // 発見できなかった場合
			GenerateMessageBox(MB_ICONERROR, "Error (1)", "Scriptの読み込み開始位置が指定されていません。");
			return E_FAIL;
		}
	}
}

//==================================================================================
//
// ここから InitScript 用の内部処理
// 各 ReadXxxSet は END_XXX の行(またはEOF)まで読み進めてから戻る
//
//==================================================================================

//==================================================================================
// --- テクスチャパスの読み取り及びテクスチャロード ---
//==================================================================================
static void ReadTexturePath(SCRIPT_READER *pReader, int nCntTexture)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();				// デバイス
	char *pFileName = g_aTextureFileName[nCntTexture];		// 格納先のテクスチャパス

	/*** テクスチャパスの読み取り ***/
	memset(pFileName, 0, MAX_PATH);
	(void)sscanf(GetValue(pReader->aLine), "%s", pFileName);
	CheckPath(pFileName);

	/*** テクスチャの読み込み ***/
	if (FAILED(D3DXCreateTextureFromFile(pDevice, pFileName, &g_apTextureModel[nCntTexture])))
	{ // テクスチャの読み込み失敗時
		GenerateMessageBox(MB_ICONERROR, "Error (1)", "%d番目のテクスチャの読み込みに失敗しました。", nCntTexture);
	}
}

//==================================================================================
// --- モデルパスの読み取り及びモデルロード ---
//==================================================================================
static void ReadModelPath(SCRIPT_READER *pReader, int nCntModel)
{
	char *pFileName = g_aXFileName[nCntModel];		// 格納先のモデルパス

	/*** モデルパスの読み取り ***/
	memset(pFileName, 0, MAX_PATH);
	(void)sscanf(GetValue(pReader->aLine), "%s", pFileName);

	/*** モデルの読み込み ***/
	if (FAILED(LoadObject(pFileName)))
	{ // モデルの読み込みに失敗
		GenerateMessageBox(MB_ICONERROR, "Error (2)", "%d番目のモデルの読み込みに失敗しました。", nCntModel);
	}
}

//==================================================================================
// --- CAMERASET ～ END_CAMERASET の読み込み及びカメラの設置 ---
//==================================================================================
static void ReadCameraSet(SCRIPT_READER *pReader)
{
	INT_VECTOR3 nPosV = INT_VECTOR3_NULL;		// 視点位置(整数)
	INT_VECTOR3 nPosR = INT_VECTOR3_NULL;		// 注視点位置(整数)
	int nZLength = 0;							// 注視点との距離(整数)
	int nViewAngle = 0;							// 視野角(整数)
	int nZNear = 0;								// 描画開始距離(整数)
	int nZFar = 0;								// 描画終了距離(整数)
	float fRotSpd = 0.0f;						// 回転速度
	float fSpd = 0.0f;							// 移動速度

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_POSV) != NULL)			(void)sscanf(GetValue(pLine), "%d %d %d", &nPosV.x, &nPosV.y, &nPosV.z);
			else if (strstr(pLine, READ_POSR) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPosR.x, &nPosR.y, &nPosR.z);
			else if (strstr(pLine, READ_ZLENGTH) != NULL)	(void)sscanf(GetValue(pLine), "%d", &nZLength);
			else if (strstr(pLine, READ_VANGLE) != NULL)	(void)sscanf(GetValue(pLine), "%d", &nViewAngle);
			else if (strstr(pLine, READ_ZN) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nZNear);
			else if (strstr(pLine, READ_ZF) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nZFar);
			else if (strstr(pLine, READ_SPD) != NULL)		(void)sscanf(GetValue(pLine), "%f", &fSpd);
			else if (strstr(pLine, READ_ROTSPD) != NULL)	(void)sscanf(GetValue(pLine), "%f", &fRotSpd);

			if (strstr(pLine, READ_ENDCAMERASET) != NULL)
			{ // カメラ情報の終了 → 設置
				SetCamera(INTToFloat(nPosV),
					INTToFloat(nPosR),
					(float)nViewAngle,
					(float)nZNear,
					(float)nZFar,
					(float)nZLength,
					fSpd,
					fRotSpd);
				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- ORNAMENTSET ～ END_ORNAMENTSET の読み込み及びオーナメント情報の登録 ---
//==================================================================================
static void ReadOrnamentSet(SCRIPT_READER *pReader)
{
	int nNumOrnament = -1;		// 登録するオーナメント数
	int nCntOrnament = 0;		// 登録したオーナメント数

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_NUMORNAMENT) != NULL)
			{ // 登録数
				(void)sscanf(GetValue(pLine), "%d", &nNumOrnament);
			}
			else if (strcmp(pLine, READ_ORNAMENT) == 0 && nCntOrnament < nNumOrnament)
			{ // ORNAMENT ～ END_ORNAMENT : 1種類分の情報
				int nOrnamentType = -1;		// オーナメントの種類(ISSTAR)
				int nModelType = -1;		// モデル番号
				float fRadius = 0.0f;		// 当たり判定の半径
				float fWeight = 0.0f;		// 重さ

				while (1)
				{
					if (ReadScriptLine(pReader))
					{
						pLine = pReader->aLine;

						if (strstr(pLine, READ_TYPE) != NULL)			(void)sscanf(GetValue(pLine), "%d", &nModelType);
						else if (strstr(pLine, READ_ISSTAR) != NULL)	(void)sscanf(GetValue(pLine), "%d", &nOrnamentType);
						else if (strstr(pLine, READ_RADIUS) != NULL)	(void)sscanf(GetValue(pLine), "%f", &fRadius);
						else if (strstr(pLine, READ_WEIGHT) != NULL)	(void)sscanf(GetValue(pLine), "%f", &fWeight);
						else if (strstr(pLine, READ_ENDORNAMENT) != NULL)
						{ // オーナメント情報の登録
							SettingOrnamentInfo(nOrnamentType, nModelType, fRadius, fWeight);
							nCntOrnament++;
							break;
						}
					}

					if (pReader->bEof) return;		// 終了位置が無いままEOF
				}
			}
			else if (strcmp(pLine, READ_ENDORNAMENTSET) == 0)
			{ // オーナメント情報の終了
				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- MODELSET ～ END_MODELSET の読み込み及びモデルの設置 ---
//==================================================================================
static void ReadModelSet(SCRIPT_READER *pReader, int nCntModel)
{
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// 位置(整数)
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// 角度(度数・整数)
	int nIndexModel = 0;						// 設置するモデルの番号
	int nTypeModel = OBJECTTYPE;				// 設置タイプ(プレイヤー or オブジェクト)
	float fSpd = 0.0f;							// 移動速度(キャラクターのみ・現在未使用)
	int nCollisionType = 1;						// 当たり判定の種類

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_INDEX) != NULL)			(void)sscanf(GetValue(pLine), "%d", &nTypeModel);
			else if (strstr(pLine, READ_TYPE) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nIndexModel);
			else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
			else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
			else if (strstr(pLine, READ_SPD) != NULL)		(void)sscanf(GetValue(pLine), "%f", &fSpd);
			else if (strstr(pLine, READ_COLLISION) != NULL)
			{ // 当たり判定の種類 (範囲外なら0)
				(void)sscanf(GetValue(pLine), "%d", &nCollisionType);
				if (nCollisionType < 0 || nCollisionType >= COLLISIONTYPE_MAX) nCollisionType = 0;
			}

			if (strstr(pLine, READ_ENDMODELSET) != NULL)
			{ // モデル情報の終了 → 設置
				if (nIndexModel < g_nModelNum)
				{ // インデックス範囲内
					D3DXVECTOR3 pos = INTToFloat(nPos);								// 位置
					D3DXVECTOR3 rot = DegreeToRadian(INTToFloat(nDegree));			// 角度(ラジアン)

					switch (nTypeModel)
					{
					case PLAYERTYPE:		// プレイヤー
						SetPlayer(pos, rot, NULL);
						break;

					case OBJECTTYPE:		// オブジェクト
						SetObject(pos, rot, nIndexModel, (COLLISIONTYPE)nCollisionType);
						break;
					}
				}
				else
				{ // インデックス範囲外
					GenerateMessageBox(MB_ICONERROR, "Error (2)", "%d番目のモデルのインデックス設定に失敗しました。", nCntModel);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- XMASTREESET ～ END_XMASTREESET の読み込み及びクリスマスツリーの設置 ---
//==================================================================================
static void ReadXmasTreeSet(SCRIPT_READER *pReader, int nCntModel)
{
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// 位置(整数)
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// 角度(度数・整数)
	int nIndexModel = 0;						// 設置するモデルの番号

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_TYPE) != NULL)			(void)sscanf(GetValue(pLine), "%d", &nIndexModel);
			else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
			else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);

			if (strstr(pLine, READ_ENDXMASTREESET) != NULL)
			{ // ツリー情報の終了 → 設置
				if (nIndexModel < g_nModelNum)
				{ // インデックス範囲内
					SetXmasTree(INTToFloat(nPos), DegreeToRadian(INTToFloat(nDegree)), nIndexModel);
				}
				else
				{ // インデックス範囲外
					GenerateMessageBox(MB_ICONERROR, "Error (2)", "%d番目のモデルのインデックス設定に失敗しました。", nCntModel);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- FIELDSET ～ END_FIELDSET の読み込み及び床の設置 ---
//==================================================================================
static void ReadFieldSet(SCRIPT_READER *pReader, int nCntField)
{
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// 位置(整数)
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// 角度(度数・整数)
	INT_VECTOR3 nSize = INT_VECTOR3_NULL;		// サイズ(整数, x:幅 y:奥行)
	INT_VECTOR2 nBlock = INT_VECTOR2_NULL;		// 分割数(整数)
	D3DXVECTOR3 move = D3DXVECTOR3_NULL;		// 移動量(テクスチャスクロール)
	int nCullType = D3DCULL_CCW;				// カリングタイプ
	int nTexType = 0;							// テクスチャの番号

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_TEXTYPE) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nTexType);
			else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
			else if (strstr(pLine, READ_MOVE) != NULL)		(void)sscanf(GetValue(pLine), "%f %f", &move.x, &move.z);
			else if (strstr(pLine, READ_SIZE) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nSize.x, &nSize.y);
			else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
			else if (strstr(pLine, READ_BLOCK) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nBlock.x, &nBlock.y);
			else if (strstr(pLine, READ_CULL) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nCullType);

			if (strstr(pLine, READ_ENDFIELDSET) != NULL)
			{ // 床情報の終了 → 設置
				if (nTexType < g_nTextureNum)
				{ // テクスチャインデックス範囲内
					D3DXVECTOR3 size = INTToFloat(nSize);		// サイズ
					D3DXVECTOR2 block = INTToFloat(nBlock);		// 分割数

					SetField(INTToFloat(nPos), move, DegreeToRadian(INTToFloat(nDegree)),
						size.x, size.y, nTexType, (int)block.x, (int)block.y, (D3DCULL)nCullType);
				}
				else
				{ // テクスチャインデックス範囲外
					GenerateMessageBox(MB_ICONERROR, "Error (4)", "%d番目の床のテクスチャ設定に失敗しました。", nCntField);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- MESHFIELDSET ～ END_MESHFIELDSET の読み込み及びメッシュフィールドの設置 ---
//==================================================================================
static void ReadMeshFieldSet(SCRIPT_READER *pReader, int nCntField)
{
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// 位置(整数)
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// 角度(度数・整数)
	INT_VECTOR3 nSize = INT_VECTOR3_NULL;		// サイズ(整数, x:幅 y:奥行)
	INT_VECTOR2 nBlock = INT_VECTOR2_NULL;		// 分割数(整数)
	int nCullType = D3DCULL_CCW;				// カリングタイプ
	int nTexType = 0;							// テクスチャの番号

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_TEXTYPE) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nTexType);
			else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
			else if (strstr(pLine, READ_SIZE) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nSize.x, &nSize.y);
			else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
			else if (strstr(pLine, READ_BLOCK) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nBlock.x, &nBlock.y);
			else if (strstr(pLine, READ_CULL) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nCullType);

			if (strstr(pLine, READ_ENDMESHFIELDSET) != NULL)
			{ // メッシュフィールド情報の終了 → 設置
				if (nTexType < g_nTextureNum)
				{ // テクスチャインデックス範囲内
					D3DXVECTOR3 size = INTToFloat(nSize);		// サイズ
					D3DXVECTOR2 block = INTToFloat(nBlock);		// 分割数

					SetMeshField(INTToFloat(nPos), DegreeToRadian(INTToFloat(nDegree)),
						size.x, size.y, (int)block.x, (int)block.y, nTexType, (D3DCULL)nCullType);
				}
				else
				{ // テクスチャインデックス範囲外
					GenerateMessageBox(MB_ICONERROR, "Error (4)", "%d番目の床のテクスチャ設定に失敗しました。", nCntField);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- WALLSET ～ END_WALLSET の読み込み及び壁の設置 ---
//==================================================================================
static void ReadWallSet(SCRIPT_READER *pReader, int nCntWall)
{
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// 位置(整数)
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// 角度(度数・整数)
	INT_VECTOR3 nSize = INT_VECTOR3_NULL;		// サイズ(整数, x:幅 y:高さ)
	INT_VECTOR2 nBlock = INT_VECTOR2_NULL;		// 分割数(整数)
	D3DXCOLOR col = D3DXCOLOR_NULL;				// 色
	int nCullType = D3DCULL_CCW;				// カリングタイプ
	int nTexType = 0;							// テクスチャの番号

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_TEXTYPE) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nTexType);
			else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
			else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
			else if (strstr(pLine, READ_COL) != NULL)		(void)sscanf(GetValue(pLine), "%f %f %f %f", &col.r, &col.g, &col.b, &col.a);
			else if (strstr(pLine, READ_BLOCK) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nBlock.x, &nBlock.y);
			else if (strstr(pLine, READ_SIZE) != NULL)		(void)sscanf(GetValue(pLine), "%d %d", &nSize.x, &nSize.y);
			else if (strstr(pLine, READ_CULL) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nCullType);

			if (strstr(pLine, READ_ENDWALLSET) != NULL)
			{ // 壁情報の終了 → 設置
				if (nTexType < g_nTextureNum)
				{ // テクスチャインデックス範囲内
					D3DXVECTOR3 size = INTToFloat(nSize);		// サイズ
					D3DXVECTOR2 block = INTToFloat(nBlock);		// 分割数

					SetWall(INTToFloat(nPos), DegreeToRadian(INTToFloat(nDegree)),
						size.x, size.y, nTexType, (int)block.x, (int)block.y, col, (D3DCULL)nCullType);
				}
				else
				{ // テクスチャインデックス範囲外
					GenerateMessageBox(MB_ICONERROR, "Error (5)", "%d番目の壁のテクスチャ設定に失敗しました。", nCntWall);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- 現在は使っていないブロック(TREESET / BULLETSET / EXPLOSIONSET)を読み飛ばす ---
// ※ 木・弾・爆発の処理は削除したが、既存のスクリプトをそのまま読めるように残している
//==================================================================================
static void SkipScriptBlock(SCRIPT_READER *pReader, const char *pEndText)
{
	while (1)
	{
		if (ReadScriptLine(pReader) && strstr(pReader->aLine, pEndText) != NULL)
		{ // ブロックの終了
			return;
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
// --- テクスチャ番号のみを持つブロック(EFFECTSET / SHADOWSET)の読み込み ---
// pEndText      : ブロックの終了文字列
// pSetFunc      : 読み込んだテクスチャ番号を渡す設定関数
// pErrorCaption : 番号が範囲外の時のエラー見出し
// pErrorText    : 番号が範囲外の時のエラー文
//==================================================================================
static void ReadTextureIndexSet(SCRIPT_READER *pReader,
	const char *pEndText,
	void (*pSetFunc)(int),
	const char *pErrorCaption,
	const char *pErrorText)
{
	int nTexType = 0;		// テクスチャの番号

	while (1)
	{
		if (ReadScriptLine(pReader))
		{ // 有効な行
			const char *pLine = pReader->aLine;		// 読み込んだ行

			if (strstr(pLine, READ_TEXTYPE) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nTexType);

			if (strstr(pLine, pEndText) != NULL)
			{ // ブロックの終了 → 設定
				if (nTexType < g_nTextureNum)
				{ // テクスチャインデックス範囲内
					pSetFunc(nTexType);
				}
				else
				{ // テクスチャインデックス範囲外
					GenerateMessageBox(MB_ICONERROR, pErrorCaption, pErrorText);
				}

				return;
			}
		}

		if (pReader->bEof) return;		// 終了位置が無いままEOF
	}
}

//==================================================================================
//
// ここから LoadMotion 用の内部処理
// 終了位置の前にEOFに達した場合は、エラー表示とファイルクローズを行い E_FAIL を返す
//
//==================================================================================

//==================================================================================
// --- モーションスクリプトの1行読み込み ---
// 戻り値 : S_OK = 読み込む行 / S_FALSE = コメント行・空行 / E_FAIL = EOF (クローズ済み)
//==================================================================================
static HRESULT ReadMotionLine(SCRIPT_READER *pReader)
{
	bool bValid = ReadScriptLine(pReader);		// 有効な行か

	if (pReader->bEof)
	{ // 終了位置の前にEOFに達した
		GenerateMessageBox(MB_ICONERROR, "Error (2)", "読み込みが異常終了しました。");
		fclose(pReader->pFile);
		return E_FAIL;
	}

	return bValid ? S_OK : S_FALSE;
}

//==================================================================================
// --- パーツのモデルパスの読み込み (未読み込みのモデルのみ新規ロード) ---
//==================================================================================
static void ReadMotionModelPath(SCRIPT_READER *pReader, MOTION_LOADER *pLoader)
{
	if (pLoader->nNumModelPath >= MAX_MOTION_MODEL)
	{ // 対応表が満杯
		return;
	}

	char aModelPath[MAX_PATH] = {};								// Xファイルパス
	int *pIdxObject = &pLoader->aIdxObject[pLoader->nNumModelPath];	// 格納先の対応表

	(void)sscanf(GetValue(pReader->aLine), "%s", aModelPath);

	/*** InitScript で読み込んだファイル名と重複していないか確認 ***/
	*pIdxObject = -1;
	for (int nCntModel = 0; nCntModel < g_nModelNum; nCntModel++)
	{
		if (strcmp(aModelPath, g_aXFileName[nCntModel]) == 0)
		{ // 重複していたら、その番号を使う
			*pIdxObject = nCntModel;
			break;
		}
	}

	if (*pIdxObject == -1)
	{ // 未読み込みのモデル → 新規で読み込み、末尾の番号を割り当てる
		(void)LoadObject(aModelPath);
		*pIdxObject = g_nModelNum;
		g_nModelNum++;
	}

	pLoader->nNumModelPath++;
}

//==================================================================================
// --- CHARACTERSET ～ END_CHARACTERSET の読み込み及びキャラクター情報の登録 ---
//==================================================================================
static HRESULT ReadCharacterSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader)
{
	PartsInfo parts = {};		// パーツ情報 (省略された項目は前のパーツの値を引き継ぐ)
	int nNumParts = 0;			// パーツ総数
	float fRadius = 0.0f;		// 当たり判定の半径
	float fHeight = 0.0f;		// 当たり判定の高さ
	float fMove = 0.0f;			// 移動量
	float fJump = 0.0f;			// 跳躍力
	int nIdxHead = 0;			// 頭パーツの番号
	int nIdxHand = 0;			// 手パーツの番号

	while (1)
	{
		HRESULT hr = ReadMotionLine(pReader);		// 1行読み込み
		if (FAILED(hr)) return hr;
		if (hr == S_FALSE) continue;				// コメント行

		const char *pLine = pReader->aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDCHARACTER) != NULL)
		{ // キャラクター情報の終了 → 登録
			if (pLoader->mode == MODE_TITLE)
			{ // タイトル用のプレイヤー
				SetMotionPartsNumPlayerTitle(nNumParts);
			}
			else
			{ // ゲーム用のプレイヤー
				SetPlayerSetting(fMove, fJump);
				SetMotionPartsNum(nNumParts);
				SetCollisionSize(fRadius, fHeight);
				SetPlayerPartsIndices(nIdxHead, nIdxHand);
			}

			return S_OK;
		}
		else if (strstr(pLine, READ_PARTS) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nNumParts);
		else if (strstr(pLine, READ_RADIUS) != NULL)	(void)sscanf(GetValue(pLine), "%f", &fRadius);
		else if (strstr(pLine, READ_HEIGHT) != NULL)	(void)sscanf(GetValue(pLine), "%f", &fHeight);
		else if (strstr(pLine, READ_MOVE) != NULL)		(void)sscanf(GetValue(pLine), "%f", &fMove);
		else if (strstr(pLine, READ_JUMP) != NULL)		(void)sscanf(GetValue(pLine), "%f", &fJump);
		else if (strstr(pLine, READ_HEAD) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nIdxHead);
		else if (strstr(pLine, READ_HAND) != NULL)		(void)sscanf(GetValue(pLine), "%d", &nIdxHand);
		else if (strstr(pLine, READ_PARTSSET) != NULL)
		{ // パーツ1つ分の情報
			hr = ReadPartsSet(pReader, pLoader, &parts);
			if (FAILED(hr)) return hr;
		}
	}
}

//==================================================================================
// --- PARTSSET ～ END_PARTSSET の読み込み及びパーツ情報の登録 ---
//==================================================================================
static HRESULT ReadPartsSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader, PartsInfo *pParts)
{
	while (1)
	{
		HRESULT hr = ReadMotionLine(pReader);		// 1行読み込み
		if (FAILED(hr)) return hr;
		if (hr == S_FALSE) continue;				// コメント行

		const char *pLine = pReader->aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDPARTSSET) != NULL)
		{ // パーツ情報の終了 → 登録
			if (pLoader->mode == MODE_TITLE)
			{
				LoadPartsPlayerTitle(*pParts);
			}
			else
			{
				LoadPartsPlayer(*pParts);
			}

			return S_OK;
		}
		else if (strstr(pLine, READ_INDEX) != NULL)
		{ // モデル番号 → 対応するオブジェクト番号も取得
			(void)sscanf(GetValue(pLine), "%d", &pParts->nIdxModel);

			if (pParts->nIdxModel >= 0 && pParts->nIdxModel < MAX_MOTION_MODEL)
			{
				pParts->nIdxObject = pLoader->aIdxObject[pParts->nIdxModel];
			}
		}
		else if (strstr(pLine, READ_PARENT) != NULL)	(void)sscanf(GetValue(pLine), "%d", &pParts->nIdxModelParent);
		else if (strstr(pLine, READ_POS) != NULL)		(void)sscanf(GetValue(pLine), "%f %f %f", &pParts->pos.x, &pParts->pos.y, &pParts->pos.z);
		else if (strstr(pLine, READ_ROT) != NULL)		(void)sscanf(GetValue(pLine), "%f %f %f", &pParts->rot.x, &pParts->rot.y, &pParts->rot.z);
	}
}

//==================================================================================
// --- MOTIONSET ～ END_MOTIONSET の読み込み及びモーションの登録 ---
//==================================================================================
static HRESULT ReadMotionSet(SCRIPT_READER *pReader, MOTION_LOADER *pLoader)
{
	int nKeySet = 0;		// 読み込み中のキー情報番号

	while (1)
	{
		HRESULT hr = ReadMotionLine(pReader);		// 1行読み込み
		if (FAILED(hr)) return hr;
		if (hr == S_FALSE) continue;				// コメント行

		const char *pLine = pReader->aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDMOTIONSET) != NULL)
		{ // モーション情報の終了 → 登録
			if (pLoader->mode == MODE_TITLE)
			{
				LoadMotionPlayerTitle(&g_motionInfo, pLoader->motionType);
			}
			else
			{
				LoadMotionPlayer(&g_motionInfo, pLoader->motionType);
			}

			ZeroMemory(&g_motionInfo, sizeof(MOTION_INFO));

			/*** 次のモーションの種類へ (最後の種類で止める) ***/
			if (pLoader->motionType < MOTIONTYPE_MAX - 1)
			{
				pLoader->motionType = (MOTIONTYPE)(pLoader->motionType + 1);
			}

			return S_OK;
		}
		else if (strstr(pLine, READ_LOOP) != NULL)
		{ // ループするか (1以上 => ループ)
			int nLoop = 0;		// ループ指定(整数)
			(void)sscanf(GetValue(pLine), "%d", &nLoop);
			g_motionInfo.bLoop = (nLoop > 0);
		}
		else if (strstr(pLine, READ_NUMKEY) != NULL)
		{ // キー情報の数
			(void)sscanf(GetValue(pLine), "%d", &g_motionInfo.nNumKey);
		}
		else if (strcmp(pLine, READ_KEYSET) == 0)
		{ // キー情報1つ分
			hr = ReadKeySet(pReader, nKeySet);
			if (FAILED(hr)) return hr;

			if (nKeySet < MAX_KEY_INFO - 1) nKeySet++;
		}
	}
}

//==================================================================================
// --- KEYSET ～ END_KEYSET の読み込み ---
//==================================================================================
static HRESULT ReadKeySet(SCRIPT_READER *pReader, int nKeySet)
{
	int nKey = 0;		// 読み込み中のキー番号(パーツ番号)

	while (1)
	{
		HRESULT hr = ReadMotionLine(pReader);		// 1行読み込み
		if (FAILED(hr)) return hr;
		if (hr == S_FALSE) continue;				// コメント行

		const char *pLine = pReader->aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDKEYSET) != NULL)
		{ // キー情報の終了
			return S_OK;
		}
		else if (strstr(pLine, READ_FRAME) != NULL)
		{ // 再生フレーム数
			(void)sscanf(GetValue(pLine), "%d", &g_motionInfo.aKeyInfo[nKeySet].nFrame);
		}
		else if (strcmp(pLine, READ_KEY) == 0)
		{ // パーツ1つ分のキー
			hr = ReadKey(pReader, nKeySet, nKey);
			if (FAILED(hr)) return hr;

			if (nKey < MAX_KEY - 1) nKey++;
		}
	}
}

//==================================================================================
// --- KEY ～ END_KEY の読み込み ---
//==================================================================================
static HRESULT ReadKey(SCRIPT_READER *pReader, int nKeySet, int nKey)
{
	KEY *pKey = &g_motionInfo.aKeyInfo[nKeySet].aKey[nKey];		// 格納先のキー

	while (1)
	{
		HRESULT hr = ReadMotionLine(pReader);		// 1行読み込み
		if (FAILED(hr)) return hr;
		if (hr == S_FALSE) continue;				// コメント行

		const char *pLine = pReader->aLine;			// 読み込んだ行

		if (strstr(pLine, READ_ENDKEY) != NULL)
		{ // キーの終了
			return S_OK;
		}
		else if (strstr(pLine, READ_POS) != NULL)	(void)sscanf(GetValue(pLine), "%f %f %f", &pKey->fPosX, &pKey->fPosY, &pKey->fPosZ);
		else if (strstr(pLine, READ_ROT) != NULL)	(void)sscanf(GetValue(pLine), "%f %f %f", &pKey->fRotX, &pKey->fRotY, &pKey->fRotZ);
	}
}

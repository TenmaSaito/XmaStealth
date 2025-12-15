//================================================================================================================
//
// DirectXのXファイル読み込み処理 [loadxfile.cpp]
// Author : TENMA
//
//================================================================================================================
//******************************************************************************************************
//*** インクルードファイル ***
//******************************************************************************************************
#include "loadxfile.h"
#include "camera.h"
#include "enemy.h"
#include "player.h"
#include "object.h"
#include "wall.h"
#include "field.h"
#include "tree.h"
#include "explosion.h"
#include "effect.h"
#include "bullet.h"
#include "shadow.h"
#include "mesh.h"
#include "XmasTree.h"
#include "ornament.h"

//******************************************************************************************************
//*** マクロ定義 ***
//******************************************************************************************************
#define MAX_MODEL				(MAX_PLAYER + MAX_ENEMY + MAX_OBJECT)		// 読み込めるモデル最大数
#define MAX_TEXTURE				(64)				// 読み込めるテクスチャの最大数
#define READ_TEXTURENUM			"NUM_TEXTURE"		// 読み込むテクスチャ数の前文字列
#define READ_TEXTUREPATH		"TEXTURE_FILENAME"	// テクスチャを読み込む印
#define READ_MODELNUM			"NUM_MODEL"			// 読み込むモデル数の前文字列
#define READ_MODELPATH			"MODEL_FILENAME"	// ファイル名を読み込む印
#define READ_CAMERASET			"CAMERASET"			// カメラ情報の開始位置
#define READ_ENDCAMERASET		"END_CAMERASET"		// カメラ情報の終了位置
#define READ_MODELSET			"MODELSET"			// モデル情報の開始位置
#define READ_ENDMODELSET		"END_MODELSET"		// モデル情報の終了位置
#define READ_ORNAMENTSET		"ORNAMENTSET"		// ORNAMENT情報の開始位置
#define READ_ENDORNAMENTSET		"END_ORNAMENTSET"	// ORNAMENT情報の終了位置
#define READ_ORNAMENT			"ORNAMENT"			// ORNAMENTの開始位置
#define READ_ENDORNAMENT		"END_ORNAMENT"		// ORNAMENTの終了位置
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
#define READ_POSV				"POSV"				// 視点位置
#define READ_POSR				"POSR"				// 注視点位置
#define READ_ZLENGTH			"ZLEN"				// 注視点距離
#define READ_VANGLE				"VANGLE"			// 視野角
#define READ_ZN					"ZN"				// 描画開始距離
#define READ_ZF					"ZF"				// 描画限界距離
#define READ_TYPE				"TYPE"				// 種類
#define READ_TEXTYPE			"TEXTYPE"			// テクスチャの種類
#define READ_INDEX				"INDEX"				// インデックス
#define READ_POS				"POS"				// 位置
#define READ_ROT				"ROT"				// 角度
#define READ_COL				"COL"				// 色
#define READ_MOVE				"MOVE"				// 移動量
#define READ_SPD				"SPD"				// 速度
#define READ_ZFUNC				"ZFUNC"				// Zテストの有無
#define READ_COLLISION			"COLLISION"			// 当たり判定の有無
#define READ_CULL				"CULL"				// カリングタイプ
#define READ_ROTSPD				"ROTSPEED"			// 回転速度
#define READ_BLOCK				"BLOCK"				// 分割数
#define READ_SIZE				"SIZE"				// サイズ
#define READ_STAGERECT			"STAGERECT"			// 移動可能範囲
#define READ_NUMORNAMENT		"NUM_ORNAMENT"		// 設定するオーナメントの数
#define READ_WEIGHT				"WEIGHT"			// 重量
#define READ_TEXTURECOUNT		"TEXANIMCNT"		// テクスチャアニメーション数
#define READ_CODETEXT			"CODENAME"			// 判別用特殊文字列(デバッグ用)
#define PLAYERTYPE				(-1)				// プレイヤーとなるXファイル
#define ENEMYTYPE				(0)					// エネミーとなるXファイル
#define OBJECTTYPE				(1)					// オブジェクトとなるXファイル
#define READ_MOTION				"MOTION_FILENAME"	// キャラクターのモーションファイル名

#define READ_CHARACTER			"CHARACTERSET"		// 階層構造のキャラクター情報
#define READ_ENDCHARACTER		"END_CHARACTERSET"	// キャラクター情報設定の解除
#define READ_PARTS				"NUM_PARTS"			// パーツ数
#define READ_RADIUS				"RADIUS"			// 球の当たり判定の半径
#define READ_HEIGHT				"HEIGHT"			// 上下の当たり判定の大きさ
#define READ_JUMP				"JUMP"				// 跳躍力
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
#define	READ_NUMKEY				"NUM_KEY"			// キー情報の数
#define READ_FRAME				"FRAME"				// フレーム数

//****************************************************************************************************************
//*** プロトタイプ宣言 ***
//****************************************************************************************************************
bool DeleteComments(char *aStr);

//****************************************************************************************************************
//*** グローバル変数 ***
//****************************************************************************************************************
LPDIRECT3DTEXTURE9 g_apTextureModel[MAX_TEXTURE] = {};		// 読み込んだテクスチャ
char g_aXFileName[MAX_MODEL][MAX_PATH];				// Xファイルのパス
char g_aTextureFileName[MAX_TEXTURE][MAX_PATH];		// テクスチャパス
int g_nTextureNum = 0;				// 読み込むテクスチャ数
int g_nModelNum = 0;				// 読み込んだモデルの数
MOTION_INFO g_mInfo = {};			// モーション情報

//================================================================================================================
// --- スクリプトの読み込み ---
//================================================================================================================
HRESULT InitScript(_In_ const char* pScriptFileName)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	HWND hWnd = GetHandleWindow();
	if (hWnd == NULL)
	{ // ハンドル有無確認
		return E_FAIL;
	}

	if (pScriptFileName == NULL)
	{
		return E_FAIL;
	}

	/** スクリプト読み込み関連 **/
	FILE *pFile = NULL;				// ファイルポインタ
	HRESULT hr = E_FAIL;			// ファイルの読み込み確認処理
	char aStr[MAX_PATH] = {};		// Xファイルのファイル名
	char aTexturePath[MAX_PATH] = {};		// テクスチャパス
	char *PosTrash;					// ゴミ捨て場(コメント消去用変数)
	char *pStart;					// 値の開始位置
	const char *pNull = "\0";		// 何もなし
	char aErrorText[MAX_PATH] = {};	// エラー文用

	/** 設置前パス読み込み関連 **/
	int nCntTexture = 0;			// 読み込んだテクスチャ数
	int nCntModel = 0;				// 現在のモデル数

	/** 設置共通関連 **/
	INT_VECTOR3 nPos = INT_VECTOR3_NULL;		// float変換前の位置
	D3DXVECTOR3 pos = D3DXVECTOR3_NULL;			// Xファイルの位置
	INT_VECTOR3 nDegree = INT_VECTOR3_NULL;		// float変換前の角度
	D3DXVECTOR3 degree = D3DXVECTOR3_NULL;		// 変換前の角度
	D3DXVECTOR3 rot = D3DXVECTOR3_NULL;			// Xファイルの変換後角度
	int nTexType = 0;							// テクスチャの種類

	/** カメラ関連 **/
	INT_VECTOR3 nPosV = INT_VECTOR3_NULL;		// 変換前カメラ位置
	D3DXVECTOR3 posV = D3DXVECTOR3_NULL;		// カメラ位置
	INT_VECTOR3 nPosR = INT_VECTOR3_NULL;		// 変換前注視点位置
	D3DXVECTOR3 posR = D3DXVECTOR3_NULL;		// 注視点位置
	int nZlen = 0;			// 変換前注視点距離
	float fZlen = 0.0f;		// 注視点距離
	int nAngle = 0;			// 変換前視野角
	float vAngle = 0.0f;	// 視野角
	int nZn = 0;			// 変換前描画開始距離
	float zn = 0.0f;		// 描画開始距離
	int nZf = 0;			// 変換前描画終了距離
	float zf = 0.0f;		// 描画終了距離
	float fRotSpd = 0.0f;	// 回転速度

	/** モデル設置関連 **/
	RECT stageRect = RECT_NULL;			// ステージの移動可能範囲
	char aCodeName[MAX_PATH] = {};		// コードネーム
	int nModel = 0;					// 設置したモデル数
	int nIndexModel = 0;			// 読み込んだモデルのインデックス
	int nTypeModel = 1;				// 読み込んだモデルのタイプ
	float fSpd = 0.0f;				// 速度
	int nUseCollision;				// 当たり判定の有無
	bool bUseCollision;				// 当たり判定の有無

	/** 壁、床設置関連 **/
	int nField = 0;							// 床の数
	int nWall = 0;							// 壁の数
	INT_VECTOR3 nSize = INT_VECTOR3_NULL;	// 変換前サイズ
	D3DXVECTOR3 size = D3DXVECTOR3_NULL;	// 大きさ
	INT_VECTOR2 nBlock = INT_VECTOR2_NULL;	// 変換前分割数
	D3DXVECTOR2 block = D3DXVECTOR2_NULL;	// 分割数
	D3DXVECTOR3 move = D3DXVECTOR3_NULL;	// 移動量
	//INT_VECTOR3 nCol = D3DCOLOR_NULL;			// 変換前色
	D3DXCOLOR col = D3DXCOLOR_NULL;			// 色
	D3DCULL cullType = D3DCULL_CCW;			// カリングタイプ

	/** 木(ビルボード)設置関連 **/
	int nTree = 0;				// 木の数
	int nTexAnimCnt = 0;		// テクスチャアニメーション数
	int nZFunc = 0;				// Zテストを使うか(1以上 => TRUE)
	bool bZFunc = false;		// Zテストを使うか

	/*** 読み込んだモデル数のリセット ***/
	g_nModelNum = 0;

	/*** ファイルオープン ***/
	pFile = fopen(pScriptFileName, "r");
	if (pFile == NULL)
	{ // ファイルオープン失敗時
		GenerateMessageBox(MB_ICONERROR, "Error (0)", "Scriptファイルの読み込みに失敗しました。");
		return E_FAIL;
	}

	/*** SCRIPT確認 ***/
	while (1)
	{
		memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
		(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
		/*** コメント消去処理 ***/
		if (strncmp(aStr, "#", 1) != 0 && strncmp(aStr, "\n", 1) != 0)
		{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
			PosTrash = strstr(aStr, "\n");					// 改行がないか確認
			if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば消去

			PosTrash = strstr(aStr, "#");					// 列の途中に#がないか確認
			if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば、それ以降の文字列を消去

			PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
			while (PosTrash != NULL)
			{
				if (PosTrash != NULL)
				{
					if (PosTrash[0] == aStr[0])
					{
						strcpy(aStr, &aStr[1]);
						PosTrash = &PosTrash[1];				// あれば消去
					}
					else
					{
						strncpy(PosTrash, "", sizeof(char));	// あれば消去
					}
				}

				PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
			}

			if (strcmp(aStr, "SCRIPT") == NULL)
			{ // SCRIPT発見時読み込み開始
				break;
			}
			else if (feof(pFile) != NULL)
			{ // 発見できなかった場合読み込み終了 E_FAIL
				GenerateMessageBox(MB_ICONERROR, "Error (1)", "Scriptの読み込み開始位置が指定されていません。");
				return E_FAIL;
			}
		}
	}

	/*** 読み込みループ処理 ***/
	while (1)
	{
		memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
		(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

		if (feof(pFile) != 0) break;						// もし取得後EOFの場合、読み込み終了
		/*** コメント消去処理 ***/
		if (strncmp(aStr, "#", 1) != 0 && strncmp(aStr, "\n", 1) != 0)
		{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
			PosTrash = strstr(aStr, "\n");					// 改行がないか確認
			if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば消去

			PosTrash = strstr(aStr, "#");					// 列の途中に#がないか確認
			if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば、それ以降の文字列を消去

			PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
			while (PosTrash != NULL)
			{
				if (PosTrash != NULL)
				{
					if (PosTrash[0] == aStr[0])
					{
						strcpy(aStr, &aStr[1]);
						PosTrash = &PosTrash[1];				// あれば消去
					}
					else
					{
						strncpy(PosTrash, "", sizeof(char));	// あれば消去
					}
				}

				PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
			}

			/*** END_SCRIPTか確認 ***/
			if (strstr(aStr, READ_TEXTURENUM) != NULL)
			{ // 読み込むテクスチャ数を取得
				pStart = strchr(aStr, '=');

				/** テクスチャ数を取得 **/
				(void)sscanf(pStart + 1, "%d", &g_nTextureNum);
				if (g_nTextureNum > MAX_TEXTURE)
				{ // 読み込むテクスチャ数が上限を超えていた場合、上限以下に制限
					g_nTextureNum = MAX_TEXTURE;
				}
			}
			else if (strstr(aStr, READ_TEXTUREPATH) != NULL && nCntTexture < g_nTextureNum)
			{ // テクスチャパスの読み取り及びテクスチャロード (読み込む数分のみ)
				memset(g_aTextureFileName[nCntTexture], NULL, sizeof(g_aTextureFileName[nCntTexture]));		// 文字列を初期化

				pStart = strchr(aStr, '=');

				/** テクスチャパスの読み取り **/
				(void)sscanf(pStart + 1, "%s", &g_aTextureFileName[nCntTexture]);

				CheckPath(&g_aTextureFileName[nCntTexture][0]);

				/*** テクスチャの読み込み ***/
				hr = D3DXCreateTextureFromFile(pDevice,
					g_aTextureFileName[nCntTexture],
					&g_apTextureModel[nCntTexture]);

				if (FAILED(hr))
				{ // テクスチャの読み込み失敗時
					GenerateMessageBox(MB_ICONERROR, 
						"Error (1)", 
						"%d番目のテクスチャの読み込みに失敗しました。",
						nCntTexture);
				}

				nCntTexture++;
			}
			else if (strstr(aStr, READ_MODELNUM) != NULL)
			{ // モデル数の読み込み
				pStart = strchr(aStr, '=');

				/** モデル数の読み込み **/
				(void)sscanf(pStart + 1, "%d", &g_nModelNum);
				if (g_nModelNum > MAX_MODEL)
				{ // モデル数が上限を超えた場合、制限
					g_nModelNum = MAX_MODEL;
				}
			}
			else if (strstr(aStr, READ_MODELPATH) != NULL && nCntModel < g_nModelNum)
			{ // モデルパスの読み込み
				memset(g_aXFileName[nCntModel], NULL, sizeof(g_aXFileName[nCntModel]));		// 文字列を初期化
				pStart = strchr(aStr, '=');

				/** モデルパスの読み込み **/
				(void)sscanf(pStart + 1, "%s", &g_aXFileName[nCntModel]);
				hr = LoadObject(g_aXFileName[nCntModel]);

				if (FAILED(hr))
				{ // モデルの読み込みに失敗
					GenerateMessageBox(MB_ICONERROR,
						"Error (2)",
						"%d番目のモデルの読み込みに失敗しました。",
						nCntModel);
				}

				nCntModel++;
			}
			else if (strcmp(aStr, READ_CAMERASET) == NULL)
			{ // カメラの設置
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_POSV) != NULL)
						{ // カメラの視点位置
							pStart = strchr(aStr, '=');

							/** カメラの視点位置の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPosV.x, &nPosV.y, &nPosV.z);
						}
						else if (strstr(aStr, READ_POSR) != NULL)
						{ // カメラの注視点の位置
							pStart = strchr(aStr, '=');

							/** カメラの注視点の位置の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPosR.x, &nPosR.y, &nPosR.z);
						}
						else if (strstr(aStr, READ_ZLENGTH) != NULL)
						{ // カメラの注視点との距離
							pStart = strchr(aStr, '=');

							/** カメラの注視点との距離の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nZlen);
						}
						else if (strstr(aStr, READ_VANGLE) != NULL)
						{ // 視野角
							pStart = strchr(aStr, '=');

							/** 視野角の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nAngle);
						}
						else if (strstr(aStr, READ_ZN) != NULL)
						{ // 描画開始距離
							pStart = strchr(aStr, '=');

							/** 描画開始距離の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nZn);
						}
						else if (strstr(aStr, READ_ZF) != NULL)
						{ // 描画終了距離
							pStart = strchr(aStr, '=');

							/** 描画終了距離の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nZf);
						}
						else if (strstr(aStr, READ_SPD) != NULL)
						{ // 移動速度
							pStart = strchr(aStr, '=');

							/** 移動速度の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fSpd);
						}
						else if (strstr(aStr, READ_ROTSPD) != NULL)
						{ // 回転速度
							pStart = strchr(aStr, '=');

							/** 回転速度の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fRotSpd);
						}

						/*** モデル情報の終了 ***/
						if (strstr(aStr, READ_ENDCAMERASET) != NULL)
						{ // カメラの設置
							posV = INTToFloat(nPosV);	// 視点位置をfloatに変換
							posR = INTToFloat(nPosR);	// 注視点位置をfloatに変換
							
							fZlen = (float)nZlen;		// 注視点距離をfloatに変換
							vAngle = (float)nAngle;		// 視野角をfloatに変換
							zn = (float)nZn;			// 描画開始距離をfloatに変換
							zf = (float)nZf;			// 描画終了距離をfloatに変換

							/** カメラの設置 **/
							SetCamera(posV, posR, vAngle, zn, zf, fZlen, fSpd, fRotSpd);

							/*** 変数の初期化 ***/
							posV = D3DXVECTOR3_NULL;
							nPosV = INT_VECTOR3_NULL;
							posR = D3DXVECTOR3_NULL;
							nPosR = INT_VECTOR3_NULL;
							vAngle = 0.0f;
							zn = 0.0f;
							zf = 0.0f;
							fZlen = 0.0f;
							fSpd = 0.0f;
							fRotSpd = 0.0f;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_ORNAMENTSET) == NULL)
			{
				int nNumOrnament = -1;
				int nCntOrnament = 0;
				
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{
						if (strstr(aStr, READ_NUMORNAMENT) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 設定数の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nNumOrnament);
						}
						else if (strcmp(aStr, READ_ORNAMENT) == NULL && nCntOrnament < nNumOrnament)
						{
							int nType = -1;
							float fRadius = 0.0f;
							float fWeight = 0.0f;

							while (1)
							{
								memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
								(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
								/*** コメント消去処理 ***/
								if (DeleteComments(&aStr[0]))
								{
									if (strstr(aStr, READ_TYPE) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** モデル番号の読み込み **/
										(void)sscanf(pStart + 1, "%d", &nType);
									}
									else if (strstr(aStr, READ_RADIUS) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 当たり判定の半径の読み込み **/
										(void)sscanf(pStart + 1, "%f", &fRadius);
									}
									else if (strstr(aStr, READ_WEIGHT) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 当たり判定の半径の読み込み **/
										(void)sscanf(pStart + 1, "%f", &fWeight);
									}
									else if (strstr(aStr, READ_ENDORNAMENT) != NULL)
									{
										SettingOrnamentInfo(nType, fRadius, fWeight);
										nCntOrnament++;
										break;
									}
								}
							}
						}
						else if (strcmp(aStr, READ_ENDORNAMENTSET) == NULL)
						{
							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_MODELSET) == NULL && nModel < MAX_MODEL)
			{ // Xファイルのモデル設置
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ 
						if (strstr(aStr, READ_INDEX) != NULL)
						{ // モデル設置タイプ
							pStart = strchr(aStr, '=');
							
							/** モデル設置タイプの読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTypeModel);
						}
						else if (strstr(aStr, READ_TYPE) != NULL)
						{ // 設置モデルの種類
							pStart = strchr(aStr, '=');

							/** 設置モデルの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nIndexModel);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // モデルの位置
							pStart = strchr(aStr, '=');

							/** モデルの位置の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_ROT) != NULL)
						{ // モデルの角度
							pStart = strchr(aStr, '=');

							/** モデルの角度の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
						}
						else if (strstr(aStr, READ_SPD) != NULL)
						{ // モデルの移動速度 (キャラクターのみ)
							pStart = strchr(aStr, '=');

							/** モデルの移動速度の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fSpd);
						}
						else if (strstr(aStr, READ_STAGERECT) != NULL)
						{ // モデルのステージ移動可能範囲
							pStart = strchr(aStr, '=');

							/** モデルのステージ移動可能範囲の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d %d", &stageRect.left, &stageRect.top, &stageRect.right, &stageRect.bottom);
						}
						else if (strstr(aStr, READ_CODETEXT) != NULL)
						{ // モデルのステージ移動可能範囲
							pStart = strchr(aStr, '=');

							/** モデルのステージ移動可能範囲の読み込み **/
							(void)sscanf(pStart + 1, "%s", &aCodeName[0]);
						}
						else if (strstr(aStr, READ_COLLISION) != NULL)
						{ // カメラの注視点の位置
							pStart = strchr(aStr, '=');

							/** カメラの注視点の位置の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nUseCollision);

							nUseCollision = 1;

							if (nUseCollision < 0 || nUseCollision >= COLLISIONTYPE_MAX)
							{
								nUseCollision = 0;
							}
						}

						/*** モデル情報の終了 ***/
						if (strstr(aStr, READ_ENDMODELSET) != NULL)
						{ // モデルの読み込みの終了
							if (nIndexModel < g_nModelNum)
							{ // インデックス範囲内
								pos = INTToFloat(nPos);			// モデルの位置をfloatに変換
								
								degree = INTToFloat(nDegree);	// モデルの角度をfloatに変換
								rot = DegreeToRadian(degree);	// 角度をRadianに変換

								/*** モデルのタイプにより設置タイプを変更 ***/
								switch (nTypeModel)
								{
								// プレイヤーの場合
								case PLAYERTYPE:

									/** プレイヤーの設置 **/
									SetPlayer(pos, rot, NULL);

									break;

								// 敵の場合
								case ENEMYTYPE:

									/** 敵の設置 **/
									SetEnemy(pos, rot, nIndexModel, fSpd);

									break;

								// オブジェクトの場合
								case OBJECTTYPE:

									/** オブジェクトの設置 **/
									SetObject(pos, rot, nIndexModel, (COLLISIONTYPE)nUseCollision);

									break;
								}

								if (FAILED(hr))
								{ // モデルの読み込みに失敗
									GenerateMessageBox(MB_ICONERROR,
										"Error (2)",
										"%d番目のモデルの読み込みに失敗しました。",
										nModel);
								}
							}
							else
							{ // インデックス範囲外
								// インデックスの指定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (2)",
									"%d番目のモデルのインデックス設定に失敗しました。",
									nModel);
							}

							/*** 変数の初期化 ***/
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							rot = D3DXVECTOR3_NULL;
							nDegree = INT_VECTOR3_NULL;
							degree = D3DXVECTOR3_NULL;
							stageRect = RECT_NULL;
							memset(aCodeName, NULL, sizeof(char) * MAX_PATH);
							fSpd = 0.0f;
							nTypeModel = 1;
							nIndexModel = 0;
							nUseCollision = 1;
							bUseCollision = true;

							/*** 設置したモデル数を増加 ***/
							nModel++;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_XMASTREESET) == NULL && nModel < MAX_MODEL)
			{ // Xファイルのモデル設置
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{
						if (strstr(aStr, READ_TYPE) != NULL)
						{ // 設置モデルの種類
							pStart = strchr(aStr, '=');

							/** 設置モデルの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nIndexModel);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // モデルの位置
							pStart = strchr(aStr, '=');

							/** モデルの位置の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_ROT) != NULL)
						{ // モデルの角度
							pStart = strchr(aStr, '=');

							/** モデルの角度の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
						}

						/*** モデル情報の終了 ***/
						if (strstr(aStr, READ_ENDXMASTREESET) != NULL)
						{ // モデルの読み込みの終了
							if (nIndexModel < g_nModelNum)
							{ // インデックス範囲内
								pos = INTToFloat(nPos);			// モデルの位置をfloatに変換

								degree = INTToFloat(nDegree);	// モデルの角度をfloatに変換
								rot = DegreeToRadian(degree);	// 角度をRadianに変換

								/*** 設置 ***/
								SetXmasTree(pos, rot, nIndexModel);

								if (FAILED(hr))
								{ // モデルの読み込みに失敗
									GenerateMessageBox(MB_ICONERROR,
										"Error (2)",
										"%d番目のモデルの読み込みに失敗しました。",
										nModel);
								}
							}
							else
							{ // インデックス範囲外
								// インデックスの指定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (2)",
									"%d番目のモデルのインデックス設定に失敗しました。",
									nModel);
							}

							/*** 変数の初期化 ***/
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							rot = D3DXVECTOR3_NULL;
							nDegree = INT_VECTOR3_NULL;
							degree = D3DXVECTOR3_NULL;
							stageRect = RECT_NULL;
							memset(aCodeName, NULL, sizeof(char) * MAX_PATH);
							fSpd = 0.0f;
							nTypeModel = 1;
							nIndexModel = 0;
							nUseCollision = 1;
							bUseCollision = true;

							/*** 設置したモデル数を増加 ***/
							nModel++;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_FIELDSET) == NULL)
			{ // 床情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // 床の移動量
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_MOVE) != NULL)
						{ // 床の移動量
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%f %f", &move.x, &move.z);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 床のサイズ
							pStart = strchr(aStr, '=');

							/** 床のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_ROT) != NULL)
						{ // 床の角度
							pStart = strchr(aStr, '=');

							/** 床の角度の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
						}
						else if (strstr(aStr, READ_BLOCK) != NULL)
						{ // 床の拡大倍率
							pStart = strchr(aStr, '=');

							/** 床の拡大倍率の読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nBlock.x, &nBlock.y);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 床のサイズ
							pStart = strchr(aStr, '=');

							/** 床のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_CULL) != NULL)
						{ // カリングタイプ設定
							pStart = strchr(aStr, '=');

							/** カリングタイプの読み込み **/
							(void)sscanf(pStart + 1, "%d", &cullType);
						}

						/*** 床情報の終了 ***/
						if (strstr(aStr, READ_ENDFIELDSET) != NULL)
						{ // 床情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								pos = INTToFloat(nPos);			// 床の位置をfloatに変換

								degree = INTToFloat(nDegree);	// 床の角度をfloatに変換
								rot = DegreeToRadian(degree);	// 角度をRadianに変換

								block = INTToFloat(nBlock);		// 床の拡大倍率のfloatに変換
								size = INTToFloat(nSize);		// 床のサイズをfloatに変換

								/** 床の設置 **/
								SetField(pos, move, rot, size.x, size.y, nTexType, (int)block.x, (int)block.y, cullType);
							}
							else
							{ // テクスチャインデックス範囲外
								// テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (4)",
									"%d番目の床のテクスチャ設定に失敗しました。",
									nField);
							}

							/*** 変数の初期化 ***/
							nTexType = 0;
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							rot = D3DXVECTOR3_NULL;
							nDegree = INT_VECTOR3_NULL;
							degree = D3DXVECTOR3_NULL;
							nBlock = INT_VECTOR2_NULL;
							block = D3DXVECTOR2_NULL;
							nSize = INT_VECTOR3_NULL;
							size = D3DXVECTOR3_NULL;
							move = D3DXVECTOR3_NULL;
							cullType = D3DCULL_CCW;

							nField++;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_MESHFIELDSET) == NULL)
			{ // 床情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // 床の移動量
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_MOVE) != NULL)
						{ // 床の移動量
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%f %f", &move.x, &move.z);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 床のサイズ
							pStart = strchr(aStr, '=');

							/** 床のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_ROT) != NULL)
						{ // 床の角度
							pStart = strchr(aStr, '=');

							/** 床の角度の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
						}
						else if (strstr(aStr, READ_BLOCK) != NULL)
						{ // 床の拡大倍率
							pStart = strchr(aStr, '=');

							/** 床の拡大倍率の読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nBlock.x, &nBlock.y);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 床のサイズ
							pStart = strchr(aStr, '=');

							/** 床のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_CULL) != NULL)
						{ // カリングタイプ設定
							pStart = strchr(aStr, '=');

							/** カリングタイプの読み込み **/
							(void)sscanf(pStart + 1, "%d", &cullType);
						}

						/*** 床情報の終了 ***/
						if (strstr(aStr, READ_ENDMESHFIELDSET) != NULL)
						{ // 床情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								pos = INTToFloat(nPos);			// 床の位置をfloatに変換

								degree = INTToFloat(nDegree);	// 床の角度をfloatに変換
								rot = DegreeToRadian(degree);	// 角度をRadianに変換

								block = INTToFloat(nBlock);		// 床の拡大倍率のfloatに変換
								size = INTToFloat(nSize);		// 床のサイズをfloatに変換

								/** 床の設置 **/
								SetMeshField(pos, rot, size.x, size.y, (int)block.x, (int)block.y, nTexType, cullType);
							}
							else
							{ // テクスチャインデックス範囲外
								// テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (4)",
									"%d番目の床のテクスチャ設定に失敗しました。",
									nField);
							}

							/*** 変数の初期化 ***/
							nTexType = 0;
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							rot = D3DXVECTOR3_NULL;
							nDegree = INT_VECTOR3_NULL;
							degree = D3DXVECTOR3_NULL;
							nBlock = INT_VECTOR2_NULL;
							block = D3DXVECTOR2_NULL;
							nSize = INT_VECTOR3_NULL;
							size = D3DXVECTOR3_NULL;
							move = D3DXVECTOR3_NULL;
							cullType = D3DCULL_CCW;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_WALLSET) == NULL)
			{ // 壁情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // 壁の位置
							pStart = strchr(aStr, '=');

							/** 壁の位置の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_ROT) != NULL)
						{ // 壁の角度
							pStart = strchr(aStr, '=');

							/** 壁の角度の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nDegree.x, &nDegree.y, &nDegree.z);
						}
						else if (strstr(aStr, READ_COL) != NULL)
						{ // 壁の角度
							pStart = strchr(aStr, '=');

							/** 壁の角度の読み込み **/
							(void)sscanf(pStart + 1, "%f %f %f %f", &col.r, &col.g, &col.b, &col.a);
						}
						else if (strstr(aStr, READ_BLOCK) != NULL)
						{ // 壁の拡大倍率
							pStart = strchr(aStr, '=');

							/** 壁の拡大倍率の読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nBlock.x, &nBlock.y);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 壁のサイズ
							pStart = strchr(aStr, '=');

							/** 壁のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_CULL) != NULL)
						{ // カリングタイプ設定
							pStart = strchr(aStr, '=');

							/** カリングタイプの読み込み **/
							(void)sscanf(pStart + 1, "%d", &cullType);
						}

						/*** 壁情報の終了 ***/
						if (strstr(aStr, READ_ENDWALLSET) != NULL)
						{ // 壁情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								pos = INTToFloat(nPos);			 // 壁の位置をfloatに変換

								degree = INTToFloat(nDegree);	 // 壁の角度をfloatに変換
								rot = DegreeToRadian(degree);	 // 角度をRadianに変換

								block = INTToFloat(nBlock);		 // 壁の拡大倍率のfloatに変換
								size = INTToFloat(nSize);		 // 壁のサイズをfloatに変換

								/** 壁の設置 **/
								SetWall(pos, rot, size.x, size.y, nTexType, (int)block.x, (int)block.y, col, cullType);
							}
							else
							{ // テクスチャインデックス範囲外
								// テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (5)",
									"%d番目の壁のテクスチャ設定に失敗しました。",
									nWall);
							}

							/*** 変数の初期化 ***/
							nTexType = 0;
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							rot = D3DXVECTOR3_NULL;
							nDegree = INT_VECTOR3_NULL;
							degree = D3DXVECTOR3_NULL;
							nBlock = INT_VECTOR2_NULL;
							block = D3DXVECTOR2_NULL;
							nSize = INT_VECTOR3_NULL;
							size = D3DXVECTOR3_NULL;
							col = D3DXCOLOR_NULL;
							cullType = D3DCULL_CCW;

							nWall++;

							break;
						}
					}
				}
			}
			else if (strcmp(aStr, READ_TREESET) == NULL)
			{ // 木情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}
						else if (strstr(aStr, READ_POS) != NULL)
						{ // 木の位置
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%d %d %d", &nPos.x, &nPos.y, &nPos.z);
						}
						else if (strstr(aStr, READ_SIZE) != NULL)
						{ // 木の位置
							pStart = strchr(aStr, '=');

							/** 床の移動量の読み込み **/
							(void)sscanf(pStart + 1, "%d %d", &nSize.x, &nSize.y);
						}
						else if (strstr(aStr, READ_ZFUNC) != NULL)
						{ // 木のZテストの有無
							pStart = strchr(aStr, '=');

							/** 床のサイズの読み込み **/
							(void)sscanf(pStart + 1, "%d", &nZFunc);
						}

						/*** 木情報の終了 ***/
						if (strstr(aStr, READ_ENDTREESET) != NULL)
						{ // 木情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								pos = INTToFloat(nPos);			// 木の位置をfloatに変換
								size = INTToFloat(nSize);		// 木の大きさをfloatに変換

								/*** Zテスト判定 ***/
								if (nZFunc >= 1)
								{
									bZFunc = true;
								}

								/** 木の設置 **/
								SetTree(pos, size.x, size.y, nTexType, bZFunc);
							}
							else
							{ // テクスチャインデックス範囲外
								 // テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (6)",
									"%d番目の木のテクスチャ設定に失敗しました。",
									nTree);
							}

							/*** 変数の初期化 ***/
							nTexType = 0;
							pos = D3DXVECTOR3_NULL;
							nPos = INT_VECTOR3_NULL;
							nSize = INT_VECTOR3_NULL;
							size = D3DXVECTOR3_NULL;
							bZFunc = false;

							nTree++;

							break;
						}
					}
				}
			}
			else if (strstr(aStr, READ_EFFECTSET) != NULL)
			{ // エフェクト情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}

						/*** エフェクト情報の終了 ***/
						if (strstr(aStr, READ_ENDEFFECTSET) != NULL)
						{ // エフェクト情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								/** エフェクトの設定 **/
								SetIndexTextureEffect(nTexType);
							}
							else
							{ // テクスチャインデックス範囲外
								 // テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (7)",
									"エフェクトのテクスチャ設定に失敗しました。");
							}

							/*** 変数の初期化 ***/
							nTexType = 0;

							break;
						}
					}
				}
			}
			else if (strstr(aStr, READ_SHADOWSET) != NULL)
			{ // 影情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}

						/*** 影情報の終了 ***/
						if (strstr(aStr, READ_ENDSHADOWSET) != NULL)
						{ // 影情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								/** 影の設定 **/
								SetIndexTextureShadow(nTexType);
							}
							else
							{ // テクスチャインデックス範囲外
								 // テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (8)",
									"影のテクスチャ設定に失敗しました。");
							}

							/*** 変数の初期化 ***/
							nTexType = 0;

							break;
						}
					}
				}
			}
			else if (strstr(aStr, READ_BULLETSET) != NULL)
			{ // 弾情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}

						/*** 弾情報の終了 ***/
						if (strstr(aStr, READ_ENDBULLETSET) != NULL)
						{ // 弾情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								/** 弾の設定 **/
								SetIndexTextureBullet(nTexType);
							}
							else
							{ // テクスチャインデックス範囲外
								 // テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (9)",
									"弾のテクスチャ設定に失敗しました。");
							}

							/*** 変数の初期化 ***/
							nTexType = 0;

							break;
						}
					}
				}
			}
			else if (strstr(aStr, READ_EXPLOSIONSET) != NULL)
			{ // 爆発情報
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
					/*** コメント消去処理 ***/
					if (DeleteComments(&aStr[0]))
					{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
						if (strstr(aStr, READ_TEXTYPE) != NULL)
						{ // テクスチャの種類
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexType);
						}
						else if (strstr(aStr, READ_TEXTURECOUNT) != NULL)
						{ // テクスチャアニメーション数
							pStart = strchr(aStr, '=');

							/** テクスチャの種類の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nTexAnimCnt);
						}

						/*** 爆発情報の終了 ***/
						if (strstr(aStr, READ_ENDEXPLOSIONSET) != NULL)
						{ // 爆発情報の終了
							if (nTexType < g_nTextureNum)
							{ // テクスチャインデックス範囲内
								/** 爆発情報の設定 **/
								SetIndexTextureExplosion(nTexType, nTexAnimCnt);
							}
							else
							{ // テクスチャインデックス範囲外
								 // テクスチャの設定に失敗
								GenerateMessageBox(MB_ICONERROR,
									"Error (10)",
									"爆発のテクスチャ設定に失敗しました。");
							}

							/*** 変数の初期化 ***/
							nTexType = 0;
							nTexAnimCnt = 0;

							break;
						}
					}
				}
			}
			else if (strstr(aStr, "END_SCRIPT") != NULL)
			{ // 読み込み終了の合図
				break;
			}
		}
	}

	/*** 読み込み終了 ***/
	fclose(pFile);

	/*** 読み込み成功 ***/
	return S_OK;
}

//================================================================================================================
// --- モーションスクリプトの読み込み ---
//================================================================================================================
HRESULT LoadMotion(_In_ const char* pMotionFileName)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	HWND hWnd = GetHandleWindow();
	if (hWnd == NULL)
	{ // ハンドル有無確認
		return E_FAIL;
	}

	/** スクリプト読み込み関連 **/
	FILE* pFile = NULL;				// ファイルポインタ
	HRESULT hr = E_FAIL;			// ファイルの読み込み確認処理
	char aStr[MAX_PATH] = {};		// Xファイルのファイル名
	char aModelPath[MAX_PATH] = {};	// Xファイルパス
	char* PosTrash;					// ゴミ捨て場(コメント消去用変数)
	char* pStart;					// 値の開始位置
	const char* pNull = "\0";		// 何もなし
	char aErrorText[MAX_PATH] = {};	// エラー文用

	int nModel = 0;					// 設置したモデル数
	int nLoadModelPath = 0;			// 読み込んだモデル数
	int aIdxObject[30] = {};		// 読み込んだモデルパスのオブジェクトでのインデックス
	int nIndexModel = 0;			// 読み込んだモデルのインデックス
	int nTypeModel = 1;				// 読み込んだモデルのタイプ
	int nParts = 0;					// 読み込んだパーツ情報

	/*** モーション変数 ***/
	int nNumParts = 0;						// パーツ総数
	int nLoop = 0;							// ループの有無
	int nKeySet = 0;						// 現在設定中のキー情報
	int nKey = 0;							// 現在設定中のキー
	bool bLoop = false;						// キャスト用
	float fRadius = 0.0f;					// 半径
	float fHeight = 0.0f;					// 高さ
	float fMove = 0.0f;						// 移動量
	float fJump = 0.0f;						// 跳躍力
	PartsInfo pInfo = {};					// パーツ情報
	MOTIONTYPE mType = MOTIONTYPE_NEUTRAL;	// モーションタイプ
	int nType = MOTIONTYPE_NEUTRAL;			// 現在設定中のモーションタイプ(キャスト及び判定用)

	/*** ファイルオープン ***/
	pFile = fopen(pMotionFileName, "r");
	if (pFile == NULL)
	{ // ファイルオープン失敗時
		GenerateMessageBox(MB_ICONERROR,
			"Error (0)",
			"Scriptファイルの読み込みに失敗しました。");
		return E_FAIL;
	}

	/*** SCRIPT確認 ***/
	while (1)
	{
		memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
		(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得
		/*** コメント消去処理 ***/
		if (DeleteComments(&aStr[0]))
		{
			if (strcmp(aStr, "SCRIPT") == NULL)
			{ // SCRIPT発見時読み込み開始
				break;
			}
			else if (feof(pFile) != NULL)
			{ // 発見できなかった場合読み込み終了 E_FAIL
				GenerateMessageBox(MB_ICONERROR,
					"Error (1)",
					"Scriptの読み込み開始位置が指定されていません。");

				return E_FAIL;
			}
		}
	}

	/*** 読み込みループ処理 ***/
	while (1)
	{
		memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
		(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

		if (feof(pFile) != 0)
		{
			GenerateMessageBox(MB_ICONERROR,
				"Error (2)",
				"読み込みが異常終了しました。");

			return E_FAIL;				// もし取得後EOFの場合、読み込み終了
		}

		/*** コメント消去処理 ***/
		if (DeleteComments(&aStr[0]))
		{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
			/*** END_SCRIPTか確認 ***/
			if (strstr(aStr, "END_SCRIPT") != NULL)
			{ // 読み込み終了の合図
				break;
			}
			else if (strstr(aStr, READ_MODELNUM) != NULL)
			{
				pStart = strchr(aStr, '=');

				/** パーツ数の読み込み **/
				(void)sscanf(pStart + 1, "%d", &nModel);
			}
			else if (strstr(aStr, READ_MODELPATH) != NULL)
			{
				/** モデルパス初期化 **/
				memset(&aModelPath[0], NULL, sizeof(char) * MAX_PATH);

				/** インデックスを初期化 **/
				aIdxObject[nLoadModelPath] = -1;

				pStart = strchr(aStr, '=');

				/** Xファイル名の読み込み **/
				(void)sscanf(pStart + 1, "%s", &aModelPath[0]);

				/** 過去に読み込んだファイル名と重複していないか確認 **/
				for (int nCntModelNum = 0; nCntModelNum < g_nModelNum; nCntModelNum++)
				{
					if (strcmp(&aModelPath[0], &g_aXFileName[nCntModelNum][0]) == NULL)
					{ // 重複していたら
						aIdxObject[nLoadModelPath] = nCntModelNum;		// その番号を指定
						break;
					}
				}

				if (aIdxObject[nLoadModelPath] == -1)
				{ // 過去に読み込んでいないモデルだった場合
					hr = LoadObject(&aModelPath[0]);	// 新規でそのモデルを読み込み
					aIdxObject[nLoadModelPath] = g_nModelNum;		// インデックスを指定
					g_nModelNum++;						// 総モデル数を増加
					nLoadModelPath++;
				}
			}
			else if (strstr(aStr, READ_CHARACTER) != NULL)
			{
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

					if (feof(pFile) != 0)
					{
						GenerateMessageBox(MB_ICONERROR,
							"Error (2)",
							"読み込みが異常終了しました。");
						return E_FAIL;				// もし取得後EOFの場合、読み込み終了
					}

					if (DeleteComments(&aStr[0]))
					{
						if (strstr(aStr, READ_ENDCHARACTER) != NULL)
						{
							SetPlayerSetting(fMove, fJump);
							SetMotionPartsNum(nNumParts);
							SetCollisionSize(fRadius, fHeight);

							break;
						}
						else if (strstr(aStr, READ_PARTS) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** パーツ数の読み込み **/
							(void)sscanf(pStart + 1, "%d", &nNumParts);
						}
						else if (strstr(aStr, READ_RADIUS) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 当たり判定の半径の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fRadius);
						}
						else if (strstr(aStr, READ_HEIGHT) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 当たり判定の高さの読み込み **/
							(void)sscanf(pStart + 1, "%f", &fHeight);
						}
						else if (strstr(aStr, READ_MOVE) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 移動量の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fMove);
						}
						else if (strstr(aStr, READ_JUMP) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 移動量の読み込み **/
							(void)sscanf(pStart + 1, "%f", &fJump);
						}
						else if (strstr(aStr, READ_PARTSSET) != NULL)
						{
							while (1)
							{
								memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
								(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

								if (feof(pFile) != 0)
								{
									GenerateMessageBox(MB_ICONERROR,
										"Error (2)",
										"読み込みが異常終了しました。");
									return E_FAIL;				// もし取得後EOFの場合、読み込み終了
								}

								if (DeleteComments(&aStr[0]))
								{
									if (strstr(aStr, READ_ENDPARTSSET) != NULL)
									{
										LoadPartsPlayer(pInfo);
										nParts++;
										break;
									}
									else if (strstr(aStr, READ_INDEX) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** モデルインデックスの読み込み **/
										(void)sscanf(pStart + 1, "%d",
											&pInfo.nIdxModel);

										/** モデルインデックスに対応した、オブジェクトインデックスの取得 **/
										pInfo.nIdxObject = aIdxObject[pInfo.nIdxModel];
									}
									else if (strstr(aStr, READ_PARENT) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 移動量の読み込み **/
										(void)sscanf(pStart + 1, "%d",
											&pInfo.nIdxModelParent);
									}
									else if (strstr(aStr, READ_POS) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 移動量の読み込み **/
										(void)sscanf(pStart + 1, "%f %f %f",
											&pInfo.pos.x,
											&pInfo.pos.y,
											&pInfo.pos.z);
									}
									else if (strstr(aStr, READ_ROT) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 移動量の読み込み **/
										(void)sscanf(pStart + 1, "%f %f %f",
											&pInfo.rot.x,
											&pInfo.rot.y,
											&pInfo.rot.z);
									}
								}
							}
						}

					}
				}
			}
			else if (strstr(aStr, READ_MOTIONSET) != NULL)
			{
				while (1)
				{
					memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
					(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

					if (feof(pFile) != 0)
					{
						GenerateMessageBox(MB_ICONERROR,
							"Error (2)",
							"読み込みが異常終了しました。");
						return E_FAIL;				// もし取得後EOFの場合、読み込み終了
					}

					if (DeleteComments(&aStr[0]))
					{
						if (strstr(aStr, READ_ENDMOTIONSET) != NULL)
						{
							LoadMotionPlayer(&g_mInfo, mType);
							ZeroMemory(&g_mInfo, sizeof(MOTION_INFO));
							nKeySet = 0;
							nType = mType;
							nType++;
							if (nType >= MOTIONTYPE_MAX)
							{
								nType = MOTIONTYPE_MAX - 1;
							}

							mType = (MOTIONTYPE)nType;

							break;
						}
						else if (strstr(aStr, READ_LOOP) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** ループの読み込み **/
							(void)sscanf(pStart + 1, "%d", &nLoop);

							bLoop = (nLoop > 0) ? true : false;

							g_mInfo.bLoop = bLoop;
						}
						else if (strstr(aStr, READ_NUMKEY) != NULL)
						{
							pStart = strchr(aStr, '=');

							/** 移動量の読み込み **/
							(void)sscanf(pStart + 1, "%d", &g_mInfo.nNumKey);
						}
						else if (strcmp(aStr, READ_KEYSET) == NULL)
						{
							while (1)
							{
								memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
								(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

								if (feof(pFile) != 0)
								{
									GenerateMessageBox(MB_ICONERROR,
										"Error (2)",
										"読み込みが異常終了しました。");
									return E_FAIL;				// もし取得後EOFの場合、読み込み終了
								}

								if (DeleteComments(&aStr[0]))
								{
									if (strstr(aStr, READ_ENDKEYSET) != NULL)
									{
										nKeySet++;
										nKey = 0;
										break;
									}
									else if (strstr(aStr, READ_FRAME) != NULL)
									{
										pStart = strchr(aStr, '=');

										/** 移動量の読み込み **/
										(void)sscanf(pStart + 1, "%d", &g_mInfo.aKeyInfo[nKeySet].nFrame);
									}
									else if (strcmp(aStr, READ_KEY) == NULL)
									{
										while (1)
										{
											memset(aStr, NULL, sizeof(aStr));					// 文字列を初期化
											(void)fgets(aStr, sizeof(aStr), pFile);				// メモ帳から一列取得

											if (feof(pFile) != 0)
											{
												GenerateMessageBox(MB_ICONERROR,
													"Error (2)",
													"読み込みが異常終了しました。");
												return E_FAIL;				// もし取得後EOFの場合、読み込み終了
											}

											if (DeleteComments(&aStr[0]))
											{
												if (strstr(aStr, READ_ENDKEY) != NULL)
												{
													nKey++;
													break;
												}
												else if (strstr(aStr, READ_POS) != NULL)
												{
													pStart = strchr(aStr, '=');

													/** 移動量の読み込み **/
													(void)sscanf(pStart + 1, "%f %f %f",
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fPosX,
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fPosY,
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fPosZ);
												}
												else if (strstr(aStr, READ_ROT) != NULL)
												{
													pStart = strchr(aStr, '=');

													/** 移動量の読み込み **/
													(void)sscanf(pStart + 1, "%f %f %f",
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fRotX,
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fRotY,
														&g_mInfo.aKeyInfo[nKeySet].aKey[nKey].fRotZ);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	/*** 読み込み終了 ***/
	fclose(pFile);

	/*** 読み込み成功 ***/
	return S_OK;
}

//================================================================================================================
// --- コメントアウト関連消去処理 ---
//================================================================================================================
bool DeleteComments(char* aStr)
{
	char* PosTrash;					// ゴミ捨て場(コメント消去用変数)

	/*** コメント消去処理 ***/
	if (strncmp(aStr, "#", 1) != 0 && strncmp(aStr, "\n", 1) != 0)
	{ // 取得後、最初の文字が#(コメントアウト宣言)だった場合、読み込まない
		PosTrash = strstr(aStr, "\n");					// 改行がないか確認
		if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば消去

		PosTrash = strstr(aStr, "#");					// 列の途中に#がないか確認
		if (PosTrash != NULL) strcpy(PosTrash, "");		// あれば、それ以降の文字列を消去

		PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
		while (PosTrash != NULL)
		{
			if (PosTrash != NULL)
			{
				if (PosTrash[0] == aStr[0])
				{
					strcpy(aStr, &aStr[1]);
					PosTrash = &PosTrash[1];				// あれば消去
				}
				else
				{
					strncpy(PosTrash, "", sizeof(char));	// あれば消去
				}
			}

			PosTrash = strstr(aStr, "\t");					// タブスペースがないか確認
		}

		/*** 読み込み開始 ***/
		return true;
	}
	else
	{
		/*** 読み込み終了***/
		return false;
	}
}

//================================================================================================================
// --- 読み込んだテクスチャの終了処理 ---
//================================================================================================================
void UninitScript(void)
{
	/*** テクスチャの破棄 ***/
	for (int nCntScript = 0; nCntScript < MAX_TEXTURE; nCntScript++)
	{
		if (g_apTextureModel[nCntScript] != NULL)
		{
			g_apTextureModel[nCntScript]->Release();
			g_apTextureModel[nCntScript] = NULL;
		}
	}
}

//================================================================================================================
// --- 読み込んだテクスチャの取得 ---
//================================================================================================================
LPDIRECT3DTEXTURE9 GetTexture(_In_ int nIndexTexture)
{
	if (FAILED(CheckIndex(g_nTextureNum, nIndexTexture)))
	{ // インデックス
		return NULL;
	}

	return g_apTextureModel[nIndexTexture];
}
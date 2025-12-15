//================================================================================================================
//
// DirectXのプレイヤー表示処理 [player.cpp]
// Author : TENMA
//
//================================================================================================================
//**********************************************************************************
//*** インクルードファイル ***
//**********************************************************************************
#include "enemy.h"
#include "player.h"
#include "shadow.h"
#include "object.h"

//*************************************************************************************************
//*** マクロ定義 ***
//*************************************************************************************************
#define ENEMY_SIZE_X		(10)		// プレイヤーの基本サイズ - X
#define ENEMY_SIZE_Z		(10)		// プレイヤーの基本サイズ - Y
#define ENEMY_SPD			(2.0f)		// プレイヤーの移動スピード
#define ENEMY_ROTSPD		(0.1f)		// プレイヤーの回転スピード
#define ENEMY_WDSPD			(0.1f)		// プレイヤーの拡縮スピード
#define MAX_TEXTURE			(14)		// 読み込むテクスチャの最大数

//*************************************************************************************************
//*** グローバル変数 ***
//*************************************************************************************************
Enemy g_aEnemy[MAX_ENEMY];				// モデルの情報

//================================================================================================================
// --- Xファイル表示の初期化処理 ---
//================================================================================================================
void InitEnemy(void)
{
	/*** 各変数の初期化 ***/
	for (int nCntEnemy = 0; nCntEnemy < MAX_ENEMY; nCntEnemy++)
	{
		g_aEnemy[nCntEnemy].pos = D3DXVECTOR3_NULL;
		g_aEnemy[nCntEnemy].pos.y = 15.0f;
		g_aEnemy[nCntEnemy].move = D3DXVECTOR3_NULL;
		g_aEnemy[nCntEnemy].rot = D3DXVECTOR3_NULL;
		g_aEnemy[nCntEnemy].fAngle = 0.0f;
		g_aEnemy[nCntEnemy].fWidth = 0.0f;
		g_aEnemy[nCntEnemy].fHeight = 0.0f;
		g_aEnemy[nCntEnemy].fDepth = 0.0f;
		g_aEnemy[nCntEnemy].nIdShadow = -1;
		g_aEnemy[nCntEnemy].nType = 0;
		g_aEnemy[nCntEnemy].bDisp = false;
		g_aEnemy[nCntEnemy].bUse = false;
	}
}

//================================================================================================================
// --- Xファイル表示の終了 ---
//================================================================================================================
void UninitEnemy(void)
{

}

//================================================================================================================
// --- Xファイル表示の更新 ---
//================================================================================================================
void UpdateEnemy(void)
{
	float fAngle = 0.0f;
	for (int nCntEnemy = 0; nCntEnemy < MAX_ENEMY; nCntEnemy++)
	{
		if (g_aEnemy[nCntEnemy].bUse == true)
		{
			HomingPosToPos(GetPlayer()->pos, &g_aEnemy[nCntEnemy].pos, g_aEnemy[nCntEnemy].fSpd);
			g_aEnemy[nCntEnemy].fAngle = GetPosToPos(GetPlayer()->pos, g_aEnemy[nCntEnemy].pos) + D3DX_PI;

#if 0
			/*** カメラ情報の取得 ***/
			Camera* pCamera = GetCamera();
			float fAngle = 0.0f;		// 角度修正用変数

			/*** ポリゴンの移動！ (※ 頂点バッファのロック・アンロックは必要なし！) ***/
			if (GetKeyboardPress(DIK_UP))
			{ // Wを押したとき
				/** 追加入力確認！ **/
				if (GetKeyboardPress(DIK_LEFT))
				{ // Aを押したとき
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.z += cosf(pCamera->rot.y - (D3DX_PI * 0.25f)) * ENEMY_SPD;
					g_enemy.pos.x += sinf(pCamera->rot.y - (D3DX_PI * 0.25f)) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = (pCamera->rot.y + (D3DX_PI * 0.75f));
				}
				else if (GetKeyboardPress(DIK_RIGHT))
				{ // Dを押したとき
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.z += cosf(pCamera->rot.y + (D3DX_PI * 0.25f)) * ENEMY_SPD;
					g_enemy.pos.x += sinf(pCamera->rot.y + (D3DX_PI * 0.25f)) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = (pCamera->rot.y + (D3DX_PI * 1.25f));
				}
				else
				{ // 純粋なW入力時
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.x += sinf(pCamera->rot.y) * ENEMY_SPD;
					g_enemy.pos.z += cosf(pCamera->rot.y) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = (pCamera->rot.y + D3DX_PI);
				}
			}
			else if (GetKeyboardPress(DIK_DOWN))
			{ // Sを押したとき
				/** 追加入力確認！ **/
				if (GetKeyboardPress(DIK_LEFT))
				{ // Aを押したとき
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.z += cosf(pCamera->rot.y + (D3DX_PI * 1.25f)) * ENEMY_SPD;
					g_enemy.pos.x += sinf(pCamera->rot.y + (D3DX_PI * 1.25f)) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = (pCamera->rot.y + (D3DX_PI * 0.25f));
				}
				else if (GetKeyboardPress(DIK_RIGHT))
				{ // Dを押したとき
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.z += cosf(pCamera->rot.y - (D3DX_PI * 1.25f)) * ENEMY_SPD;
					g_enemy.pos.x += sinf(pCamera->rot.y - (D3DX_PI * 1.25f)) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = (pCamera->rot.y - (D3DX_PI * 0.25f));
				}
				else
				{ // 純粋なS入力時
					/* カメラの角度に合わせて、平行移動！ */
					g_enemy.pos.x += sinf(pCamera->rot.y + D3DX_PI) * ENEMY_SPD;
					g_enemy.pos.z += cosf(pCamera->rot.y + D3DX_PI) * ENEMY_SPD;

					/* カメラの角度に合わせて、モデルの目標角度を求める！ */
					g_fAngleEnemy = pCamera->rot.y;
				}
			}
			else if (GetKeyboardPress(DIK_LEFT))
			{ // Aを押したとき
				/** カメラの角度に合わせて、平行移動！ **/
				g_enemy.pos.z += cosf(pCamera->rot.y + (D3DX_PI * 1.5f)) * ENEMY_SPD;
				g_enemy.pos.x += sinf(pCamera->rot.y + (D3DX_PI * 1.5f)) * ENEMY_SPD;

				/** カメラの角度に合わせて、モデルの目標角度を求める！ **/
				g_fAngleEnemy = (pCamera->rot.y + (D3DX_PI * 0.5f));
			}
			else if (GetKeyboardPress(DIK_RIGHT))
			{ // Dを押したとき
				/** カメラの角度に合わせて、平行移動！ **/
				g_enemy.pos.z += cosf(pCamera->rot.y + (D3DX_PI * 0.5f)) * ENEMY_SPD;
				g_enemy.pos.x += sinf(pCamera->rot.y + (D3DX_PI * 0.5f)) * ENEMY_SPD;

				/** カメラの角度に合わせて、モデルの目標角度を求める！ **/
				g_fAngleEnemy = (pCamera->rot.y + (D3DX_PI * 1.5f));
			}

			/*** モデルの高さ変更処理！ ***/
			if (GetKeyboardPress(DIK_7))
			{ // 7を押したとき
				g_enemy.pos.y += ENEMY_SPD;
			}
			else if (GetKeyboardPress(DIK_8))
			{ // 8を押したとき
				g_enemy.pos.y -= ENEMY_SPD;
			}

			/*** モデルの向きの慣性修正! ***/
			/*** 角度の修正! ***/
			if (g_fAngleEnemy < -D3DX_PI)
			{
				g_fAngleEnemy += D3DX_PI * 2.0f;
			}
			else if (g_fAngleEnemy >= D3DX_PI)
			{
				g_fAngleEnemy -= D3DX_PI * 2.0f;
			}

			/*** 角度の差を求める! ***/
			fAngle = (g_fAngleEnemy - g_enemy.rot.y);
			/** 角度の修正! **/
			if (fAngle < -D3DX_PI)
			{
				fAngle += D3DX_PI * 2.0f;
			}
			else if (fAngle >= D3DX_PI)
			{
				fAngle -= D3DX_PI * 2.0f;
			}

			/*** 角度の差分だけ角度を徐々に回転する！ ***/
			g_enemy.rot.y += fAngle * 0.05f;

			/*** 角度の修正! ***/
			if (g_enemy.rot.y < -D3DX_PI)
			{
				g_enemy.rot.y += D3DX_PI * 2.0f;
			}
			else if (g_enemy.rot.y >= D3DX_PI)
			{
				g_enemy.rot.y -= D3DX_PI * 2.0f;
			}

			/*** 角度の差分一定の範囲よりも小さくなった場合、矯正！ ***/
			if ((g_fAngleEnemy - g_enemy.rot.y) < 0.001f && (g_fAngleEnemy - g_enemy.rot.y) > -0.001f)
			{
				g_enemy.rot.y = g_fAngleEnemy;
			}

			/*** ポリゴンの回転！ ***/
			/** X球 **/
			if (GetKeyboardPress(DIK_U))
			{ // Uを押したとき
				g_enemy.rot.x += ENEMY_ROTSPD;
			}
			else if (GetKeyboardPress(DIK_J))
			{ // Jを押したとき
				g_enemy.rot.x -= ENEMY_ROTSPD;
			}
			/** Y球 **/
			if (GetKeyboardPress(DIK_I))
			{ // Iを押したとき
				g_enemy.rot.y += ENEMY_ROTSPD;
			}
			else if (GetKeyboardPress(DIK_K))
			{ // Kを押したとき
				g_enemy.rot.y -= ENEMY_ROTSPD;
			}
			/** Z球 **/
			if (GetKeyboardPress(DIK_O))
			{ // Oを押したとき
				g_enemy.rot.z += ENEMY_ROTSPD;
			}
			else if (GetKeyboardPress(DIK_L))
			{ // Lを押したとき
				g_enemy.rot.z -= ENEMY_ROTSPD;
			}

			/*** 落下防止処理 ***/
			if (g_enemy.pos.y <= 0.0f)
			{
				g_enemy.pos.y = 0.0f;
			}

			/*** リセット処理 ***/
			if (GetKeyboardTrigger(DIK_SPACE))
			{
				/*** 各変数の初期化 ***/
				g_enemy.pos = D3DXVECTOR3_NULL;
				g_enemy.move = D3DXVECTOR3_NULL;
				g_enemy.rot = D3DXVECTOR3_NULL;
				g_fAngleEnemy = 0.0f;
			}
#endif

			/*** 角度の差を求める! ***/
			fAngle = (g_aEnemy[nCntEnemy].fAngle - g_aEnemy[nCntEnemy].rot.y);
			/** 角度の修正! **/
			if (fAngle < -D3DX_PI)
			{
				fAngle += D3DX_PI * 2.0f;
			}
			else if (fAngle >= D3DX_PI)
			{
				fAngle -= D3DX_PI * 2.0f;
			}

			/*** 角度の差分だけ角度を徐々に回転する！ ***/
			g_aEnemy[nCntEnemy].rot.y += fAngle * 0.05f;

			/*** 角度の修正! ***/
			if (g_aEnemy[nCntEnemy].rot.y < -D3DX_PI)
			{
				g_aEnemy[nCntEnemy].rot.y += D3DX_PI * 2.0f;
			}
			else if (g_aEnemy[nCntEnemy].rot.y >= D3DX_PI)
			{
				g_aEnemy[nCntEnemy].rot.y -= D3DX_PI * 2.0f;
			}

			/*** 角度の差分一定の範囲よりも小さくなった場合、矯正！ ***/
			if ((g_aEnemy[nCntEnemy].fAngle - g_aEnemy[nCntEnemy].rot.y) < 0.001f && (g_aEnemy[nCntEnemy].fAngle - g_aEnemy[nCntEnemy].rot.y) > -0.001f)
			{
				g_aEnemy[nCntEnemy].rot.y = g_aEnemy[nCntEnemy].fAngle;
			}

			/*** 影の移動 ***/
			SetPositionShadow(g_aEnemy[nCntEnemy].nIdShadow, g_aEnemy[nCntEnemy].pos, g_aEnemy[nCntEnemy].rot);
		}
	}
}

//================================================================================================================
// --- Xファイル表示の描画 ---
//================================================================================================================
void DrawEnemy(void)
{
	/*** デバイスの取得 ***/
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;		// 計算用マトリックス
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	D3DXMATERIAL *pMat;					// マテリアルデータへのポインタ
	D3DMATERIAL9 mat;					// マテリアルデータのコピー
	LPOBJECTINFO pObj = NULL;			// オブジェクト情報へのポインタ

	for (int nCntEnemy = 0; nCntEnemy < MAX_ENEMY; nCntEnemy++)
	{
		if (g_aEnemy[nCntEnemy].bUse && g_aEnemy[nCntEnemy].bDisp)
		{
			/*** オブジェクト情報へのポインタ取得 ***/
			pObj = GetObjectInfo(g_aEnemy[nCntEnemy].nType);

			/*** ワールドマトリックスの初期化 ***/
			D3DXMatrixIdentity(&g_aEnemy[nCntEnemy].mtxWorld);

			/*** 向きを反映 (※ 位置を反映する前に必ず行うこと！) ***/
			D3DXMatrixRotationYawPitchRoll(&mtxRot,
				g_aEnemy[nCntEnemy].rot.y,			// Y軸回転
				g_aEnemy[nCntEnemy].rot.x,			// X軸回転
				g_aEnemy[nCntEnemy].rot.z);			// Z軸回転

			D3DXMatrixMultiply(&g_aEnemy[nCntEnemy].mtxWorld, &g_aEnemy[nCntEnemy].mtxWorld, &mtxRot);

			/*** 位置を反映 (※ 向きを反映したのちに行うこと！) ***/
			D3DXMatrixTranslation(&mtxTrans,
				g_aEnemy[nCntEnemy].pos.x,
				g_aEnemy[nCntEnemy].pos.y,
				g_aEnemy[nCntEnemy].pos.z);

			D3DXMatrixMultiply(&g_aEnemy[nCntEnemy].mtxWorld, &g_aEnemy[nCntEnemy].mtxWorld, &mtxTrans);

			/*** ワールドマトリックスの設定 ***/
			pDevice->SetTransform(D3DTS_WORLD, &g_aEnemy[nCntEnemy].mtxWorld);

			/*** 現在のマテリアルを保存 ***/
			pDevice->GetMaterial(&matDef);

			/*** マテリアルデータへのポインタを取得 ***/
			pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();

			for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
			{
				if (FALSE)
				{ // マテリアルカラー変更
					mat = pMat[nCntMat].MatD3D;

					mat.Diffuse = GetRandomColor(false);

					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&mat);
				}
				else
				{
					/*** マテリアルの設定 ***/
					pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
				}

				/*** マテリアルの設定 ***/
				pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

				/*** テクスチャの設定 ***/
				pDevice->SetTexture(0, pObj->apTexture[nCntMat]);

				/*** モデル(パーツ)の描画 ***/
				pObj->pMesh->DrawSubset(nCntMat);
			}

			/*** 保存していたマテリアルを戻す！ ***/
			pDevice->SetMaterial(&matDef);
		}
	}
}

//================================================================================================================
// --- 敵の設置 ---
//================================================================================================================
void SetEnemy(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int nType, float fSpd)
{
	/*** オブジェクト情報へのポインタ取得 ***/
	LPOBJECTINFO pObj = GetObjectInfo(nType);

	if (pObj == NULL)
	{
		return;
	}

	/*** Xファイルの読み込み ***/
	for (int nCntEnemy = 0; nCntEnemy < MAX_ENEMY; nCntEnemy++)
	{
		if (g_aEnemy[nCntEnemy].bUse != true)
		{
			/*** 影の生成 ***/
			g_aEnemy[nCntEnemy].nIdShadow = SetShadow(ENEMY_SIZE_X, ENEMY_SIZE_Z);

			/*** 敵の情報の適用 ***/
			g_aEnemy[nCntEnemy].pos = pos;
			g_aEnemy[nCntEnemy].rot = rot;
			g_aEnemy[nCntEnemy].fSpd = fSpd;
			g_aEnemy[nCntEnemy].fWidth = pObj->mtxMax.x - pObj->mtxMin.x;
			g_aEnemy[nCntEnemy].fHeight = pObj->mtxMax.y - pObj->mtxMin.y;
			g_aEnemy[nCntEnemy].fDepth = pObj->mtxMax.z - pObj->mtxMin.z;
			g_aEnemy[nCntEnemy].fAngle = 0.0f;
			g_aEnemy[nCntEnemy].bDisp = true;
			g_aEnemy[nCntEnemy].bUse = true;

			break;
		}
	}
}

//================================================================================================================
// --- モデルの取得 ---
//================================================================================================================
Enemy *GetEnemy(void)
{
	return &g_aEnemy[0];
}
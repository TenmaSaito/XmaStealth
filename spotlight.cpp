//==================================================================================
//
// スポットライト処理 [spotlight.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "spotlight.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define LIGHT_NUM			(1)			// ライトの数
#define SPOT_RANGE			(5000.0f)	// ライトの有効範囲距離
#define SPOT_FALLOFF		(1.0f)		// 内側コーンから外側コーンへの減衰
#define SPOT_ATTENUATION	(1.0f)		// 定常減衰係数
#define SPOT_THETA			(0.214f)	// 内側コーンの角度(ラジアン)
#define SPOT_PHI			(0.57f)		// 外側コーンの角度(ラジアン)

//**********************************************************************************
// *** グローバル変数 ***
//**********************************************************************************
static D3DLIGHT9 g_aSpotLight[LIGHT_NUM];		// スポットライトの情報

//==================================================================================
// --- スポットライトの初期化 ---
//==================================================================================
void InitSpotLight(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DLIGHT9 *pSpotLight = &g_aSpotLight[0];		// 設定するライト

	/*** ライトの情報を初期化 ***/
	ZeroMemory(&g_aSpotLight[0], sizeof g_aSpotLight);

	/*** ライトの種類・色を設定 ***/
	pSpotLight->Type = D3DLIGHT_SPOT;
	pSpotLight->Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pSpotLight->Specular = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	pSpotLight->Ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);

	/*** ライトの位置・方向を設定 (SetSpotLightで毎フレーム更新される) ***/
	pSpotLight->Position = VECNULL;
	pSpotLight->Direction = VEC_Z(1.0f);

	/*** 照射範囲・減衰を設定 ***/
	pSpotLight->Range = SPOT_RANGE;
	pSpotLight->Falloff = SPOT_FALLOFF;
	pSpotLight->Attenuation0 = SPOT_ATTENUATION;
	pSpotLight->Theta = SPOT_THETA;
	pSpotLight->Phi = SPOT_PHI;

	/*** ライトを設定して有効にする ***/
	pDevice->SetLight(0, pSpotLight);
	pDevice->LightEnable(0, TRUE);
}

//==================================================================================
// --- スポットライトの終了 ---
//==================================================================================
void UninitSpotLight(void)
{

}

//==================================================================================
// --- スポットライトの更新 ---
//==================================================================================
void UpdateSpotLight(void)
{

}

//==================================================================================
// --- スポットライトの位置・方向の設定 ---
//==================================================================================
void SetSpotLight(D3DXVECTOR3 pos, D3DXVECTOR3 vec)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DLIGHT9 *pSpotLight = &g_aSpotLight[0];		// 設定するライト

	pSpotLight->Position = pos;
	pSpotLight->Direction = vec;

	/*** ライトを設定する ***/
	pDevice->SetLight(0, pSpotLight);
}

//==================================================================================
//
// 階層モデル(キャラクター)のモーション・描画の共通処理 [characterMotion.cpp]
// Author : TENMA SAITO
//
// モーションはキー(各パーツの位置・向き)を順に線形補間して再生する。
// ブレンド中は「現在のモーション」と「次のモーション」をそれぞれ補間した結果を、
// ブレンドの進み具合に応じてさらに補間する。
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "characterMotion.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define MOTION_END_BLEND_FRAME	(120)		// ループしないモーションが終わった後、ニュートラルへ戻るブレンドフレーム

//**********************************************************************************
// *** プロトタイプ宣言 ***
//**********************************************************************************
static D3DXVECTOR3 GetKeyPos(const KEY *pKey, const KEY *pKeyNext, float fRate);
static D3DXVECTOR3 GetKeyRot(const KEY *pKey, const KEY *pKeyNext, float fRate);
static void GetMotionPose(const Player *pChara, MOTIONTYPE motionType, int nKey, int nCounter, int nIdxParts, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pRot);
static bool IsValidMotionType(MOTIONTYPE motionType);

//==================================================================================
// --- モーションの更新 ---
//==================================================================================
void UpdateCharacterMotion(Player *pChara)
{
	/*** 全パーツの位置・向きを更新 ***/
	for (int nCntParts = 0; nCntParts < pChara->nNumModel; nCntParts++)
	{
		PartsInfo *pParts = &pChara->aModel[nCntParts];		// 更新するパーツ
		D3DXVECTOR3 posMotion, rotMotion;					// モーションによる位置・向き

		/*** 現在のモーションの姿勢 ***/
		GetMotionPose(pChara, pChara->motionType, pChara->nKey, pChara->nCounterMotion, nCntParts, &posMotion, &rotMotion);

		if (pChara->bBlendMotion == true)
		{ // ブレンド中 : 次のモーションの姿勢との間を補間
			D3DXVECTOR3 posBlend, rotBlend;											// 次のモーションの姿勢
			float fRateBlend = (float)pChara->nCounterBlend / (float)pChara->nFrameBlend;	// ブレンドの進み具合

			GetMotionPose(pChara, pChara->motionTypeBlend, pChara->nKeyBlend, pChara->nCounterMotionBlend, nCntParts, &posBlend, &rotBlend);

			posMotion += (posBlend - posMotion) * fRateBlend;
			rotMotion += (rotBlend - rotMotion) * fRateBlend;
		}

		/*** オフセット(初期姿勢)にモーションを加える ***/
		pParts->pos = pParts->posLocal + posMotion;
		pParts->rot = pParts->rotLocal + rotMotion;
	}

	/*** 再生位置を進める ***/
	if (pChara->bBlendMotion == false)
	{ // 通常再生
		KEY_INFO *pKeyInfo = &pChara->aMotionInfo[pChara->motionType].aKeyInfo[pChara->nKey];		// 現在のキー情報

		pChara->nCounterMotion++;
		if (pChara->nCounterMotion >= pKeyInfo->nFrame)
		{ // 現在のキーの再生が終わった
			if (IsValidMotionType(pChara->motionType) == false)
			{
				return;
			}

			/*** 次のキーへ (ループしないモーションが一周したらニュートラルへ) ***/
			pChara->nKey = (pChara->nKey + 1) % pChara->aMotionInfo[pChara->motionType].nNumKey;
			if (pChara->nKey == 0 && pChara->bLoop == false)
			{
				SetCharacterMotionType(pChara, MOTIONTYPE_NEUTRAL, true, MOTION_END_BLEND_FRAME);
			}

			pChara->nCounterMotion = 0;
		}
	}
	else
	{ // ブレンド中 : 次のモーションも再生を進める
		KEY_INFO *pKeyInfoBlend = &pChara->aMotionInfo[pChara->motionTypeBlend].aKeyInfo[pChara->nKeyBlend];	// 次のモーションの現在のキー情報

		pChara->nCounterMotionBlend++;
		if (pChara->nCounterMotionBlend >= pKeyInfoBlend->nFrame)
		{ // 次のモーションのキーを進める
			pChara->nKeyBlend = (pChara->nKeyBlend + 1) % pChara->aMotionInfo[pChara->motionTypeBlend].nNumKey;
			pChara->nCounterMotionBlend = 0;
		}

		pChara->nCounterBlend++;
		if (pChara->nCounterBlend >= pChara->nFrameBlend)
		{ // ブレンド完了 → 次のモーションを現在のモーションにする
			SetCharacterMotionType(pChara, pChara->motionTypeBlend, false, 0);
		}
	}
}

//==================================================================================
// --- モーションの変更 ---
//==================================================================================
void SetCharacterMotionType(Player *pChara, MOTIONTYPE motionTypeNext, bool bBlend, int nFrameBlend)
{
	if (IsValidMotionType(motionTypeNext) == false)
	{ // モーションの種類が範囲外
		return;
	}

	pChara->bBlendMotion = bBlend;
	pChara->bFinishMotion = false;

	if (bBlend == true)
	{ // ブレンドして切り替える : 次のモーションを最初から再生し始める
		pChara->motionTypeBlend = motionTypeNext;
		pChara->nCounterBlend = 0;
		pChara->nCounterMotionBlend = 0;
		pChara->nFrameBlend = nFrameBlend;
		pChara->nNumKeyBlend = pChara->aMotionInfo[motionTypeNext].nNumKey;
		pChara->nKeyBlend = 0;
		return;
	}

	/*** すぐに切り替える : ブレンド側の再生位置を引き継ぐ ***/
	pChara->nCounterMotion = pChara->nCounterMotionBlend;
	pChara->nKey = pChara->nKeyBlend;
	pChara->motionType = motionTypeNext;
	pChara->bLoop = pChara->aMotionInfo[motionTypeNext].bLoop;
	pChara->nNumKey = pChara->aMotionInfo[motionTypeNext].nNumKey;

	/*** 切り替えた時点の姿勢を反映 ***/
	for (int nCntParts = 0; nCntParts < pChara->nNumModel; nCntParts++)
	{
		PartsInfo *pParts = &pChara->aModel[nCntParts];		// 更新するパーツ
		D3DXVECTOR3 posMotion, rotMotion;					// モーションによる位置・向き

		GetMotionPose(pChara, pChara->motionType, pChara->nKey, pChara->nCounterMotion, nCntParts, &posMotion, &rotMotion);

		pParts->pos = pParts->posLocal + posMotion;
		pParts->rot = pParts->rotLocal + rotMotion;
	}
}

//==================================================================================
// --- 全パーツの描画 ---
//==================================================================================
void DrawCharacter(Player *pChara)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();		// デバイス
	D3DXMATRIX mtxRot, mtxTrans;					// 計算用マトリックス
	D3DMATERIAL9 matDef;							// 現在のマテリアル保存用

	if (pChara->bDisp == false || pChara->bUse == false)
	{ // 非表示 or 未設置
		return;
	}

	/*** キャラクター全体のワールドマトリックス (回転 → 平行移動の順) ***/
	D3DXMatrixIdentity(&pChara->mtxWorld);

	D3DXMatrixRotationYawPitchRoll(&mtxRot, pChara->rot.y, pChara->rot.x, pChara->rot.z);
	D3DXMatrixMultiply(&pChara->mtxWorld, &pChara->mtxWorld, &mtxRot);

	D3DXMatrixTranslation(&mtxTrans, pChara->pos.x, pChara->pos.y, pChara->pos.z);
	D3DXMatrixMultiply(&pChara->mtxWorld, &pChara->mtxWorld, &mtxTrans);

	pDevice->SetTransform(D3DTS_WORLD, &pChara->mtxWorld);

	/*** 現在のマテリアルを保存 ***/
	pDevice->GetMaterial(&matDef);

	/*** 全パーツの描画 (親パーツが先に並んでいる前提) ***/
	for (int nCntParts = 0; nCntParts < pChara->nNumModel; nCntParts++)
	{
		PartsInfo *pParts = &pChara->aModel[nCntParts];			// 描画するパーツ
		LPOBJECTINFO pObj = GetObjectInfo(pParts->nIdxObject);		// パーツのモデル

		if (pObj == NULL)
		{ // モデルが無い
			continue;
		}

		/*** パーツのワールドマトリックス = パーツ自身の行列 × 親の行列 ***/
		D3DXMatrixIdentity(&pParts->mtxWorld);

		D3DXMatrixRotationYawPitchRoll(&mtxRot, pParts->rot.y, pParts->rot.x, pParts->rot.z);
		D3DXMatrixMultiply(&pParts->mtxWorld, &pParts->mtxWorld, &mtxRot);

		D3DXMatrixTranslation(&mtxTrans, pParts->pos.x, pParts->pos.y, pParts->pos.z);
		D3DXMatrixMultiply(&pParts->mtxWorld, &pParts->mtxWorld, &mtxTrans);

		D3DXMATRIX mtxParent = (pParts->nIdxModelParent != -1)
			? pChara->aModel[pParts->nIdxModelParent].mtxWorld		// 親パーツ
			: pChara->mtxWorld;										// 親が無ければキャラクター全体
		D3DXMatrixMultiply(&pParts->mtxWorld, &pParts->mtxWorld, &mtxParent);

		pDevice->SetTransform(D3DTS_WORLD, &pParts->mtxWorld);

		/*** マテリアルごとに描画 ***/
		D3DXMATERIAL *pMat = (D3DXMATERIAL*)pObj->pBuffMat->GetBufferPointer();		// マテリアルデータ

		for (int nCntMat = 0; nCntMat < (int)pObj->dwNumMat; nCntMat++)
		{
			pDevice->SetMaterial(&pMat[nCntMat].MatD3D);
			pDevice->SetTexture(0, pObj->apTexture[nCntMat]);
			pObj->pMesh->DrawSubset(nCntMat);
		}
	}

	/*** 保存していたマテリアルを戻す ***/
	pDevice->SetMaterial(&matDef);
}

//==================================================================================
// --- パーツの登録 (空いている枠に追加) ---
//==================================================================================
void LoadCharacterParts(Player *pChara, PartsInfo parts)
{
	for (int nCntParts = 0; nCntParts < MAX_MODEL_PLAYER; nCntParts++)
	{
		PartsInfo *pParts = &pChara->aModel[nCntParts];		// 登録先のパーツ

		if (pParts->bUse == true)
		{ // 使用中
			continue;
		}

		*pParts = parts;
		pParts->posLocal = parts.pos;		// 初期姿勢を保存 (モーションはこれに加算する)
		pParts->rotLocal = parts.rot;
		pParts->bUse = true;
		pParts->nIdxCollision = -1;

		return;
	}
}

//==================================================================================
// --- モーションの登録 ---
//==================================================================================
void LoadCharacterMotion(Player *pChara, MOTION_INFO *pMotionInfo, MOTIONTYPE motionType)
{
	if (pMotionInfo == NULL)
	{ // 読み込み失敗
		pChara->bUse = false;
		return;
	}

	if (motionType < 0 || motionType >= MAX_MOTION)
	{ // 登録先が範囲外
		return;
	}

	pChara->aMotionInfo[motionType] = *pMotionInfo;
}

//==================================================================================
// --- 指定したモーション・キー・経過フレームでのパーツの姿勢(モーション分のみ) ---
//==================================================================================
static void GetMotionPose(const Player *pChara, MOTIONTYPE motionType, int nKey, int nCounter, int nIdxParts, D3DXVECTOR3 *pPos, D3DXVECTOR3 *pRot)
{
	const MOTION_INFO *pMotion = &pChara->aMotionInfo[motionType];		// モーション情報
	const KEY_INFO *pKeyInfo = &pMotion->aKeyInfo[nKey];				// 現在のキー情報
	int nKeyNext = (nKey + 1) % pMotion->nNumKey;						// 次のキー
	float fRate = (float)nCounter / (float)pKeyInfo->nFrame;			// 現在のキーの進み具合

	const KEY *pKey = &pKeyInfo->aKey[nIdxParts];						// 現在のキー
	const KEY *pKeyNext = &pMotion->aKeyInfo[nKeyNext].aKey[nIdxParts];	// 次のキー

	*pPos = GetKeyPos(pKey, pKeyNext, fRate);
	*pRot = GetKeyRot(pKey, pKeyNext, fRate);
}

//==================================================================================
// --- 2つのキーの間の位置を線形補間 ---
//==================================================================================
static D3DXVECTOR3 GetKeyPos(const KEY *pKey, const KEY *pKeyNext, float fRate)
{
	D3DXVECTOR3 pos = D3DXVECTOR3(pKey->fPosX, pKey->fPosY, pKey->fPosZ);					// 現在のキーの位置
	D3DXVECTOR3 posNext = D3DXVECTOR3(pKeyNext->fPosX, pKeyNext->fPosY, pKeyNext->fPosZ);	// 次のキーの位置

	return pos + (posNext - pos) * fRate;
}

//==================================================================================
// --- 2つのキーの間の向きを線形補間 (近い方の回り方で補間し、結果も -π～π に収める) ---
//==================================================================================
static D3DXVECTOR3 GetKeyRot(const KEY *pKey, const KEY *pKeyNext, float fRate)
{
	D3DXVECTOR3 rot = D3DXVECTOR3(pKey->fRotX, pKey->fRotY, pKey->fRotZ);					// 現在のキーの向き
	D3DXVECTOR3 rotNext = D3DXVECTOR3(pKeyNext->fRotX, pKeyNext->fRotY, pKeyNext->fRotZ);	// 次のキーの向き
	D3DXVECTOR3 rotDiff = rotNext - rot;													// 向きの差分

	RepairRot(&rotDiff, &rotDiff);

	D3DXVECTOR3 rotOut = rot + rotDiff * fRate;		// 補間した向き
	RepairRot(&rotOut, &rotOut);

	return rotOut;
}

//==================================================================================
// --- モーションの種類が範囲内か ---
//==================================================================================
static bool IsValidMotionType(MOTIONTYPE motionType)
{
	return (motionType >= 0 && motionType < MOTIONTYPE_MAX);
}

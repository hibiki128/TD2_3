#include "ClearScene.h"
#include <LightGroup.h>
#include"SceneManager.h"
#include <line/DrawLine3D.h>

void ClearScene::Finalize()
{
	sceneManager_->SetFilePath(filePath_);
}

void ClearScene::Initialize()
{

	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	vp_.Initialize();
	vp_.translation_ = { 12.0f, -6.5f, -30.0f };

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	InitFilePath();
}

void ClearScene::Update()
{
#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	///
	///	各オブジェクト更新
	/// 

	// シーン切り替え
	ChangeScene();
}

void ClearScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----



	//------------------------

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	///
	///	各オブジェクト描画
	/// 

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------

	//-----------------------------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----


	//------------------------

	//-----線描画-----

	DrawLine3D::GetInstance()->Draw(vp_);

	//---------------

	/// ----------------------------------

	/// -------描画処理終了-------
}

void ClearScene::DrawForOffScreen()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----

	//------------------------

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------

	//-----------------------------


	/// ----------------------------------

	/// -------描画処理終了-------
}

void ClearScene::Debug()
{
	ImGui::Begin("ClearScene:Debug");
	debugCamera_->imgui();
	LightGroup::GetInstance()->imgui();
	ImGui::End();
}

void ClearScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void ClearScene::ChangeScene()
{
	if (input_->TriggerKey(DIK_SPACE)) {
		sceneManager_->NextSceneReservation("GAME");
	}
}

void ClearScene::InitFilePath()
{
	// 現在のファイルパスを取得
	filePath_ = sceneManager_->GetFilePath();

	// 数字部分を探してインクリメントする
	size_t stagePos = filePath_.find("stage");
	if (stagePos != std::string::npos) {
		size_t numberStart = filePath_.find_first_of("0123456789", stagePos);
		if (numberStart != std::string::npos) {
			size_t numberEnd = filePath_.find_first_not_of("0123456789", numberStart);
			std::string numberStr = filePath_.substr(numberStart, numberEnd - numberStart);
			int stageNumber = std::stoi(numberStr); // 数字部分を取得
			++stageNumber; // 数字を一つ進める

			// 新しいファイルパスを生成
			filePath_ = filePath_.substr(0, numberStart) + std::to_string(stageNumber) + filePath_.substr(numberEnd);
		}
	}
}

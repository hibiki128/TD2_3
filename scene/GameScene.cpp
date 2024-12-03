#include "GameScene.h"
#include <LightGroup.h>
#include"SceneManager.h"

void GameScene::Finalize()
{

}

void GameScene::Initialize()
{

	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);
}

void GameScene::Update() 
{
	// デバッグ
	Debug();

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();
}

void GameScene::Draw()
{
	/// -------描画処理開始-------

	/// 3Dオブジェクトの描画準備
	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------

	//-----------------------------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始----

	//------------------------



	/// ----------------------------------

	/// -------描画処理終了-------
}

void GameScene::Debug()
{
	ImGui::Begin("TitleScene:Debug");
	LightGroup::GetInstance()->imgui();
	debugCamera_->imgui();
	ImGui::End();
}

void GameScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void GameScene::ChangeScene()
{
	if (input_->TriggerKey(DIK_SPACE)) {
		sceneManager_->NextSceneReservation("TITLE");
	}
}

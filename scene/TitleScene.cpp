#include "TitleScene.h"
#include "ImGuiManager.h"
#include"SceneManager.h"
#include"SrvManager.h"

#ifdef _DEBUG
#include<imgui.h>
#endif // _DEBUG
#include <LightGroup.h>

void TitleScene::Initialize()
{
	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	vp_.Initialize();
	wt1_.Initialize();
	wt2_.Initialize();

	wt1_.translation_ = { -2.0f,0.0f,0.0f };
	wt2_.translation_ = { 2.0f,0.0f,0.0f };

	suzannu_ = std::make_unique<Object3d>();
	suzannu_->Initialize("debug/suzannu.obj");
	sphere_ = std::make_unique<Object3d>();
	sphere_->Initialize("debug/sphere.obj");

}

void TitleScene::Finalize()
{

}

void TitleScene::Update()
{
	// デバッグ
	Debug();

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();

	wt1_.UpdateMatrix();
	wt2_.UpdateMatrix();
}

void TitleScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----

	//------------------------

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----
	suzannu_->Draw(wt1_, vp_);
	sphere_->Draw(wt2_, vp_);
	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------
	
	//-----------------------------


	/// ----------------------------------

	/// -------描画処理終了-------
}


void TitleScene::Debug()
{
	ImGui::Begin("TitleScene:Debug");
	LightGroup::GetInstance()->imgui();
	debugCamera_->imgui();
	ImGui::End();
}

void TitleScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void TitleScene::ChangeScene()
{
	if (input_->TriggerKey(DIK_SPACE)) {
		sceneManager_->NextSceneReservation("GAME");
	}
}

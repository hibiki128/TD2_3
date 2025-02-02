#include "DemoScene.h"
#include "ImGuiManager.h"
#include"SceneManager.h"
#include"SrvManager.h"

#ifdef _DEBUG
#include<imgui.h>
#endif // _DEBUG
#include <LightGroup.h>
#include"line/DrawLine3D.h"

void DemoScene::Initialize()
{
	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	vp_.Initialize();
	vp_.translation_ = { 12.0f,-6.0f,-30.0f };

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	emitter_ = std::make_unique<ParticleEmitter>();
	emitter_->Initialize("clearDesition", "debug/sphere.obj");
	emitter_->SetTexture("clear/UI2_1x1.png");

	pos_ = { 0.0,0.0f };

	numbers_ = std::make_unique<Sprite>();
	numbers_->Initialize("menu/numbers.png", pos_, { 1.0f,1.0f,1.0f,1.0f }, { 0.5f,0.5f });

	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0f,0.0f };
	translation_ = { 0.0f,0.0f,0.0f };
	
}

void DemoScene::Finalize()
{

}

void DemoScene::Update()
{

#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();

	
	if (isAuto_) {
		emitter_->Update();
	}
	transform_ = MakeAffineMatrix(scale_, rotate_, translation_);
	numbers_->SetPosition(pos_);
	numbers_->SetUVTransform(transform_);
	

}

void DemoScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----
	numbers_->Draw();
	//------------------------------

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------
	emitter_->Draw(vp_);
	//-----------------------------

	//-----線描画-----
	DrawLine3D::GetInstance()->Draw(vp_);
	//---------------

	/// ----------------------------------

	/// -------描画処理終了-------
}

void DemoScene::DrawForOffScreen()
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


void DemoScene::Debug()
{
	ImGui::Begin("DemoScene:Debug");
	debugCamera_->imgui();
	LightGroup::GetInstance()->imgui();
	ImGui::End();
	emitter_->imgui();
	ImGui::Begin("パーティクル");
	if (ImGui::Button("生成")) {
		emitter_->UpdateOnce();
	}
	ImGui::Checkbox("自動生成", &isAuto_);
	ImGui::End();

	ImGui::Begin("スプライト");
	ImGui::DragFloat2("位置", &pos_.x, 0.1f);
	ImGui::DragFloat3("画像の位置", &translation_.x, 0.1f);
	ImGui::DragFloat3("画像の大きさ", &scale_.x, 0.1f);
	ImGui::DragFloat3("画像の回転", &rotate_.x, 0.1f);
	ImGui::End();

}

void DemoScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void DemoScene::ChangeScene()
{

}

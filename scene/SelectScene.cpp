#include "SelectScene.h"
#include "ImGuiManager.h"
#include"SceneManager.h"
#include"SrvManager.h"

#ifdef _DEBUG
#include<imgui.h>
#endif // _DEBUG
#include <LightGroup.h>
#include"line/DrawLine3D.h"

void SelectScene::Initialize()
{
	audio_ = Audio::GetInstance();
	objCommon_ = Object3dCommon::GetInstance();
	spCommon_ = SpriteCommon::GetInstance();
	ptCommon_ = ParticleCommon::GetInstance();
	input_ = Input::GetInstance();
	vp_.Initialize();
	vp_.translation_ = { 0.0f,0.0f,-30.0f };

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(&vp_);

	MapLoad();

	/*mapPrevs_ = std::make_unique<MapPrev>();
	mapPrevs_->Init("resources/Maps/stage1.csv");*/
}

void SelectScene::Finalize()
{

}

void SelectScene::Update()
{
#ifdef _DEBUG
	// デバッグ
	Debug();
#endif // _DEBUG

	// カメラ更新
	CameraUpdate();

	// シーン切り替え
	ChangeScene();

	///
	///	各オブジェクト更新
	/// 
	MapSelect();
	// マップチップフィールド更新
	for (auto& mapPrev : mapPrevs_) {
		mapPrev->Update();
	}
}

void SelectScene::Draw()
{
	/// -------描画処理開始-------

	/// Spriteの描画準備
	spCommon_->DrawCommonSetting();
	//-----Spriteの描画開始-----

	//------------------------------

	objCommon_->DrawCommonSetting();
	//-----3DObjectの描画開始-----

	///
	///	各オブジェクト描画
	/// 

	// マップチップフィールド描画
	for (auto& mapPrev : mapPrevs_) {
		mapPrev->Draw(vp_);
	}

	//--------------------------

	/// Particleの描画準備
	ptCommon_->DrawCommonSetting();
	//------Particleの描画開始-------

	//-----------------------------

	//-----線描画-----
	DrawLine3D::GetInstance()->Draw(vp_);
	//---------------

	/// ----------------------------------

	/// -------描画処理終了-------
}

void SelectScene::DrawForOffScreen()
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


void SelectScene::Debug()
{
	ImGui::Begin("SelectScene:Debug");
	debugCamera_->imgui();
	LightGroup::GetInstance()->imgui();
	ImGui::Text("CurrentStage %d", currentStage);
	ImGui::End();

	int index = 1;
	for (auto& mapPrev : mapPrevs_) {
		std::string name = "マッププレビュー " + std::to_string(index);
		mapPrev->Debug(name);
		++index;
	}
}

void SelectScene::CameraUpdate()
{
	if (debugCamera_->GetActive()) {
		debugCamera_->Update();
	}
	else {
		vp_.UpdateMatrix();
	}
}

void SelectScene::ChangeScene()
{
	if (input_->TriggerKey(DIK_SPACE)) {
		sceneManager_->NextSceneReservation("GAME");
	}
}

void SelectScene::MapLoad()
{
	const Vector3 Space = { 25.0f,0.0f,0.0f };
	for (int i = 0; i < stageNum; i++) {
		std::unique_ptr<MapPrev>mapPrev;
		mapPrev = std::make_unique<MapPrev>();
		mapPrev->Init("resources/Maps/stage1.csv");
		mapPrev->SetPosition(Space * i);
		mapPrevs_.push_back(std::move(mapPrev));
	}
}

void SelectScene::MapSelect()
{
	for (size_t i = 0; i < mapPrevs_.size(); ++i) {
		mapPrevs_[i]->SetIsSelect(false);
	}

	mapPrevs_[currentStage]->SetIsSelect(true);
	if (input_->TriggerKey(DIK_RIGHT)) {
		currentStage++;
	}
	if (input_->TriggerKey(DIK_LEFT)) {
		currentStage--;
	}
	if (currentStage >= stageNum) {
		currentStage = 0;
	}
	else if (currentStage < 0) {
		currentStage = stageNum - 1;
	}
}

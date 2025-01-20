#include "SelectScene.h"
#include "ImGuiManager.h"
#include"SceneManager.h"
#include"SrvManager.h"

#ifdef _DEBUG
#include<imgui.h>
#endif // _DEBUG
#include <LightGroup.h>
#include"line/DrawLine3D.h"
#include <myEngine/Frame/Frame.h>
#include"math/Easing.h"

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

	startPos = 0.0f;
	endPos = 0.0f;
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
	CameraMove();
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
	ImGui::DragFloat3("カメラ位置", &vp_.translation_.x, 0.1f);
	ImGui::DragFloat("タイマー", &cameraT_, 0.1f);
	ImGui::Checkbox("カメラ動いてるか", &isMoveCamera_);
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
	for (auto& mapPrev : mapPrevs_) {
		if (mapPrev->IsFinish()) {
			sceneManager_->NextSceneReservation("GAME");
		}
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
	// いずれかの mapPrevs_ の GetDecision() が true かを確認
	bool anyDecisionMade = false;
	for (const auto& mapPrev : mapPrevs_) {
		if (mapPrev->GetDecision()) {
			anyDecisionMade = true;
			break;
		}
	}

	// anyDecisionMade が true の場合はスペースキー処理以外をスキップ
	if (!anyDecisionMade) {
		// すべての mapPrevs_ の選択状態を false に設定
		for (size_t i = 0; i < mapPrevs_.size(); ++i) {
			mapPrevs_[i]->SetIsSelect(false);
		}

		// currentStage のみ選択状態を true に設定
		mapPrevs_[currentStage]->SetIsSelect(true);

		// キー入力に応じて currentStage を変更
		if (input_->TriggerKey(DIK_D) && !isMoveCamera_) {
			currentStage++;
		}
		if (input_->TriggerKey(DIK_A) && !isMoveCamera_) {
			currentStage--;
		}

		// currentStage が範囲外にならないように制限
		if (currentStage >= stageNum) {
			currentStage = 0; // 末尾を超えたら最初に戻る
		}
		else if (currentStage < 0) {
			currentStage = stageNum - 1; // 先頭を超えたら末尾に戻る
		}
	}

	// DIK_SPACE 入力処理（いつでも可能）
	if (input_->TriggerKey(DIK_SPACE)) {
		mapPrevs_[currentStage]->SetDecision(true);
	}
}

void SelectScene::CameraMove()
{
	const float easeTMax = 0.5f;  // イージングの最大時間（スムーズさを調整）

	// 右キーが押されたとき
	if (input_->TriggerKey(DIK_D) && !isMoveCamera_) {
		startPos = vp_.translation_.x;
		endPos = currentStage * 50.0f;
		cameraT_ = 0.0f;
		isMoveCamera_ = true;
	}
	// 左キーが押されたとき
	if (input_->TriggerKey(DIK_A) && !isMoveCamera_) {
		startPos = vp_.translation_.x;
		endPos = currentStage * 50.0f;
		cameraT_ = 0.0f;
		isMoveCamera_ = true;
	}

	vp_.translation_.x = EaseInSine<float>(startPos, endPos, cameraT_, easeTMax);

	// イージングによる補間
	if (isMoveCamera_) {
		cameraT_ += Frame::DeltaTime();
		if (cameraT_ >= easeTMax) {
			cameraT_ = easeTMax;
			isMoveCamera_ = false;
		}
	}
}

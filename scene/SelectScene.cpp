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

	BGM_ = audio_->LoadWave("select/selectBgm.wav");
	audio_->PlayWave(BGM_, 0.2f, true);
	selectSE_ = audio_->LoadWave("select/stageSelect.wav");
	desitionSE_ = audio_->LoadWave("select/stageDesition.wav");

}

void SelectScene::Finalize()
{
	sceneManager_->SetFilePath(mapPrevs_[currentStage]->GetFilePath());
	audio_->StopWave(BGM_);
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
	const Vector3 Space = { 25.0f, 0.0f, 0.0f }; // ステージ間の間隔
	for (int i = 0; i < stageNum; i++) {
		std::unique_ptr<MapPrev> mapPrev = std::make_unique<MapPrev>();

		// ステージ番号に応じたファイルパスを生成
		filePath = "resources/Maps/stage" + std::to_string(i + 1) + ".csv";

		// マップ初期化
		mapPrev->Init(filePath);

		// 配置位置を設定
		mapPrev->SetPosition(Space * i);

		// 配列に追加
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

		// キーボード入力によるステージ変更
		if (input_->PushKey(DIK_D) && !isMoveCamera_) {
			currentStage++;
			audio_->PlayWave(selectSE_, 0.2f);
		}
		if (input_->PushKey(DIK_A) && !isMoveCamera_) {
			currentStage--;
			audio_->PlayWave(selectSE_, 0.2f);
		}

		// ゲームパッドの左スティック入力によるステージ変更
		XINPUT_STATE joyState;
		if (input_->GetJoystickState(0, joyState)) {
			float stickX = joyState.Gamepad.sThumbLX;

			// 左スティックのx軸の値に基づいて currentStage を変更
			if (stickX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && !isMoveCamera_) {
				currentStage++;
				audio_->PlayWave(selectSE_, 0.2f);
			}
			else if (stickX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && !isMoveCamera_) {
				currentStage--;
				audio_->PlayWave(selectSE_, 0.2f);
			}
		}

		// currentStage が範囲外にならないように制限
		if (currentStage >= stageNum) {
			currentStage = 0; // 末尾を超えたら最初に戻る
		}
		else if (currentStage < 0) {
			currentStage = stageNum - 1; // 先頭を超えたら末尾に戻る
		}
	}

	// キーボード入力による決定処理
	if (input_->TriggerKey(DIK_SPACE) && !mapPrevs_[currentStage]->GetDecision()) {
		mapPrevs_[currentStage]->SetDecision(true);
		audio_->PlayWave(desitionSE_, 0.2f);
	}

	// ゲームパッドのボタンA入力による決定処理
	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {
		if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_A && !mapPrevs_[currentStage]->GetDecision()) {
			mapPrevs_[currentStage]->SetDecision(true);
			audio_->PlayWave(desitionSE_, 0.2f);
		}
	}
}

void SelectScene::CameraMove()
{
	const float easeTMax = 0.5f;  // イージングの最大時間（スムーズさを調整）

	// キーボードの右キーが押されたとき
	if (input_->PushKey(DIK_D) && !isMoveCamera_) {
		startPos = vp_.translation_.x;
		endPos = currentStage * 50.0f;
		cameraT_ = 0.0f;
		isMoveCamera_ = true;
	}
	// キーボードの左キーが押されたとき
	if (input_->PushKey(DIK_A) && !isMoveCamera_) {
		startPos = vp_.translation_.x;
		endPos = currentStage * 50.0f;
		cameraT_ = 0.0f;
		isMoveCamera_ = true;
	}

	// ゲームパッドの左スティック入力によるカメラ移動
	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState)) {
		float stickX = joyState.Gamepad.sThumbLX;

		// 右スティック入力 (正の値)
		if (stickX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && !isMoveCamera_) {
			startPos = vp_.translation_.x;
			endPos = currentStage * 50.0f;
			cameraT_ = 0.0f;
			isMoveCamera_ = true;
		}
		// 左スティック入力 (負の値)
		else if (stickX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && !isMoveCamera_) {
			startPos = vp_.translation_.x;
			endPos = currentStage * 50.0f;
			cameraT_ = 0.0f;
			isMoveCamera_ = true;
		}
	}

	// イージングによる補間
	vp_.translation_.x = EaseInSine<float>(startPos, endPos, cameraT_, easeTMax);

	if (isMoveCamera_) {
		cameraT_ += Frame::DeltaTime();
		if (cameraT_ >= easeTMax) {
			cameraT_ = easeTMax;
			isMoveCamera_ = false;
		}
	}
}


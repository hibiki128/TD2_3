#include "ClearUI.h"
#include"Input.h"

void ClearUI::Init()
{
	book_ = std::make_unique<BaseObject>();
	book_->Init("book");
	book_->CreateModel("game/openBook.obj");

	stage_ = std::make_unique<BaseObject>();
	stage_->Init("clear_stage");
	stage_->CreateModel("clear/stage.obj");

	nextStage_ = std::make_unique<BaseObject>();
	nextStage_->Init("clear_nextStage");
	nextStage_->CreateModel("clear/nextStage.obj");

	backSelect_ = std::make_unique<BaseObject>();
	backSelect_->Init("clear_backSelect");
	backSelect_->CreateModel("clear/backSelect.obj");

	restart_ = std::make_unique<BaseObject>();
	restart_->Init("clear_restart");
	restart_->CreateModel("clear/Retry.obj");

	InitNumbers();

	singleDigit_ = std::make_unique<BaseObject>();
	twoDigit_ = std::make_unique<BaseObject>();
	singleDigit_->Init("singleDigit");
	twoDigit_->Init("twoDigit");
	singleDigit_->CreateModel("clear/0.obj");
	twoDigit_->CreateModel("clear/0.obj");


	input_ = Input::GetInstance();
	currentItem_ = 0;
}

void ClearUI::Update()
{
	book_->Update();
	stage_->Update();
	nextStage_->Update();
	backSelect_->Update();
	restart_->Update();
	MenuOperation();
}

void ClearUI::Draw(const ViewProjection& vp)
{
	book_->Draw(vp);
	stage_->Draw(vp);
	nextStage_->Draw(vp);
	backSelect_->Draw(vp);
	restart_->Draw(vp);
}

void ClearUI::Debug()
{
	ImGui::Begin("Clear");
	ImGui::Text("選択してる項目 : %d", currentItem_);
	ImGui::End();
	book_->DebugImGui();
	stage_->DebugImGui();
	nextStage_->DebugImGui();
	backSelect_->DebugImGui();
	restart_->DebugImGui();
}

void ClearUI::MenuOperation()
{
	if (input_->PushKey(DIK_W) && coolTime_ == 0.0f)
	{
		--currentItem_;
		coolTime_ = 0.1f;
	}
	if (input_->PushKey(DIK_S) && coolTime_ == 0.0f)
	{
		++currentItem_;
		coolTime_ = 0.1f;
	}

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState))
	{
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP || joyState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			--currentItem_;
			coolTime_ = 0.1f;
		}
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN || joyState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			++currentItem_;
			coolTime_ = 0.1f;
		}
	}

	if (coolTime_ > 0.0f) {
		coolTime_ -= 1.0f / 60.0f;
	}
	if (coolTime_ < 0.0f) {
		coolTime_ = 0.0f;
	}

	if (currentItem_ < 0)
	{
		currentItem_ = 2;
	}
	if (currentItem_ > 2)
	{
		currentItem_ = 0;
	}
}

void ClearUI::InitNumbers()
{
	for (int i = 0; i < 10; i++) {
		// numbers_ の i 番目に Object3d のユニークポインタを作成
		numbers_[i] = std::make_unique<Object3d>();

		// i に応じたファイルパスを生成（"clear/0.obj" ～ "clear/9.obj"）
		std::string filePath = "clear/" + std::to_string(i) + ".obj";

		// 生成したパスを使って初期化
		numbers_[i]->Initialize(filePath);
	}
}

void ClearUI::SetNumber()
{
	stageNum_;
	singleDigit_->SetModel(numbers_[0].get());
	twoDigit_->SetModel(numbers_[0].get());
}

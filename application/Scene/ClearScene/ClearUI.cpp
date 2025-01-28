#include "ClearUI.h"
#include"Input.h"

void ClearUI::Init()
{
	book_ = std::make_unique<BaseObject>();
	book_->Init("book");
	book_->CreateModel("game/openBook.obj");
	input_ = Input::GetInstance();
	currentItem_ = 0;
}

void ClearUI::Update()
{
	book_->Update();
	MenuOperation();
}

void ClearUI::Draw(const ViewProjection& vp)
{
	book_->Draw(vp);
}

void ClearUI::Debug()
{
	ImGui::Begin("Clear");
	ImGui::Text("選択してる項目 : %d", currentItem_);
	ImGui::End();
	book_->DebugImGui();
}

void ClearUI::MenuOperation()
{
	if (input_->PushKey(DIK_W) && coolTime_ == 0.0f)
	{
		--currentItem_;
		coolTime_ = 0.5f;
	}
	if (input_->PushKey(DIK_S) && coolTime_ == 0.0f)
	{
		++currentItem_;
		coolTime_ = 0.5f;
	}

	XINPUT_STATE joyState;
	if (input_->GetJoystickState(0, joyState))
	{
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP || joyState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			--currentItem_;
			coolTime_ = 0.5f;
		}
		if ((joyState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN || joyState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && coolTime_ == 0.0f)
		{
			++currentItem_;
			coolTime_ = 0.5f;
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
#include "Pause.h"
#include"myEngine/Frame/Frame.h"
#include"SpriteCommon.h"
#include"math/Easing.h"

void Pause::Init()
{
	// インスタンス生成
	input_ = Input::GetInstance();
	backGround_ = std::make_unique<Sprite>();
	backGround_->Initialize("menu/backGround.png", spritePosition_, color_);

	// 各要素初期化
	startAlpha_ = 0.0f;
	endAlpha_ = 0.0f;
	alphaT_ = 0.0f;
	EscapeCoolTime_ = 0.0f;

	// 各フラグ初期化
	isPause_ = false;
}

void Pause::Update()
{
	OpenMenu();
	MenuOperation();
	backGround_->SetSize({ 1280.0f,720.0f });
	backGround_->SetAlpha(color_.w);
}

void Pause::Draw()
{
	SpriteCommon::GetInstance()->DrawCommonSetting();
	backGround_->Draw();
}

void Pause::OpenMenu()
{


	if (input_->TriggerKey(DIK_ESCAPE) && !isPause_ && CanEscape_) {
		startAlpha_ = 0.0f;
		endAlpha_ = 0.95f;
		alphaT_ = 0.0f;
		isPause_ = true;
		EscapeCoolTime_ = 1.0f;
	}

	if (EscapeCoolTime_ > 0.0f) {
		EscapeCoolTime_ -= Frame::DeltaTime();
		CanEscape_ = false;
	}
	else {
		CanEscape_ = true;
	}

	if (input_->TriggerKey(DIK_ESCAPE) && isPause_ && CanEscape_) {
		startAlpha_ = 0.95f;
		endAlpha_ = 0.0f;
		alphaT_ = 0.0f;
		isPause_ = false;
		EscapeCoolTime_ = 1.0f;
	}

	alphaT_ += Frame::DeltaTime();
	if (alphaT_ >= easeTMax_) {
		alphaT_ = easeTMax_;
	}

	color_.w = EaseInSine<float>(startAlpha_, endAlpha_, alphaT_, easeTMax_);

}

void Pause::MenuOperation()
{
	if (isPause_) {
		if (input_->TriggerKey(DIK_UP)) {
			currentItem_++;
		}
		if (input_->TriggerKey(DIK_DOWN)) {
			currentItem_--;
		}
	}
}

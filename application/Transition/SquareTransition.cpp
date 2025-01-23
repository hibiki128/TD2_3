#include "SquareTransition.h"

// Engine
#include "math/Easing.h"

void SquareTransition::Initialize() { 
	for (size_t i = 0; i < 4; i++) {
		sprites_[i] = std::make_unique<Sprite>();
		sprites_[i]->Initialize(
		    "white.png", 
			{0.0f, 0.0f}, // 座標
			{0.0f, 0.0f, 0.0f, 1.0}, // 色（黒くしておく）
		    {0.5f, 0.5f} // アンカーポイント
		);
	} 
	sprites_[0]->SetPosition({-360.0f, 360.0f});
	sprites_[1]->SetPosition({640.0f, -640.0f});
	sprites_[2]->SetPosition({1640.0f, 360.0f});
	sprites_[3]->SetPosition({640.0f, 1360.0f});

	currentSize_ = kStartSize;
}

void SquareTransition::Update() {
	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		break;
	case Status::SquareIn:
		counter_ += kDeltaTime;
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		currentSize_ = EaseInCubic(kStartSize, kTargetSize, counter_, duration_); // 初期サイズから目標サイズへ拡大

		break;
	case Status::SquareOut:
		counter_ += kDeltaTime;
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		currentSize_ = EaseInCubic(kTargetSize, kStartSize, counter_, duration_); // 目標サイズから初期サイズへ縮小

		break;
	}

	// 全てのスプライトのサイズを設定
	for (size_t i = 0; i < 4; i++) {
		sprites_[i]->SetSize({currentSize_, currentSize_});
	}
}

void SquareTransition::Draw() { 
	if (status_ == Status::None) {
		return;
	}

	for (size_t i = 0; i < 4; i++) {
		sprites_[i]->Draw();
	}
}

void SquareTransition::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void SquareTransition::Stop() { 
	status_ = Status::None; 
}

bool SquareTransition::IsFinished() const { 
	// フェード状態による分岐
	switch (status_) {
	case Status::SquareIn:
	case Status::SquareOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}

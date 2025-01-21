#pragma once

// C++
#include <memory>
#include <array>
#include <functional>

// Engine
#include "myEngine/2d/Sprite.h"

class SquareTransition 
{
public:
	enum class Status {
		None,
		SquareIn,
		SquareOut,
	};
public:
	void Initialize();
	void Update();
	void Draw();

	void Start(Status status, float duration);
	void Stop();

	bool IsFinished() const;
	Status GetCurrentStatus() { return status_; }

private:
	const float kDeltaTime = 1.0f / 60.0f;

	std::array<std::unique_ptr<Sprite>, 4> sprites_;

	float currentSize_ = 0.0f; // 現在サイズ
	const float kStartSize = 720.0f; // 初期サイズ
	const float kTargetSize = 2400.0f; // 目標サイズ

	Status status_ = Status::None;
	float duration_ = 0.0f;
	float counter_ = 0.0f;
};

#pragma once
#include "Audio.h"
#include"BaseScene.h"
#include "Input.h"
#include"Object3dCommon.h"
#include "SpriteCommon.h"
#include"ParticleCommon.h"
#include"DebugCamera.h"
#include"Object3d.h"
#include"WorldTransform.h"
#include"ParticleEmitter.h"
#include"Sprite.h"

class DemoScene :public BaseScene
{
public: // メンバ関数

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize()override;

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update()override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;

	/// <summary>
	/// オフスクリーン上に描画
	/// </summary>
	void DrawForOffScreen()override;

	ViewProjection* GetViewProjection()override { return &vp_; }

private:

	void Debug();

	void CameraUpdate();

	void ChangeScene();

private:
	Audio* audio_;
	Input* input_;
	Object3dCommon* objCommon_;
	SpriteCommon* spCommon_;
	ParticleCommon* ptCommon_;

	ViewProjection vp_;
	std::unique_ptr<DebugCamera> debugCamera_;

	std::unique_ptr<ParticleEmitter> emitter_;

	bool isAuto_ = false;

	std::unique_ptr<Sprite> numbers_;
	Vector2 pos_;

	Vector3 scale_;
	Vector3 rotate_;
	Vector3 translation_;
	Matrix4x4 transform_;
};

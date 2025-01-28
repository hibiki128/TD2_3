#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "Input.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "ParticleCommon.h"
#include "ViewProjection.h"
#include "DebugCamera.h"

// Application
#include "application/Object/Player/Player.h"
#include "application/Object/MapChip/MapChipField.h"
#include"application/Pause/Pause.h"

class GameScene : public BaseScene
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

	// ビュープロジェクション
	ViewProjection vp_;
	std::unique_ptr<DebugCamera> debugCamera_;

	///
	///	各オブジェクト
	/// 
	
	// プレイヤー
	std::unique_ptr<Player> player_;

	// マップチップフィールド
	std::unique_ptr<MapChipField> mapChipField_;

	// 音関連
	uint32_t BGM_;

// JSON
private:
	using json = nlohmann::json;
	// ポーズ
	std::unique_ptr<Pause> pause_;

	///
	///	スプライト（中間プレイ会のため一時的に）
	/// 

	void SaveToJson();
	void LoadFromJson();
};

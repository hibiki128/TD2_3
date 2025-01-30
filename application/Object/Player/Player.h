#pragma once

// Engine
#include "Input.h"
#include "myEngine/utility/collider/Collider.h"

// Application
#include "application/Base/BaseObject.h"
#include "application/Object/MapChip/MapChipField.h"
#include "application/Transition/SquareTransition.h"

class Player : public BaseObject
{
public:
	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool hittingGround_ = false;
		bool hittingCeiling_ = false;
		bool hittingLeft_ = false;
		bool hittingRight_ = false;

		bool isOverlapping_ = false; // ブロックとの重なり状態

		Block* blockX = nullptr; // X方向で衝突したブロック
		Block* blockY = nullptr; // Y方向で衝突したブロック
	};

	void Init(const std::string className)override;
	void Update(MapChipField* mapChipField);
	void Draw(const ViewProjection& viewProjection)override;
	void DrawSprite(const ViewProjection& viewProjection);
	void DebugImGui()override;

	// プレイヤーがゴールに触れたか判定（プレイヤーが接地しているかつ、Bボタンを押した時のみ）
	bool IsGoalReached();
	// 現在の取得コイン数
	uint32_t GetCurrentCoinCount() { return currentCoinCount_; }
	
	// プレイヤーの位置を設定
	void SetInitialPosition(Vector3 playerInitialPosition) { this->transform_.translation_ = playerInitialPosition; }

private:
	const float kDeltaTime = 1.0f / 60.0f;

	// 入力
	Input* input_;

	///
	/// 基本的なパラメータ
	/// 
	
	// マップとの当たり判定情報
	CollisionMapInfo collisionMapInfo_;
	// 微小な値
	const float kBlank = 0.0001f;

	// サイズ
	const float kWidth = 1.8f;
	const float kHeight = 1.8f;

	// リセット時のトランジションにかける時間
	const float kResetTransitionTime = 0.3f;

	/*ブロック反転中、プレイヤーが動かないようにするために使用*/
	bool isInverting_ = false;          // ブロック反転中かどうか
	float invertTimer_ = 0.0f;          // タイマー
	const float invertDuration_ = 0.4f; // 反転アニメーションの合計時間

	// 重力反転状態かどうか
	bool isGravityReversed_ = false; // 初期状態は通常

	/*プレイヤーの現在の色*/
	enum class ColorState {
		White,
		Black,
	};
	// プレイヤーの色の状態
	ColorState colorState_ = ColorState::White;

	// 現在取得したコインの枚数
	uint32_t currentCoinCount_ = 0;

	//////////////////
	/*調整パラメーター*/
	/////////////////

	// 移動関連
	Vector3 velocity_; // 速度
	const float kMoveSpeed = 0.15f; // 移動速度

	// ジャンプ関連
	float gravityAcceleration_; // 重力加速度
	float jumpAcceleration_; // ジャンプ初速

	// 反転可能範囲
	int xInvertRange_;
	int yInvertRange_;

	///
	///	その他
	///	

	// リセット時のトランジション
	std::unique_ptr<SquareTransition> squareTransition_;
	// プレイヤー反転範囲スプライト
	std::unique_ptr<Sprite> spritePlayerArea_;

	// プレイヤー反転範囲スプライトのサイズ
	float xSpritePlayerAreaSize_ = 0.0f;
	float ySpritePlayerAreaSize_ = 0.0f;

	// ブロック反転のクールタイム
	float blockInvertCooldown_ = 0.0f;
	const float kBlockInvertCooldownTime = 0.02f; // 再使用までの時間

private:
	// 入力操作
	void HandleInput();
	// 全ての衝突判定とプレイヤーの押し戻し
	void CheckCollisionAndResolve();

	// 衝突判定
	/*void OnCollision([[maybe_unused]] Collider* other)override;*/

	// マップチップフィールドを保持
	MapChipField* mapChipField_ = nullptr;
	// マップとの当たり判定情報を返す
	CollisionMapInfo GetMapCollisionInfo();

	// 反転可能範囲のAABBを描画
	void DrawInvertArea();
	// 反転可能範囲画像をプレイヤーの座標にセット
	void InvertAreaSpriteToPlayerPosition(const ViewProjection& viewProjection);
	// 反転可能範囲画像サイズを現在の範囲によって変更（ごり押しで）
	void InvertAreaSpriteAdjust();

	// プレイヤーがコインオブジェクトに触れたかを判定
	bool IsCollidingCoin(const Coin& coin);
	// 取得したコインの座標を保存しておく
	Vector3 lastCollectedCoinPosition_ = {0.0f, 0.0f, 0.0f};

	// リセット
	void Reset();

private:
	using json = nlohmann::json;

	void SaveToJson();
	void LoadFromJson();

///
/// SE・エフェクト用のフラグ
/// 
public:
	// ジャンプした瞬間を判定
	bool IsJumpOccurred() { return isJumpOccurred_; }
	// ブロック反転した瞬間を判定
	bool IsBlockInversionOccurred() { return isBlockInversionOccurred_; }
	// リセットした瞬間を判定
	bool IsResetOccurred() { return isResetOccurred_; }
	// 重力反転した瞬間を判定
	bool IsGravityReversedOccurred() { return isGravityReversedOccurred_; }
	// 着地した瞬間を判定
	bool IsLandedOccurred();
	// コインを取得した瞬間を判定
	bool IsCollectCoinOccurred() { return isCollectCoinOccurred_; }

private:
	// ジャンプした瞬間を判定
	bool isJumpOccurred_ = false;
	// ブロック反転した瞬間を判定
	bool isBlockInversionOccurred_ = false;
	// リセットした瞬間を判定
	bool isResetOccurred_ = false;
	// 重力反転した瞬間を判定
	bool isGravityReversedOccurred_ = false;

	// 前フレームの接地状態を記録
	bool prevHittingGround_ = false;

	// コインを取得した瞬間を判定
	bool isCollectCoinOccurred_ = false;
};
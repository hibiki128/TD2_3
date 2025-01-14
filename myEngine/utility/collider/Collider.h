#pragma once
#include"Vector3.h"
#include"WorldTransform.h"
#include"Object3d.h"
#include"ViewProjection.h"
#include <fstream>
#include <filesystem>
#include "externals/nlohmann/json.hpp"

class Collider {
public:

	Collider();

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~Collider();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string className);

	/// <summary>
	/// ワールドトランスフォームの更新
	/// </summary>
	void UpdateWorldTransform();

	void DebugDraw(const ViewProjection& viewProjection);

	void OffsetImgui();

	/// <summary>
	/// 当たってる間
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollision([[maybe_unused]] Collider* other) {};

	/// <summary>
	/// 当たった瞬間
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollisionEnter([[maybe_unused]] Collider* other) {};

	/// <summary>
	/// 当たり終わった瞬間
	/// </summary>
	/// <param name="other"></param>
	virtual void OnCollisionOut([[maybe_unused]] Collider* other) {};

#pragma region ゲッター
	/// <summary>
	/// getter
	/// </summary>
	/// <returns></returns>
	// 半径を取得
	float GetRadius() { return radius_; }
	// 中心座標を取得
	virtual Vector3 GetCenterPosition() const = 0;
	virtual Vector3 GetCenterRotation() const = 0;

	AABB GetAABB() { return aabb; }
	OBB GetOBB() { return obb; }
	Sphere GetSphere() { return sphere; }
	bool IsCollisionEnabled() const { return isCollisionEnabled_; }
	bool IsColliding() const { return isColliding; }
	bool WasColliding() const { return wasColliding; }
	bool IsOBB() { return isOBB; }
	bool IsSphere() { return isSphere; }
	bool IsAABB() { return isAABB; }
	bool IsVisible() { return isVisible; }
#pragma endregion

#pragma region セッター


	void SetIsCollidingInCurrentFrame(bool isColliding) {
		isCollidingInCurrentFrame_ = isColliding;
	}

	bool IsCollidingInCurrentFrame() const {
		return isCollidingInCurrentFrame_;
	}

	void ResetCollisionFlag() {
		isCollidingInCurrentFrame_ = false;
	}

	/// <summary>
	/// setter
	/// </summary>
	/// <param name="radius"></param>
	void SetRadius(float radius) { radius_ = radius; }
	void SetIsColliding(bool colliding) { isColliding = colliding; wasColliding = isColliding; }
	void SetCollisionEnabled(bool enabled) { isCollisionEnabled_ = enabled; }
	void SetHitColor() { color_ = { 1.0f,0.0f,0.0f,1.0f }; }
	void SetDefaultColor() { color_ = { 1.0f,1.0f,1.0f,1.0f }; }

#pragma endregion

private:
	void MakeOBBOrientations(OBB& obb, const Vector3& rotate);
	void UpdateOBB();
	void SaveToJson();
	void LoadFromJson();

#pragma region デバッグ描画
	void DrawSphere(const ViewProjection& viewProjection);

	void DrawAABB(const ViewProjection& viewProjection);

	void DrawOBB(const ViewProjection& viewProjection);

	void DrawRotationCenter(const ViewProjection& viewProjection);

	// 球を描画する関数
	void DrawSphereAtCenter(const ViewProjection& viewProjection, const Vector3& center, float radius);
#pragma endregion

private:
	using json = nlohmann::json;

	// 衝突半径
	float radius_ = 1.0f;

	std::unique_ptr<Object3d>sphere_;
	std::unique_ptr<Object3d>AABB_;
	std::unique_ptr<Object3d>OBB_;

	AABB aabb;
	OBB obb;
	Sphere sphere;
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	static int counter; // 静的カウンタ
	Sphere SphereOffset;
	AABB AABBOffset;
	OBB OBBOffset;
	std::string className_;

	bool isCollisionEnabled_ = true;  // デフォルトではコリジョンを有効化
	bool isColliding = false;   // 現在のフレームの衝突状態
	bool wasColliding = false;  // 前フレームの衝突状態
	bool isCollidingInCurrentFrame_ = false; // 現フレームで衝突しているか

	bool isAABB = true;
	bool isOBB = true;
	bool isSphere = true;
	bool isVisible = true;
};
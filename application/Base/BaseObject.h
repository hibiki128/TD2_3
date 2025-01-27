#pragma once
#include"ViewProjection.h"
#include"WorldTransform.h"
#include"Object3d.h"
#include"ObjColor.h"
#include"myEngine/utility/collider/Collider.h"
#include "externals/nlohmann/json.hpp"
#include<string>

class BaseObject : public Collider {
protected:

	/// ===================================================
	///protected variaus
	/// ===================================================

	// モデル配列データ
	std::unique_ptr<Object3d> obj3d_;
	// ベースのワールド変換データ
	WorldTransform transform_;
	//カラー
	ObjColor objColor_;
	// ライティング
	bool isLighting_;
	bool isLoop_;
	bool skeletonDraw_;

	std::string className_;

private:
	using json = nlohmann::json;

public:

	/// ===================================================
	///public method
	/// ===================================================

	//初期化、更新、描画
	virtual void Init(const std::string className);
	virtual void Update();
	virtual void Draw(const ViewProjection& viewProjection);

	virtual void CreateModel(const std::string modelname);
	virtual void CreateCollider();

	virtual void DebugImGui();

	Vector3 GetCenterPosition()const override;
	Vector3 GetCenterRotation()const override;

	// 中心座標取得
	virtual Vector3 GetWorldPosition() const;
	virtual const WorldTransform& GetWorldTransform() const { return transform_; }

	/// ===================================================
	///getter 
	/// ===================================================
	const WorldTransform& GetTransform() { return transform_; }

	/// ===================================================
	///setter 
	/// ===================================================
	void SetObjColor(Vector4 c) { objColor_.SetColor(c); }
	void SetWorldPosition(Vector3 pos) { transform_.translation_ = pos; }
	void SetWorldPositionY(float pos) { transform_.translation_.y = pos; }
	void SetWorldPositionX(float pos) { transform_.translation_.x = pos; }
	void SetWorldPositionZ(float pos) { transform_.translation_.z = pos; }
	void AddPosition(Vector3 pos) { transform_.translation_ += pos; }
	void SetRotation(Vector3 rotate) { transform_.rotation_ = rotate; }
	void SetRotationY(float rotate) { transform_.rotation_.y = rotate; }
	void SetScale(Vector3 scale) { transform_.scale_ = scale; }
	void SetLighting(bool isLighting) { isLighting_ = isLighting; }
	void SetTexture(const std::string& filePath) { obj3d_->SetTexture(filePath); }


private:
	void DebugTransform();
	void DebugCollider();
	void SaveToJson();
	void LoadFromJson();
	void ShowFileSelector();

	std::vector<std::string> GetGltfFiles();

};

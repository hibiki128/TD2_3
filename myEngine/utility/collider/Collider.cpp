#include "Collider.h"
#include"CollisionManager.h"

int Collider::counter = -1;  // 初期値を-1に変更

Collider::Collider() {
	Cubewt_.Initialize();
	AABBwt_.Initialize();
	OBBwt_.Initialize();
	CollisionManager::AddCollider(this);
	sphere_ = std::make_unique<Object3d>();
	sphere_->Initialize("debug/Collider.obj");
	AABB_ = std::make_unique<Object3d>();
	AABB_->Initialize("debug/AABB.obj");
	OBB_ = std::make_unique<Object3d>();
	OBB_->Initialize("debug/OBB.obj");
	variables_ = GlobalVariables::GetInstance();

	// 派生クラス名を取得して整形
	std::string className = typeid(*this).name();
	size_t pos = className.find("Collider");
	if (pos != std::string::npos) {
		className.replace(pos, 8, ""); // "Collider"部分を削除
	}

	counter++;
	groupName = className + " Collider" + std::to_string(counter);

	// 初期化
	SphereOffset = { 0.0f, 0.0f, 0.0f };
	AABBOffset.min = { 0.0f, 0.0f, 0.0f };
	AABBOffset.max = { 0.0f, 0.0f, 0.0f };
	OBBOffset.center = { 0.0f,0.0f,0.0f };
	OBBOffset.size = { 0.0f,0.0f,0.0f };

	// グループがまだ存在しない場合のみ作成
	if (!variables_->GroupExists(groupName)) {
		variables_->CreateGroup(groupName);
		variables_->AddItem(groupName, "Sphere Translation", SphereOffset);
		variables_->AddItem(groupName, "AABB Min", AABBOffset.min);
		variables_->AddItem(groupName, "AABB Max", AABBOffset.max);
		variables_->AddItem(groupName, "OBB center", OBBOffset.center);
		variables_->AddItem(groupName, "OBB size", OBBOffset.size);
	}
	color_.SetColor({ 0.0f,0.0f,0.0f,1.0f });
}

Collider::~Collider()
{
	CollisionManager::RemoveCollider(this);
	counter--;  // カウンターをデクリメント
}

void Collider::Initialize() {
}

void Collider::UpdateWorldTransform() {
	ApplyVariables();

	// 球用のワールドトランスフォームを更新
	Cubewt_.translation_ = GetCenterPosition() + SphereOffset;
	Cubewt_.scale_ = { radius_, radius_, radius_ };
	Cubewt_.UpdateMatrix();

	// AABBの現在の最小点と最大点を取得
	aabb.min = GetCenterPosition() - scale_;
	aabb.max = GetCenterPosition() + scale_;
	aabb.min = aabb.min + AABBOffset.min;
	aabb.max = aabb.max + AABBOffset.max;

	// AABBの最小点と最大点からスケールと中心位置を計算
	aabbCenter = (aabb.min + aabb.max) * 0.5f;
	aabbScale = (aabb.max - aabb.min) * 0.5f;

	// AABB用ワールドトランスフォームの更新
	AABBwt_.translation_ = aabbCenter;
	AABBwt_.scale_ = aabbScale;
	AABBwt_.UpdateMatrix();

	obb.center = GetCenterPosition();
	obb.center = obb.center + OBBOffset.center;
	MakeOBBOrientations(obb, GetCenterRotation());
	obb.size = { 1.0f,1.0f,1.0f };
	obb.size = obb.size + OBBOffset.size;

	OBBwt_.translation_ = obb.center;
	OBBwt_.rotation_ = GetCenterRotation();
	OBBwt_.scale_ = obb.size;
	OBBwt_.UpdateMatrix();
}

void Collider::DrawSphere(const ViewProjection& viewProjection) {
	sphere_->Draw(Cubewt_, viewProjection, &color_, true);
}

void Collider::DrawAABB(const ViewProjection& viewProjection)
{
	AABB_->Draw(AABBwt_, viewProjection, &color_, false);
}

void Collider::DrawOBB(const ViewProjection& viewProjection) {
	// OBBの描画処理（AABBと似た形で描画）
	OBB_->Draw(OBBwt_, viewProjection, &color_, false);
}

void Collider::ApplyVariables()
{
	SphereOffset = variables_->GetVector3Value(groupName, "Sphere Translation");
	AABBOffset.min = variables_->GetVector3Value(groupName, "AABB Min");
	AABBOffset.max = variables_->GetVector3Value(groupName, "AABB Max");
	OBBOffset.center = variables_->GetVector3Value(groupName, "OBB center");
	OBBOffset.size = variables_->GetVector3Value(groupName, "OBB size");
}

void Collider::MakeOBBOrientations(OBB& obb, const Vector3& rotate) {
	Matrix4x4 rotateMatrix = MakeRotateXMatrix(rotate.x) * (MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z));
	obb.orientations[0].x = rotateMatrix.m[0][0];
	obb.orientations[0].y = rotateMatrix.m[0][1];
	obb.orientations[0].z = rotateMatrix.m[0][2];

	obb.orientations[1].x = rotateMatrix.m[1][0];
	obb.orientations[1].y = rotateMatrix.m[1][1];
	obb.orientations[1].z = rotateMatrix.m[1][2];

	obb.orientations[2].x = rotateMatrix.m[2][0];
	obb.orientations[2].y = rotateMatrix.m[2][1];
	obb.orientations[2].z = rotateMatrix.m[2][2];
}

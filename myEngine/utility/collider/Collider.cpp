#define NOMINMAX
#include "Collider.h"
#include"CollisionManager.h"
#include <line/DrawLine3D.h>

int Collider::counter = -1;  // 初期値を-1に変更

Collider::Collider() {
	
}

Collider::~Collider()
{
	CollisionManager::RemoveCollider(this);
	counter--;  // カウンターをデクリメント
}

void Collider::Initialize(const std::string className) {
	CollisionManager::AddCollider(this);

	counter++;

	// 初期化
	SphereOffset.center = { 0.0f, 0.0f, 0.0f };
	SphereOffset.radius = 0.0f;
	AABBOffset.min = { 0.0f, 0.0f, 0.0f };
	AABBOffset.max = { 0.0f, 0.0f, 0.0f };
	OBBOffset.rotationCenter = { 0.0f,0.0f,0.0f };
	OBBOffset.scaleCenter = { 0.0f,0.0f,0.0f };
	OBBOffset.size = { 1.0f,1.0f,1.0f };

	className_ = className;
	LoadFromJson();
}

void Collider::UpdateWorldTransform() {

	// 球用のワールドトランスフォームを更新
	sphere.center = GetCenterPosition() + SphereOffset.center;
	sphere.radius = radius_ + SphereOffset.radius;

	// AABBの現在の最小点と最大点を取得
	aabb.min = GetCenterPosition() - Vector3(1.0f, 1.0f, 1.0f);
	aabb.max = GetCenterPosition() + Vector3(1.0f, 1.0f, 1.0f);
	aabb.min = aabb.min + AABBOffset.min;
	aabb.max = aabb.max + AABBOffset.max;

	// OBBの各プロパティを更新
	obb.rotationCenter = GetCenterPosition() + OBBOffset.rotationCenter; // 回転中心
	obb.scaleCenter = GetCenterPosition() + OBBOffset.scaleCenter;       // スケール中心

	// OBBの向きベクトルを計算
	MakeOBBOrientations(obb, GetCenterRotation());

	// サイズを更新
	obb.size = OBBOffset.size;

	UpdateOBB();

}

void Collider::DebugDraw(const ViewProjection& viewProjection)
{
	if (!isVisible || !isCollisionEnabled_) {
		return;
	}
	if (isSphere) {
		DrawSphere(viewProjection);
	}
	if (isAABB) {
		DrawAABB(viewProjection);
	}
	if (isOBB) {
		DrawOBB(viewProjection);
	}
}

void Collider::DrawSphere(const ViewProjection& viewProjection) {
	const uint32_t kSubdivision = 10;                                        // 分割数
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision;        // 緯度分割1つ分の角度

	// 緯度の方向に分割　-π/2 ～ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // 現在の緯度

		// 経度の方向に分割 0 ～ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度

			// 現在の点を求める
			Vector3 start = {
				sphere.center.x + sphere.radius * std::cosf(lat) * std::cosf(lon),
				sphere.center.y + sphere.radius * std::sinf(lat),
				sphere.center.z + sphere.radius * std::cosf(lat) * std::sinf(lon)
			};

			// 次の点を求める（経度方向）
			Vector3 end1 = {
				sphere.center.x + sphere.radius * std::cosf(lat) * std::cosf(lon + kLonEvery),
				sphere.center.y + sphere.radius * std::sinf(lat),
				sphere.center.z + sphere.radius * std::cosf(lat) * std::sinf(lon + kLonEvery),
			};

			// 次の点を求める（緯度方向）
			Vector3 end2 = {
				sphere.center.x + sphere.radius * std::cosf(lat + kLatEvery) * std::cosf(lon),
				sphere.center.y + sphere.radius * std::sinf(lat + kLatEvery),
				sphere.center.z + sphere.radius * std::cosf(lat + kLatEvery) * std::sinf(lon),
			};

			// 線を描画（経度方向）
			DrawLine3D::GetInstance()->SetPoints(start, end1, color_);
			// 線を描画（緯度方向）
			DrawLine3D::GetInstance()->SetPoints(start, end2, color_);
		}
	}
}


void Collider::DrawAABB(const ViewProjection& viewProjection)
{
	// AABBの頂点リスト
	std::array<Vector3, 8> vertices = {
		aabb.min,
		{ aabb.max.x, aabb.min.y, aabb.min.z },
		{ aabb.min.x, aabb.max.y, aabb.min.z },
		{ aabb.max.x, aabb.max.y, aabb.min.z },
		{ aabb.min.x, aabb.min.y, aabb.max.z },
		{ aabb.max.x, aabb.min.y, aabb.max.z },
		{ aabb.min.x, aabb.max.y, aabb.max.z },
		{ aabb.max.x, aabb.max.y, aabb.max.z }
	};

	// エッジ接続リスト
	const std::array<std::pair<int, int>, 12> edges = {
		std::make_pair(0, 1), std::make_pair(1, 3), std::make_pair(3, 2), std::make_pair(2, 0), // 前面
		std::make_pair(4, 5), std::make_pair(5, 7), std::make_pair(7, 6), std::make_pair(6, 4), // 背面
		std::make_pair(0, 4), std::make_pair(1, 5), std::make_pair(2, 6), std::make_pair(3, 7)  // 側面
	};

	// 線を描画
	for (const auto& edge : edges) {
		DrawLine3D::GetInstance()->SetPoints(vertices[edge.first], vertices[edge.second], color_);
	}
}

void Collider::DrawOBB(const ViewProjection& viewProjection) {
	// OBBの8つの頂点を計算
	std::array<Vector3, 8> vertices;
	Vector3 halfSize = obb.size; // サイズの半分を計算

	// OBBの8頂点を計算するループ
	for (int i = 0; i < 8; i++) {
		// 各頂点のローカル座標を計算
		Vector3 localPosition = Vector3(
			(i & 1) ? halfSize.x : -halfSize.x,
			(i & 2) ? halfSize.y : -halfSize.y,
			(i & 4) ? halfSize.z : -halfSize.z
		);

		// scaleCenter を基準にスケール変換
		Vector3 scaledPosition = localPosition + (obb.scaleCenter - obb.rotationCenter);

		// 回転中心を基準に回転を適用
		Vector3 rotatedPosition =
			obb.orientations[0] * scaledPosition.x +
			obb.orientations[1] * scaledPosition.y +
			obb.orientations[2] * scaledPosition.z;

		// ワールド座標へ変換
		vertices[i] = obb.rotationCenter + rotatedPosition;
	}

	// scaleCenterに球を描画
	DrawSphereAtCenter(viewProjection, obb.scaleCenterRotated, 0.1f);  // 半径0.1fで球を描画

	// エッジ接続リスト
	const std::array<std::pair<int, int>, 12> edges = {
		std::make_pair(0, 1), std::make_pair(1, 3), std::make_pair(3, 2), std::make_pair(2, 0), // 前面
		std::make_pair(4, 5), std::make_pair(5, 7), std::make_pair(7, 6), std::make_pair(6, 4), // 背面
		std::make_pair(0, 4), std::make_pair(1, 5), std::make_pair(2, 6), std::make_pair(3, 7)  // 側面
	};

	// 線を描画
	for (const auto& edge : edges) {
		DrawLine3D::GetInstance()->SetPoints(vertices[edge.first], vertices[edge.second], color_);
	}

	DrawRotationCenter(viewProjection);
}

// 球を描画する関数
void Collider::DrawSphereAtCenter(const ViewProjection& viewProjection, const Vector3& center, float radius) {
	const int slices = 16;  // 球の横方向の分割数
	const int stacks = 8;   // 球の縦方向の分割数

	for (int i = 0; i < slices; i++) {
		float theta1 = (i * 2.0f * std::numbers::pi_v<float>) / slices;
		float theta2 = ((i + 1) * 2.0f * std::numbers::pi_v<float>) / slices;

		for (int j = 0; j < stacks; j++) {
			float phi1 = (j * std::numbers::pi_v<float>) / stacks;
			float phi2 = ((j + 1) * std::numbers::pi_v<float>) / stacks;

			// 球の4つの頂点を計算
			Vector3 p1 = Vector3(
				center.x + radius * sin(phi1) * cos(theta1),
				center.y + radius * cos(phi1),
				center.z + radius * sin(phi1) * sin(theta1)
			);

			Vector3 p2 = Vector3(
				center.x + radius * sin(phi1) * cos(theta2),
				center.y + radius * cos(phi1),
				center.z + radius * sin(phi1) * sin(theta2)
			);

			Vector3 p3 = Vector3(
				center.x + radius * sin(phi2) * cos(theta1),
				center.y + radius * cos(phi2),
				center.z + radius * sin(phi2) * sin(theta1)
			);

			Vector3 p4 = Vector3(
				center.x + radius * sin(phi2) * cos(theta2),
				center.y + radius * cos(phi2),
				center.z + radius * sin(phi2) * sin(theta2)
			);

			// 4つの三角形で球の断片を描画
			DrawLine3D::GetInstance()->SetPoints(p1, p2);
			DrawLine3D::GetInstance()->SetPoints(p2, p4);
			DrawLine3D::GetInstance()->SetPoints(p4, p3);
			DrawLine3D::GetInstance()->SetPoints(p3, p1);
		}
	}
}

void Collider::OffsetImgui() {
	if (ImGui::BeginTabItem("コライダー")) {
		ImGui::Checkbox("可視化", &isVisible);
		ImGui::Checkbox("コライダーの有無", &isCollisionEnabled_);

		if (isCollisionEnabled_) {
			ImGui::Checkbox("球判定を使用する", &isSphere);
			if (isSphere) {
				ImGui::DragFloat3("中心点", &SphereOffset.center.x, 0.1f);
				ImGui::DragFloat("半径", &SphereOffset.radius, 0.1f);
			}
			ImGui::Separator();

			ImGui::Checkbox("AABB判定を使用する", &isAABB);
			if (isAABB) {
				ImGui::DragFloat3("最大値", &AABBOffset.max.x, 0.1f);
				ImGui::DragFloat3("最小値", &AABBOffset.min.x, 0.1f);
			}
			ImGui::Separator();

			ImGui::Checkbox("OBB判定を使用する", &isOBB);
			if (isOBB) {
				ImGui::DragFloat3("中心", &OBBOffset.scaleCenter.x, 0.1f);
				ImGui::DragFloat3("大きさ", &OBBOffset.size.x, 0.1f);
			}
			ImGui::Separator();
		}

		if (ImGui::Button("セーブ")) {
			SaveToJson();
			std::string message = std::format("Collider saved.");
			MessageBoxA(nullptr, message.c_str(), "Object", 0);
		}

		ImGui::EndTabItem();
	}
}

void Collider::DrawRotationCenter(const ViewProjection& viewProjection) {
	// 回転中心を表す球の半径
	float rotationCenterRadius = 0.1f;

	// 球を構成する点を描画
	const uint32_t kSubdivision = 10; // 分割数
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / kSubdivision;
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision;

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			Vector3 start = {
				obb.rotationCenter.x + rotationCenterRadius * std::cosf(lat) * std::cosf(lon),
				obb.rotationCenter.y + rotationCenterRadius * std::sinf(lat),
				obb.rotationCenter.z + rotationCenterRadius * std::cosf(lat) * std::sinf(lon)
			};

			Vector3 end1 = {
				obb.rotationCenter.x + rotationCenterRadius * std::cosf(lat) * std::cosf(lon + kLonEvery),
				obb.rotationCenter.y + rotationCenterRadius * std::sinf(lat),
				obb.rotationCenter.z + rotationCenterRadius * std::cosf(lat) * std::sinf(lon + kLonEvery),
			};

			Vector3 end2 = {
				obb.rotationCenter.x + rotationCenterRadius * std::cosf(lat + kLatEvery) * std::cosf(lon),
				obb.rotationCenter.y + rotationCenterRadius * std::sinf(lat + kLatEvery),
				obb.rotationCenter.z + rotationCenterRadius * std::cosf(lat + kLatEvery) * std::sinf(lon),
			};

			DrawLine3D::GetInstance()->SetPoints(start, end1);
			DrawLine3D::GetInstance()->SetPoints(start, end2);
		}
	}
}

void Collider::MakeOBBOrientations(OBB& obb, const Vector3& rotate) {
	// 回転行列を作成
	Matrix4x4 rotateMatrix = MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z);

	// 各方向ベクトルを計算
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

void Collider::UpdateOBB()
{
	// 回転後にscaleCenterの位置を計算
	obb.scaleCenterRotated = obb.orientations[0] * (obb.scaleCenter.x - obb.rotationCenter.x) +
		obb.orientations[1] * (obb.scaleCenter.y - obb.rotationCenter.y) +
		obb.orientations[2] * (obb.scaleCenter.z - obb.rotationCenter.z) + obb.rotationCenter;
}

void Collider::SaveToJson() {
	json j;

	// 各種フラグをJSONに保存
	j["isVisible"] = isVisible;
	j["isCollisionEnabled"] = isCollisionEnabled_;
	j["isSphere"] = isSphere;
	j["isAABB"] = isAABB;
	j["isOBB"] = isOBB;

	// 各オフセット値をJSONに保存
	j["SphereOffset"]["center"] = { SphereOffset.center.x, SphereOffset.center.y, SphereOffset.center.z };
	j["SphereOffset"]["radius"] = SphereOffset.radius;
	j["AABBOffset"]["min"] = { AABBOffset.min.x, AABBOffset.min.y, AABBOffset.min.z };
	j["AABBOffset"]["max"] = { AABBOffset.max.x, AABBOffset.max.y, AABBOffset.max.z };
	j["OBBOffset"]["scaleCenter"] = { OBBOffset.scaleCenter.x, OBBOffset.scaleCenter.y, OBBOffset.scaleCenter.z };
	j["OBBOffset"]["size"] = { OBBOffset.size.x, OBBOffset.size.y, OBBOffset.size.z };

	// ディレクトリを作成し、JSONファイルを保存
	std::filesystem::create_directories("resources/jsons/Collider/");
	std::ofstream outFile("resources/jsons/Collider/" + className_ + ".json");
	outFile << j.dump(4);
}

void Collider::LoadFromJson() {
	std::ifstream inFile("resources/jsons/Collider/" + className_ + ".json");
	if (!inFile.is_open()) {
		return; // JSONファイルがない場合は早期リターン
	}

	json j;
	inFile >> j;

	// 各種フラグをJSONから読み込み
	isVisible = j["isVisible"].get<bool>();
	isCollisionEnabled_ = j["isCollisionEnabled"].get<bool>();
	isSphere = j["isSphere"].get<bool>();
	isAABB = j["isAABB"].get<bool>();
	isOBB = j["isOBB"].get<bool>();

	// 各オフセット値をJSONから読み込み
	SphereOffset.center = { j["SphereOffset"]["center"][0], j["SphereOffset"]["center"][1], j["SphereOffset"]["center"][2] };
	SphereOffset.radius = j["SphereOffset"]["radius"];
	AABBOffset.min = { j["AABBOffset"]["min"][0], j["AABBOffset"]["min"][1], j["AABBOffset"]["min"][2] };
	AABBOffset.max = { j["AABBOffset"]["max"][0], j["AABBOffset"]["max"][1], j["AABBOffset"]["max"][2] };
	OBBOffset.scaleCenter = { j["OBBOffset"]["scaleCenter"][0], j["OBBOffset"]["scaleCenter"][1], j["OBBOffset"]["scaleCenter"][2] };
	OBBOffset.size = { j["OBBOffset"]["size"][0], j["OBBOffset"]["size"][1], j["OBBOffset"]["size"][2] };
}

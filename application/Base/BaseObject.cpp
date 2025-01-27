#include "BaseObject.h"


void BaseObject::Init(const std::string className) {
	className_ = className;
	/// ワールドトランスフォームの初期化
	transform_.Initialize();
	//カラーのセット
	objColor_.Initialize();
	objColor_.SetColor(Vector4(1, 1, 1, 1));
	// ライティングのセット
	isLighting_ = true;

	LoadFromJson();
}

void BaseObject::Update() {

	//元となるワールドトランスフォームの更新
	transform_.UpdateMatrix();
	/// 色転送
	objColor_.TransferMatrix();
	if (obj3d_->GetHaveAnimation()) {
		obj3d_->AnimationUpdate(isLoop_);
	}
}

void BaseObject::Draw(const ViewProjection& viewProjection) {
	obj3d_->Draw(transform_, viewProjection, &objColor_, isLighting_);
	if (skeletonDraw_) {
		obj3d_->DrawSkeleton(transform_, viewProjection);
	}
}

Vector3 BaseObject::GetWorldPosition() const {
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = transform_.matWorld_.m[3][0];
	worldPos.y = transform_.matWorld_.m[3][1];
	worldPos.z = transform_.matWorld_.m[3][2];

	return worldPos;
}

void BaseObject::CreateModel(const std::string modelname) {
	obj3d_ = std::make_unique<Object3d>();
	obj3d_->Initialize(modelname);
}

void BaseObject::CreateCollider()
{

	Collider::Initialize(className_);
}

void BaseObject::DebugImGui()
{
	ImGui::Begin(className_.c_str());
	if (ImGui::BeginTabBar(className_.c_str())) {
		DebugTransform();
		DebugCollider();
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void BaseObject::DebugTransform()
{
	if (ImGui::BeginTabItem("トランスフォーム")) {
		ImGui::DragFloat3("位置", &transform_.translation_.x, 0.1f);
		float rotationDegrees[3] = {
		radiansToDegrees(transform_.rotation_.x),
		radiansToDegrees(transform_.rotation_.y),
		radiansToDegrees(transform_.rotation_.z)
		};
		if (ImGui::DragFloat3("回転", rotationDegrees, 0.1f, -360.0f, 360.0f)) {
			// 操作後、度数法からラジアンに戻して保存
			transform_.rotation_.x = degreesToRadians(rotationDegrees[0]);
			transform_.rotation_.y = degreesToRadians(rotationDegrees[1]);
			transform_.rotation_.z = degreesToRadians(rotationDegrees[2]);
		}
		ImGui::DragFloat3("大きさ", &transform_.scale_.x, 0.1f);
		if (ImGui::Button("セーブ")) {
			SaveToJson();
			std::string message = std::format("Transform saved.");
			MessageBoxA(nullptr, message.c_str(), "Object", 0);
		}
		ImGui::EndTabItem();
	}
	if (obj3d_->GetHaveAnimation()) {
		if (ImGui::BeginTabItem("アニメーション")) {
			ImGui::Checkbox("ループ", &isLoop_);
			ImGui::Checkbox("スケルトン描画", &skeletonDraw_);
			if (ImGui::Button("アニメーション再生")) {
				obj3d_->PlayAnimation();
			}
			if (ImGui::TreeNode("setAnima")) {
				ShowFileSelector();
				ImGui::TreePop();
			}
			ImGui::EndTabItem();
		}
	}
}

void BaseObject::DebugCollider()
{
	Collider::OffsetImgui();
}

Vector3 BaseObject::GetCenterPosition() const
{
	return transform_.translation_;
}

Vector3 BaseObject::GetCenterRotation() const
{
	return transform_.rotation_;
}

void BaseObject::SaveToJson() {
	json j;

	j["translate"] = { transform_.translation_.x,transform_.translation_.y,transform_.translation_.z };
	j["rotation"] = { transform_.rotation_.x,transform_.rotation_.y,transform_.rotation_.z };
	j["scale"] = { transform_.scale_.x,transform_.scale_.y,transform_.scale_.z };

	// ディレクトリを作成し、JSONファイルを保存
	std::filesystem::create_directories("resources/jsons/Transform/");
	std::ofstream outFile("resources/jsons/Transform/" + className_ + ".json");
	outFile << j.dump(4);
}

void BaseObject::LoadFromJson() {
	std::ifstream inFile("resources/jsons/Transform/" + className_ + ".json");
	if (!inFile.is_open()) {
		return; // JSONファイルがない場合は早期リターン
	}

	json j;
	inFile >> j;

	// 各種JSONから読み込み
	transform_.translation_ = { j["translate"][0],j["translate"][1], j["translate"][2] };
	transform_.rotation_ = { j["rotation"][0],j["rotation"][1], j["rotation"][2] };
	transform_.scale_ = { j["scale"][0],j["scale"][1], j["scale"][2] };

}

void BaseObject::ShowFileSelector()
{
	static int selectedIndex = -1; // 選択中のインデックス（-1は未選択）
	static std::vector<std::string> gltfFiles = GetGltfFiles(); // GLTFファイルのリスト

	// ファイルリストをCスタイル文字列の配列に変換
	std::vector<const char*> fileNames;
	for (const auto& filePath : gltfFiles) {
		fileNames.push_back(filePath.c_str());
	}

	ImGui::Text("Select a GLTF file:");
	ImGui::Separator();

	// Comboボックスでファイル選択
	if (ImGui::Combo("GLTF Files", &selectedIndex, fileNames.data(), static_cast<int>(fileNames.size()))) {
		// ファイル選択時の動作（選択されたファイル名を表示）
		if (selectedIndex >= 0) {
			ImGui::Text("Selected File:");
			ImGui::TextWrapped("%s", gltfFiles[selectedIndex].c_str());
		}
	}

	// ボタンでアニメーションをセット
	if (selectedIndex >= 0 && ImGui::Button("Set Animation")) {
		obj3d_->SetAnimation(gltfFiles[selectedIndex]); // 選択されたファイルをSetAnimationに渡す
	}
}


std::vector<std::string> BaseObject::GetGltfFiles()
{
	std::vector<std::string> gltfFiles;
	std::filesystem::path baseDir = "resources/models/animation"; // ベースディレクトリ
	for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
		if (entry.path().extension() == ".gltf") {
			// フルパスではなく相対パスを取得
			gltfFiles.push_back(std::filesystem::relative(entry.path(), baseDir.parent_path()).string());
		}
	}
	return gltfFiles;
}



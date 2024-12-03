#include "Model.h"
#include "myMath.h"
#include"fstream"
#include"sstream"
#include "TextureManager.h"

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{
	// 引数で受け取ってメンバ変数に記録する
	modelCommon_ = modelCommon;

	// モデル読み込み
	modelData = LoadObjFile(directorypath, filename);

	CreateVartexData();

	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	// 単位行列を書き込んでおく
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);
}

void Model::Draw()
{

	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である
	srvManager_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath));
	// 描画！（DrawCall/ドローコール）
	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

void Model::CreateVartexData()
{
	// Sprite用の頂点リソースを作る
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点データの設定
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // 開けなかったら止める
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "../images/" + textureFilename;
		}
	}

	// テクスチャが張られていない場合の処理
	if (materialData.textureFilePath.empty()) {
		materialData.textureFilePath = directoryPath + "/" + "white1x1.png";
	}

	return materialData;
}


Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行目を格納するもの

	// ファイル名からフォルダ部分を取得
	std::string folderPath;
	size_t lastSlashPos = filename.find_last_of("/\\");
	if (lastSlashPos != std::string::npos) {
		// ファイル名の前にフォルダがある場合は、そのフォルダ部分を使用する
		folderPath = filename.substr(0, lastSlashPos);
	}

	std::ifstream file(directoryPath + filename); // ファイルを開く
	assert(file.is_open()); // ファイルが開けなかったら停止

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読み込む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}
		}
		else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;

			if (!folderPath.empty()) {
				// ファイル名の前にフォルダがあればそれを追加する
				modelData.material = LoadMaterialTemplateFile(directoryPath + folderPath, materialFilename);
			}
			else {
				// ファイル名の前にフォルダがあればそれを追加する
				modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
			}
		}
	}
	return modelData;
}
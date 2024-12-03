#include "ModelManager.h"
ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みモデルを探索
	if (models.contains(filePath)) {
		// 読み込み済みなら早期リターン
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon, "resources/models/", filePath);
	model->SetSrv(srvManager);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込みモデルを戻り値としてreturn
		return models.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}

void ModelManager::Initialize(SrvManager* srvManager)
{
	modelCommon = new ModelCommon;
	modelCommon->Initialize();
	this->srvManager = srvManager;

}

void ModelManager::Finalize()
{

	delete instance;
	instance = nullptr;
}

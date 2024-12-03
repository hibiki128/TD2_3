#pragma once
#include"ModelCommon.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector2.h"
#include"SrvManager.h"
class Model
{
private:

	// 頂点データ
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	ModelCommon* modelCommon_;

	// Objファイルのデータ
	ModelData modelData;
	SrvManager* srvManager_;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="modelCommon"></param>
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	ModelData GetModelData() { return modelData; }
	void SetSrv(SrvManager* srvManager) { srvManager_ = srvManager; }

private:

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVartexData();

	/// <summary>
	/// .mtlファイルの読み取り
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	///  .objファイルの読み取り
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

};


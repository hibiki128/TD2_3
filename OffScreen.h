#pragma once
#include <d3d12.h>
#include"wrl.h"
#include <PipeLineManager.h>
#include"SrvManager.h"
#include <Matrix4x4.h>
#include"myMath.h"
#include <Vector2.h>
class DirectXCommon;
class OffScreen
{
public:
	void Initialize();

	void Draw();

	void DrawCommonSetting();

	void SetProjection(Matrix4x4 projectionMatrix) { projectionInverse_ = projectionMatrix; }
private:

	void CreateSmooth();
	void CreateGauss();
	void CreateVignette();
	void CreateDepth();
private:
	DirectXCommon* dxCommon;
	SrvManager* srvManager_;
	std::unique_ptr<PipeLineManager> psoManager_ = nullptr;
	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature[5];

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState[7];
	ShaderMode shaderMode_ = ShaderMode::kNone;


	struct KernelSettings {
		int kernelSize;
	};

	struct GaussianParams {
		int kernelSize;
		float sigma;
	};

	struct VignetteParameter {
		float vignetteStrength;
		float vignetteRadius;
		float vignetteExponent;
		float padding;
		Vector2 vignetteCenter;
	};

	struct Material
	{
		Matrix4x4 projectionInverse;
	};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteResource;
	// バッファリソース内のデータを指すポインタ
	VignetteParameter* vignetteData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> smoothResource;
	// バッファリソース内のデータを指すポインタ
	KernelSettings* smoothData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> gaussianResouce;
	// バッファリソース内のデータを指すポインタ
	GaussianParams* gaussianData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthResouce;
	// バッファリソース内のデータを指すポインタ
	Material* depthData = nullptr;

	Matrix4x4 projectionInverse_;
};


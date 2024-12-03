#include "cassert"
#include "ModelManager.h"
#include "myMath.h"
#include "Object3d.h"
#include"Object3dCommon.h"



void Object3d::Initialize(const std::string& filePath)
{
	this->obj3dCommon = Object3dCommon::GetInstance();

	CreateTransformationMatrix();

	CreateMaterial();

	lightGroup = LightGroup::GetInstance();

	ModelManager::GetInstance()->LoadModel(filePath);

	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::Update(const WorldTransform& worldTransform, const ViewProjection& viewProjection)
{
	if (lightGroup) {
		lightGroup->Update(viewProjection);
	}
	Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	if (worldTransform.parent_) {
		worldMatrix *= worldTransform.parent_->matWorld_;
	}
	Matrix4x4 worldViewProjectionMatrix;
	const Matrix4x4& viewProjectionMatrix = viewProjection.matView_ * viewProjection.matProjection_;
	worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldTransform.matWorld_;
	Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
	transformationMatrixData->WorldInverseTranspose = Transpose(worldInverseMatrix);

}

void Object3d::Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection, ObjColor* color, bool Lighting)
{

	//if (viewProjection.IsOutsideViewFrustum(worldTransform)) {
	//	return;
	//}

	if (color) {
		materialData->color = color->GetColor();
	}
	materialData->enableLighting = Lighting;
	Update(worldTransform, viewProjection);

	obj3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	obj3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	if (materialData->enableLighting != 0 && lightGroup) {
		lightGroup->Draw();
	}
	// マテリアルCBufferの場所を設定
	if (model) {
		model->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetShininess(float shininess)
{
	materialData->shininess = shininess;
}

void Object3d::CreateTransformationMatrix()
{
	transformationMatrixResource = obj3dCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	// 書き込むかめのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WorldInverseTranspose = MakeIdentity4x4();
}

void Object3d::CreateMaterial()
{
	// Sprite用のマテリアルリソースをつくる
	materialResource = obj3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 色の設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// Lightingの設定
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->shininess = 20.0f;
}

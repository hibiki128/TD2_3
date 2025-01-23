#include "cassert"
#include "ModelManager.h"
#include "myMath.h"
#include "Object3d.h"
#include"Object3dCommon.h"
#include <line/DrawLine3D.h>
#include"TextureManager.h"



void Object3d::Initialize(const std::string& filePath)
{
	this->obj3dCommon = Object3dCommon::GetInstance();

	CreateTransformationMatrix();

	CreateMaterial();

	lightGroup = LightGroup::GetInstance();

	ModelManager::GetInstance()->LoadModel(filePath);

	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);

	materialData->textureFilePath = model->GetModelData().material.textureFilePath;
	materialData->textureIndex = model->GetModelData().material.textureIndex;

	modelAnimation_ = std::make_unique<ModelAnimation>();
	modelAnimation_->SetModelData(model->GetModelData());
	modelAnimation_->Initialize("resources/models/", filePath);

	model->SetAnimator(modelAnimation_->GetAnimator());
	model->SetBone(modelAnimation_->GetBone());
	model->SetSkin(modelAnimation_->GetSkin());
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

void Object3d::AnimationUpdate(bool roop)
{
	if (modelAnimation_) {
		modelAnimation_->Update(roop);
	}
}

void Object3d::SetAnimation(const std::string& fileName)
{
	modelAnimation_ = std::make_unique<ModelAnimation>();
	modelAnimation_->SetModelData(model->GetModelData());
	modelAnimation_->Initialize("resources/models/", fileName);
	modelAnimation_->GetAnimator()->SetAnimationTime(0.0f);
	model->SetAnimator(modelAnimation_->GetAnimator());
	model->SetBone(modelAnimation_->GetBone());
	model->SetSkin(modelAnimation_->GetSkin());
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

	if (modelAnimation_->GetAnimator()->HaveAnimation()) {
		Object3dCommon::GetInstance()->skinningDrawCommonSetting();
	}

	obj3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	obj3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, materialData->textureIndex);
	if (materialData->enableLighting != 0 && lightGroup) {
		lightGroup->Draw();
	}
	// マテリアルCBufferの場所を設定
	if (model) {
		model->SetAnimator(modelAnimation_->GetAnimator());
		model->Draw();
	}
}

void Object3d::DrawSkeleton(const WorldTransform& worldTransform, const ViewProjection& viewProjection)
{
	Update(worldTransform, viewProjection);
	// スケルトンデータを取得
	const Skeleton& skeleton = modelAnimation_->GetSkeletonData();

	// 各ジョイントを巡回して親子関係の線を生成
	for (const auto& joint : skeleton.joints) {
		// 親がいない場合、このジョイントはルートなのでスキップ
		if (!joint.parent.has_value()) {
			continue;
		}

		// 親ジョイントを取得
		const auto& parentJoint = skeleton.joints[*joint.parent];

		// 親と子のスケルトン空間座標を取得
		Vector3 parentPosition = ExtractTranslation(parentJoint.skeletonSpaceMatrix);
		Vector3 childPosition = ExtractTranslation(joint.skeletonSpaceMatrix);

		// 線の色を設定（デフォルトで白色）
		Vector4 lineColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// LineManagerに現在の線分を登録
		DrawLine3D::GetInstance()->SetPoints(parentPosition, childPosition, lineColor);
	}
}

void Object3d::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetTexture(const std::string& filePath)
{
	materialData->textureFilePath = filePath;
	TextureManager::GetInstance()->LoadTexture(filePath);
	materialData->textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(filePath);
	model->SetMaterialData({ materialData->textureFilePath ,materialData->textureIndex });
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

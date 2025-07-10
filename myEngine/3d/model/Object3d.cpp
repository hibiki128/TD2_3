#include "Object3d.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "cassert"
#include "myMath.h"
#include <TextureManager.h>
#include <line/DrawLine3D.h>

void Object3d::Initialize(const std::string &filePath) {
    this->obj3dCommon = Object3dCommon::GetInstance();

    CreateTransformationMatrix();

    CreateMaterial();

    filePath_ = filePath;

    lightGroup = LightGroup::GetInstance();

    ModelManager::GetInstance()->LoadModel(filePath_);

    // モデルを検索してセットする
    model = ModelManager::GetInstance()->FindModel(filePath_);

    materialData->textureFilePath = model->GetModelData().material.textureFilePath;
    materialData->textureIndex = model->GetModelData().material.textureIndex;
    if (model->IsGltf()) {
        currentModelAnimation_ = std::make_unique<ModelAnimation>();
        currentModelAnimation_->SetModelData(model->GetModelData());
        currentModelAnimation_->Initialize("resources/models/", filePath_);

        model->SetAnimator(currentModelAnimation_->GetAnimator());
        model->SetBone(currentModelAnimation_->GetBone());
        model->SetSkin(currentModelAnimation_->GetSkin());
    }
}

void Object3d::Update(const WorldTransform &worldTransform, const ViewProjection &viewProjection) {
    if (lightGroup) {
        lightGroup->Update(viewProjection);
    }
    Matrix4x4 worldMatrix = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

    if (worldTransform.parent_) {
        worldMatrix *= worldTransform.parent_->matWorld_;
    }
    Matrix4x4 worldViewProjectionMatrix;
    const Matrix4x4 &viewProjectionMatrix = viewProjection.matView_ * viewProjection.matProjection_;
    worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;

    transformationMatrixData->WVP = worldViewProjectionMatrix;
    transformationMatrixData->World = worldTransform.matWorld_;
    Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
    transformationMatrixData->WorldInverseTranspose = Transpose(worldInverseMatrix);
    if (model && model->IsGltf()) {
        if (currentModelAnimation_->GetAnimator()->HaveAnimation()) {
            obj3dCommon->computeSkinningDrawCommonSetting();
            model->Update();
        }
    }
}

void Object3d::AnimationUpdate(bool roop) {
    if (currentModelAnimation_) {
        currentModelAnimation_->Update(roop);

        // 補間完了後の切り替え処理
        if (isAnimationSwitchPending_) {
            Animator *currentAnimator = currentModelAnimation_->GetAnimator();

            // 補間が完了しているかチェック
            if (!currentAnimator->IsBlending()) {
                // ファイル名の比較（パスを除いた部分のみ比較）
                std::string currentFile = currentAnimator->GetCurrentFilename();
                std::string nextFile = nextAnimationFileName_;

                // ファイル名が異なる場合のみ更新
                if (currentFile != nextFile) {
                    currentAnimator->UpdateCurrentFileInfo("resources/models/", nextFile);
                    filePath_ = nextFile;
                }

                // 切り替え完了フラグをリセット
                isAnimationSwitchPending_ = false;
                nextAnimationFileName_.clear();
            }
        }
    }
}

void Object3d::SetAnimationImmediate(const std::string &fileName) {
    if (fileName == filePath_) {
        return;
    }

    auto it = modelAnimations_.find(fileName);
    assert(it != modelAnimations_.end() && "Error: Animation file not found in modelAnimations_!");

    currentModelAnimation_ = it->second;
    currentModelAnimation_->GetAnimator()->SetAnimationTime(0.0f);
    currentModelAnimation_->GetAnimator()->SetIsAnimation(true);

    model->SetAnimator(currentModelAnimation_->GetAnimator());
    model->SetBone(currentModelAnimation_->GetBone());
    model->SetSkin(currentModelAnimation_->GetSkin());

    filePath_ = fileName;
}


void Object3d::SetAnimation(const std::string &animationFileName) {
    if (!currentModelAnimation_) {
        return;
    }

    Animator *animator = currentModelAnimation_->GetAnimator();
    if (!animator) {
        return;
    }

    // 現在のファイル名と比較
    std::string currentFile = animator->GetCurrentFilename();

    // 同じアニメーションの場合は何もしない
    if (currentFile == animationFileName && !isAnimationSwitchPending_) {
        return;
    }

    // 既に同じアニメーションへの切り替えが待機中の場合は何もしない
    if (isAnimationSwitchPending_ && nextAnimationFileName_ == animationFileName) {
        return;
    }

    // 新しいアニメーションへの補間開始
    animator->BlendToAnimation("resources/models/", animationFileName, 0.25f); // 0.5秒で補間

    // 切り替え待機状態にする
    isAnimationSwitchPending_ = true;
    nextAnimationFileName_ = animationFileName;
}

void Object3d::AddAnimation(const std::string &fileName) {
    auto animation = std::make_unique<ModelAnimation>();

    animation->SetModelData(model->GetModelData());
    animation->Initialize("resources/models/", fileName);
    animation->GetAnimator()->SetAnimationTime(0.0f);

    modelAnimations_.emplace(fileName, std::move(animation));
}

void Object3d::Draw(const WorldTransform &worldTransform, const ViewProjection &viewProjection, ObjColor *color, bool Lighting) {

    /*if (viewProjection.IsOutsideViewFrustum(worldTransform)) {
            return;
    }*/

    if (color) {
        materialData->color = color->GetColor();
    }
    materialData->enableLighting = Lighting;
    Update(worldTransform, viewProjection);

    if (model->IsGltf()) {
        if (currentModelAnimation_->GetAnimator()->HaveAnimation()) {
            HaveAnimation = true;
            Object3dCommon::GetInstance()->skinningDrawCommonSetting();
        } else {
            HaveAnimation = false;
        }
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
        model->Draw();
    }
}

void Object3d::DrawSkeleton(const WorldTransform &worldTransform, const ViewProjection &viewProjection) {
    Update(worldTransform, viewProjection);
    // スケルトンデータを取得
    const Skeleton &skeleton = currentModelAnimation_->GetSkeletonData();

    // 各ジョイントを巡回して親子関係の線を生成
    for (const auto &joint : skeleton.joints) {
        // 親がいない場合、このジョイントはルートなのでスキップ
        if (!joint.parent.has_value()) {
            continue;
        }

        // 親ジョイントを取得
        const auto &parentJoint = skeleton.joints[*joint.parent];

        // 親と子のスケルトン空間座標を取得
        Vector3 parentPosition = ExtractTranslation(parentJoint.skeletonSpaceMatrix);
        Vector3 childPosition = ExtractTranslation(joint.skeletonSpaceMatrix);

        // 線の色を設定（デフォルトで白色）
        Vector4 lineColor = {1.0f, 1.0f, 1.0f, 1.0f};

        // LineManagerに現在の線分を登録
        DrawLine3D::GetInstance()->SetPoints(parentPosition, childPosition, lineColor);
    }
}

void Object3d::SetModel(const std::string &filePath) {
    // モデルを検索してセットする
    ModelManager::GetInstance()->LoadModel(filePath);
    model = ModelManager::GetInstance()->FindModel(filePath);

    materialData->textureFilePath = model->GetModelData().material.textureFilePath;
    materialData->textureIndex = model->GetModelData().material.textureIndex;
    if (model->IsGltf()) {

        currentModelAnimation_->SetModelData(model->GetModelData());

        currentModelAnimation_->Initialize("resources/models/", filePath);

        model->SetAnimator(currentModelAnimation_->GetAnimator());
        model->SetBone(currentModelAnimation_->GetBone());
        model->SetSkin(currentModelAnimation_->GetSkin());
    }
}

void Object3d::SetTexture(const std::string &filePath) {
    materialData->textureFilePath = filePath;
    TextureManager::GetInstance()->LoadTexture(filePath);
    materialData->textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(filePath);
    model->SetMaterialData({materialData->textureFilePath, materialData->textureIndex});
}

void Object3d::SetShininess(float shininess) {
    materialData->shininess = shininess;
}

void Object3d::CreateTransformationMatrix() {
    transformationMatrixResource = obj3dCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
    // 書き込むかめのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void **>(&transformationMatrixData));
    // 単位行列を書き込んでおく
    transformationMatrixData->WVP = MakeIdentity4x4();
    transformationMatrixData->World = MakeIdentity4x4();
    transformationMatrixData->WorldInverseTranspose = MakeIdentity4x4();
}

void Object3d::CreateMaterial() {
    // Sprite用のマテリアルリソースをつくる
    materialResource = obj3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
    // 書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void **>(&materialData));
    // 色の設定
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    // Lightingの設定
    materialData->enableLighting = true;
    materialData->uvTransform = MakeIdentity4x4();
    materialData->shininess = 20.0f;
}

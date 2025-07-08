#include "Skin.h"
#include "SrvManager.h"
#include "algorithm"
#include <DirectXCommon.h>
#include <cassert>
#include <myMath.h>

void Skin::Initialize(const Skeleton &skeleton, const ModelData &modelData) {
    dxCommon_ = DirectXCommon::GetInstance();
    srvManager_ = SrvManager::GetInstance();
    skinCluster_ = CreateSkinCluster(skeleton, modelData);
}

void Skin::Update(const Skeleton &skeleton) {
    for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
        assert(jointIndex < skinCluster_.inverseBindPoseMatrices.size());
        skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix =
            skinCluster_.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
        skinCluster_.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
            Transpose(Inverse(skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix));
    }
}

void Skin::UpdateInputVertices(const ModelData &modelData) {
    // 入力頂点データを更新
    size_t vertexOffset = 0;

    // メッシュごとの頂点データをコピー
    for (size_t i = 0; i < modelData.vertices.size(); ++i) {
        if (vertexOffset + i < totalVertexCount) {
            skinCluster_.mappedVertex[vertexOffset + i] = modelData.vertices[i];
        }
    }
    vertexOffset += modelData.vertices.size();
}

void Skin::ExecuteSkinning(ID3D12GraphicsCommandList *commandList) {
    // リソースをバインド
    // t0: MatrixPalette
    commandList->SetComputeRootDescriptorTable(0, skinCluster_.paletteSrvHandle.second);

    // t1: InputVertices
    commandList->SetComputeRootDescriptorTable(1, skinCluster_.inputVertexSrvHandle.second);

    // t2: Influences
    commandList->SetComputeRootDescriptorTable(2, skinCluster_.influenceSrvHandle.second);

    // u0: OutputVertices
    commandList->SetComputeRootDescriptorTable(3, skinCluster_.outputVertexSrvHandle.second);

    // b0: SkinningInformation
    commandList->SetComputeRootConstantBufferView(4,
                                                  skinCluster_.skinningInformationResource->GetGPUVirtualAddress());

    // Dispatch実行
    uint32_t numGroups = (static_cast<uint32_t>(totalVertexCount) + 1023) / 1024;
    commandList->Dispatch(numGroups, 1, 1);
}

SkinCluster Skin::CreateSkinCluster(const Skeleton &skeleton, const ModelData &modelData) {
    SkinCluster skinCluster;
    DirectXCommon *dxCommon = DirectXCommon::GetInstance();
    SrvManager *srvManager_ = SrvManager::GetInstance();

    totalVertexCount = modelData.vertices.size();

    CreatePaletteResource(skinCluster, skeleton);

    CreateInfluenceResource(skinCluster, skeleton);

    CreateInputVertexResource(skinCluster, skeleton);

    CreateOutputVertexResource(skinCluster, skeleton);

    CreateSkinningInformationResource(skinCluster, skeleton);

    // InverseBindPoseMatrixの保存領域を作成
    skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
    std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), []() { return MakeIdentity4x4(); });

    // ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
    for (const auto &jointWeight : modelData.skinClusterData) { // ModelのSkinClusterの情報を解析
        auto it = skeleton.jointMap.find(jointWeight.first);    // jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
        if (it == skeleton.jointMap.end()) {                    // そんな名前のJointは存在しない。なので次に回す
            continue;
        }
        // (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
        skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
        for (const auto &vertexWeight : jointWeight.second.vertexWeights) {
            auto &currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex]; // 該当のvertexIndexのinfluence情報を参照しておく
            for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {                   // 空いてるところに入れる
                if (currentInfluence.weights[index] == 0.0f) {                              // weight==0が空いている状態なので、その場所にweightとjointのindexを代入
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = (*it).second;
                    break;
                }
            }
        }
    }

    return skinCluster;
}

void Skin::CreatePaletteResource(SkinCluster &skinCluster, const Skeleton &skeleton) {
    // palette用のResourceを確保
    skinCluster.paletteResource = dxCommon_->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
    WellForGPU *mappedPalette = nullptr;
    skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void **>(&mappedPalette));
    skinCluster.mappedPalette = {mappedPalette, skeleton.joints.size()};
    skinClusterPaletteSrvIndex_ = srvManager_->Allocate() + 1;
    skinCluster.paletteSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinClusterPaletteSrvIndex_);
    skinCluster.paletteSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinClusterPaletteSrvIndex_);

    // palette用のSRVを作成
    srvManager_->CreateSRVforStructuredBuffer(skinClusterPaletteSrvIndex_, skinCluster.paletteResource.Get(), UINT(skeleton.joints.size()), sizeof(WellForGPU));
}

void Skin::CreateInfluenceResource(SkinCluster &skinCluster, const Skeleton &skeleton) {

    // influence用のResourceを確保（全メッシュ分）
    skinCluster.influenceResource = dxCommon_->CreateBufferResource(sizeof(VertexInfluence) * totalVertexCount);
    VertexInfluence *mappedInfluence = nullptr;
    skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void **>(&mappedInfluence));
    std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * totalVertexCount);
    skinCluster.mappedInfluence = {mappedInfluence, totalVertexCount};
    skinClusterInfluenceSrvIndex_ = srvManager_->Allocate() + 1;
    skinCluster.influenceSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinClusterInfluenceSrvIndex_);
    skinCluster.influenceSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinClusterInfluenceSrvIndex_);

    srvManager_->CreateSRVforStructuredBuffer(skinClusterInfluenceSrvIndex_, skinCluster.influenceResource.Get(), UINT(totalVertexCount), sizeof(VertexInfluence));
}

void Skin::CreateInputVertexResource(SkinCluster &skinCluster, const Skeleton &skeleton) {

    // inputVertex用のResourceを確保
    skinCluster.inputVertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * totalVertexCount);
    VertexData *mappedVertex = nullptr;
    skinCluster.inputVertexResource->Map(0, nullptr, reinterpret_cast<void **>(&mappedVertex));
    skinCluster.mappedVertex = {mappedVertex, totalVertexCount};
    skinClusterInputVertexSrvIndex_ = srvManager_->Allocate() + 1;
    skinCluster.inputVertexSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinClusterInputVertexSrvIndex_);
    skinCluster.inputVertexSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinClusterInputVertexSrvIndex_);

    srvManager_->CreateSRVforStructuredBuffer(skinClusterInputVertexSrvIndex_, skinCluster.inputVertexResource.Get(), UINT(totalVertexCount), sizeof(VertexData));
}

void Skin::CreateOutputVertexResource(SkinCluster &skinCluster, const Skeleton &skeleton) {

    // outPutVertex用のResourceを確保
    skinCluster.outputVertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * totalVertexCount, true);
    skinClusterOutputVertexSrvIndex_ = srvManager_->Allocate() + 1;
    skinCluster.outputVertexSrvHandle.first = srvManager_->GetCPUDescriptorHandle(skinClusterOutputVertexSrvIndex_);
    skinCluster.outputVertexSrvHandle.second = srvManager_->GetGPUDescriptorHandle(skinClusterOutputVertexSrvIndex_);

    srvManager_->CreateUAVStructuredBuffer(skinClusterOutputVertexSrvIndex_, skinCluster.outputVertexResource.Get(), static_cast<uint32_t>(totalVertexCount), sizeof(VertexData));

    skinCluster.outputVertexBufferView.BufferLocation = skinCluster.outputVertexResource->GetGPUVirtualAddress();
    skinCluster.outputVertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * totalVertexCount);
    skinCluster.outputVertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Skin::CreateSkinningInformationResource(SkinCluster &skinCluster, const Skeleton &skeleton) {

    // SkinningInformation用のResourceを確保
    skinCluster.skinningInformationResource = dxCommon_->CreateBufferResource(sizeof(SkinningInformationForGPU));
    skinCluster.SkinningInfomationData = nullptr;
    skinCluster.skinningInformationResource->Map(0, nullptr, reinterpret_cast<void **>(&skinCluster.SkinningInfomationData));
    skinCluster.SkinningInfomationData->numVertices = static_cast<uint32_t>(totalVertexCount);
}

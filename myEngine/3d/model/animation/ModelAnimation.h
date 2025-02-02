#pragma once
#include <memory>
#include "Bone.h"
#include "Animator.h"
#include "Skin.h"
class ModelAnimation
{
private:
	std::unique_ptr<Animator> animator_;
	std::unique_ptr<Bone> bone_;
	std::unique_ptr<Skin> skin_;
	std::string directorypath_;
	std::string filename_;

	ModelData modelData_;
public:
	void Initialize(const std::string& directorypath, const std::string& filename);

	void Update(bool roop);

	void PlayAnimation();

	void SetModelData(ModelData modelData) { modelData_ = modelData; }
	Skeleton GetSkeletonData() { return bone_->GetSkeleton(); }
	Animator* GetAnimator() { return animator_.get(); }
	Bone* GetBone() { return bone_.get(); }
	Skin* GetSkin() { return skin_.get(); }

	void SetIsAnimation(bool anime) { animator_->SetIsAnimation(anime); }
};


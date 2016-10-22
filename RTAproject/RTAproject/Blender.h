#pragma once
#include "Utilities.h"
class Interpolator;
class BindPose;

class Blender
{
private:
	Interpolator* currAnim;
	Interpolator* nextAnim;
	BindPose* skeleton;
	UMLKeyFrame betweenFrame;

	double m_currBlendTime;
	double m_totalBlendTime;

	std::vector<DirectX::XMFLOAT4X4>* invBindPose;
	std::vector<DirectX::XMFLOAT4X4> skinningMatrixArray;

public:
	Blender();
	const std::vector<DirectX::XMFLOAT4X4>* GetSkinningMatrix() { return &skinningMatrixArray; }
	void SetBindPose(std::vector<DirectX::XMFLOAT4X4>* _invBindPose) { invBindPose = _invBindPose; }
	const Interpolator* GetCurrAnim() { return currAnim; }
	const Interpolator* GetNextAnim() { return nextAnim; }
	void SetCurrAnim(Interpolator* _currAnim) { currAnim = _currAnim; }
	void SetNextAnim(Interpolator* _nextAnim) { nextAnim = _nextAnim; }
	//SetNextAnim
	void Update(double _time);
};


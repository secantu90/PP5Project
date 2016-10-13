#pragma once
#include "Utilities.h"
#include <vector>


class Animation 
{
private:
	ANIM_TYPE m_animType;
	int m_numKeyFrames;
	std::vector<Keyframe> m_bones; // needs to be size of m_numKeyFrames
	float m_totalTime;
public:
	Animation();
	const std::vector<Joint>& GetFrame(int _index);
	int GetNumBones();
	void init(int _num, ANIM_TYPE _type);
};


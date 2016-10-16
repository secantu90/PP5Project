#pragma once
#include "Utilities.h"
#include <vector>


class Animation 
{

public:
	ANIM_TYPE m_animType;
	int m_numKeyFrames;
	std::vector<UMLKeyFrame> m_frames; // needs to be size of m_numKeyFrames
	float m_totalTime;
	Animation();
	const std::vector<Boint>& GetFrame(int _index);
	int GetNumBones();
	void SetFrame(int _frameIndex, int _bointIndex, Boint _boint);
	void SetNumKeyFrames(int _numKeyFrames);
	void SetTotalTime(float _totalTime);
	void init(int _num, ANIM_TYPE _type);
};


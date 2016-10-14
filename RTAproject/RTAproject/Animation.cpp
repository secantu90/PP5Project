#include "pch.h"
#include "Animation.h"


Animation::Animation()
{

}

//enum types include: LOOP, RETURN_DEFAULT, RETURN_LASTB, RUN_ONCE
void Animation::init(int _numKeyFrames, ANIM_TYPE _animType)
{
	m_animType = _animType;
	m_numKeyFrames = _numKeyFrames;
}

 const std::vector<Boint>& Animation::GetFrame(int _index)
{
	return m_frames[_index].m_bones;
}

int Animation::GetNumBones()
{
	return m_frames[0].m_numBones;
}
void Animation::SetFrame(int _frameIndex, int _bointIndex, Boint _boint)
{
	m_frames[_frameIndex].m_bones[_bointIndex] = _boint;

}
void Animation::SetNumKeyFrames(int _numKeyFrames)
{

}
void Animation::SetTotalTime(float _totalTime)
{

}

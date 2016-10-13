#include "pch.h"
#include "Animation.h"


Animation::Animation()
{
}

 const std::vector<Joint>& Animation::GetFrame(int _index)
{
	return m_bones[_index].skeleton->m_joints;
}

int Animation::GetNumBones()
{
	return m_bones[0].m_numBones;
}
//enum types include: LOOP, RETURN_DEFAULT, RETURN_LASTB, RUN_ONCE
void Animation::init(int _numKeyFrames, ANIM_TYPE _animType)
{
	m_animType = _animType;
	m_numKeyFrames = _numKeyFrames;
}

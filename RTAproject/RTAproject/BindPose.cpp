#include "pch.h"
#include "BindPose.h"


BindPose::BindPose()
{
}

const std::vector<DirectX::XMFLOAT4X4>& BindPose::GetBindPose()
{
	return m_InvBindPose;
}

int BindPose::GetNumBones()
{
	return m_numBones;
}

void BindPose::init(int _numBones, std::vector<DirectX::XMFLOAT4X4>& _matrix)
{
	m_numBones = _numBones;
	m_InvBindPose = _matrix;
}

#pragma once
#include "Content\ShaderStructures.h"
#include <vector>
class BindPose 
{
public:
	std::vector<DirectX::XMFLOAT4X4> m_InvBindPose; //size of m_numBones
	int m_numBones;
	BindPose();
	const std::vector<DirectX::XMFLOAT4X4> GetBindPose();
	int GetNumBones();
	void init(int _numBones,std::vector<DirectX::XMFLOAT4X4>& _matrix);
};

                                         
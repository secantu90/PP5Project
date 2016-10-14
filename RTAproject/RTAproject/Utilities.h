#pragma once
#include <fbxsdk.h>
#include <iostream>
#include <string>
#include "Content\ShaderStructures.h"
#include <vector>

struct Skeleton;
struct Boint;

struct BlendingIndexWeightPair
{
	unsigned int mBlendingIndex;
	float mBlendingWeight;

	BlendingIndexWeightPair() :
		mBlendingIndex(0),
		mBlendingWeight(0)
	{}
};

struct CtrlPoint
{
	DirectX::XMFLOAT3 mPosition;

	std::vector<BlendingIndexWeightPair> mBlendingInfo;

	CtrlPoint()
	{
		mBlendingInfo.reserve(4);
	}
};
struct UMLKeyFrame
{
	///////////////////////////////////////
	//Emilio Refactoring
	int m_numBones;
	double m_time;
	std::vector<Boint> m_bones;
	//End Emilio
	///////////////////////////////////////
};

struct Boint
{
	///////////////////////////////////////
	//Emilio Refactoring
	DirectX::XMFLOAT4X4 m_boneMatrix;
	//End Emilio
	///////////////////////////////////////
};

struct Keyframe
{
	double m_time;


	long long m_frameNum;
	DirectX::XMFLOAT4X4 m_worldMatrix;
	Keyframe* m_nextFrame;

	Keyframe() : m_nextFrame(nullptr) {}
};



struct Joint
{
	std::string m_jointName;
	int m_parentIndex;
	DirectX::XMFLOAT4X4 m_globalBindposeInverse;
	Keyframe* m_keyframe;
	FbxNode* m_node;

	Joint() : m_node(nullptr), m_keyframe(nullptr)
	{
		XMStoreFloat4x4(&m_globalBindposeInverse, DirectX::XMMatrixIdentity());
		m_parentIndex = -1;
	}

	~Joint()
	{
		while (m_keyframe)
		{
			Keyframe* tempFrame = m_keyframe;
			m_keyframe = m_keyframe->m_nextFrame;
			delete tempFrame;
		}
	}
};
struct Skeleton
{
	std::vector<Joint> m_joints;
};

//Created by Emilio
enum ANIM_TYPE
{
	LOOP = 0,
	RETURN_DEFAULT,
	RETURN_LAST,
	RUN_ONCE
};
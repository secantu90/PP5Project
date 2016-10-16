#pragma once
#include <map>
#include "Utilities.h"
#include "..\Content\ShaderStructures.h"
#include "Animation.h"
#include "BindPose.h"

using namespace RTAproject;

class FBXExporter
{
public:
	FbxManager* m_FBXManager;
	FbxScene* m_FBXScene;
	std::string m_InputFilePath;
	std::string m_OutputFilePath;
	bool m_HasAnimation;
	std::map<unsigned int, CtrlPoint*> m_controlPoints;

	std::vector<BlendingIndexWeightPair> m_BlendingInfos;
	unsigned int m_TriangleCount;
	std::vector<unsigned short> m_Indices;
	std::vector<RobustVertex> m_Vertices;
	Skeleton m_Skeleton;

	FbxLongLong m_AnimationLength;
	std::string m_AnimationName;
	////////////////////////
	//Emilio
	Animation m_animation;
	BindPose m_bindPose;
	std::vector<DirectX::XMFLOAT4X4> m_convertToBindPose;
	//End Emilio
	////////////////////////
	FBXExporter();
	~FBXExporter();
	bool Initialize();
	bool LoadScene(const char* inFileName);

	//void ExportFBX();
	void ProcessGeometry(FbxNode* inNode);
	void ProcessJointsAndAnimations(FbxNode* inNode);
	unsigned int FindJointIndexUsingName(const std::string& inJointName);
	void ProcessMesh(FbxNode* inNode);
	void Optimize();
	int FindVertex(const RobustVertex& inTargetVertex, const std::vector<RobustVertex>& uniqueVertices);

	void ConvertToLHS();
	void ConvertMatrixFtoX(FbxAMatrix toConvert, DirectX::XMFLOAT4X4 &matrix);
	void ConvertMatrixXtoF(DirectX::XMFLOAT4X4 matrix, FbxAMatrix &toConvert);


	void CleanupFbxManager();
	FbxAMatrix GetGeometryTransformation(FbxNode* inNode);


	void ProcessControlPoints(FbxNode* inNode);

	void ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outNormal);
	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outTangent);
	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outBinormal);

	void ProcessSkeletonHierarchy(FbxNode* inRootNode);
	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int myIndex, int inParentIndex);



	void ConvertToUML();
};


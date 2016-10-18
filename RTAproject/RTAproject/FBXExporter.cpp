#include "pch.h"
using namespace DirectX;
#include <fbxsdk.h>
#include "FBXExporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Interpolator.h"

FBXExporter::FBXExporter()
{
	m_FBXManager = nullptr;
	m_FBXScene = nullptr;
	m_TriangleCount = 0;
	m_HasAnimation = true;
}

FBXExporter::~FBXExporter()
{
	CleanupFbxManager();
}

bool FBXExporter::Initialize()
{
	m_FBXManager = FbxManager::Create();
	if (!m_FBXManager)
	{
		return false;
	}

	FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(m_FBXManager, IOSROOT);
	m_FBXManager->SetIOSettings(fbxIOSettings);

	m_FBXScene = FbxScene::Create(m_FBXManager, "myScene");

	return true;
}

bool FBXExporter::LoadScene(const char* inFileName)
{
	m_InputFilePath = inFileName;

	FbxImporter* fbxImporter = FbxImporter::Create(m_FBXManager, "myImporter");

	if (!fbxImporter)
	{
		return false;
	}

	if (!fbxImporter->Initialize(inFileName, -1, m_FBXManager->GetIOSettings()))
	{
		return false;
	}

	if (!fbxImporter->Import(m_FBXScene))
	{
		return false;
	}
	fbxImporter->Destroy();

	return true;
}

void FBXExporter::ProcessGeometry(FbxNode* inNode)
{
	if (inNode->GetNodeAttribute())
	{
		switch (inNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessControlPoints(inNode);
			if (m_HasAnimation)
			{
				ProcessJointsAndAnimations(inNode);
			}
			ProcessMesh(inNode);
			break;
		}
	}

	for (int i = 0; i < inNode->GetChildCount(); ++i)
	{
		ProcessGeometry(inNode->GetChild(i));
	}
}

void FBXExporter::ProcessSkeletonHierarchy(FbxNode* inRootNode)
{
	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = inRootNode->GetChild(childIndex);
		ProcessSkeletonHierarchyRecursively(currNode, 0, -1);
	}
}

void FBXExporter::ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int myIndex, int inParentIndex)
{
	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint currJoint;
		currJoint.m_parentIndex = inParentIndex;
		currJoint.m_jointName = inNode->GetName();
		m_Skeleton.m_joints.push_back(currJoint);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), static_cast<int>(m_Skeleton.m_joints.size()), myIndex);
	}
}

void FBXExporter::ProcessControlPoints(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	unsigned int ctrlPointCount = currMesh->GetControlPointsCount();
	for (unsigned int i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* currCtrlPoint = new CtrlPoint();
		XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->mPosition = currPosition;
		m_controlPoints[i] = currCtrlPoint;
	}
}

void FBXExporter::ProcessJointsAndAnimations(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount();
	FbxAMatrix geometryTransform = GetGeometryTransformation(inNode);

	for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (!currSkin)
		{
			continue;
		}

		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
		{
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			std::string currJointName = currCluster->GetLink()->GetName();
			unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			// Update the information in mSkeleton
			ConvertMatrixFtoX(globalBindposeInverseMatrix, m_Skeleton.m_joints[currJointIndex].m_globalBindposeInverse);
			m_Skeleton.m_joints[currJointIndex].m_node = currCluster->GetLink();

			// Associate each joint with the control points it affects
			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i < numOfIndices; ++i)
			{
				BlendingIndexWeightPair currBlendingIndexWeightPair;
				currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
				currBlendingIndexWeightPair.mBlendingWeight = static_cast<float>(currCluster->GetControlPointWeights()[i]);
				m_controlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
			}

			// Get animation information
			// Now only supports one take
			FbxAnimStack* currAnimStack = m_FBXScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			m_AnimationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = m_FBXScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			m_AnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
			Keyframe** currAnim = &m_Skeleton.m_joints[currJointIndex].m_keyframe;
			m_animation.init(static_cast<int>(end.GetFrameCount(FbxTime::eFrames24)), ANIM_TYPE::LOOP);
			m_animation.m_totalTime = static_cast<float>(m_AnimationLength);
			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= m_animation.m_numKeyFrames; ++i)
			{
				FbxTime currTime;
				currTime.SetFrame(i, FbxTime::eFrames24);
				*currAnim = new Keyframe();
				(*currAnim)->m_frameNum = i;
				(*currAnim)->m_time = currTime.GetSecondDouble();
				FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;

				ConvertMatrixFtoX(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime), (*currAnim)->m_worldMatrix);
				currAnim = &((*currAnim)->m_nextFrame);
			}
		}
	}

	// Some of the control points only have less than 4 joints
	// affecting them.
	// For a normal renderer, there are usually 4 joints
	// I am adding more dummy joints if there isn't enough
	BlendingIndexWeightPair currBlendingIndexWeightPair;
	currBlendingIndexWeightPair.mBlendingIndex = 0;
	currBlendingIndexWeightPair.mBlendingWeight = 0;
	for (auto itr = m_controlPoints.begin(); itr != m_controlPoints.end(); ++itr)
	{
		for (size_t i = itr->second->mBlendingInfo.size(); i < 4; ++i)
		{
			itr->second->mBlendingInfo.push_back(currBlendingIndexWeightPair);
		}
	}
}

unsigned int FBXExporter::FindJointIndexUsingName(const std::string& inJointName)
{
	for (unsigned int i = 0; i < m_Skeleton.m_joints.size(); ++i)
	{
		if (m_Skeleton.m_joints[i].m_jointName == inJointName)
		{
			return i;
		}
	}
	return UINT32_MAX;
}

void FBXExporter::ProcessMesh(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();

	m_TriangleCount = currMesh->GetPolygonCount();
	int vertexCounter = 0;
	m_Indices.reserve(static_cast<uint64_t>(m_TriangleCount*3.0f));

	for (unsigned int i = 0; i < m_TriangleCount; ++i)
	{
		XMFLOAT3 normal[3];
		XMFLOAT3 binormal[3];
		XMFLOAT2 UV[3][2];

		for (unsigned int j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = m_controlPoints[ctrlPointIndex];


			ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);
			// We only have diffuse texture
			for (int k = 0; k < 1; ++k)
			{
				ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), UV[j][k]);
			}


			RobustVertex temp;
			temp.position = currCtrlPoint->mPosition;
			temp.position.z *= -1.0f;
			temp.normal = normal[j];
			temp.normal.z *= -1;
			temp.uv = UV[j][0];
			temp.uv.y = 1.0f - temp.uv.y;

			temp.blendingWeight.x = currCtrlPoint->mBlendingInfo[0].mBlendingWeight;
			temp.blendingWeight.y = currCtrlPoint->mBlendingInfo[1].mBlendingWeight;
			temp.blendingWeight.z = currCtrlPoint->mBlendingInfo[2].mBlendingWeight;
			temp.blendingWeight.w = currCtrlPoint->mBlendingInfo[3].mBlendingWeight;

			temp.blendingIndex.x = static_cast<float>(currCtrlPoint->mBlendingInfo[0].mBlendingIndex);
			temp.blendingIndex.y = static_cast<float>(currCtrlPoint->mBlendingInfo[1].mBlendingIndex);
			temp.blendingIndex.z = static_cast<float>(currCtrlPoint->mBlendingInfo[2].mBlendingIndex);
			temp.blendingIndex.w = static_cast<float>(currCtrlPoint->mBlendingInfo[3].mBlendingIndex);


			for (unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
			{
				BlendingIndexWeightPair currBlendingInfo;
				currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
				currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight;
				m_BlendingInfos.push_back(currBlendingInfo);
			}


			m_Vertices.push_back(temp);
			m_Indices.push_back(vertexCounter);
			++vertexCounter;
		}
	}

	//for (auto itr = m_controlPoints.begin(); itr != m_controlPoints.end(); ++itr)
	//{
	//	delete itr->second;
	//}
	//m_controlPoints.clear();
}

void FBXExporter::ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, XMFLOAT2& outUV)
{
	FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(0);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	if (inMesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");
	}

	FbxGeometryElementNormal* vertexNormal = inMesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
{
	if (inMesh->GetElementBinormalCount() < 1)
	{
		throw std::exception("Invalid Binormal Number");
	}

	FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::Optimize()
{
	// First get a list of unique vertices
	std::vector<RobustVertex> uniqueVertices;
	std::vector<BlendingIndexWeightPair> uniqueBlendingInfos;
	for (unsigned int i = 0; i < m_TriangleCount; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			if (FindVertex(m_Vertices[i * 3 + j], uniqueVertices) == -1)
			{
				uniqueVertices.push_back(m_Vertices[i * 3 + j]);
				//uniqueBlendingInfos.push_back(m_BlendingInfos[i * 3 + j]);
			}
		}
	}

	// Now we update the index buffer
	for (unsigned int i = 0; i < m_Indices.size(); i += 3)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			m_Indices[j + i] = FindVertex(m_Vertices[i + j], uniqueVertices);
		}
		//std::swap(m_Indices[i], m_Indices[i + 1]);
	}



	

	m_Vertices.clear();
	m_Vertices = uniqueVertices;
	uniqueVertices.clear();

	m_Skeleton.m_joints[0].m_firstFrame = m_Skeleton.m_joints[0].m_keyframe;
	m_Skeleton.m_joints[1].m_firstFrame = m_Skeleton.m_joints[1].m_keyframe;
	m_Skeleton.m_joints[2].m_firstFrame = m_Skeleton.m_joints[2].m_keyframe;
	m_Skeleton.m_joints[3].m_firstFrame = m_Skeleton.m_joints[3].m_keyframe;
	ConvertToUML();
	m_Skeleton.m_joints[0].m_keyframe = m_Skeleton.m_joints[0].m_firstFrame;
	m_Skeleton.m_joints[1].m_keyframe = m_Skeleton.m_joints[1].m_firstFrame;
	m_Skeleton.m_joints[2].m_keyframe = m_Skeleton.m_joints[2].m_firstFrame;
	m_Skeleton.m_joints[3].m_keyframe = m_Skeleton.m_joints[3].m_firstFrame;

	ConvertToLHS();
}

void FBXExporter::ConvertToLHS()
{
	for (unsigned int i = 0; i < m_Skeleton.m_joints.size(); ++i)
	{
		m_Skeleton.m_joints[i].m_globalBindposeInverse._13 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._13;
		m_Skeleton.m_joints[i].m_globalBindposeInverse._23 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._23;
		m_Skeleton.m_joints[i].m_globalBindposeInverse._43 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._43;

		m_Skeleton.m_joints[i].m_globalBindposeInverse._31 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._31;
		m_Skeleton.m_joints[i].m_globalBindposeInverse._32 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._32;
		m_Skeleton.m_joints[i].m_globalBindposeInverse._34 = -m_Skeleton.m_joints[i].m_globalBindposeInverse._34;
		
	}

	for (size_t i = 0; i < m_animation.m_frames.size(); ++i)
	{
		for (size_t j = 0; j < m_animation.GetNumBones(); ++j)
		{
			m_animation.m_frames[i].m_bones[j].m_boneMatrix._13 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._13;
			m_animation.m_frames[i].m_bones[j].m_boneMatrix._23 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._23;
			m_animation.m_frames[i].m_bones[j].m_boneMatrix._43 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._43;

			m_animation.m_frames[i].m_bones[j].m_boneMatrix._31 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._31;
			m_animation.m_frames[i].m_bones[j].m_boneMatrix._32 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._32;
			m_animation.m_frames[i].m_bones[j].m_boneMatrix._34 = -m_animation.m_frames[i].m_bones[j].m_boneMatrix._34;
		}
	}

}

void FBXExporter::ConvertMatrixFtoX(FbxAMatrix toConvert, DirectX::XMFLOAT4X4 &matrix)
{
	matrix._11 = static_cast<float>(toConvert.mData[0].mData[0]);
	matrix._12 = static_cast<float>(toConvert.mData[0].mData[1]);
	matrix._13 = static_cast<float>(toConvert.mData[0].mData[2]);
	matrix._14 = static_cast<float>(toConvert.mData[0].mData[3]);
	matrix._21 = static_cast<float>(toConvert.mData[1].mData[0]);
	matrix._22 = static_cast<float>(toConvert.mData[1].mData[1]);
	matrix._23 = static_cast<float>(toConvert.mData[1].mData[2]);
	matrix._24 = static_cast<float>(toConvert.mData[1].mData[3]);
	matrix._31 = static_cast<float>(toConvert.mData[2].mData[0]);
	matrix._32 = static_cast<float>(toConvert.mData[2].mData[1]);
	matrix._33 = static_cast<float>(toConvert.mData[2].mData[2]);
	matrix._34 = static_cast<float>(toConvert.mData[2].mData[3]);
	matrix._41 = static_cast<float>(toConvert.mData[3].mData[0]);
	matrix._42 = static_cast<float>(toConvert.mData[3].mData[1]);
	matrix._43 = static_cast<float>(toConvert.mData[3].mData[2]);
	matrix._44 = static_cast<float>(toConvert.mData[3].mData[3]);
}

void FBXExporter::ConvertMatrixXtoF(DirectX::XMFLOAT4X4 matrix, FbxAMatrix &toConvert)
{
	toConvert.mData[0].mData[0] = matrix._11;
	toConvert.mData[0].mData[1] = matrix._12;
	toConvert.mData[0].mData[2] = matrix._13;
	toConvert.mData[0].mData[3] = matrix._14;

	toConvert.mData[1].mData[0] = matrix._21;
	toConvert.mData[1].mData[1] = matrix._22;
	toConvert.mData[1].mData[2] = matrix._23;
	toConvert.mData[1].mData[3] = matrix._24;

	toConvert.mData[2].mData[0] = matrix._31;
	toConvert.mData[2].mData[1] = matrix._32;
	toConvert.mData[2].mData[2] = matrix._33;
	toConvert.mData[2].mData[3] = matrix._34;

	toConvert.mData[3].mData[0] = matrix._41;
	toConvert.mData[3].mData[1] = matrix._42;
	toConvert.mData[3].mData[2] = matrix._43;
	toConvert.mData[3].mData[3] = matrix._44;
}

int FBXExporter::FindVertex(const RobustVertex& inTargetVertex, const std::vector<RobustVertex>& uniqueVertices)
{
	for (unsigned int i = 0; i < uniqueVertices.size(); ++i)
	{
		if (inTargetVertex == uniqueVertices[i])
		{
			return i;
		}
	}

	return -1;
}

FbxAMatrix FBXExporter::GetGeometryTransformation(FbxNode* inNode)
{
	if (!inNode)
	{
		throw std::exception("Null for mesh geometry");
	}

	const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

void FBXExporter::CleanupFbxManager()
{
	m_FBXScene->Destroy();
	m_FBXManager->Destroy();

	m_Indices.clear();
	m_Vertices.clear();
	m_Skeleton.m_joints.clear();
}

void FBXExporter::ConvertToUML()
{

	//fill in animation
	m_animation.m_frames.resize(m_animation.m_numKeyFrames);
	for (int i = 0; i < m_animation.m_numKeyFrames; ++i)
	{
		m_animation.m_frames[i].m_numBones = static_cast<int>(m_Skeleton.m_joints.size());
		m_animation.m_frames[i].m_bones.resize(m_animation.m_frames[i].m_numBones);
		m_animation.m_frames[i].m_time = m_Skeleton.m_joints[0].m_keyframe->m_time;
		for ( int j = 0; j < m_animation.m_frames[i].m_numBones; ++j)
		{
			m_animation.m_frames[i].m_bones[j].m_boneMatrix = m_Skeleton.m_joints[j].m_keyframe->m_worldMatrix;
			m_Skeleton.m_joints[j].m_keyframe = m_Skeleton.m_joints[j].m_keyframe->m_nextFrame;
		}

	}
	//fill in bind pose
	for (unsigned int i = 0; i < m_Skeleton.m_joints.size(); ++i)
	{
		m_convertToBindPose.push_back(m_Skeleton.m_joints[i].m_globalBindposeInverse);
	}
	m_bindPose.init(static_cast<int>(m_Skeleton.m_joints.size()), m_convertToBindPose);


	



}
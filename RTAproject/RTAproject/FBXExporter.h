#pragma once
#include <map>
#include "Utilities.h"
#include "..\Content\ShaderStructures.h"
#include "Animation.h"
#include "BindPose.h"
#include <time.h>
#include <memory.h>
#include <stdint.h>
#include <sys/stat.h>
#define EXPORTER_VERSION_NUMBER 1
using namespace RTAproject;
enum class FILE_TYPES : int8_t { MESH, BIND_POSE, ANIMATION, NAV_MESH };
enum class MODEL_TYPES : int8_t { COLOR, TEXTURE, TEXTURE_LIT, NORMALMAP, NORMALMAP_ANIMATED, BASIC, MAX_TYPES };
enum class INDEX_TYPES : int8_t { INDEX32, INDEX16, TRI_STRIP };
struct ExporterHeader {
	union {
		struct {
			uint32_t		numPoints;
			size_t			numIndex;
			size_t			vertSize;
			MODEL_TYPES		modelType;
			INDEX_TYPES		index;
		}mesh;

		struct {
			uint32_t		numBones;
			uint32_t		nameSize;
		}bind;

		struct {
			uint32_t		numBones;
			uint32_t		numFrames;
			float			Totaltime;
			ANIM_TYPE		AnimType;
		}anim;

		struct {
			uint32_t		totalSize;
			uint32_t		rowSize;
			uint32_t		numTriangles;
			uint32_t		elementSize;
		}navMesh;
	};
	FILE_TYPES file;
	uint32_t version;
	time_t updateTime;
	ExporterHeader() {}

	ExporterHeader(FILE_TYPES _type, const char * _FBXFileLocation)
	{
		memset(this, 0, sizeof(ExporterHeader));
		version = EXPORTER_VERSION_NUMBER;
		file = _type;
		struct stat fileProperty;
		int retVal = stat(_FBXFileLocation, &fileProperty);
		updateTime = fileProperty.st_mtime;
		struct tm time;
		gmtime_s(&time, &(fileProperty.st_mtime));
	}
	bool check(_Out_ FILE ** file, _In_z_ const char* _Binaryfilelocation, _In_z_ const char * _FBXFileLocation)
	{
		fopen_s(file, _Binaryfilelocation, "rb");
		if (*file == nullptr) return false;
		fread(this, sizeof(ExporterHeader),1, *file);
		if (EXPORTER_VERSION_NUMBER != version)
		{
			fclose(*file);
			*file = nullptr;
			return false;
		}
		struct stat fileProperty;
		stat(_FBXFileLocation, &fileProperty);
		if (updateTime < fileProperty.st_mtime)
		{
			fclose(*file);
			*file = nullptr;
			return false;
		}

		return true;
	}
};
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
	Animation m_animation2;
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


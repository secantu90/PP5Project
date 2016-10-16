#pragma once
#include "pch.h"
#include "Utilities.h"
#include "..\Content\ShaderStructures.h"
struct fhead
{
	size_t indexsize;
	size_t vertexsize;
	size_t trianglecountsize;
	size_t jointsize;
	size_t blendsize;
};
struct binaryblend
{
	unsigned int blendingIndex;
	float blendingweight;
};
struct binaryjoint
{
	std::string m_jointName;
	int m_parentIndex;
	DirectX::XMFLOAT4X4 m_globalBindposeInverse;
	Keyframe* m_keyframe;
	FbxNode* m_node;
};
struct Binaryvertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 blendingindex;
	DirectX::XMFLOAT4 blendingweight;
};
class Filemanager
{
public:
	Filemanager();
	~Filemanager();
	bool Readfile(std::string _filename, std::vector<Joint> *_skeleton, std::vector<RobustVertex> *m_Vertices, std::vector<unsigned short> *m_Indices, unsigned int *m_TriangleCount, std::vector<BlendingIndexWeightPair> *blendinginfo);
	bool Writefile(std::string _filename);
	//Skeleton *m_skeleton;
	bool NametoBinary(std::string _filename);
	void Convert2Bin();
	bool ExtractData(std::vector<Joint> _skeleton, std::vector<RobustVertex> m_Vertices, std::vector<unsigned short> m_Indices, unsigned int m_TriangleCount, std::vector<BlendingIndexWeightPair> blendinginfo);
private:
	std::string m_Filename;
	fhead filehead;
	std::vector<unsigned char> binbuff;
	std::vector<binaryblend> blenddata;
	std::vector<binaryjoint> jointdata;
	std::vector<Binaryvertex> vertdata;
	std::vector<unsigned short> indexdata;
	unsigned int tricount;
	unsigned int vertsize;
	unsigned int jointsize;
	unsigned int trisize;
	unsigned int blendsize;
	unsigned int indexsize;
};

Filemanager::Filemanager()
{

}

Filemanager::~Filemanager()
{

}

bool Filemanager::Readfile(std::string _filename, std::vector<Joint> *_skeleton, std::vector<RobustVertex> *m_Vertices, std::vector<unsigned short> *m_Indices, unsigned int *m_TriangleCount, std::vector<BlendingIndexWeightPair> *blendinginfo)
{
	std::string path = "..//Bin//";
	path += _filename;
	FILE *file;
	fopen_s(&file, path.c_str(), "rb");
	if (!file) return false;
	fread(&filehead, sizeof(fhead), 1, file);
	binbuff.resize((sizeof(Binaryvertex)* filehead.vertexsize) + (sizeof(binaryjoint)* filehead.jointsize) + (sizeof(binaryblend)* filehead.blendsize) + (sizeof(unsigned short)* filehead.indexsize) + sizeof(unsigned int));
	_skeleton->resize(filehead.jointsize);
	m_Vertices->resize(filehead.vertexsize);
	m_Indices->resize(filehead.indexsize);
	blendinginfo->resize(filehead.blendsize);

	memcpy(m_Vertices->data(),binbuff.data(),sizeof(Binaryvertex) * filehead.vertexsize);
	memcpy(_skeleton->data(), binbuff.data() + (sizeof(Binaryvertex) * filehead.vertexsize), sizeof(binaryjoint)*  filehead.jointsize);
	memcpy(blendinginfo->data(), binbuff.data() + (sizeof(Binaryvertex) * filehead.vertexsize)+ (sizeof(binaryjoint)*  filehead.jointsize), sizeof(binaryblend)* filehead.blendsize);
	memcpy(m_Indices->data(), binbuff.data() + (sizeof(Binaryvertex) * filehead.vertexsize) + (sizeof(binaryjoint)*  filehead.jointsize) + (sizeof(binaryblend)* filehead.blendsize), sizeof(unsigned short)* filehead.indexsize);
	memcpy(m_TriangleCount, binbuff.data() + (sizeof(Binaryvertex) * filehead.vertexsize) + (sizeof(binaryjoint)*  filehead.jointsize) + (sizeof(binaryblend)* filehead.blendsize) + (sizeof(unsigned short)* indexsize), sizeof(unsigned int) *filehead.trianglecountsize);
	fclose(file);
	return true;
}

bool Filemanager::Writefile(std::string _filename)
{
	FILE* ofile;
	NametoBinary(_filename);
	fopen_s(&ofile, m_Filename.c_str(), "wb");
	fwrite(&filehead, sizeof(fhead), 1, ofile);
	fwrite(binbuff.data(), sizeof(unsigned char), binbuff.size(), ofile);
	fclose(ofile);
	return true;
}

bool Filemanager::NametoBinary(std::string _filename)
{
	m_Filename = _filename;
	m_Filename += ".bin";
}
bool Filemanager::ExtractData(std::vector<Joint> _skeleton, std::vector<RobustVertex> m_Vertices, std::vector<unsigned short> m_Indices, unsigned int m_TriangleCount, std::vector<BlendingIndexWeightPair> blendinginfo)
{
	tricount = m_TriangleCount;
	for (size_t i = 0; i < _skeleton.size(); i++)
	{
		jointdata[i].m_globalBindposeInverse = _skeleton[i].m_globalBindposeInverse;
		jointdata[i].m_jointName = _skeleton[i].m_jointName;
		jointdata[i].m_keyframe = _skeleton[i].m_keyframe;
		jointdata[i].m_node = _skeleton[i].m_node;
		jointdata[i].m_parentIndex = _skeleton[i].m_parentIndex;
	}
	for (size_t i = 0; i < m_Vertices.size(); i++)
	{
		vertdata[i].blendingindex = m_Vertices[i].blendingindex;
		vertdata[i].blendingweight = m_Vertices[i].blendingweight;
		vertdata[i].normal = m_Vertices[i].normal;
		vertdata[i].pos = m_Vertices[i].pos;
		vertdata[i].uv = m_Vertices[i].uv;
	}
	for (size_t i = 0; i < m_Indices.size(); i++)
	{
		indexdata[i] = m_Indices[i];
	}
	for (size_t i = 0; i < blendinginfo.size(); i++)
	{
		blenddata[i].blendingIndex = blendinginfo[i].mBlendingIndex;
		blenddata[i].blendingweight = blendinginfo[i].mBlendingWeight;
	}
}
void Filemanager::Convert2Bin()
{
	filehead.vertexsize = vertdata.size();
	filehead.jointsize = jointdata.size();
	filehead.blendsize = blenddata.size();
	filehead.indexsize = indexdata.size();
	filehead.trianglecountsize = 1;

	vertsize = (sizeof(Binaryvertex)* filehead.vertexsize);
	jointsize = (sizeof(binaryjoint)* filehead.jointsize);
	trisize = sizeof(unsigned int);
	blendsize = (sizeof(binaryblend)* filehead.blendsize);
	indexsize = (sizeof(unsigned short)* filehead.indexsize);

	binbuff.resize(vertsize + jointsize + blendsize + indexsize + trisize);
	memcpy(binbuff.data(), vertdata.data(), vertsize);
	memcpy(binbuff.data() + vertsize, jointdata.data(), jointsize);
	memcpy(binbuff.data() + vertsize + jointsize, blenddata.data(), blendsize);
	memcpy(binbuff.data() + vertsize + jointsize + blendsize, indexdata.data(), indexsize);
	memcpy(binbuff.data() + vertsize + jointsize + blendsize + indexsize, (unsigned int*)tricount, trisize);


}
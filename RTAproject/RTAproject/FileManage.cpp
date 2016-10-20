#include "pch.h"
#include "FileManage.h"



Filemanage::Filemanage()
{

}

Filemanage::~Filemanage()
{
	
}

bool Filemanage::WriteBindData(std::string _filename, BindPose* _bind, const char* _FBXLocation)
{
	FILE* ofile;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&ofile, full.c_str(), "wb");
	if (nullptr == ofile) return false;
	ExporterHeader header(FILE_TYPES::BIND_POSE,_FBXLocation);
	header.bind.numBones = _bind->m_numBones;
	fwrite(&header, sizeof(ExporterHeader), 1, ofile);
	for(int i = 0; i < 4; ++i)
		fwrite(&_bind->m_InvBindPose[i], sizeof(DirectX::XMFLOAT4X4), 1, ofile);
	fclose(ofile);
	return true;
}
bool Filemanage::ReadBindData(std::string _filename, BindPose& _bind)
{
	FILE* file;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&file, full.c_str(), "rb");
	ExporterHeader header;
	if (nullptr == file) return false;
	fread(&header, sizeof(ExporterHeader), 1, file);
	_bind.m_InvBindPose.resize(header.bind.numBones);
	for (int i = 0; i < 4; ++i)
		fread(&_bind.m_InvBindPose[i], sizeof(DirectX::XMFLOAT4X4), 1, file);
	_bind.m_numBones = header.bind.numBones;
	fclose(file);
	return true;
}
bool Filemanage::WriteVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices, const char* _FBXLocation)
{
	FILE* ofile;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&ofile, full.c_str(), "wb");
	if (nullptr == ofile) return false;
	ExporterHeader header(FILE_TYPES::MESH, _FBXLocation);
	header.mesh.numIndex = _indices.size();
	header.mesh.vertSize = _Vertices.size();
	fwrite(&header, sizeof(ExporterHeader), 1, ofile);
	fwrite(&_Vertices[0], sizeof(RTAproject::RobustVertex), _Vertices.size(), ofile);
	fwrite(&_indices[0], sizeof(unsigned short), _indices.size(), ofile);
	fclose(ofile);
	return true;
}
bool Filemanage::ReadVertexData(std::string _filename, std::vector<RTAproject::RobustVertex>& _Vertices, std::vector<unsigned short>& _indices)
{
	FILE* file;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&file, full.c_str(), "rb");
	if (nullptr == file) return false;
	ExporterHeader header;
	fread(&header, sizeof(ExporterHeader), 1, file);
	_Vertices.resize(header.mesh.vertSize);
	_indices.resize(header.mesh.numIndex);
	fread(&_Vertices[0], sizeof(RTAproject::RobustVertex), _Vertices.size(), file);
	fread(&_indices[0], sizeof(unsigned int), _indices.size(), file);
	fclose(file);
	return true;
}
void Filemanage::NametoBinary(std::string _filename)
{
	m_Filename.clear();
	m_Filename += _filename;
	m_Filename += ".bin";
}
bool Filemanage::ReadAnimation(std::string _filename, Animation& _anim)
{
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string path(ws.begin(), ws.end());
	path += "\\";
	path += _filename;
	FILE *file;
	fopen_s(&file, path.c_str(), "rb");
	//read header
	if (nullptr == file) return false;
	ExporterHeader header;
	fread(&header, sizeof(ExporterHeader), 1, file);
	//set up
	_anim.m_frames.resize(header.anim.numFrames);
	_anim.m_animType = header.anim.AnimType;
	_anim.m_totalTime = header.anim.Totaltime;
	_anim.m_numKeyFrames = header.anim.numFrames;
	for (size_t i = 0; i <  header.anim.numFrames; i++)
	{
		_anim.m_frames[i].m_bones.resize(header.anim.numBones);
		_anim.m_frames[i].m_numBones = header.anim.numBones;
	}
	//read data
	for (int i = 0; i < _anim.m_frames.size(); i++)
	{
		fread(&_anim.m_frames[i].m_time, sizeof(double), 1, file);
		fread(&_anim.m_frames[i].m_bones[0], sizeof(Boint), _anim.m_frames[i].m_bones.size(), file);
	}
	fclose(file);
	return true;
}
bool Filemanage::WriteAnimfile(std::string _filename, Animation* _anim, const char* _FBXLocation)
{
	FILE* ofile = nullptr;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&ofile, full.c_str(), "wb");
	if (nullptr == ofile) return false;
	ExporterHeader header(FILE_TYPES::ANIMATION, _FBXLocation);
	header.anim.numBones = _anim->m_frames[0].m_numBones;
	header.anim.numFrames = _anim->m_numKeyFrames;
	header.anim.Totaltime = _anim->m_totalTime;
	header.anim.AnimType = _anim->m_animType;
	fwrite(&header, sizeof(ExporterHeader), 1, ofile);
	for (int i = 0; i < _anim->m_frames.size(); i++)
	{
		fwrite(&_anim->m_frames[i].m_time, sizeof(double), 1, ofile);
		fwrite(&_anim->m_frames[i].m_bones[0], sizeof(Boint),_anim->m_frames[i].m_bones.size(), ofile);
	}
	fclose(ofile);
	return true;
}

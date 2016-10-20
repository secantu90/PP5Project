#include "pch.h"
#include "FileManage.h"
#include <fstream>
#include <iostream>


Filemanage::Filemanage()
{

}

Filemanage::~Filemanage()
{

}
bool Filemanage::WriteBindData(std::string _filename, BindPose* _bind)
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
	bindhead.num_bones = _bind->m_numBones;
	fwrite(&bindhead, sizeof(Bindfhead), 1, ofile);
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
	bindhead.num_bones = 0;
	if (nullptr == file) return false;
	fread(&bindhead, sizeof(Bindfhead), 1, file);
	_bind.m_InvBindPose.resize(bindhead.num_bones);
	for (int i = 0; i < 4; ++i)
		fread(&_bind.m_InvBindPose[i], sizeof(DirectX::XMFLOAT4X4), 1, file);
	_bind.m_numBones = bindhead.num_bones;
	fclose(file);
	return true;
}
bool Filemanage::WriteVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices)
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
	verthead.indexsize = static_cast<int>(_indices.size());
	verthead.vertsize = static_cast<int>(_Vertices.size());
	fwrite(&verthead, sizeof(Vertfhead), 1, ofile);
	fwrite(&_Vertices[0], sizeof(RTAproject::RobustVertex), _Vertices.size(), ofile);
	fwrite(&_indices[0], sizeof(unsigned short), _indices.size(), ofile);
	fclose(ofile);
	return true;
}
bool Filemanage::ReadVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices)
{
	FILE* file;
	NametoBinary(_filename);
	auto folder = Windows::Storage::ApplicationData::Current->RoamingFolder;
	std::wstring ws(folder->Path->Data());
	std::string full(ws.begin(), ws.end());
	full += "\\";
	full += _filename;
	fopen_s(&file, full.c_str(), "rb");
	verthead.vertsize = 0;
	verthead.indexsize = 0;
	if (nullptr == file) return false;
	fread(&verthead, sizeof(Vertfhead), 1, file);
	_Vertices.resize(verthead.vertsize);
	_indices.resize(verthead.indexsize);
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
void Filemanage::ProcessAnimation(Animation* _anim)
{
	animhead.m_animtype = _anim->m_animType;
	animhead.m_numkframes = _anim->m_numKeyFrames;
	animhead.m_TotalTime = _anim->m_totalTime;
	animhead.animsize = _anim->m_frames[0].m_numBones;
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
	animhead.animsize = 0;
	animhead.m_animtype = 0;
	animhead.m_numkframes = 0;
	animhead.m_TotalTime = 0;
	if (nullptr == file) return false;
	fread(&animhead, sizeof(Afhead), 1, file);
	//set up
	_anim.m_frames.resize(animhead.m_numkframes);
	_anim.m_animType = (ANIM_TYPE)animhead.m_animtype;
	_anim.m_totalTime = animhead.m_TotalTime;
	_anim.m_numKeyFrames = animhead.m_numkframes;
	for (size_t i = 0; i < animhead.m_numkframes; i++)
	{
		_anim.m_frames[i].m_bones.resize(animhead.animsize);
		_anim.m_frames[i].m_numBones = animhead.animsize;
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
bool Filemanage::WriteAnimfile(std::string _filename, Animation* _anim)
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
	fwrite(&animhead, sizeof(Afhead), 1, ofile);
	for (int i = 0; i < _anim->m_frames.size(); i++)
	{
		fwrite(&_anim->m_frames[i].m_time, sizeof(double), 1, ofile);
		fwrite(&_anim->m_frames[i].m_bones[0], sizeof(Boint),_anim->m_frames[i].m_bones.size(), ofile);
	}
	fclose(ofile);
	return true;
}

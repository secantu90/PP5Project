#pragma once
#include "Utilities.h"
#include "..\Content\ShaderStructures.h"
#include "Animation.h"
#include "BindPose.h"
#include <fstream>
#include <iostream>
#include "FBXExporter.h"

class Filemanage
{
public:
	Filemanage();
	~Filemanage();
	
	bool WriteAnimfile(std::string _filename, Animation* _anim, const char* _FBXLocation);
	bool WriteBindData(std::string _filename, BindPose* _bind, const char* _FBXLocation);
	bool WriteVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices, const char* _FBXLocation);
	bool ReadBindData(std::string _filename, BindPose& _bind);
	bool ReadVertexData(std::string _filename, std::vector<RTAproject::RobustVertex>& _Vertices, std::vector<unsigned short>& _indices);
	void NametoBinary(std::string _filename);
	bool ReadAnimation(std::string _filename, Animation& _anim);
	std::string m_Filename;
	ExporterHeader header;
	
private:
};



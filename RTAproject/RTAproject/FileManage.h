#pragma once
#include "Utilities.h"
#include "..\Content\ShaderStructures.h"
#include "Animation.h"
#include "BindPose.h"
struct Bindfhead
{
	int num_bones;
};
struct Vertfhead
{
	int vertsize;
	int indexsize;
};
struct Afhead
{
	unsigned int animsize;
	int m_animtype;
	int m_numkframes;
	float m_TotalTime;
};
class Filemanage
{
public:
	Filemanage();
	~Filemanage();
	bool WriteAnimfile(std::string _filename, Animation* _anim);
	bool WriteBindData(std::string _filename, BindPose* _bind);
	bool WriteVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices);
	bool ReadBindData(std::string _filename, BindPose& _bind);
	bool ReadVertexData(std::string _filename, std::vector<RTAproject::RobustVertex> _Vertices, std::vector<unsigned short> _indices);
	void NametoBinary(std::string _filename);
	void ProcessAnimation(Animation* _anim);
	bool ReadAnimation(std::string _filename, Animation& _anim);
	std::string m_Filename;
	Bindfhead bindhead;
	Afhead animhead;
	Vertfhead verthead;
private:
};



#pragma once
#include "pch.h"
#include "Content\ShaderStructures.h"
#include "Common\DirectXHelper.h"
#include <vector>
using namespace std;
using namespace RTAproject;
class Model
{
private:

	void LoadObject(char* fileName, wchar_t* textureName, ID3D11Device* device, DirectX::XMFLOAT3 offset);

public:
	Model();
	Model(char* filename, wchar_t* textureName, ID3D11Device* device, DirectX::XMFLOAT3 offset);
	struct OBJECT
	{
		DirectX::XMFLOAT3 vertices;
		DirectX::XMFLOAT2 uvs;
		DirectX::XMFLOAT3 norms;
	};
	DirectX::XMMATRIX WM;
	std::vector<unsigned int> indecies;
	std::vector<OBJECT> objects;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	VB;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	IB;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> resourceView;

	DirectX::XMFLOAT3 offset;
	
	void SetBuffers(ID3D11DeviceContext* context);
	void Draw(ID3D11DeviceContext* context);
	std::vector<VERTEX3>						interleaved_vert;
	std::vector<unsigned int>					interleaved_index;

};


#include "pch.h"
#include "Model.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

Model::Model()
{

}

Model::Model(char* filename, wchar_t* textureName, ID3D11Device* device, DirectX::XMFLOAT3 offset)
{
	LoadObject(filename, textureName, device, offset);
}

void Model::Draw(ID3D11DeviceContext* context)
{
	context->DrawIndexed(
<<<<<<< HEAD
		static_cast<UINT>(interleaved_index.size()),
=======
		interleaved_index.size(),
>>>>>>> 3dd00589c8e0f44917eb17046cd962b3c24b4487
		0,
		0
		);
}

void Model::SetBuffers(ID3D11DeviceContext* context)
{
	UINT stride = sizeof(VERTEX3);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		VB.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		IB.Get(),
		DXGI_FORMAT_R32_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);
}

void Model::LoadObject(char* fileName, wchar_t* textureName, ID3D11Device* device, DirectX::XMFLOAT3 offset)
{
	std::vector<XMFLOAT3> tempVerts;
	std::vector<XMFLOAT2> tempUV;
	std::vector<XMFLOAT3> tempNorms;

	std::vector<unsigned int> vertIndices;
	std::vector<unsigned int> uvIndices;
	std::vector<unsigned int> normalIndices;


	FILE * file;
	fopen_s(&file, fileName, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			XMFLOAT3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertex.x = -1 * vertex.x;
			tempVerts.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			XMFLOAT2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			tempUV.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.x = -1 * normal.x;
			tempNorms.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return;
			}
			vertIndices.push_back(vertexIndex[0]);
			vertIndices.push_back(vertexIndex[1]);
			vertIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertIndices.size(); i++)
	{
		VERTEX3 temp;
		unsigned int vertexIndex = vertIndices[i];
		XMFLOAT3 vertex = tempVerts[vertexIndex - 1];
		temp.pos = vertex;
		unsigned int uvIndex = uvIndices[i];
		XMFLOAT2 uv = tempUV[uvIndex - 1];
		temp.uv = uv;
		unsigned int normIndex = normalIndices[i];
		XMFLOAT3 norm = tempNorms[normIndex - 1];
		temp.norm = norm;
		interleaved_vert.push_back(temp);
		interleaved_index.push_back(i);
	}

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = interleaved_vert.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
<<<<<<< HEAD
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX3) * static_cast<UINT>(interleaved_vert.size()), D3D11_BIND_VERTEX_BUFFER);
=======
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX3) * interleaved_vert.size(), D3D11_BIND_VERTEX_BUFFER);
>>>>>>> 3dd00589c8e0f44917eb17046cd962b3c24b4487
	DX::ThrowIfFailed(
		device->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			VB.GetAddressOf()
			)
		);
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = interleaved_index.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
<<<<<<< HEAD
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * static_cast<UINT>(interleaved_index.size()), D3D11_BIND_INDEX_BUFFER);
=======
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * interleaved_index.size(), D3D11_BIND_INDEX_BUFFER);
>>>>>>> 3dd00589c8e0f44917eb17046cd962b3c24b4487
	DX::ThrowIfFailed(
		device->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			IB.GetAddressOf()
			)
		);
	//sets texture2D and Sample Shader
	HRESULT h = CreateDDSTextureFromFile(device, textureName, NULL, &resourceView);


}
#pragma once

namespace RTAproject
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};
	//Geometry Shader constant buffer
	struct ViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	//Geometry vertex shader constant buffer
	struct ModelConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
	};
	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};
	struct GeoVertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};


	////////////////////////////////////////////////
	//Emilio

	struct VERTEX3
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 norm;
	};


	//End Emilio
	////////////////////////////////////////////////


	struct LightBuffer
	{
		float						Ar, Ag, Ab, Aa;
		float						r, g, b, a;
		float						x, y, z, w;
		float						sX, sY, sZ, sW;
		float						cX, cY, cZ, cW;
	};

	struct PLightPosBuffer
	{
		float						x, y, z, w;
		float						sX, sY, sZ, sW;
	};


	struct BoneOffsets
	{
		DirectX::XMFLOAT4X4 offsets[32];
	};

	struct RobustVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT4 blendingIndex;
		DirectX::XMFLOAT4 blendingWeight;

		bool operator==(const RobustVertex& rhs) const
		{
			if (position.x == rhs.position.x &&
				position.y == rhs.position.y &&
				position.z == rhs.position.z)
			{
				if (uv.x == rhs.uv.x &&
					uv.y == rhs.uv.y)
				{
					if (normal.x == rhs.normal.x &&
						normal.y == rhs.normal.y &&
						normal.z == rhs.normal.z)
					{
						return true;
					}
				}
			}

			return false;
		}
	};
}
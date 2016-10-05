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

	struct BoneOffsets
	{
		DirectX::XMFLOAT4X4 offsets[4];
	};

	struct RobustVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
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
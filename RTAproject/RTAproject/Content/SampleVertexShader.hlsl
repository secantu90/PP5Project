// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	//matrix world;
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 tex : UV;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : UV;
	float4 norm : NORMALS;
	float3 sLightPos : TEXCOORD1;
	float4 worldPosition : TEXCOORD2;
	float1 ImageRef : TEXCOORD3;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	output.worldPosition = pos;

	float4 worldPosition;
	worldPosition = mul(pos, model);
	output.sLightPos.x = worldPosition.x;
	output.sLightPos.y = worldPosition.y;
	output.sLightPos.z = worldPosition.z;

	float4 norm;
	norm.x = 0;
	norm.y = 1;
	norm.z = 0;
	norm.w = 1;

	output.norm = mul(norm, model);
	output.norm = normalize(output.norm);

	// Transform the vertex position into projected space.
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	//output.color = input.color;
	output.tex = input.tex;

	output.ImageRef = 0;

	return output;
}

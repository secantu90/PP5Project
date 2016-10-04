// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix world;
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
	float3 norm : NORMALS;
	float3 lightPos : TEXCOORD0;
	float3 sLightPos : TEXCOORD1;
	float4 worldPosition : TEXCOORD2;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	output.worldPosition = pos;

	float4 worldPosition;
	worldPosition = mul(pos, world);
	output.sLightPos = worldPosition;

	float3 norm;
	norm.x = 0;
	norm.y = 1;
	norm.z = 0;

	output.norm = mul(norm, world);
	output.norm = normalize(output.norm);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	//output.color = input.color;
	output.tex = input.tex;

	return output;
}

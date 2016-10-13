// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelConstantBuffer : register(b0)
{
	matrix model;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
};

// Per-pixel color data passed through the pixel shader.
struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.pos = pos;

	// Pass the color through without modification.
	output.color = input.color;

	return output;
}
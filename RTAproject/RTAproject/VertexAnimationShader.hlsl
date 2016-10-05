// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer BoneOffsets : register(b1)
{
	float4x4 offsets[4];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCORD;
	float3 normal : NORMAL;
	float4 blendingIndicies : BINDICIES;
	float4 blendingWeights : BWEIGHTS;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);


	output.pos = mul(pos, offsets[input.blendingIndicies.x] * input.blendingWeights.x);
	output.pos += mul(output.pos, offsets[input.blendingIndicies.y] * input.blendingWeights.y);
	output.pos += mul(output.pos, offsets[input.blendingIndicies.z] * input.blendingWeights.z);
	output.pos += mul(output.pos, offsets[input.blendingIndicies.w] * input.blendingWeights.w);


	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.color = output.pos.xyz;

	return output;
}

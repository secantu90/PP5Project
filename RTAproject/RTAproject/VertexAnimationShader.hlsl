// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer BoneOffsets : register(b1)
{
	float4x4 offsets[32];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 tex : UV;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float4 blendingIndicies : BINDICIES;
	float4 blendingWeights : BWEIGHTS;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : UV;
	float4 norm : NORMALS;
	float4 tangent : TANGENT;
	float4 bitangent : BITANGENT;
	float3 sLightPos : TEXCOORD1;
	float4 worldPosition : TEXCOORD2;
	float1 ImageRef : TEXCOORD3;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 norm = float4(input.normal, 0.0f);
	float4 tan = float4(input.tangent, 0.0f);

	output.worldPosition = pos;

	float4 worldPosition;
	worldPosition = mul(pos, model);
	output.sLightPos.x = worldPosition.x;
	output.sLightPos.y = worldPosition.y;
	output.sLightPos.z = worldPosition.z;


	output.pos = mul(pos, offsets[input.blendingIndicies.x] * input.blendingWeights.x);
	output.pos += mul(pos, offsets[input.blendingIndicies.y] * input.blendingWeights.y);
	output.pos += mul(pos, offsets[input.blendingIndicies.z] * input.blendingWeights.z);
	output.pos += mul(pos, offsets[input.blendingIndicies.w] * input.blendingWeights.w);

	output.norm = mul(norm, offsets[input.blendingIndicies.x] * input.blendingWeights.x);
	output.norm += mul(norm, offsets[input.blendingIndicies.y] * input.blendingWeights.y);
	output.norm += mul(norm, offsets[input.blendingIndicies.z] * input.blendingWeights.z);
	output.norm += mul(norm, offsets[input.blendingIndicies.w] * input.blendingWeights.w);
	output.norm = mul(output.norm, model);
	output.norm = normalize(output.norm);

	output.tangent = mul(tan, offsets[input.blendingIndicies.x] * input.blendingWeights.x);
	output.tangent += mul(tan, offsets[input.blendingIndicies.y] * input.blendingWeights.y);
	output.tangent += mul(tan, offsets[input.blendingIndicies.z] * input.blendingWeights.z);
	output.tangent += mul(tan, offsets[input.blendingIndicies.w] * input.blendingWeights.w);
	output.tangent = mul(output.tangent, model);
	output.tangent = normalize(output.tangent);

	output.bitangent = mul(float4(cross(output.norm.xyz, output.tangent.xyz), 0.0f), model);


	// Transform the vertex position into projected space.
	output.pos = mul(output.pos, model);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	//output.color = output.pos.xyz;
	output.tex = input.tex;

	output.ImageRef = 1;

	return output;
}

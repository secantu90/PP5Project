
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uvw : UVW0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	//return position and uvw
	output.pos = pos;
	output.uvw = input.pos;

	return output;
}

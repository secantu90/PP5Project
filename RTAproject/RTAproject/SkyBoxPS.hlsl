TextureCube tex : register(t0);
SamplerState filter : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uvw : UVW0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 tex_color = tex.Sample(filter, input.uvw);
	return tex_color;
}
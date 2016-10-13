cbuffer ViewProjectionConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
}
struct GeometryShaderOutput
{
	float4 posH : SV_POSITION;
	float3 color : COLOR;
};
struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};
[maxvertexcount(4)] // max vertex data to be output (limit 1024 total scalars)
void main(point  GeometryShaderInput input[1], inout TriangleStream< GeometryShaderOutput > output)
{
	GeometryShaderOutput v[4];
	v[0].posH = float4(input[0].pos + float3(1.0f, 0.0f, 1.0f), 1.0f);
	v[1].posH = float4(input[0].pos + float3(-1.0f, 0.0f, 1.0f), 1.0f);
	v[2].posH = float4(input[0].pos + float3(1.0f, 0.0f, -1.0f), 1.0f);
	v[3].posH = float4(input[0].pos + float3(-1.0f, 0.0f, -1.0f), 1.0f);

	GeometryShaderOutput gout;
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		float4 p = mul(v[i].posH, View);
		p = mul(p, Projection);
		gout.posH = p;
		gout.color = input[0].color;
		output.Append(gout);
	}
}
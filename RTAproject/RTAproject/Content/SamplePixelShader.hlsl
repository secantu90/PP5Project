// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : UV;
	float4 norm : NORMALS;
	float3 sLightPos : TEXCOORD1;
	float4 worldPosition : TEXCOORD2;
};

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 lightDirection;
	float4 sLightDirection;
	float4 cDirection;
};

cbuffer LightPositionBuffer : register(b1)
{
	float4 PL_Position;
	float4 SL_Position;
}

texture2D baseTexture : register(t0);

SamplerState filter : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//Directional light
	float4 lightDir;
	float lightIntensity;
	float4 color;

	color = ambientColor;

	lightDir = -lightDirection;
	lightIntensity = saturate(dot(lightDir, input.norm));
	if (lightIntensity > 0.0f)
	{
		color += (diffuseColor * lightIntensity);
		color = saturate(color);
	}

	float4 finalColor = baseTexture.Sample(filter, input.tex);

	//float4 finalColor = 0;
	//finalColor.x = 1;
	//finalColor.y = 0;
	//finalColor.z = 0;

	//Point light
	float4 pLColor;
	float lightIntensity3;
	float4 color2;

	float4 slightPos = float4(input.sLightPos, 1);
	float4 Position = PL_Position - slightPos;
	Position = normalize(Position);

	pLColor.x = 0;
	pLColor.y = 1;
	pLColor.z = 0;
	pLColor.w = 1;
	lightIntensity3 = saturate(dot(input.norm, Position));
	color2 = pLColor * lightIntensity3;

	//Spot light
	float surfaceRatio;
	float spotFactor;
	float lightIntensity2;
	float4 lightDir2;
	float result;

	lightDir2 = normalize(SL_Position - slightPos);
	surfaceRatio = saturate(dot(-lightDir2, sLightDirection));
	if (surfaceRatio > .8f)
		spotFactor = 1;
	else
		spotFactor = 0;
	lightIntensity2 = saturate(dot(lightDir2, input.norm));
	result = (spotFactor * lightIntensity2 * diffuseColor.x) + (spotFactor * lightIntensity2 * diffuseColor.y) + (spotFactor * lightIntensity2 * diffuseColor.z) + (spotFactor * lightIntensity2 * diffuseColor.w);

	float attenuation = 1 - saturate((1 - surfaceRatio) / (1 - .8f));

	result = result * attenuation * attenuation;

	//Specular light
	float4 reflection;
	float4 specular;

	float4 direction = normalize(cDirection - input.worldPosition);
	reflection = normalize(lightDir + direction);
	specular = pow(saturate(dot(input.norm, normalize(reflection))), 32);
	float4 result2 = diffuseColor * .7f * specular;	

	color = (color + result + color2) * finalColor;

	color = saturate(color + result2);


	return color;
}

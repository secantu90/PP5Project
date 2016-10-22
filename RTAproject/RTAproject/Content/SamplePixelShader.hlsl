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

texture2D baseTexture[4] : register(t0);

SamplerState filter : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//Specular Mapping
	float4 specularSample = float4(0.0f,0.0f,0.0f,0.0f);

	//Normal Mapping
	float4 normalMapColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 finalNorm = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 finalColor1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (input.ImageRef == 0)
	{
		//Ground
		finalColor1 = baseTexture[0].Sample(filter, input.tex);
		finalNorm = input.norm;
	}
	else
	{
		//Wizard
		finalColor1 = baseTexture[1].Sample(filter, input.tex);
		normalMapColor = baseTexture[2].Sample(filter, input.tex);

		specularSample = baseTexture[3].Sample(filter, input.tex);

		normalMapColor.xyz = (normalMapColor.xyz * 2.0f) - float3(1.0f, 1.0f, 1.0f);

		float3x3 TBN;
		TBN[0] = input.tangent.xyz;
		TBN[1] = input.bitangent.xyz;
		TBN[2] = input.norm.xyz;

		float3 newNormal = normalize(normalMapColor.xyz);
		newNormal = mul(newNormal, TBN);

		float4 tempNorm = float4(newNormal, 1.0f);
		finalNorm = tempNorm;

	}

	//Directional light
	float4 lightDir;
	float lightIntensity;
	float4 color;

	color = ambientColor;

	lightDir = -lightDirection;
	lightIntensity = saturate(dot(lightDir, finalNorm));
	if (lightIntensity > 0.0f)
	{
		color += (diffuseColor * lightIntensity);
		color = saturate(color);
	}

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
	lightIntensity3 = saturate(dot(finalNorm, Position));
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
	lightIntensity2 = saturate(dot(lightDir2, finalNorm));
	result = (spotFactor * lightIntensity2 * diffuseColor.x) + (spotFactor * lightIntensity2 * diffuseColor.y) + (spotFactor * lightIntensity2 * diffuseColor.z) + (spotFactor * lightIntensity2 * diffuseColor.w);

	float attenuation = 1 - saturate((1 - surfaceRatio) / (1 - .8f));

	result = result * attenuation * attenuation;

	//Specular light
	float4 reflection;
	float4 specular;

	float4 direction = normalize(cDirection - input.worldPosition);
	reflection = normalize(lightDir + direction);
	specular = pow(saturate(dot(finalNorm, normalize(reflection))), 32);
	float4 result2 = diffuseColor * .7f * specular;	
	result2 *= specularSample;

	color = (color + result + color2) * finalColor1;
	color = saturate(color + result2);


	return color;
}

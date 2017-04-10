//--------------------------------------------------------------------------------------
// File: lecture 8.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D txDepth : register(t1);
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
matrix World;
matrix View;
matrix Projection;
float4 info;
float4 CameraPos;
};



//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
	float3 Norm : NORMAL0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
	float3 WorldPos : TEXCOORD1;
	float4 Norm : Normal0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_INPUT VS(VS_INPUT input)
	{
	PS_INPUT output = (PS_INPUT)0;
	float4 pos = input.Pos;
	
	pos = mul(pos, World);
	output.Pos = mul(pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = input.Tex;
	//lighing:
	//also turn the light normals in case of a rotation:
	output.Norm = normalize( mul(input.Norm, World));
	output.WorldPos = pos;


	return output;
	}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
float4 texture_color = txDiffuse.Sample(samLinear, input.Tex);
float4 color = texture_color;
// Phong relfection is ambient + light-diffuse + spec highlights.
// I = Ia*ka*Oda + fatt*Ip[kd*Od(N.L) + ks(R.V)^n]
// Ref: http://www.whisqu.se/per/docs/graphics8.htm
// and http://en.wikipedia.org/wiki/Phong_shading
// Get light direction for this fragment
float3 LightPosition = float3(1000, 1000, 1000);
float3 lightDir = normalize(input.WorldPos - LightPosition);

// Note: Non-uniform scaling not supported
float diffuseLighting = saturate(dot(input.Norm, -lightDir)); // per pixel diffuse lighting
float LightDistanceSquared = 3000000;
															// Introduce fall-off of light intensity
diffuseLighting *= (LightDistanceSquared / dot(LightPosition - input.WorldPos, LightPosition - input.WorldPos));



// Using Blinn half angle modification for perofrmance over correctness
float3 h = normalize(normalize(-CameraPos.xyz - input.WorldPos) - lightDir);
float SpecularPower = 15;
float specLighting = pow(saturate(dot(h, input.Norm)), SpecularPower);
float3 AmbientLightColor = float3(1, 1, 1)*0.01;
float3 SpecularColor = float3(1, 1, 1);
	color = (saturate(
	//AmbientLightColor +
	(texture_color *  diffuseLighting * 0.6) + // Use light diffuse vector as intensity multiplier
	(SpecularColor * specLighting * 0.5) // Use light specular vector as intensity multiplier
	), 1);
	//color.rgb = diffuseLighting;
	color.rgb = texture_color * diffuseLighting + specLighting;
return color;
}


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

float3 LightPosition = float3(0, 30, 10);
float3 lightDir = normalize(input.WorldPos - LightPosition);

// Note: Non-uniform scaling not supported
float diffuseLighting = saturate(dot(input.Norm, -lightDir)); // per pixel diffuse lighting



// Using Blinn half angle modification for perofrmance over correctness
float3 h = normalize(normalize(-CameraPos.xyz - input.WorldPos) - lightDir);
float SpecularPower = 15;
float specLighting = pow(saturate(dot(h, input.Norm)), SpecularPower);

color.rgb = texture_color * diffuseLighting + specLighting;

return color;
}

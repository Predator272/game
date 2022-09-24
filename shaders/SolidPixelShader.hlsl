struct VS_STRUCT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

struct PS_STRUCT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};



SamplerState Sampler : register(s0);
Texture2D Texture : register(t0);



float4 main(PS_STRUCT Input) : SV_TARGET
{
	return Texture.Sample(Sampler, Input.TexCoord);
}

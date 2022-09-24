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
	const float4 TextureColor = Texture.Sample(Sampler, Input.TexCoord);

	const float3 AmbientColor = TextureColor.rgb * 1.0f;
	const float3 DiffuseColor = TextureColor.rgb * 0.6f * dot(Input.Normal, float3(0.0f, 1.0f, 1.0f));

	return float4(AmbientColor + DiffuseColor, TextureColor.a);
}

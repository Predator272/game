struct VS_STRUCT
{
	unsigned int PositionIndex : POSITION;
	unsigned int TexCoordIndex : TEXCOORD;
	unsigned int NormalIndex : NORMAL;
};

struct PS_STRUCT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};



cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

StructuredBuffer<float3> PositionBuffer;
StructuredBuffer<float2> TexCoordBuffer;
StructuredBuffer<float3> NormalBuffer;

PS_STRUCT main(VS_STRUCT Input)
{
	const float3 Position = PositionBuffer[Input.PositionIndex];
	const float2 TexCoord = TexCoordBuffer[Input.TexCoordIndex];
	const float3 Normal = NormalBuffer[Input.NormalIndex];

	PS_STRUCT Output;
	Output.Position = mul(World, float4(Position, 1.0f));
	Output.Position = mul(View, Output.Position);
	Output.Position = mul(Projection, Output.Position);
	Output.TexCoord = TexCoord;
	Output.Normal = normalize(mul(World, Normal));
	return Output;
}

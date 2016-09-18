
struct VS_IN
{
	float3 Pos : POSITION;
	float2 tCoord : TEXCOORD;
	float3 Norm : NORMAL;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 tCoord : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float3 wPos : POSITION;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	float4x4 worldMatrix;
}



VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;
	output.Pos = mul(float4(input.Pos, 1), worldMatrix);
	output.tCoord = input.tCoord;
	output.Norm = mul(input.Norm, worldMatrix);
	output.wPos = mul(float4(input.Pos, 1), worldMatrix);


	return output;
}


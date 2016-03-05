cbuffer LightMatrix : register(b0)
{
	float4x4 projViewMatrix;
};
cbuffer Light2Matrix : register(b1)
{
	float4x4 worldMatrix;
};

struct VS_IN
{
	float3 Pos : POSITION;
	float2 tCoord : TEXCOORD;
	float3 Norm : NORMAL;
};

float4 VS_main(VS_IN input) : SV_POSITION
{
	return mul(mul(float4(input.Pos, 1.0f), worldMatrix), projViewMatrix);
}
struct VS_IN
{
	float3 Pos : POSITION;
};

cbuffer projViewBuffer : register(b0)
{
	float4x4 worldMatrix;
};

cbuffer projViewBuffer : register(b1)
{
	float4x4 projViewMatrix;
};

float4 VS_main(VS_IN input) : SV_POSITION
{
	return mul(mul(float4(input.Pos, 1.0f), worldMatrix), projViewMatrix);
}
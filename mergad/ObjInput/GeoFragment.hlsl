Texture2D tex : register(t0);
SamplerState sampAni;

struct PS_IN
{
	float4 Pos : SV_POSITION;
	float2 tCoord :TEXCOORD0;
	float3 Norm :TEXCOORD1;
	float3 wPos : POSITION;
};

struct PS_OUT
{
	float4 Normal : SV_Target0;
	float4 Position : SV_Target1;
	float4 Diffuse : SV_Target2;
	float4 Specular : SV_Target3;
};


cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4 Ka;
	float4 Kd;
	float4 Ks;
}


PS_OUT PS_main(PS_IN input)
{
	PS_OUT output;

	float3 diffuse = tex.Sample(sampAni, input.tCoord);
	float3 normalW = normalize(input.Norm);

	output.Normal = float4(normalW, 1.0f);
	output.Position = float4(input.wPos, 1.0f);
	output.Specular = Ks;
	output.Diffuse = float4(diffuse, 1.0f)* Kd;

	return output;
}
Texture2D tex1 : register(t0);
Texture2D tex2 : register(t1);
SamplerState sampAni;

struct PS_IN
{
	float4 Pos :SV_POSITION;
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

//struct Light
//{
//	float3 pos;
//	float pad;
//	float4 ambient;
//	float4 diffuse;
//};

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	//Light light;
	float4 Ka;
	float4 Kd;
	float4 Ks;
	//float3 cameraPos;
}


PS_OUT PS_main(PS_IN input)
{
	PS_OUT output;

	float3 diffuse = saturate((tex1.Sample(sampAni, input.tCoord)*tex2.Sample(sampAni, input.tCoord))*2.0); //saturate clamps the value between 0 and 1, 2.0 is the gamma value for all computers. 
	float3 normalW = normalize(input.Norm);

	output.Normal = float4(normalW, 1.0f);
	output.Position = float4(input.wPos, 1.0f);
	output.Specular = Ks;
	output.Diffuse = float4(diffuse, 1.0f);
//	output.Diffuse = float4(0.1, 0.1, 0.1, 1);
	return output;
}
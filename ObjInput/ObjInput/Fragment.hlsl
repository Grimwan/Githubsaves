Texture2D tex : register(t0);
SamplerState sampAni;

struct PS_IN
{
	float4 Pos :SV_POSITION;
	float2 tCoord :TEXCOORD0;
	float3 Norm :TEXCOORD1;
	float3 vPos : POSITION;
};

struct Light
{
	float3 dir;
	float pad;
	float4 ambient;
	float4 diffuse;
};

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	Light light;
	float4 Ka;
	float4 Kd;
	float4 Ks;
	float Ns;
	float3 cameraPos;
}


float4 PS_main(PS_IN input) : SV_Target
{

	float4 texDiffuse = tex.Sample(sampAni, input.tCoord);
	float3 s = normalize(light.dir - input.vPos);
	float3 n = normalize(input.Norm);

	float3 ambientLight = light.ambient;
	float3 diffuseLight = light.diffuse * saturate(dot(s, n)) * Kd;
	//Specular
	float3 v = normalize(cameraPos-input.vPos);
	float3 r = reflect(-s, n);
	float3 specularLight = Ks * pow(saturate(dot(r, v)), Ns);

	float3 result = (ambientLight + diffuseLight + specularLight)*texDiffuse;

	return float4(result, texDiffuse.a);


	//return (1,1,1,1);
}
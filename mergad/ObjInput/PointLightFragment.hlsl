Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D specularTexture : register(t3);
Texture2D ShadowMap : register(t4);



struct DirLight
{
	float3 dir;
	float pad;
	float4 ambient;
	float4 diffuse;
};

struct PointLight
{
	float3 pos;
	float range;
	float4 ambient;
	float4 diffuse;
};


cbuffer LightParam : register(b1)
{
	DirLight dirLight;
	PointLight pointLight;
};

cbuffer CameraParam : register(b0)
{
	float3 cameraPos;
	float pad;
};

cbuffer LightMatrix : register(b2)
{
	float4x4 projViewMatrix;
};

void GetGBuffer(in float2 screenPos, out float3 normal, out float3 position, out float3 diffuse, out float3 specular, out float specularPower)
{
	int3 sampleIndices = int3(screenPos.xy, 0);

	normal = normalTexture.Load(sampleIndices).xyz;
	position = positionTexture.Load(sampleIndices).xyz;
	diffuse = diffuseTexture.Load(sampleIndices).xyz;
	float4 spec = specularTexture.Load(sampleIndices).xyzw;

	specular = spec.xyz;
	specularPower = spec.w;
}

SamplerState shadowMapSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};
float4 PS_main(float4 screenPos : SV_POSITION) : SV_Target0
{
	float3 normal;
	float3 position;
	float3 diffuse;
	float3 specular;
	float specularPower;

	GetGBuffer(screenPos.xy, normal, position, diffuse, specular, specularPower);

	//Point light
	float3 pointLightDist = pointLight.pos - position;
	float3 s = normalize(pointLightDist);
	float3 n = normalize(normal);

	float3 ambientLight = pointLight.ambient;
	float3 diffuseLight = pointLight.diffuse * saturate(dot(s, n));
	float3	v = normalize(cameraPos - position);
	float3 r = reflect(-s, n);
	float3 specularLight = specular * pow(saturate(dot(r, v)), specularPower);

	//Dístance attenuation
	float distAtten = clamp( (pointLight.range - length(pointLightDist))/(pointLight.range - (3.7f / 4.0f) * pointLight.range), 0, 1);

	float3 result = ((ambientLight + diffuseLight)*diffuse + specularLight) * distAtten;

	return float4((normal.x == 0 && normal.y == 0 && normal.z == 0) ? float3(0.0f,0.0f,0.0f) : result, 1.0f);

}

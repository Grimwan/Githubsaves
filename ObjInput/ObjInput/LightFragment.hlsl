Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D specularTexture : register(t3);



struct Light
{
	float3 pos;
	float pad;
	float4 ambient;
	float4 diffuse;
};


cbuffer LightParam : register(b1)
{
	Light light;
};

cbuffer CameraParam : register(b0)
{
	float3 cameraPos;
	float pad;
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



float4 PS_main(in float4 screenPos : SV_Position) : SV_Target0
{
	float3 normal;
	float3 position;
	float3 diffuse;
	float3 specular;
	float specularPower;

	GetGBuffer(screenPos.xy, normal, position, diffuse, specular, specularPower);


	float3 s = normalize(light.pos - position);
	float3 n = normalize(normal);

	float3 ambientLight = light.ambient;
	float3 diffuseLight = light.diffuse * saturate(dot(s, n));
	//Specular
	float3 v = normalize(cameraPos-position);
	float3 r = reflect(-s, n);
	float3 specularLight = specular * pow(saturate(dot(r, v)), specularPower);

	float3 result = (ambientLight + diffuseLight + specularLight)*diffuse;

	return float4(result, 1);


	//return (1,1,1,1);
}

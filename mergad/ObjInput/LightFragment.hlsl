Texture2D normalTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D diffuseTexture : register(t2);
Texture2D specularTexture : register(t3);
Texture2D ShadowMap : register(t4);



struct Light
{
	float3 dir;
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

	//shadow Mapping
	float4 posLightWVP = mul(float4(position, 1), projViewMatrix);

	posLightWVP.xy /= posLightWVP.w; // now in NDC
										 
	float2 smTex = float2(0.5f*posLightWVP.x + 0.5f, -0.5f*posLightWVP.y + 0.5f);
	// Compute pixel depth for shadowing.
	float depth = posLightWVP.z / posLightWVP.w;

	float mapDepth = ShadowMap.Sample(shadowMapSampler, smTex).r;

	float shadowCoeff = (mapDepth + 0.00001 < depth) ? 0.0f : 1.0f;





	//Lightning
	float3 s = light.dir; /*normalize(light.pos - position);*/
	float3 n = normalize(normal);

	float3 ambientLight = light.ambient;
	float3 diffuseLight = light.diffuse * saturate(dot(s, n));
	//Specular
	float3 v = normalize(cameraPos - position);
	float3 r = reflect(-s, n);
	float3 specularLight = specular * pow(saturate(dot(r, v)), specularPower);

	float3 result = (ambientLight + (diffuseLight + specularLight)* shadowCoeff)*diffuse;

	return float4(result, 1.0f);

}

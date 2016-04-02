Texture2D<float4> Input : register( t0 );
RWTexture2D<float4> Output : register( u0 );

#define size_x 25
#define size_y 25


#define FXAA_REDUCE_MIN 1.0f/128.0f
#define FXAA_REDUCE_MUL 1.0f/8.0f
#define FXAA_SPAN_MAX 8.0f

SamplerState textureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

float FxaaLuma(float3 rgb)
{
	float3 lumen = float3(0.299, 0.587, 0.114);
	return dot(rgb, lumen);
}



float3 FXAA(in uint3 pos)
{
	float2 texPos = float2(pos.x / (1199.0f), pos.y / (949.0f));
	float2 offset = float2(1.0f / 1199.0f, 1.0f / 949.0f);
	
	float3 rgbM = Input.SampleLevel(textureSampler, texPos , 0).rgb;
	float3 rgbNW = Input.SampleLevel(textureSampler, texPos.xy + float2(-1, -1) * offset, 0).rgb;
	float3 rgbNE = Input.SampleLevel(textureSampler, texPos.xy + float2(1, -1) * offset, 0).rgb;
	float3 rgbSW = Input.SampleLevel(textureSampler, texPos.xy + float2(-1, 1) * offset, 0).rgb;
	float3 rgbSE = Input.SampleLevel(textureSampler, texPos.xy + float2(1, 1) * offset, 0).rgb;

	float lumaM = FxaaLuma(rgbM);
	float lumaNW = FxaaLuma(rgbNW);
	float lumaNE = FxaaLuma(rgbNE);
	float lumaSW = FxaaLuma(rgbSW);
	float lumaSE = FxaaLuma(rgbSE);

	float2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE)*(0.25f* FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);

	//Testing test so the sampling won´t be to far away from the original texel
	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir*rcpDirMin))*offset;

	float3 rgbA = 0.5f*(Input.SampleLevel(textureSampler, texPos.xy + dir.xy * (1.0f / 3.0f - 0.5f), 0).rgb + Input.SampleLevel(textureSampler, texPos.xy + dir.xy * (2.0f / 3.0f - 0.5f), 0).rgb);

	float3 rgbB = 0.5f * rgbA + 0.25f * (Input.SampleLevel(textureSampler, texPos.xy + dir.xy * (0.0f / 3.0f - 0.5f), 0).rgb + Input.SampleLevel(textureSampler, texPos.xy + dir.xy * (3.0f / 3.0f - 0.5f), 0).rgb);

	//Testing rgbB
	float lumaB = FxaaLuma(rgbB);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	return (lumaB < lumaMin || lumaMax < lumaB) ? rgbA : rgbB;
}

[numthreads(size_x,size_y, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint3 texPos = threadID;
	float4 finalColor = float4(FXAA(texPos),1.0f);
	Output[threadID.xy] = finalColor;
}
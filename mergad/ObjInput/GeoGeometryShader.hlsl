struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 tCoord : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float3 wPos : POSITION;
};

struct GS_OUT
{
	float4 Pos : SV_POSITION;
	float2 tCoord : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float3 wPos : POSITION;
};

cbuffer GS_CONSTANT_BUFFER : register(b0)
{
	float4x4 projViewMatrix;
}

cbuffer cameraParam : register(b1)
{
	float3 cameraPos;
	float pad;
}



[maxvertexcount(3)]
void GS_main(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> OutputStream)
{
	GS_OUT output = (GS_OUT)0;

	float3 edge0 = input[1].wPos - input[0].wPos;
	float3 edge1 = input[2].wPos - input[0].wPos;

	float3 normal = cross(edge0, edge1);
	normalize(normal);

	float3 dir = normalize(cameraPos - input[0].wPos);
	float product = dot(normal, dir);

	if (product >= 0)
	{
		for (uint i = 0; i < 3; i++)
		{
			output.Pos = mul(input[i].Pos, projViewMatrix);
			output.tCoord = input[i].tCoord;
			output.Norm = input[i].Norm;
			output.wPos = input[i].wPos;

			OutputStream.Append(output);
		}

		OutputStream.RestartStrip();
	}
	else
	{

		output.Pos = mul(input[0].Pos, projViewMatrix);
		output.tCoord = input[0].tCoord;
		output.Norm = -input[0].Norm;
		output.wPos = input[0].wPos;

		OutputStream.Append(output);

		output.Pos = mul(input[2].Pos, projViewMatrix);
		output.tCoord = input[2].tCoord;
		output.Norm = -input[2].Norm;
		output.wPos = input[2].wPos;

		OutputStream.Append(output);

		output.Pos = mul(input[1].Pos, projViewMatrix);
		output.tCoord = input[1].tCoord;
		output.Norm = -input[1].Norm;
		output.wPos = input[1].wPos;

		OutputStream.Append(output);
		
		OutputStream.RestartStrip();
	}


	return;

}
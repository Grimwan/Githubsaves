struct VS_IN
{
	float3 Pos : POSITION;
};








float4 VS_main(VS_IN input) : SV_POSITION
{
	return float4(input.Pos,1);
}
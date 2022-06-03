Texture2D uTexDiffuse : register(t0);
SamplerState uTexDiffuseSampler : register(s0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex0 : TEXCOORD0;
	float4 worldPosition: WORLDPOSITION;
	float3 worldNormal: WORLDNORMAL;
};

struct PS_OUTPUT
{
	float4 Diffuse: SV_TARGET0;
	float4 Position: SV_TARGET1;
	float4 Normal: SV_TARGET2;
	float4 SG: SV_TARGET3;
};

cbuffer cbPerFrame
{
	float uSpec;
	float uGloss;
};

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output;

	float3 baseMap = uTexDiffuse.Sample(uTexDiffuseSampler, input.tex0).xyz;

	output.Diffuse = float4(baseMap, 1.0);
	output.Position = input.worldPosition;
	output.Normal = float4(input.worldNormal, 1.0);
	output.SG = float4(uSpec, uGloss, 0.0, 1.0);

	return output;
}

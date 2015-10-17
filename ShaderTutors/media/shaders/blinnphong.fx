
sampler mytex0 : register(s0);

matrix matWorld;
matrix matWorldInv;
matrix matViewProj;

float4 lightPos		= { -10, 10, -10, 1 };
float4 lightColor	= { 1, 1, 1, 1 };
float4 eyePos;
float4 ambient		= { 0.2f, 0.2f, 0.2f, 1 };
float2 uv			= { 1, 1 };

void vs_main(
	in out	float4 pos		: POSITION,
	in		float3 norm		: NORMAL,
	in out	float2 tex		: TEXCOORD0,
	out		float3 wnorm	: TEXCOORD1,
	out		float3 ldir		: TEXCOORD2,
	out		float3 vdir		: TEXCOORD3)
{
	pos = mul(pos, matWorld);
	wnorm = mul(matWorldInv, float4(norm, 0)).xyz;

	ldir = lightPos.xyz - pos.xyz;
	vdir = eyePos.xyz - pos.xyz;

	pos = mul(pos, matViewProj);
	tex *= uv;
}

void ps_main(
	in	float2 tex		: TEXCOORD0,
	in	float3 wnorm	: TEXCOORD1,
	in	float3 ldir		: TEXCOORD2,
	in	float3 vdir		: TEXCOORD3,
	out	float4 color	: COLOR0)
{
	float3 l = normalize(ldir);
	float3 v = normalize(vdir);
	float3 h = normalize(l + v);
	float3 n = normalize(wnorm);

	float diffuse = saturate(dot(n, l));
	float specular = saturate(dot(n, h));

	diffuse = lerp(diffuse, 1, ambient);

	// approximating Phong model (needs larger exponent)
	specular = pow(specular, 60);

	// final color
	color = (tex2D(mytex0, tex) * diffuse + specular) * lightColor;
	color.a = 1;
}

technique blinnphong
{
	pass p0
	{
		vertexshader = compile vs_2_0 vs_main();
		pixelshader = compile ps_2_0 ps_main();
	}
}

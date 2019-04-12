
layout(location = 0) in vec2 iTexCoords;
layout(location = 1) in vec2 iVtxTexCoords;

uniform vec4 DOF_Params; //near, far, scale, max

#ifdef OUTPUT_COC
layout(binding = 0) uniform sampler2D DepthTexture;
#else
layout(binding = 0) uniform sampler2D COCBuffer;
layout(binding = 1) uniform sampler2D BlurCOCBuffer;
layout(binding = 2) uniform sampler2D ColorBuffer;
layout(binding = 3) uniform sampler2D BlurColorBuffer;
#endif

void main()
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

#ifdef OUTPUT_COC

	float fDepth = texture2D(DepthTexture, iTexCoords).r;

	float ne = DOF_Params.x;
	float nb = DOF_Params.y;
	float fb = DOF_Params.z;
	float fe = DOF_Params.w;

	gl_FragColor.r = min(1.0, -min((fDepth - ne) / (nb - ne), 0.0));
	gl_FragColor.g = min(1.0, max((fDepth - fb) / (fe - fb), 0.0));

//	gl_FragColor.r = 1 - min((1 - clamp(fDepth, 0.0, 1.0) * DOF_Params.z), DOF_Params.w);
//	gl_FragColor.g = min(clamp(fDepth, 0.0, 1.0) * DOF_Params.z, DOF_Params.w);

#else

	vec2 CoC_RG = texture2D(COCBuffer, iTexCoords).xy;

	float CoC_R = texture2D(BlurCOCBuffer, iTexCoords).x; //max(CoC_RG.x, texture2D(BlurCOCBuffer, iTexCoords).x);
	float CoC_G = CoC_RG.y;

	vec3 OutputColor = texture2D(ColorBuffer, iTexCoords).xyz;
	vec3 BlurColor = texture2D(BlurColorBuffer, iTexCoords).xyz;

	OutputColor = mix(OutputColor, BlurColor, smoothstep(0.0, 1.0, CoC_R));
	OutputColor = mix(OutputColor, BlurColor, CoC_G);

	gl_FragColor.xyz = OutputColor;
#endif
}

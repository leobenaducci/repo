
layout(location = 0) in vec4 iPosition;
layout(location = 1) in vec4 iClipPosition;
layout(location = 2) in vec4 iClipPrevPosition;
layout(location = 3) in vec3 iNormal;
layout(location = 4) in vec2 iTexCoord;
layout(location = 5) in vec4 iTangent;

uniform vec4 MaterialColor;
uniform vec4 MaterialProperties; //x=Mettalic, y=Roughness, z=Emissive

layout(binding = 0) uniform sampler2D SamplerAlbedo;
layout(binding = 1) uniform sampler2D SamplerGloss;
layout(binding = 2) uniform sampler2D SamplerNormalMap;

void main()
{
	//Normal Metallic
	mat3 TBN = mat3(iTangent.xyz, cross(iNormal.xyz, iTangent.xyz) * iTangent.w, iNormal.xyz);
	
	gl_FragData[0].xyz = normalize(TBN * (texture2D(SamplerNormalMap, iTexCoord).xyz * 2 - 1)) * 0.5 + 0.5;
	gl_FragData[0].w = MaterialProperties.x;

	//Color Roughness
	vec4 Albedo = texture2D(SamplerAlbedo, iTexCoord).xyzw;
	gl_FragData[1].xyz = MaterialColor.xyz * Albedo.xyz;
	gl_FragData[1].w = clamp(MaterialProperties.y * (1.0-texture2D(SamplerGloss, iTexCoord).x), 0.25f, 0.99f);

	//Linear Depth
	gl_FragData[2] = vec4((iPosition.w / 1000), 0.0, 0.0, 0.0);

	//Velocities
	gl_FragData[3].xy = (iClipPosition.xy/iClipPosition.ww) - (iClipPrevPosition.xy/iClipPrevPosition.ww);
	gl_FragData[3].z = MaterialProperties.z;
	gl_FragData[3].w = 0.0;

	if (Albedo.w < 0.5f)
		discard;
}
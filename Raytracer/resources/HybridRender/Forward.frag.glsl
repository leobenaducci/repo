
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;

uniform vec4 MaterialColor;
uniform vec4 MaterialProperties;
uniform vec3 CameraPos;

void main()
{
	gl_FragColor = vec4(0.0);

/*
#if SHADOW_MAP
	gl_FragColor.xyz = vec3(0.25) + CookTorranceDir(iPosition, CameraPos, vec4(iNormal.xyz*0.5+0.5, MaterialProperties.x), vec4(MaterialColor.xyz, MaterialProperties.y));
#endif
*/
}
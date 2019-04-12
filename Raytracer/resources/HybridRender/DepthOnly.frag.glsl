
layout(location = 0) in vec3 iTexCoord;
layout(binding = 0) uniform sampler2D SamplerAlbedo;

void main()
{
	gl_FragColor = vec4(iTexCoord.x);
}
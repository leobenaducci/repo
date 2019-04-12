

layout(location = 0) in vec3 Vertex;
layout(location = 0) out vec3 oTexCoords;

void main()
{
	gl_Position = vec4(Vertex.xy, 0.5, 1.0);

#ifdef CUSTOM_TEXCOORDS
	oTexCoords = gl_MultiTexCoord0.xyz;
#else
	oTexCoords = Vertex.xyz * 0.5 + 0.5;
#endif
}
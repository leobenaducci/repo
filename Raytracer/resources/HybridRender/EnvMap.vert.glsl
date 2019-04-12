

layout(location = 0) in vec3 Vertex;
layout(location = 0) out vec3 oTexCoords;

void main()
{
	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
	oTexCoords = Vertex.xyz * 0.5 + 0.5;
}
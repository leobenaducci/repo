
uniform mat4 WorldMatrix;
uniform mat4 ViewProjectionMatrix;

layout(location = 0) in vec3 Vertex;
layout(location = 2) in vec3 Normal;

layout(location = 0) out vec3 oTexCoord;

void main()
{
	//World position
	vec4 Pos = WorldMatrix * vec4(Vertex.xyz, 1.0);

	//Clip position
	gl_Position = ViewProjectionMatrix * vec4(Pos.xyz, 1.0);

	//texcoord
	oTexCoord.xyz = vec3(gl_Position.z);
}
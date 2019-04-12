

uniform mat4 WorldMatrix;
uniform mat4 NormalMatrix;
uniform mat4 ViewProjectionMatrix;

layout(location = 0) in vec3 Vertex;
layout(location = 2) in vec3 Normal;

layout(location = 0) out vec3 oPosition;
layout(location = 1) out vec3 oNormal;

//glutSolids modifies gl_ModelViewMatrix, I must transform our Vertex (and normals, and tangents) to get the "real" model space position
void main()
{
	//World position
	oPosition.xyz = (WorldMatrix * gl_ModelViewMatrix * vec4(Vertex.xyz, 1.0)).xyz;
	gl_Position = ViewProjectionMatrix * vec4(oPosition.xyz, 1.0);

	//World normal
	oNormal.xyz = mat3(NormalMatrix * gl_ModelViewMatrix) * Normal.xyz;
	oNormal = normalize(oNormal);
}
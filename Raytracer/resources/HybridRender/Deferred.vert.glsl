

uniform mat4 WorldMatrix;
uniform mat4 NormalMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ViewProjectionMatrix;
uniform mat4 ProjectionMatrix;

uniform mat4 PrevWorldMatrix;
uniform mat4 PrevViewProjectionMatrix;

uniform float RandX, RandY;

layout(location = 0) in vec3 Vertex;
layout(location = 2) in vec3 Normal;

layout(location = 0) out vec4 oPosition;
layout(location = 1) out vec4 oClipPosition;
layout(location = 2) out vec4 oClipPrevPosition;
layout(location = 3) out vec3 oNormal;
layout(location = 4) out vec2 oTexCoord;
layout(location = 5) out vec4 oTangent;

//glutSolids modifies gl_ModelViewMatrix, I must transform our Vertex (and normals, and tangents) to get the "real" model space position
void main()
{
	vec3 PrevPosition;
	vec4 Pos;

	//World position
	oPosition.xyz = (WorldMatrix * vec4(Vertex.xyz, 1.0)).xyz;
	PrevPosition.xyz = (PrevWorldMatrix * vec4(Vertex.xyz, 1.0)).xyz;

	//Clip position
	Pos = ViewProjectionMatrix * vec4(oPosition.xyz, 1.0);

	gl_Position.xy = Pos.xy + vec2(RandX, RandY) * Pos.w;
	gl_Position.zw = Pos.zw;

	oPosition.w = (ViewMatrix * vec4(oPosition.xyz, 1.0)).z;

	oClipPosition = ViewProjectionMatrix * vec4(oPosition.xyz, 1.0);
	oClipPrevPosition = PrevViewProjectionMatrix * vec4(PrevPosition.xyz, 1.0);

	//World normal
	oNormal.xyz = mat3(NormalMatrix) * Normal.xyz;
	oNormal = normalize(oNormal);

	//World tangent
	oTangent = vec4(mat3(NormalMatrix) * gl_MultiTexCoord3.xyz, gl_MultiTexCoord3.w);
	oTangent.xyz = normalize(oTangent.xyz);// * gl_MultiTexCoord3.w;

	//texcoord
	oTexCoord.xy = gl_MultiTexCoord0.xy;
}
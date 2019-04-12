
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;
//layout(invocations = 6) in;

uniform mat4 CubeViewMatrix[6];
uniform mat4 CubeProjMatrix;
uniform mat4 ViewProjectionMatrix;

layout(location = 0) in vec4 iPosition[];
layout(location = 1) in vec3 iNormal[];

layout(location = 0) out vec3 oPosition;
layout(location = 1) out vec3 oNormal;


void main()
{
	//Clip position
	for (int iCubeFace = 0; iCubeFace < 6; iCubeFace++)
	{
		for(int i = 0; i < 3; i++)
		{
			gl_Layer = iCubeFace;

			gl_Position = CubeProjMatrix * vec4((CubeViewMatrix[iCubeFace] * vec4(iPosition[i].xyz, 1.0)).xyz, 1);
			//gl_Position = ViewProjectionMatrix * vec4(iPosition[i].xyz, 1.0);
		
			oPosition = iPosition[i].xyz;
			oNormal = iNormal[i];
			EmitVertex();
		}

		EndPrimitive();
	}
}

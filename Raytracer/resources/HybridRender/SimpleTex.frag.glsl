

layout(location = 0) in vec3 iTexCoords;
layout(binding = 0) uniform sampler2D InputTex;

uniform int InputTexLevel = 0;

void main()
{
	gl_FragColor = texture(InputTex, iTexCoords.xy, InputTexLevel);
}

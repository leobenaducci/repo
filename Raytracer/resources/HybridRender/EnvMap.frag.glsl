

layout(location = 0) in vec3 iTexCoords;
layout(binding = 0) uniform sampler2D SkyBox2D;
layout(binding = 1) uniform sampler2D InputColorRoughness;

uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];
uniform mat4 CubemapRotation;
uniform float SkyBoxIntensity;

void main()
{
	const vec2 FragUV = iTexCoords.xy;
	const vec3 CameraEnd = mix(mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, FragUV.x),
		mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, FragUV.x),
		FragUV.y).xyz;

	const vec3 Dir = (vec4(normalize(CameraEnd - CameraPos), 0.f) * CubemapRotation).xyz;
	
	float theta = atan(Dir.z, Dir.x) + PI;
	vec4 col = texture(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)));

	vec4 ColorRoughness = texture(InputColorRoughness, iTexCoords.xy);

	if (ColorRoughness.w > 0)
		discard;

	gl_FragColor = col.xyzw * vec4(vec3(SkyBoxIntensity), 1);
}

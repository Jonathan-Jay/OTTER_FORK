#version 410
layout(location = 0) in vec3 inPos1;
layout(location = 1) in vec3 inPos2;
layout(location = 2) in vec3 inNorm1;
layout(location = 3) in vec3 inNorm2;

uniform mat4 MVP;
uniform mat4 transform;

uniform float t;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

void main() {
	// vertex position in clip space
	vec3 pos = mix(inPos1, inPos2, t);

	outPosition = (transform * vec4(pos, 1)).xyz;
	
	outNormal = mat3(transform) * mix(inNorm1, inNorm2, t);

	gl_Position = MVP * vec4(pos, 1);
}



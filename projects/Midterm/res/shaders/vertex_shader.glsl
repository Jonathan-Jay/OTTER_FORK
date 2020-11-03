#version 410
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

uniform mat4 MVP;
uniform mat4 transform;
uniform mat3 rotation;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

void main() {
	// vertex position in clip space
	gl_Position = MVP * vec4(inPosition, 1.0);

	outNormal = rotation * inNormal;
	outPosition = (transform * vec4(inPosition, 1)).xyz;
}



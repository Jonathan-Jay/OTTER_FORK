#version 410
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inSpecStregnth;

uniform mat4 MVP;
uniform mat4 transform;
uniform mat3 rotation;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outColour;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outSpecStrength;

void main() {
	// vertex position in clip space
	gl_Position = MVP * vec4(inPosition, 1.0);

	outPosition = (transform * vec4(inPosition, 1)).xyz;
	outColour = inColour;
	outNormal = rotation * inNormal;
	outSpecStrength = inSpecStregnth;
}



#version 410
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 vertex_uv;

layout(location = 0) out vec2 texUV;

uniform mat4 MVP;

uniform float time;
uniform float tiling;
uniform float waterHeight;

void main() {
	texUV = vertex_uv * tiling;

	vec3 pos = vertex_pos;

	//add water height here to easily reuse the plane
	pos.y = (waterHeight + cos(pos.z) * 0.125 + sin(
			(pos.x + (pos.z * cos(time * 0.1)) + time * 0.1) * 15
		) * 0.125
	) * 0.1;


	gl_Position = MVP * vec4(pos, 1.0);
}
	
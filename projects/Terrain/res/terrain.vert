#version 410
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 vertex_uv;

layout(location = 0) out float vertexHeight;
layout(location = 1) out vec2 texUV;

uniform mat4 MVP;

uniform float tiling;

uniform sampler2D heightmap;

void main() {
	texUV = vertex_uv * tiling;

	vec3 pos = vertex_pos;

	pos.y = texture(heightmap, vertex_uv).r;

	vertexHeight = pos.y * 10;

	gl_Position = MVP * vec4(pos, 1.0);
}
	
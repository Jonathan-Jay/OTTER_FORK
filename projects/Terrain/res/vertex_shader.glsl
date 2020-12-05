#version 410
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 vertex_uv;

layout(location = 0) out vec2 texUV;
layout(location = 1) out vec2 texUV2;

uniform mat4 MVP;

uniform float time;
uniform float tiling;

uniform sampler2D myTextureSampler;

void main() {
	texUV = vertex_uv;
	texUV2 = vertex_uv * tiling;

	vec3 pos = vertex_pos;

	//pos.y = sin(time + (vertex_uv.x + pos.y * 0.75) * 7.5) * 0.075;
	pos.y = pos.y + texture(myTextureSampler, vertex_uv).r;// * sin(time);

	pos.y *= 0.1;

	gl_Position = MVP * vec4(pos, 1.0);
}
	
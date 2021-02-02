#version 410
layout(location = 0) in vec2 texUV;

out vec4 frag_color;

uniform sampler2D water;

void main() {

	vec4 colour = texture(water, texUV);

	//manually add a base transparency
	colour.a *= 0.75;

	frag_color = colour;
}
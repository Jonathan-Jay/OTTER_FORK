#version 410
layout(location = 0) in vec2 texUV;
layout(location = 1) in vec2 texUV2;

out vec4 frag_color;

uniform sampler2D myTextureSampler;
uniform sampler2D checker;
uniform sampler2D box;

uniform float t;
uniform float widtht;

void main() {

	vec4 colour = vec4(1, 1, 1, 1);
	
	if (texUV.y > widtht - t) {
		if (texUV.y < widtht + t) {
			colour = mix(texture(box, texUV2), texture(checker, texUV2), (texUV.y - (widtht - t)) / (t * 2));
			//colour = mix(vec4(0, 0, 0, 1), vec4(1, 1, 1, 1), (texUV.y - (widtht - t)) / (t * 2));
		}
		else {
			colour = texture(checker, texUV2);
			//colour = vec4(1, 1, 1, 1);
		}
	}
	else {
		colour = texture(box, texUV2);
		//colour = vec4(0, 0, 0, 1);
	}


	frag_color = colour;
}
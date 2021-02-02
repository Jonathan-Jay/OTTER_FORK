#version 410
layout(location = 0) in float height;
layout(location = 1) in vec2 texUV;

out vec4 frag_color;

uniform sampler2D sand;
uniform sampler2D grass;
uniform sampler2D rock;

uniform float spread;
uniform float waterHeight;

void main() {

	vec4 colour = vec4(1, 1, 1, 1);
	
	float wHeight = waterHeight + 0.3;	//adjust water height to be above the highest wave
	float rockHeight = 6;

	if (height < wHeight - spread) {		//below water, so sand
		colour = texture(sand, texUV);
	}
	else if (height < wHeight + spread) {	//below top of mix, so sand mixed with grass
		colour = mix(texture(sand, texUV), texture(grass, texUV), smoothstep(0, (spread * 2), (height - wHeight + spread)));
	}
	else if (height < rockHeight - spread) {			//below rock zone, so grass
		colour = texture(grass, texUV);
	}
	else if (height < rockHeight + spread) {			//below top of mix, so grass mixed with rock
		colour = mix(texture(grass, texUV), texture(rock, texUV), smoothstep(0, (spread * 2), (height - rockHeight + spread)));
	}
	else {										//anything else is tall and rock
		colour = texture(rock, texUV);
	}

	frag_color = colour;
}
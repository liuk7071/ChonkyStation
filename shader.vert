
#version 330 core
layout (location = 0) in ivec3 pos;
layout (location = 1) in vec3 colour;
layout (location = 2) in uint texpage;
layout (location = 3) in uint clut;
layout (location = 4) in vec2 texture_uv;
layout (location = 5) in uint texture_enable;
out vec4 frag_colour;

void main() {
	//pos += vec3(0.5, 0.5, 0.0);
	gl_Position = vec4(float(pos.x + 0.5) / 512 - 1, -(1 - float(pos.y + 0.5) / 256), 0.0, 1.0);
	frag_colour = vec4(float(colour.r) / 255, float(colour.g) / 255, float(colour.b) / 255, 1.f);
}

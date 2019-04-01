#version 450

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_position;
layout(location = 2) out vec4 out_normal;

in vec3 vTexcoord;

layout(binding = 0) uniform samplerCube uTexture;


void main() {
	vec4 texcol = texture(uTexture, vTexcoord);

	out_color = vec4(texcol.rgb, 1.0); 
	out_position = vec4(0);
	out_normal = vec4(0);
}


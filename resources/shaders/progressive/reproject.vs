#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in uint aValue;

uniform mat4 uWorldViewProj;

layout(binding = 0) uniform sampler2D uGradient;

out vec3 vColor;
out vec4 vVertexID;

void main() {
	
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = 2.0;

	

	vec4 vecval = unpackUnorm4x8(aValue);
	vColor = vecval.xyz;

	vec2 range = vec2(0, 469);
	//vec2 range = vec2(10, 10000);
	//vec2 range = vec2(710619, 1024861);
	
	//vColor = aColor.rgb;
	float w = (float(aValue) - range.x) / (range.y - range.x);
	w = clamp(w, 0, 1);
	vColor = texture(uGradient, vec2(w, 0.0)).rgb;

	

	vVertexID = vec4(
		float((gl_VertexID >>  0) & 255) / 255.0,
		float((gl_VertexID >>  8) & 255) / 255.0,
		float((gl_VertexID >> 16) & 255) / 255.0,
		float((gl_VertexID >> 24) & 255) / 255.0
	);

}



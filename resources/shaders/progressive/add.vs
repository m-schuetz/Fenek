#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;

uniform mat4 uWorldViewProj;

layout(binding = 0) uniform sampler2D uGradient;

out vec3 vColor;
out vec4 vVertexID;

void main() {
	
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = 2.0;
	
	//vec4 vecval = unpackUnorm4x8(aValue);
	//vColor = vecval.xyz;

	vec4 rgba = vec4(
		(0x000000FF & aValue) >>  0,
		(0x0000FF00 & aValue) >>  8,
		(0x00FF0000 & aValue) >> 16,
		(0xFF000000 & aValue) >> 24
	) / 256.0;

	vColor = rgba.xyz;

	vVertexID = vec4(
		float((gl_VertexID >>  0) & 255) / 255.0,
		float((gl_VertexID >>  8) & 255) / 255.0,
		float((gl_VertexID >> 16) & 255) / 255.0,
		float((gl_VertexID >> 24) & 255) / 255.0
	);

}



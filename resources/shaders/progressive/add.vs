#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;

uniform mat4 uWorldViewProj;

layout(binding = 0) uniform sampler2D uGradient;

out vec3 vColor;
out vec4 vVertexID;

void main() {
	
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = 2.0;
	
	vColor = aColor.rgb;
	//vColor = vec3(1.0, 0.0, 1.0);

	float w = aColor.r;
	//vColor = texture(uGradient, vec2(1 - w, 0.0)).rgb;

	vVertexID = vec4(
		float((gl_VertexID >>  0) & 255) / 255.0,
		float((gl_VertexID >>  8) & 255) / 255.0,
		float((gl_VertexID >> 16) & 255) / 255.0,
		float((gl_VertexID >> 24) & 255) / 255.0
	);

}



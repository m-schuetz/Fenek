#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;

uniform mat4 uWorldViewProj;
uniform int uOffset;

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
	//vColor = vec3(0, 1, 0);

	int vertexID = gl_VertexID + uOffset;
	vVertexID = vec4(
		float((vertexID >>  0) & 0xFF) / 255.0,
		float((vertexID >>  8) & 0xFF) / 255.0,
		float((vertexID >> 16) & 0xFF) / 255.0,
		float((vertexID >> 24) & 0xFF) / 255.0
	);
	
	// if(vertexID >= 2 * 134000000 + 10000000){
	// 	vColor = vec3(1, 0, 0);
	// }
}



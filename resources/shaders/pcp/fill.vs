#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;

uniform mat4 uWorldViewProj;
uniform int uOffset;

layout(binding = 0) uniform sampler2D uGradient;

out vec3 vColor;
out vec4 vVertexID;

vec3 getColorFromV3(){
	vec3 v = vec3(
		(aValue >>   0) & 0xFF,
		(aValue >>   8) & 0xFF,
		(aValue >>  16) & 0xFF
	);

	v = v / 255.0;

	return v;
}

void main() {
	
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = 2.0;
	
	vColor = getColorFromV3();
	vColor = vec3(1, 0, 0);

	//vColor = aValue.xyz;

	int vertexID = gl_VertexID + uOffset;
	vVertexID = vec4(
		float((vertexID >>  0) & 0xFF) / 255.0,
		float((vertexID >>  8) & 0xFF) / 255.0,
		float((vertexID >> 16) & 0xFF) / 255.0,
		float((vertexID >> 24) & 0xFF) / 255.0
	);

}



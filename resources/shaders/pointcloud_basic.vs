#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aRandom;

//layout(location = 0) uniform int uNodeIndex;
//layout(location = 1) uniform mat4 uTransform;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;

	float pointSize;
} ssArgs;

out vec3 vColor;

void main() {

	vec4 pos = ssArgs.transform * vec4(aPosition, 1.0);

	gl_Position = pos;

	vColor = aColor.rgb;

	// MOSTLY RED
	//vColor = aColor * 0.01 + vec3(1.0, 0.0, 0.0);

	// COLOR BY INDEX
	//float w = float(gl_VertexID) / float(node.numPoints);
	//vColor = vec3(w, 0, 0);

	gl_PointSize = 1.0;
}
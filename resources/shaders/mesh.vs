#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;

	float time;
	vec2 screenSize;

} ssArgs;

out vec2 vUV;
out vec4 vPositionVP;
out vec4 vNormalVP;

void main() {
	mat4 nWorld = ssArgs.world;
	nWorld[3][0] = 0;
	nWorld[3][1] = 0;
	nWorld[3][2] = 0;

	vec4 pos = ssArgs.world * vec4(aPosition, 1.0);

	gl_Position = ssArgs.proj * ssArgs.view *  pos;

	// if(gl_VertexID == 0){
	// 	gl_Position = vec4(0, 0, 0, 1);
	// }else if(gl_VertexID == 1){
	// 	gl_Position = vec4(1, 0, 0, 1);
	// }else if(gl_VertexID == 2){
	// 	gl_Position = vec4(1, 1, 0, 1);
	// }

	vUV = aUV;
	vNormalVP = nWorld * vec4(aNormal,1.0f);
	vPositionVP = pos;

	gl_PointSize = 4.0;
}
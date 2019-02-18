#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 3) in vec4 aAcc;

uniform mat4 uWorldViewProj;

out vec3 vColor;
out vec4 vVertexID;

//struct Vertex{
//	float ux;
//	float uy;
//	float uz;
//	uint colors_orig;
//	uint colors_avg;
//	float accR;
//	float accG;
//	float accB;
//	float accA;
//};

//layout(std430, binding = 2) buffer ssVBO{
//	Vertex vbo[];
//};

void main() {
	
	gl_Position = uWorldViewProj * vec4(aPosition, 1.0);
	gl_PointSize = 1.0;

	//Vertex v = vbo[gl_VertexID];

	vColor = aColor.rgb;
	vColor = aAcc.rgb / aAcc.a;

	//if(aAcc.a == 0.0){
	//	vColor = vec3(1, 0, 0);
	//}
	
	vVertexID = vec4(
		float((gl_VertexID >>  0) & 255) / 255.0,
		float((gl_VertexID >>  8) & 255) / 255.0,
		float((gl_VertexID >> 16) & 255) / 255.0,
		float((gl_VertexID >> 24) & 255) / 255.0
	);

}



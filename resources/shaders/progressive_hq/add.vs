#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColorOrig;
layout(location = 2) in vec4 aColorAvg;
//layout(location = 1) in float aTest;
//layout(location = 1) in vec4 aColor;
//layout(location = 2) in uint aID;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;

	float pointSize;
} ssArgs;

out vec3 vColor;
out vec4 vVertexID;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint colors_orig;
	uint colors_avg;
	float accR;
	float accG;
	float accB;
	float accA;
};

layout(std430, binding = 2) buffer ssVBO{
	Vertex vbo[];
};

layout(binding = 0) uniform sampler2D uPreviousFrame;

layout(rgba8ui, binding = 2) uniform uimage2D uIndices;

void main() {

	gl_Position = ssArgs.transform * vec4(aPosition, 1.0);
	gl_PointSize = ssArgs.pointSize;

	float d = gl_Position.w;
	
	vColor = aColorOrig.rgb;

	vVertexID = vec4(
		float((gl_VertexID >>  0) & 255) / 255.0,
		float((gl_VertexID >>  8) & 255) / 255.0,
		float((gl_VertexID >> 16) & 255) / 255.0,
		float((gl_VertexID >> 24) & 255) / 255.0
	);
	
	vec2 uv = (gl_Position.xy / gl_Position.w) / 2.0 + 0.5;
	vec4 cPrev = texture(uPreviousFrame, uv);

	//vColor = vColor + 0.000001 * cPrev.xyz;

	ivec2 iSize = imageSize(uIndices);
	ivec2 pixelCoords = ivec2(
		int(uv.x * float(iSize.x)),
		int(uv.y * float(iSize.y))
	);
	uvec4 vVertexID = imageLoad(uIndices, pixelCoords);
	uint vertexID = vVertexID.r | (vVertexID.g << 8) | (vVertexID.b << 16) | (vVertexID.a << 24);
	
	Vertex vOld = vbo[vertexID];
	Vertex vNew = vbo[gl_VertexID];


	float distance = length(
		vec3(vNew.ux, vNew.uy, vNew.uz) - vec3(vOld.ux, vOld.uy, vOld.uz)
	);

	

	if(distance < 0.01 * d){

		vec4 cNew = vec4(
			float((vNew.colors_orig & 0x0000FF) >> 0) / 255.0,
			float((vNew.colors_orig & 0x00FF00) >> 8) / 255.0,
			float((vNew.colors_orig & 0xFF0000) >> 16) / 255.0,
			1.0
		);

		vec4 cOld = vec4(
			float((vOld.colors_orig & 0x0000FF) >> 0) / 255.0,
			float((vOld.colors_orig & 0x00FF00) >> 8) / 255.0,
			float((vOld.colors_orig & 0xFF0000) >> 16) / 255.0,
			1.0
		);

		if(vNew.accA < 1.0){
			vNew.accR += cNew.r;
			vNew.accG += cNew.g;
			vNew.accB += cNew.b;
			vNew.accA += cNew.a;
		}

		float ww = 0.1;
		vOld.accR += ww * vOld.accA * cNew.r;
		vOld.accG += ww * vOld.accA * cNew.g;
		vOld.accB += ww * vOld.accA * cNew.b;
		vOld.accA += ww * vOld.accA * cNew.a;

		vNew.accR += ww * vNew.accA * cOld.r;
		vNew.accG += ww * vNew.accA * cOld.g;
		vNew.accB += ww * vNew.accA * cOld.b;
		vNew.accA += ww * vNew.accA * cOld.a;

		float red = 0.9;
		if(vOld.accA > 100){
			vOld.accR = vOld.accR * red;
			vOld.accG = vOld.accG * red;
			vOld.accB = vOld.accB * red;
			vOld.accA = vOld.accA * red;
		}

		if(vNew.accA > 100){
			vNew.accR = vNew.accR * red;
			vNew.accG = vNew.accG * red;
			vNew.accB = vNew.accB * red;
			vNew.accA = vNew.accA * red;
		}

	}else{
		vNew.accR += aColorOrig.r;
		vNew.accG += aColorOrig.g;
		vNew.accB += aColorOrig.b;
		vNew.accA += 1.0;
	}


	// { // RESET
	// 	vNew.accR = float((vNew.colors_orig & 0x0000FF) >> 0) / 255.0;
	// 	vNew.accG = float((vNew.colors_orig & 0x00FF00) >> 8) / 255.0;
	// 	vNew.accB = float((vNew.colors_orig & 0xFF0000) >> 16) / 255.0;
	// 	vNew.accA = 1.0;

	// 	vOld.accR = float((vOld.colors_orig & 0x0000FF) >> 0) / 255.0;
	// 	vOld.accG = float((vOld.colors_orig & 0x00FF00) >> 8) / 255.0;
	// 	vOld.accB = float((vOld.colors_orig & 0xFF0000) >> 16) / 255.0;
	// 	vOld.accA = 1.0;
	// }

	//vNew.accA = 2.0;

	vColor = vec3(
		vNew.accR / vNew.accA,
		vNew.accG / vNew.accA,
		vNew.accB / vNew.accA
	);
	
	vbo[vertexID] = vOld;
	vbo[gl_VertexID] = vNew;

	//vColor = vec3(0, 1, 0);



}



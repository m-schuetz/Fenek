#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;
layout(location = 2) in int aIndex;

uniform mat4 uWorldViewProj;

layout(binding = 0) uniform sampler2D uGradient;

out vec3 vColor;
out vec4 vVertexID;



vec3 getColorFromV1(){
	//vec2 range = vec2(10, 10000);

	//float w = (float(aValue) - range.x) / (range.y - range.x);
	//w = clamp(w, 0, 1);
	//vec3 c = texture(uGradient, vec2(w, 0.0)).rgb;

	//float w = float(aValue) * uScale.x + uOffset.x;
	//vec3 c = texture(uGradient, vec2(w, 0.0)).rgb;

	float w = intBitsToFloat(aValue);
	vec3 v = texture(uGradient, vec2(w, 0.0)).rgb;

	return v;
}

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
	//vColor = getColorFromV1();

	
	uint index = uint(aIndex);
	vVertexID = vec4(
		float((index >>  0) & 0xFF) / 255.0,
		float((index >>  8) & 0xFF) / 255.0,
		float((index >> 16) & 0xFF) / 255.0,
		float((index >> 24) & 0xFF) / 255.0
	);

	//{
	//	int low = 20 * 1000 * 1000;
	//	int hig = 100 * 1000 * 1000;

	//	//low = 62 * 1000 * 1000;
	//	//hig = 64 * 1000 * 1000;

	//	//low = 62880000;
	//	//hig = 62959000;

	//	float w = float(max(aIndex - low, 0));
	//	w = w / float(hig - low);

	//	//float w = vVertexID.a + vVertexID.z * 256 + vVertexID.y * 256 * 256;
	//	//w = w * 10;

	//	//float w = intBitsToFloat(aValue);


	//	vec3 v = texture(uGradient, vec2(w, 0.0)).rgb;

	//	vColor = v;

	//	if(w < 0){
	//		vColor = vec3(1, 0, 0);
	//	}else if(w > 1){
	//		vColor = vec3(0, 1, 0);
	//	}
	//}

}



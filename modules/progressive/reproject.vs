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
	w = clamp(w, 0, 1);
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
	gl_PointSize = 1.0;


	vColor = getColorFromV3();
	//vColor = getColorFromV1();


	//float gray = (vColor.x + vColor.y + vColor.z) / 3.0;

	//float gamma = 0.6;
	//vColor = pow(vColor, vec3(gamma));

	

	//vColor = 1.2 * vColor;
	

	//vColor = vec3(1, 1, 1);

	//float a = 0.3;
	//vColor = vec3(
	//	pow(vColor.r, a),
	//	pow(vColor.g, a),
	//	pow(vColor.b, a)
	//) + 1.0;

	
	uint index = uint(aIndex);
	vVertexID = vec4(
		float((index >>  0) & 0xFF) / 255.0,
		float((index >>  8) & 0xFF) / 255.0,
		float((index >> 16) & 0xFF) / 255.0,
		float((index >> 24) & 0xFF) / 255.0
	);

	// {
	// 	//float t = float(aIndex / 100) / 500000.0;
		
	// 	//float t = float(aIndex / 100) / 1800000.0 + 0.1;
	// 	float t = float(aIndex / 277) / 1000000;
	// 	//float t = float(aIndex / 100) / 900000.0 - 0.2;

	// 	vec3 c = texture(uGradient, vec2(t, 0.0)).xyz;
	// 	vColor = c;
	// }





	// {
		
	// 	//float t = float(aIndex / 13) / (1000 * 1000) - 0.2;
	// 	float t = float(aIndex / 1000) / (14.5 * 1000.0);

	// 	uint classes = aIndex / 100000;
	// 	t = float(classes) / 140;
		

	// 	vec3 c = texture(uGradient, vec2(t, 0.0)).xyz;
	// 	vColor = c;
	// }

}



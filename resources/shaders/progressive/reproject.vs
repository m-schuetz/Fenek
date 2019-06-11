#version 450 core

// RUNTIME GENERATED DEFINES

layout(location = 0) in vec3 aPosition;
layout(location = 1) in int aValue;
layout(location = 2) in int aIndex;

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



	//vec2 range = vec2(-400, 400);
	//vec2 range = vec2(700000, 100000);

	// Range
	//vec2 range = vec2(750619, 1004861);


	// Echo Ratio
	vec2 range = vec2(10, 10000);
	
	//vColor = aColor.rgb;
	float w = (float(aValue) - range.x) / (range.y - range.x);
	w = clamp(w, 0, 1);
	//vColor = texture(uGradient, vec2(w, 0.0)).rgb;

	// if(aValue < 0){
	// 	vColor = vec3(1, 0, 0);
	// }

	//vColor = vec3(1, 1, 1);
	
	uint index = uint(aIndex);
	vVertexID = vec4(
		float((index >>  0) & 0xFF) / 255.0,
		float((index >>  8) & 0xFF) / 255.0,
		float((index >> 16) & 0xFF) / 255.0,
		float((index >> 24) & 0xFF) / 255.0
	);

}



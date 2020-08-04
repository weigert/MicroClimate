#version 330 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec4 in_Color;
layout (location = 3) in float in_Cloud;

//Lighting
uniform vec3 lightPos;
uniform vec3 lookDir;

//Uniforms
uniform mat4 model;
uniform mat4 vp;
uniform mat4 dbmvp;

// We output the ex_Color variable to the next shader in the chain
//out vec4 ex_Color;
out vec3 ex_Model;
out vec3 ex_Normal;
//out vec2 ex_Position;
out vec4 ex_Shadow;
flat out float ex_Cloud;
/*
vec4 gouraud(){
	//Color Calculations - Per Vertex! Not Fragment.
	float diffuse = clamp(dot(in_Normal, normalize(lightPos)), 0.1, 0.9);
	float ambient = 0.1;
	float spec = 0.8*pow(max(dot(normalize(lookDir), normalize(reflect(lightPos, in_Normal))), 0.0), 32.0);

	return vec4(lightCol*lightStrength*(diffuse + ambient + spec), 1.0f);
}
*/

uniform float sealevel;

void main(void) {
	ex_Cloud = in_Cloud;
	//vec3 inPos = in_Position;
	ex_Model = (model * vec4(in_Position, 1.0f)).xyz;
	if(ex_Model.y < sealevel*15)
		ex_Model.y = sealevel*15;

	ex_Normal = in_Normal;
	ex_Shadow = dbmvp * vec4(ex_Model, 1.0f);
	gl_Position = vp * vec4(ex_Model, 1.0f);

	//ex_Color = in_Color;
	//ex_Position = ((gl_Position.xyz / gl_Position.w).xy * 0.5 + 0.5 );
}

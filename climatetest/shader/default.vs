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

out vdata{
	flat float cloud;
	vec3 normal;
	vec4 shadow;
	vec3 model;
} exv;

uniform float sealevel;
uniform bool cloudpass;

void main(void) {
	exv.cloud = in_Cloud;
	exv.model = (model * vec4(in_Position, 1.0f)).xyz;

	if(exv.model.y < sealevel*15){
		exv.model.y = sealevel*15;

	}

		if(cloudpass)
			exv.model.y += 5.0;


	exv.normal = in_Normal;
	exv.shadow= dbmvp * vec4(exv.model, 1.0f);
	gl_Position = vp * vec4(exv.model, 1.0f);

	//ex_Color = in_Color;
	//ex_Position = ((gl_Position.xyz / gl_Position.w).xy * 0.5 + 0.5 );
}

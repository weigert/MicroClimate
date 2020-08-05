#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in vdata{
	flat float cloud;
	vec3 normal;
	vec4 shadow;
	vec3 model;
} exv[];

vec4 explode(vec4 p, vec3 n){
  return p+0.1*vec4(normalize(n), 1.0);
}

out vdata2{
	flat float cloud;
	vec3 normal;
	vec4 shadow;
	vec3 model;
  vec3 y;
} exg;


void main() {
  exg.normal = exv[0].normal;
  exg.cloud = exv[0].cloud;
  exg.shadow = exv[0].shadow;
  exg.model = exv[0].model;
  exg.y = vec3(gl_in[0].gl_Position.z, gl_in[1].gl_Position.z, gl_in[2].gl_Position.z);

  gl_Position = gl_in[0].gl_Position;
  EmitVertex();
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();
  gl_Position = gl_in[2].gl_Position;


  EmitVertex();
  EndPrimitive();

}

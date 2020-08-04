#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vdata{
  vec3 normal;
} exv[];

out vec3 ex_Normal;

vec4 explode(vec4 p, vec3 n){
  return p+0.05*vec4(n, 1.0);
}

void main() {
  vec3 normal = exv[0].normal;
  ex_Normal = exv[0].normal;
  gl_Position = explode(gl_in[0].gl_Position, normal);
  EmitVertex();
  gl_Position = explode(gl_in[1].gl_Position, normal);
  EmitVertex();
  gl_Position = explode(gl_in[2].gl_Position, normal);
  EmitVertex();
  EndPrimitive();
}

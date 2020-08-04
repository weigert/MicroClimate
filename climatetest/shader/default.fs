#version 330 core
//Lighting Settings
vec3 lightCol = vec3(0.9);
uniform vec3 lightPos;
uniform vec3 lookDir;
uniform float lightStrength = 0.2;

//Sampler for the ShadowMap
uniform sampler2D shadowMap;

//IO
//in vec4 ex_Color;
in vec3 ex_Normal;
//in vec2 ex_Position;
in vec4 ex_Shadow;
in vec3 ex_Model;
out vec4 fragColor;

int viewmode = 0;

//Sample a grid..
float gridSample(int size){
  //Stuff
  float shadow = 0.0;
  float currentDepth = ex_Shadow.z;

  //Compute Bias
  float m = 1-dot(ex_Normal, normalize(lightPos));
  float bias = mix(0.002, 0.2*m, pow(m, 5));

  for(int x = -size; x <= size; ++x){
      for(int y = -size; y <= size; ++y){
          float pcfDepth = texture(shadowMap, ex_Shadow.xy + vec2(x, y) / textureSize(shadowMap, 0)).r;
          shadow += currentDepth - 0.001 > pcfDepth ? 1.0 : 0.0;
      }
  }
  //Normalize
  shadow/=12.0;
  return shadow;
}

vec4 shade(){
    //Shadow Value
    float shadow = 0.0;
    if(greaterThanEqual(ex_Shadow.xy, vec2(0.0f)) == bvec2(true) && lessThanEqual(ex_Shadow.xy, vec2(1.0f)) == bvec2(true))
      shadow = gridSample(1);

    //Sample the Shadow Value from Texture
    return vec4(vec3(1.0-shadow), 1.0f);
}

vec4 phong(){
  float diffuse = 2*max(dot(ex_Normal, normalize(lightPos)), 0.0);
  float ambient = 0.2;
  float spec = pow(max(dot(normalize(lookDir), reflect(lightPos, ex_Normal)), 0.0), 32.0);
  spec = 0.01 * spec;

  vec3 color = vec3(1);
  return vec4(color*(diffuse+ambient), 1.0f);
}

vec3 flatColor = vec3(0.40, 0.60, 0.25);
vec3 waterColor = vec3(0.17, 0.40, 0.44);

uniform float sealevel;

flat in float ex_Cloud;

void main(void) {

  if(ex_Model.y <= sealevel*15)
    fragColor = shade()*vec4(waterColor, 1.0);//phong()*vec4(color, 1.0);
  else
    fragColor = shade()*phong()*vec4(flatColor, 1.0);//phong()*vec4(color, 1.0);

    fragColor = mix(fragColor, vec4(1), ex_Cloud);//vec4(vec3(ex_Cloud), 1.0);

  if(length(ex_Model.xz) > 49 )
    discard;


//  if(viewmode == 1) //Normal Colors
  //  fragColor = vec4(ex_Normal, 1.0);
//  else{
//  }
}

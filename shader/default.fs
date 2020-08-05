#version 330 core
//Lighting Settings
vec3 lightCol = vec3(0.9);
uniform vec3 lightPos;
uniform vec3 lookDir;
uniform float lightStrength = 0.2;

//Sampler for the ShadowMap
uniform sampler2D shadowMap;

in vdata2{
	flat float cloud;
	vec3 normal;
	vec4 shadow;
	vec3 model;
  vec3 y;
} exg;

out vec4 fragColor;

int viewmode = 0;

uniform bool cloudpass;

//Sample a grid..
float gridSample(int size){
  //Stuff
  float shadow = 0.0;
  float currentDepth = exg.shadow.z;

  //Compute Bias
  float m = 1-dot(exg.normal, normalize(lightPos));
  float bias = mix(0.002, 0.2*m, pow(m, 5));

  for(int x = -size; x <= size; ++x){
      for(int y = -size; y <= size; ++y){
          float pcfDepth = texture(shadowMap, exg.shadow.xy + vec2(x, y) / textureSize(shadowMap, 0)).r;
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
    if(greaterThanEqual(exg.shadow.xy, vec2(0.0f)) == bvec2(true) && lessThanEqual(exg.shadow.xy, vec2(1.0f)) == bvec2(true))
      shadow = gridSample(1);

    //Sample the Shadow Value from Texture
    return vec4(vec3(1.0-shadow), 1.0f);
}

vec4 phong(){
  float diffuse = 2*max(dot(exg.normal, normalize(lightPos)), 0.0);
  float ambient = 0.2;
  float spec = pow(max(dot(normalize(lookDir), reflect(lightPos, exg.normal)), 0.0), 4.0);
  spec = 0.01 * spec;

  vec3 color = vec3(1);
  return vec4(color*(diffuse+ambient), 1.0f);
}

vec3 flatColor = vec3(0.40, 0.60, 0.25);
vec3 waterColor = vec3(0.17, 0.40, 0.44);
vec3 deepColor = vec3(0.17, 0.3, 0.3);

uniform float sealevel;

void main(void) {

  if(exg.model.y <= sealevel*15)
    fragColor = shade()*vec4(mix(waterColor,deepColor,0), 1.0);//phong()*vec4(color, 1.0);
  else
    fragColor = shade()*phong()*vec4(flatColor, 1.0);//phong()*vec4(color, 1.0);

  fragColor = mix(fragColor, vec4(vec3(0), 1), 0.8*exg.cloud);//vec4(vec3(ex_Cloud), 1.0);

  if(length(exg.model.xz) > 49 )
    discard;

	if(cloudpass)
		fragColor = vec4(vec3(1), exg.cloud);
}

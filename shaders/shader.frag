#version 330 core

// Inputs from vertex shader.
in vec2 UV;
in vec3 positionWorldspace;
in vec3 normalCameraspace;
in vec3 eyeDirectionCameraspace;
in vec3 lightDirectionCameraspace;
in vec4 shadowCoord;

// Output.
layout(location = 0) out vec3 color;

// Constant inputs.
uniform sampler2D thetexture;
uniform mat4 MV;
//uniform vec3 lightPositionWorldspace;
uniform sampler2DShadow shadowMap;

uniform vec3 material_ka;
uniform vec3 material_kd;
uniform vec3 material_ks;
uniform float material_shininess;


// Pre-computed poisson disk.
// opengl-tutorials.org.
vec2 poissonDisk[16] = vec2[](
   vec2(-0.94201624, -0.39906216),
   vec2(0.94558609, -0.76890725),
   vec2(-0.094184101, -0.92938870),
   vec2(0.34495938, 0.29387760),
   vec2(-0.91588581, 0.45771432),
   vec2(-0.81544232, -0.87912464),
   vec2(-0.38277543, 0.27676845),
   vec2(0.97484398, 0.75648379),
   vec2(0.44323325, -0.97511554),
   vec2(0.53742981, -0.47373420),
   vec2(-0.26496911, -0.41893023),
   vec2(0.79197514, 0.19090188),
   vec2(-0.24188840, 0.99706507),
   vec2(-0.81409955, 0.91437590),
   vec2(0.19984126, 0.78641367),
   vec2(0.14383161, -0.14100790)
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
  vec4 seed4 = vec4(seed,i);
  float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164,94.673));
  return fract(sin(dot_product) * 43758.5453);
}

void main(){

  // Light emission properties
  vec3 lightColor = vec3(1,1,1);
  float lightPower = 1.0f;

  // Material properties
  // TODO: Texture diffuse.
  //vec3 material_kd = vec3(0.8, 0, 0); //texture2D(thetexture, UV).rgb;
  //vec3 material_ka = vec3(0.1,0.1,0.1) * material_kd;
  //vec3 material_ks = vec3(0.3,0.3,0.3);

  // Distance to the light
  //float distance = length(lightPositionWorldspace - positionWorldspace);

  // Normal of the computed fragment, in camera space.
  vec3 n = normalize(normalCameraspace);
  // Direction of the light (from the fragment to the light)
  vec3 l = normalize(lightDirectionCameraspace);
  // Clamped Cosine of the angle between the normal and the light direction.
  float cosTheta = clamp(dot(n, l), 0 ,1);

  // Eye vector (towards the camera)
  vec3 E = normalize(eyeDirectionCameraspace);
  // Direction in which the triangle reflects the light
  //vec3 R = reflect(-l,n);

  // Clamped cosine of the angle between the Eye vector and the Reflect vector.
  //float cosAlpha = clamp(dot(E, R), 0, 1); // Phong.

  vec3 H = normalize(E + l); // Half-angle.
  float cosAlpha = clamp(dot(H, n), 0, 1); // Blinn-Phong.

  float visibility = 1.0;

  // Fixed bias.
  //float bias = 0.005;

  // Variable bias (based off of gradient).
  float bias = 0.005*tan(acos(cosTheta));
  bias = clamp(bias, 0,0.01);

  // Sample the shadow map n times.
  int num_samples = 4;
  float max_shadow_coverage = 1.0;
  float sample_shadow_coverage = max_shadow_coverage/num_samples;
  for (int i=0;i<num_samples;i++){
    // use either :
    //  - Always the same samples.
    //    Gives a fixed pattern in the shadow, but no noise
    int index = i;
    //  - A random sample, based on the pixel's screen location.
    //    No banding, but the shadow moves with the camera, which looks weird.
    // int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
    //  - A random sample, based on the pixel's position in world space.
    //    The position is rounded to the millimeter to avoid too much aliasing
    //int index = int(16.0*random(floor(positionWorldspace.xyz*1000.0), i))%16;

    // Lose visibility for each sample that is hidden in shadow.
    visibility -= sample_shadow_coverage*(1.0 - texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index]/700.0, (shadowCoord.z-bias)/shadowCoord.w) ));
  }

  // TODO: Spot lights.
  // if (texture(shadowMap, (shadowCoord.xy/shadowCoord.w)).z < (shadowCoord.z-bias)/shadowCoord.w)
  // if (textureProj(shadowMap, shadowCoord.xyw).z < (shadowCoord.z-bias)/shadowCoord.w)

  // TODO: SSAO.

  // Phong shading.
  color = material_ka
    + visibility * material_kd * lightColor * lightPower * cosTheta // Diffuse.
    + visibility * material_ks * lightColor * lightPower * pow(cosAlpha, material_shininess); // Specular.
}

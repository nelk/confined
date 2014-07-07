#version 330 core

// Inputs from vertex shader.
in vec2 texUV;

// Output.
layout(location = 0) out vec3 color;

// Constant inputs.
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture; // TODO - check shadow, normal samplers?
uniform sampler2D depthTexture;
uniform sampler2DShadow shadowMap;

uniform vec3 lightPositionWorldspace;
uniform vec3 lightDirectionWorldspace;
uniform vec3 lightColour;
uniform vec3 lightAmbience;
uniform vec3 lightFalloff;

uniform vec3 cameraPositionWorldspace;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 depthBiasVP;

//uniform vec3 material_ka;
//uniform vec3 material_kd;
//uniform vec3 material_ks;
//uniform float material_shininess;


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

vec2 offsetTable[4] = vec2[](
  vec2(-1.5, 0.5),
  vec2(0.5, 0.5),
  vec2(-1.5, -1.5),
  vec2(0.5, -1.5)
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
  vec4 seed4 = vec4(seed, i);
  float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
  return fract(sin(dot_product) * 43758.5453);
}

void main(){

  // Material properties
  // TODO: Texture diffuse.
  vec3 material_kd = texture2D(diffuseTexture, texUV).rgb;

  // Don't write pixels that weren't actually drawn onto this texture.
  if (material_kd == vec3(0, 0, 0))
    discard;

  vec3 material_ka = lightAmbience * material_kd;
  vec3 material_ks = vec3(0.5, 0.5, 0.5);
  float material_shininess = 96.0;

  float x = texUV.x;
  float y = texUV.y;
  float z = texture2D(depthTexture, texUV).r;

  vec4 vertexPositionScreenspace = vec4(vec3(x, y, z) * 2.0 - 1.0, 1); // Clip space.
  vec4 vertexPositionCameraspace = inverse(P) * vertexPositionScreenspace;
  //vertexPositionCameraspace = vertexPositionCameraspace / vertexPositionCameraspace.w;

  vec4 vertexPositionWorldspace = inverse(V) * vertexPositionCameraspace;
  vertexPositionWorldspace = vertexPositionWorldspace / vertexPositionWorldspace.w;

  vec4 shadowCoord = depthBiasVP * vertexPositionWorldspace;

  // Vector that goes from the vertex to the camera, in camera space.
  vec3 eyeDirectionCameraspace = -vertexPositionCameraspace.xyz / vertexPositionCameraspace.w;

  // Vector that goes from the vertex to the light, in camera space
  vec3 lightDirectionCameraspace = (V * vec4(lightDirectionWorldspace, 0)).xyz;

  // Distance to the light
  float lightDist = length(lightPositionWorldspace - vertexPositionWorldspace.xyz);
  float attenuation = 1.0/dot(lightFalloff, vec3(1, lightDist, lightDist*lightDist));

  // Normal of the computed fragment, in camera space.
  vec3 n = texture2D(normalTexture, texUV).rgb * 2.0 - 1.0;
  // Direction of the light (from the fragment to the light)
  vec3 l = normalize(lightDirectionCameraspace);


  // Clamped Cosine of the angle between the normal and the light direction.
  float cosTheta = clamp(dot(n, l), 0 ,1);

  // Eye vector (towards the camera)
  vec3 E = normalize(eyeDirectionCameraspace);
  // Direction in which the triangle reflects the light
  //vec3 R = reflect(l, n);

  // Clamped cosine of the angle between the Eye vector and the Reflect vector.
  //float cosAlpha = clamp(dot(E, R), 0, 1); // Phong.

  vec3 H = normalize(E + l); // Half-angle.
  float cosAlpha = clamp(dot(H, n), 0, 1); // Blinn-Phong.

  float shadowCoefficient = 0.0;

  // Fixed bias.
  //float bias = 0.005;

  // Variable bias (based off of gradient).
  float bias = 0.005 * tan(acos(cosTheta));
  bias = clamp(bias, 0, 0.01);

  /*vec2 offset = vec2(fract(texUV.x * 0.5) > 0.25,*/
                     /*fract(texUV.y * 0.5) > 0.25);  // mod*/
  /*offset.y += offset.x;  // y ^= x in floating point*/
  /*if (offset.y > 1.1)*/
    /*offset.y = 0;*/

  // Sample the shadow map n times.
  int num_samples = 16;
  for (int i = 0; i < num_samples; i++){
    // use either :
    //  - Always the same samples.
    //    Gives a fixed pattern in the shadow, but no noise
    int index = i;
    //  - A random sample, based on the pixel's screen location.
    //    No banding, but the shadow moves with the camera, which looks weird.
    //int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
    //  - A random sample, based on the pixel's position in world space.
    //    The position is rounded to the millimeter to avoid too much aliasing
    //int index = int(16.0 * random(floor(vertexPositionWorldspace.xyz * 1000.0), i)) % 16;

    // Lose visibility for each sample that is hidden in shadow.
    //visibility -= sample_shadow_coverage * (1.0 - texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0, (shadowCoord.z - bias) / shadowCoord.w)));


  //vec2 windowOffset = vec2(i%4*3-1.5, i/4*3-1.5);
  //shadowCoefficient += texture(shadowMap, vec3(shadowCoord.xy + (offset + windowOffset) / 2048.0, (shadowCoord.z - bias)/shadowCoord.w));
  //shadowCoefficient += texture(shadowMap, vec3(shadowCoord.xy + (offset + offsetTable[index]) / 700.0, (shadowCoord.z - bias)/shadowCoord.w));
  shadowCoefficient += texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0, (shadowCoord.z - bias)/shadowCoord.w));

    //visibility -= sample_shadow_coverage * (1.0 - texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0, (shadowCoord.z - bias) / shadowCoord.w))); // Best

    //visibility -= sample_shadow_coverage * (1.0 - shadowCoord.z/shadowCoord.w * texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w)));
    //visibility -= sample_shadow_coverage * (1.0 - textureProj(shadowMap, shadowCoord));

    /*float indexAngle = i*3.14/num_samples;*/
    /*vec2 sampleOffset = vec2(cos(indexAngle), sin(indexAngle));*/
    /*bool isShadowed = texture2D(shadowMap, shadowCoord.xy + sampleOffset / 400.0).r > (shadowCoord.z - bias) / shadowCoord.w;*/
    /*visibility -= sample_shadow_coverage * (1.0 - float(isShadowed));*/
  }

  float visibility = shadowCoefficient / num_samples;


  // TODO: Spot lights.
  // if (texture(shadowMap, (shadowCoord.xy/shadowCoord.w)).z < (shadowCoord.z-bias)/shadowCoord.w)
  // if (textureProj(shadowMap, shadowCoord.xyw).z < (shadowCoord.z-bias)/shadowCoord.w)

  // TODO: SSAO.

  color = material_ka
    + visibility * lightColour * attenuation
    * (
      material_kd * cosTheta // Diffuse.
    + material_ks * pow(cosAlpha, material_shininess) // Specular.
    );
}

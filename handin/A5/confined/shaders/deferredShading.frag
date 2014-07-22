#version 330 core

// Inputs from vertex shader.
in vec2 texUV;

// Output.
layout(location = 0) out vec3 colour;

// Constant inputs.

// Texture samplers.
uniform sampler2D diffuseTexture; // Albedo.
uniform sampler2D specularTexture; // Specular, Shininess.
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2DShadow shadowMap;
uniform samplerCubeShadow shadowMapCube;
uniform sampler2D ssaoNoiseTexture;

uniform vec3 lightPositionWorldspace;
uniform vec3 lightDirectionWorldspace;
uniform int lightType; // 0 = directional, 1 = spot, 2 = point.
uniform vec3 lightColour;
uniform vec3 lightAmbience;
uniform vec3 lightFalloff;
uniform float lightSpreadDegrees;

uniform vec3 cameraPositionWorldspace;
uniform mat4 P;
uniform mat4 V;
uniform mat4 shadowmapDepthBiasVP;

uniform bool useDiffuse = true;
uniform bool useSpecular = true;
uniform bool useShadow = true;
uniform bool useSSAO = true;
uniform vec3 ssaoKernel[4];

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

float SSAO(mat3 kernelBasis, vec3 originPos, float cmpDepth, float radius) {
  float occlusion = 0.0;
  for (int i = 0; i < 4; i++) {
    // Get sample position.
    vec3 samplePos = kernelBasis * ssaoKernel[i];
    samplePos = samplePos * radius + originPos;

    // Project sample position.
    vec4 offset = P * vec4(samplePos, 1.0);
    offset.xy /= offset.w; // Only need xy.
    offset.xy = offset.xy * 0.5 + 0.5; // Scale/bias to texcoords.

    float sampleDepth = texture(depthTexture, offset.xy).r;
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(cmpDepth - sampleDepth));

    occlusion += rangeCheck * step(sampleDepth, cmpDepth);
  }

  //occlusion = 1.0 - (occlusion / float(4));
  //return pow(occlusion, uPower);
  return occlusion / 4.0;
}

void main(){

  // Material properties
  vec3 material_kd;
  if (useDiffuse) {
    material_kd = texture2D(diffuseTexture, texUV).rgb;
  } else {
    material_kd = vec3(0, 0, 0);
  }
  vec3 material_emissive = texture2D(emissiveTexture, texUV).rgb;

  // Don't write pixels that weren't actually drawn onto this texture.
  if (material_kd == vec3(0, 0, 0) && material_emissive == vec3(0, 0, 0)) {
    colour = vec3(0, 0, 0);
    return;
  }

  vec3 material_ks;
  float material_shininess;
  if (useSpecular) {
    vec4 material_ks_shininess = texture2D(specularTexture, texUV).rgba;
    material_ks = material_ks_shininess.rgb;
    material_shininess = material_ks_shininess.a * 200.0;
  } else {
    material_ks = vec3(0, 0, 0);
    material_shininess = 0;
  }

  vec3 lightFalloffModified = lightFalloff;
  vec3 material_ka = lightAmbience * material_kd;

  float directLightToEyeIntensity = 0;

  float x = texUV.x;
  float y = texUV.y;
  float z = texture2D(depthTexture, texUV).r;

  vec4 vertexPositionScreenspace = vec4(vec3(x, y, z) * 2.0 - 1.0, 1); // Clip space.
  vec4 vertexPositionCameraspace = inverse(P) * vertexPositionScreenspace;
  vertexPositionCameraspace = vertexPositionCameraspace / vertexPositionCameraspace.w;

  vec4 vertexPositionWorldspace = inverse(V) * vertexPositionCameraspace;
  //vertexPositionWorldspace = vertexPositionWorldspace / vertexPositionWorldspace.w;

  // Vector that goes from the vertex to the camera, in camera space.
  vec3 eyeDirectionCameraspace = -vertexPositionCameraspace.xyz / vertexPositionCameraspace.w;

  vec3 lightPositionCameraspace = (V * vec4(lightPositionWorldspace, 1.0)).xyz;
  vec3 vertexPositionToLightPositionWorldspace = lightPositionWorldspace - vertexPositionWorldspace.xyz;

  // Vector in the direction light is facing, in camera space
  vec3 lightDirectionCameraspace = (V * vec4(lightDirectionWorldspace, 0)).xyz;
  // Normal of the computed fragment, in camera space.
  vec3 n = texture2D(normalTexture, texUV).rgb * 2.0 - 1.0;
  vec3 l = vec3(0, 0, 0); // Direction of the light (from the fragment to the light) in camera space;

  // Eye vector (towards the camera)
  vec3 E = normalize(eyeDirectionCameraspace);
  float lightSpreadRadians = radians(lightSpreadDegrees);

  switch (lightType) {
    case 0:
      // For directional light just reverse light direction.
      l = -normalize(lightDirectionCameraspace);
      break;
    case 1:
      // For spot light and point light use point's position.
      // This is wrong!: l = normalize((V * vec4(vertexPositionToLightPositionWorldspace, 1.0)).xyz);
      l = normalize(lightPositionCameraspace - vertexPositionCameraspace.xyz);


//dot(normalize(vertexPositionToLightPositionWorldspace), normalize(lightDirectionWorldspace))

      float dotVertexLightPos = dot(normalize(vertexPositionCameraspace.xyz), normalize(lightPositionCameraspace));

      float coneAngleFactor =
        clamp(normalize(lightDirectionCameraspace).z, 0, 1)
        *
        clamp(-normalize(lightPositionCameraspace).z, 0, 1)
        *
        clamp(dotVertexLightPos, 0, 1);
/*
        clamp(dot(vec3(0, 0, -1), normalize(-lightDirectionCameraspace)), 0, 1)
        *
        clamp(dot(vec3(0, 0, -1), normalize(lightPositionCameraspace)), 0, 1)
        *
        clamp(dot(normalize(vertexPositionCameraspace.xyz), normalize(lightPositionCameraspace)), 0, 1);
*/

// TODO: Visibility check with cone - intersect ray with cone and compare depth with fragment depth.
      directLightToEyeIntensity =
        //step(acos(dot(normalize(vertexPositionWorldspace.xyz - lightPositionWorldspace), normalize(lightDirectionWorldspace))), lightSpreadRadians) *
        pow(coneAngleFactor, 8); // TODO: Factor in spread angle?

      break;
    case 2:
      // For spot light and point light use point's position.
      // This is wrong!: l = normalize((V * vec4(vertexPositionToLightPositionWorldspace, 1.0)).xyz);
      l = normalize(lightPositionCameraspace - vertexPositionCameraspace.xyz);

      directLightToEyeIntensity =
        step(vertexPositionCameraspace.z, lightPositionCameraspace.z)
        * (
          //0.2*dot(normalize(lightPositionCameraspace), -E)
          //+
          1.2*pow(clamp(dot(normalize(vertexPositionCameraspace.xyz), normalize(lightPositionCameraspace)), 0, 1), 1000)
        );

      break;
  }

  // Clamped Cosine of the angle between the normal and the light direction.
  float cosTheta = clamp(dot(n, l), 0, 1);

  // Direction in which the triangle reflects the light
  //vec3 R = reflect(l, n);

  // Clamped cosine of the angle between the Eye vector and the Reflect vector.
  //float cosAlpha = clamp(dot(E, R), 0, 1); // Phong.

  vec3 H = normalize(E + l); // Half-angle.
  float cosAlpha = clamp(dot(H, n), 0, 1); // Blinn-Phong.

  float visibility = 0.0;

  if (!useShadow) {
    visibility = 1.0;
  } else {
    vec4 shadowCoord = shadowmapDepthBiasVP * vertexPositionWorldspace;

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
    int num_samples = 1; // TODO: Up samples for some light types.
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

    switch (lightType) {
      case 0: // Directional light.
        visibility += texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0, (shadowCoord.z - bias)/shadowCoord.w));
        break;

      case 1: // Spot light.
        float coneAngle = acos(dot(-normalize(vertexPositionToLightPositionWorldspace), normalize(lightDirectionWorldspace)));

  /*
        // Hard circle.
        if (coneAngle <= lightSpreadRadians) {
          visibility += texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w));
        }
  */

        // Quadratic falloff by angle.
        float distFrac = coneAngle/lightSpreadRadians;
        if (distFrac <= 1.0) {
          visibility += /*step(-1.0, -distFrac) * */ (1 - distFrac/2.5) * (1 - distFrac) * texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w));
          //lightFalloffModified.z += distFrac;
        }

        // Exponential light decay by angle.
        //visibility += (2 - pow(2, coneAngle/lightSpreadRadians)) * texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w));

        // Staged falloff.
  /*
        float distFrac = coneAngle/lightSpreadRadians;
        if (distFrac <= 1.0) {
          visibility += texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w));;
          if (distFrac < 0.3) {
            // Nothing extra.
          } else if (distFrac < 0.7) {
            lightFalloffModified.x += 1.0;
            //lightFalloffModified.y += 1.0;
            //lightFalloffModified.z += 1.0;
          } else {
            lightFalloffModified.x += 2.0;
            //lightFalloffModified.y += 2.0;
            //lightFalloffModified.z += 3.0;
          }
        }
  */
        break;

      case 2: // Point light.
        // Turn world space vector to depth value to compare with shadow map ortho depth.
        vec3 absvec = abs(vertexPositionToLightPositionWorldspace);
        vec3 v = vertexPositionToLightPositionWorldspace;
        float lzc = max(abs(v.x), max(abs(v.y), abs(v.z)));
        float f = 500;
        float n = 1.0;
        float nzc = (f+n)/(f-n) - (2*f*n)/(f-n)/lzc;
        float v2dv = (nzc + 1.0) * 0.5;

        visibility += texture(shadowMapCube, vec4(shadowCoord.xyz/shadowCoord.w, v2dv - bias - 0.002));

        break;
    }

      //visibility -= sample_shadow_coverage * (1.0 - texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0, (shadowCoord.z - bias) / shadowCoord.w))); // Best

      //visibility -= sample_shadow_coverage * (1.0 - shadowCoord.z/shadowCoord.w * texture(shadowMap, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w)));
      //visibility -= sample_shadow_coverage * (1.0 - textureProj(shadowMap, shadowCoord));

      /*float indexAngle = i*3.14/num_samples;*/
      /*vec2 sampleOffset = vec2(cos(indexAngle), sin(indexAngle));*/
      /*bool isShadowed = texture2D(shadowMap, shadowCoord.xy + sampleOffset / 400.0).r > (shadowCoord.z - bias) / shadowCoord.w;*/
      /*visibility -= sample_shadow_coverage * (1.0 - float(isShadowed));*/
    }

    visibility = visibility / num_samples;
  }

  float lightDist = length(vertexPositionToLightPositionWorldspace);
  float attenuation = 1.0 / dot(lightFalloffModified, vec3(1, lightDist, lightDist*lightDist));



  // SSAO.
  float ambientOcclusion = 0.0;
  if (useSSAO) {
    vec2 noiseTexCoords = texUV * vec2(textureSize(depthTexture, 0)) / vec2(4.0, 4.0);
    // Kernel basis matrix.
    vec3 rvec = texture(ssaoNoiseTexture, noiseTexCoords).rgb * 2.0 - 1.0;

    // Gram-Schmidt.
    vec3 tangent = normalize(rvec - n * dot(rvec, n));
    vec3 bitangent = cross(tangent, n);
    mat3 kernelBasis = mat3(tangent, bitangent, n);

    ambientOcclusion = SSAO(kernelBasis, vertexPositionCameraspace.xyz, z, 1.5);
  }

  colour = material_emissive                 // Emissive.
    + material_ka * (1.0 - ambientOcclusion) // Ambient.
    + lightColour * attenuation
    * (
       visibility
       * (
         material_kd * cosTheta                 // Diffuse.
       + material_ks * pow(cosAlpha, material_shininess) // Specular.
       )
   + directLightToEyeIntensity // Direct light.
   )
  ;

  // Visualize normals in camera space.
  //colour = n * 0.5 + 0.5;

  // Visualize angle with eye direction.
  //colour = vec3(cosTheta, 0, 0);

  // Visualize speculars only.
  //colour = vec3(1, 1, 1) * pow(cosAlpha, material_shininess);
  //colour = vec3(1, 1, 1) * pow(cosAlpha, 400);

  //colour = vec3(pow(clamp(dot(E, R), 0, 1), 100), 0, 0); // Phong.
}

#version 330 core

// Inputs from vertex shader.
in vec2 UV;
in vec3 normalCameraspace;
in vec3 tangentCameraspace;
in vec3 bitangentCameraspace;

// Output.
layout(location = 0) out vec3 outDiffuse;
layout(location = 1) out vec4 outSpecular; // Includes shininess in alpha (0-100).
layout(location = 2) out vec3 outEmissive;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float gl_FragDepth;

// Texture samplers.
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

uniform bool useDiffuseTexture;
uniform bool useNormalTexture;
uniform vec3 material_kd;
uniform vec3 material_ks;
uniform float material_shininess;
uniform vec3 material_emissive;

void main() {
  // Output Diffuse colour.
  if (useDiffuseTexture) {
    outDiffuse = texture2D(diffuseTexture, UV).rgb;
  } else {
    outDiffuse = material_kd;
  }

  outSpecular = vec4(material_ks, material_shininess/200.0);
  outEmissive = material_emissive;

  //vec3 normalCameraspace2 = (normalize(normalCameraspace) + 1.0) / 2.0; // Shift normal to be positive.

  if (useNormalTexture) {
    // Gram-Schmidt, in camera space.
    vec3 tangent = normalize(tangentCameraspace); //normalize(tangentCameraspace - normalCameraspace * dot(normalCameraspace, tangentCameraspace));
    vec3 bitangent = normalize(bitangentCameraspace); //cross(normalCameraspace, tangent);

    // Represents tangent space -> camera space.
    mat3 tbn = mat3(tangent, bitangent, normalize(normalCameraspace));

    // This isn't working...
    outNormal = tbn * (texture2D(normalTexture, UV).xyz * 2.0 - 1.0);
  } else {
    outNormal = normalCameraspace;
  }
  outNormal = (outNormal + 1.0)/2.0; // Shift to fit into texture colour.

  gl_FragDepth = gl_FragCoord.z;
}

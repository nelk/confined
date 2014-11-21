#version 330 core

// Inputs from vertex shader.
in vec2 UV_perspective;
noperspective in vec2 UV_noperspective;
in vec3 positionModelspace;
in vec3 normalCameraspace;
in vec3 tangentCameraspace;
in vec3 bitangentCameraspace;

// Output.
layout(location = 0) out vec3 outDiffuse;
layout(location = 1) out vec4 outSpecular; // Includes shininess in alpha (0-100).
layout(location = 2) out vec3 outEmissive;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float outPicking;
out float gl_FragDepth;

// Texture samplers.
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

uniform bool useDiffuseTexture;
uniform bool useNormalTexture;
uniform vec3 material_kd;
uniform vec3 material_ks;
uniform float material_shininess;
uniform vec3 material_emissive;

uniform vec3 halfspacePoint; // Model space.
uniform vec3 halfspaceNormal; // (0, 0, 0) means don't do test.
uniform bool useNoPerspectiveUVs = false;

uniform int meshId;

void main() {
  // Check if in halfspace.
  if (halfspaceNormal != vec3(0, 0, 0) && dot((positionModelspace - halfspacePoint), halfspaceNormal) <= 0) {
    discard;
  }

  vec2 UV;
  // TODO: Make this uncessary by un-perspective dividing on CPU?
  if (useNoPerspectiveUVs) {
    UV = UV_noperspective;
  } else {
    UV = UV_perspective;
  }

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
    // TODO: Why does negative improve this?
    vec3 bitangent = -normalize(bitangentCameraspace); //cross(normalCameraspace, tangent);

    // Represents tangent space -> camera space.
    mat3 tbn = mat3(tangent, bitangent, normalize(normalCameraspace));

    outNormal = tbn * (texture2D(normalTexture, UV).xyz * 2.0 - 1.0);
  } else {
    outNormal = normalize(normalCameraspace);
  }
  outNormal = (outNormal + 1.0)/2.0; // Shift to fit into texture colour.

  outPicking = meshId/65536.0;

  gl_FragDepth = gl_FragCoord.z;
}

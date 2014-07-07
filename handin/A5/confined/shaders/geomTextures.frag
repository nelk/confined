#version 330 core

// Inputs from vertex shader.
in vec2 UV;
in vec3 normalCameraspace;

// Output.
layout(location = 0) out vec3 outDiffuse;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out float gl_FragDepth;

// TODO: texture colour or material ID.
uniform vec3 material_kd;


void main(){
  // TODO: Texture diffuse.

  // Output render targets.
  outDiffuse = material_kd;
  outNormal = (normalize(normalCameraspace) + 1.0) / 2.0; // Shift normal to be positive.
  gl_FragDepth = gl_FragCoord.z;
}

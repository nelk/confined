#version 330 core

// Inputs.
layout(location = 0) in vec3 vertexPositionModelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormalModelspace;

// Interpolated outputs.
out vec2 UV;
out vec3 normalCameraspace;
out vec3 eyeDirectionCameraspace;

// Constant inputs.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main(){
  gl_Position = MVP * vec4(vertexPositionModelspace, 1);

  // Normal of the the vertex, in camera space.
  // Only correct if ModelMatrix does not scale the model, use its inverse transpose if not.
  normalCameraspace = (V * M * vec4(vertexNormalModelspace, 0)).xyz;

  UV = vertexUV;
}


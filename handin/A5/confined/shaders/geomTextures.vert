#version 330 core

// Inputs.
layout(location = 0) in vec3 vertexPositionModelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormalModelspace;
layout(location = 3) in vec3 vertexTangentModelspace;
layout(location = 4) in vec3 vertexBitangentModelspace;

// Interpolated outputs.
out vec2 UV_perspective;
noperspective out vec2 UV_noperspective;
out vec3 positionModelspace;
out vec3 normalCameraspace;
out vec3 tangentCameraspace;
out vec3 bitangentCameraspace;
out vec3 eyeDirectionCameraspace;

// Constant inputs.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main(){
  gl_Position = MVP * vec4(vertexPositionModelspace, 1);
  positionModelspace = vertexPositionModelspace;

  // Normal of the the vertex, in camera space.
  // Only correct if ModelMatrix does not scale the model, use its inverse transpose if not.
  // TODO: Just send in MV...
  normalCameraspace = (V * M * vec4(vertexNormalModelspace, 0)).xyz;
  tangentCameraspace = (V * M * vec4(vertexTangentModelspace, 0)).xyz;
  bitangentCameraspace = (V * M * vec4(vertexBitangentModelspace, 0)).xyz;

  UV_perspective = vertexUV;
  UV_noperspective = vertexUV;
}


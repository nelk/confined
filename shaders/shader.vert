#version 330 core

// Inputs.
layout(location = 0) in vec3 vertexPositionModelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormalModelspace;

// Interpolated outputs.
out vec2 UV;
out vec3 positionWorldspace;
out vec3 normalCameraspace;
out vec3 eyeDirectionCameraspace;
out vec3 lightDirectionCameraspace;
out vec4 shadowCoord;

// Constant inputs.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightDirectionWorldspace;
uniform mat4 depthBiasMVP;

void main(){
  gl_Position =  MVP * vec4(vertexPositionModelspace, 1);

  shadowCoord = depthBiasMVP * vec4(vertexPositionModelspace, 1);

  positionWorldspace = (M * vec4(vertexPositionModelspace, 1)).xyz;

  // Vector that goes from the vertex to the camera, in camera space.
  eyeDirectionCameraspace = vec3(0,0,0) - (V * M * vec4(vertexPositionModelspace,1)).xyz;

  // Vector that goes from the vertex to the light, in camera space
  lightDirectionCameraspace = (V*vec4(lightDirectionWorldspace,0)).xyz;

  // Normal of the the vertex, in camera space.
  // Only correct if ModelMatrix does not scale the model, use its inverse transpose if not.
  normalCameraspace = (V * M * vec4(vertexNormalModelspace, 0)).xyz;

  UV = vertexUV;
}


#version 330 core

// Inputs.
layout(location = 0) in vec3 quadPositionModelspace;

// Interpolated outputs.
out vec2 texUV;

void main(){
  gl_Position = vec4(quadPositionModelspace, 1);
  texUV = (quadPositionModelspace.xy + 1.0) / 2.0;
}


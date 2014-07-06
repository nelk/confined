#version 330 core

layout(location = 0) in vec3 vertexPositionModelspace;

out vec2 UV;

void main(){
  gl_Position = vec4(vertexPositionModelspace, 1);
  UV = (vertexPositionModelspace.xy + vec2(1, 1)) / 2.0;
}


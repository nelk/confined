#version 330 core

// Ouput data
layout(location = 0) out vec3 colour;

uniform sampler2D tex;
uniform bool useBlur;
uniform bool useMotionBlur;

in vec2 UV;

void main(){
  if (useBlur) {
    int blurSize = 4;
    vec2 texelSize = 1.0 / vec2(textureSize(tex, 0));
    colour = vec3(0, 0, 0);
    vec2 startOffset = vec2(float(1 - blurSize) / 2.0);
    for (int i = 0; i < blurSize; ++i) {
      for (int j = 0; j < blurSize; ++j) {
        vec2 offset = (startOffset + vec2(float(i), float(j))) * texelSize;
        colour += texture2D(tex, UV + offset).rgb;
      }
    }
    colour = colour / float(blurSize * blurSize);
  } else {
    colour = texture2D(tex, UV).rgb;
  }
}


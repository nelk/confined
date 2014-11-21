#version 330 core

// Ouput data
layout(location = 0) out vec3 colour;

uniform sampler2D tex;
uniform sampler2D depthTexture;
uniform sampler2D pickingTexture;

uniform bool useBlur;
uniform bool useMotionBlur;
uniform bool shudder;
uniform float currentTime;
uniform mat4 newToOldMatrix;
uniform float fpsCorrection = 1.0;
uniform int selectedMeshId;

in vec2 UV;

bool isSelected(vec2 UVCoord) {
  return selectedMeshId != 0 && abs(texture2D(pickingTexture, UVCoord).r * 65536.0 - selectedMeshId) < 0.01;
}

void main(){
  // Reasonable viewing spectrum for pick ids.
  // colour = texture2D(pickingTexture, UV).rgb * 65536.0/50.0;

  // Crosshair.
  vec2 texSize = textureSize(tex, 0);
  vec2 crossHairWidth = 2/texSize;
  vec2 crossHairLength = 20/texSize;
  if ((abs(UV.x - 0.5) <= crossHairWidth.x && abs(UV.y - 0.5) <= crossHairLength.y) || (abs(UV.y - 0.5) <= crossHairWidth.y && abs(UV.x - 0.5) <= crossHairLength.x)) {
    colour = vec3(0, 0, 0);
    return;
  }

  // Craziness.
  vec2 crazyOffset = vec2(0, 0);
  // Wobble.
  //crazyOffset = vec2(sin(10.0*UV.y) * sin(10.0*currentTime)/100.0, 0);
  // Faster Wobble.
  //crazyOffset = vec2(sin(30.0*UV.y) * sin(20.0*currentTime)/180.0, 0);
  // Shudder.
  if (shudder) {
    crazyOffset = vec2(sin(120.0*currentTime)/250.0, 0);
  }
  // Feed.
  //crazyOffset = vec2(0, currentTime);

  if (useBlur) {
    int blurSize = 4;
    vec2 texelSize = 1.0 / vec2(textureSize(tex, 0));
    colour = vec3(0, 0, 0);
    vec2 startOffset = vec2(float(1 - blurSize) / 2.0);
    for (int i = 0; i < blurSize; ++i) {
      for (int j = 0; j < blurSize; ++j) {
        vec2 offset = (startOffset + vec2(float(i), float(j))) * texelSize;
        colour += texture2D(tex, UV + offset + crazyOffset).rgb;
      }
    }
    colour = colour / float(blurSize * blurSize);
  } else {
    colour = texture2D(tex, UV + crazyOffset).rgb;
  }

  // TODO: How to mix blur and motion blur?
  if (useMotionBlur) {
    // TODO.
    float x = UV.x;
    float y = UV.y;
    float z = texture2D(depthTexture, UV).r;

    vec4 vertexPositionScreenspace = vec4(vec3(x, y, z) * 2.0 - 1.0, 1); // Clip space.
    vec4 oldVertexPositionScreenspace = newToOldMatrix * vertexPositionScreenspace;
    oldVertexPositionScreenspace /= oldVertexPositionScreenspace.w;

    vec2 blurVector = (oldVertexPositionScreenspace.xy - vertexPositionScreenspace.xy) * fpsCorrection;

    // perform blur.
    int numMotionSamples = 10;
    for (int i = 1; i < numMotionSamples; ++i) {
      // get offset in range [-0.5, 0.5]:
      vec2 offset = blurVector * (float(i) / float(numMotionSamples - 1) - 0.5);

      // Sample & add to result.
      colour += texture2D(tex, UV + offset + crazyOffset).rgb;
    }

    colour /= float(numMotionSamples);
  }

  if (isSelected(UV)) {
    colour += vec3(0.2, 0.2, 0.0);
  }
}


#ifndef SHADER_INSTANCES_HPP
#define SHADER_INSTANCES_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.hpp"
#include "material.hpp"

namespace shaders {

class GeomTexturesVertShader: public VertexShader {
public:
  GeomTexturesVertShader(): VertexShader("shaders/geomTextures.vert") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_IN_VBO_VEC3(vertexPositionModelspace, 0);
  SHADER_IN_VBO_VEC2(vertexUV, 1);
  SHADER_IN_VBO_VEC3(vertexNormalModelspace, 2);
  SHADER_IN_VBO_VEC3(vertexTangentModelspace, 3);
  SHADER_IN_VBO_VEC3(vertexBitangentModelspace, 4);

  SHADER_DRAW_TRIANGLE_ELEMENTS();

  SHADER_UNIFORM_MAT4(MVP);
  SHADER_UNIFORM_MAT4(M);
  SHADER_UNIFORM_MAT4(V);
  SHADER_UNIFORM_VEC3(halfspacePoint);
  SHADER_UNIFORM_VEC3(halfspaceNormal);
};


class GeomTexturesFragShader: public FragmentShader {
public:
  GeomTexturesFragShader(): FragmentShader("shaders/geomTextures.frag") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_SAMPLER2D(diffuseTexture, 0);
  SHADER_UNIFORM_SAMPLER2D(normalTexture, 1);

  SHADER_UNIFORM_BOOL(useDiffuseTexture);
  SHADER_UNIFORM_BOOL(useNormalTexture);
  SHADER_UNIFORM_VEC3(material_kd);
  SHADER_UNIFORM_VEC3(material_ks);
  SHADER_UNIFORM_FLOAT(material_shininess);
  SHADER_UNIFORM_VEC3(material_emissive);

  SHADER_UNIFORM_VEC3(halfspacePoint);
  SHADER_UNIFORM_VEC3(halfspaceNormal);
  SHADER_UNIFORM_BOOL(useNoPerspectiveUVs);

  SHADER_UNIFORM_INT(meshId);

  SHADER_OUT_COLOR_ATTACHMENT(outDiffuse, 0);
  SHADER_OUT_COLOR_ATTACHMENT(outSpecular, 1);
  SHADER_OUT_COLOR_ATTACHMENT(outEmissive, 2);
  SHADER_OUT_COLOR_ATTACHMENT(outNormal, 3);
  SHADER_OUT_COLOR_ATTACHMENT(outPicking, 4);
  SHADER_OUT_DEPTH_ATTACHMENT(gl_FragDepth);
};

class PassThroughVert: public VertexShader {
public:
  PassThroughVert(): VertexShader("shaders/passthrough.vert") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_IN_VBO_VEC3(vertexPositionModelspace, 0);
  SHADER_DRAW_TRIANGLE_ARRAYS();
};

class JustTextureFrag: public FragmentShader {
public:
  JustTextureFrag(): FragmentShader("shaders/justTexture.frag") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_SAMPLER2D(texture, 0);
};

class DepthShadowVert: public VertexShader {
public:
  DepthShadowVert(): VertexShader("shaders/depthShadow.vert") {}
  static std::vector<const GLchar*> shaderFieldNames;
  SHADER_UNIFORM_MAT4(depthMVP);
};

// TODO: This should just be a do nothing shader.
class DepthShadowFrag: public FragmentShader {
public:
  DepthShadowFrag(): FragmentShader("shaders/depthShadow.frag") {}
  static std::vector<const GLchar*> shaderFieldNames;
};

class DeferredShadingVert: public VertexShader {
public:
  DeferredShadingVert(): VertexShader("shaders/deferredShading.vert") {}
  static std::vector<const GLchar*> shaderFieldNames;
};

class DeferredShadingFrag: public FragmentShader {
public:
  DeferredShadingFrag(): FragmentShader("shaders/deferredShading.frag") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_SAMPLER2D(diffuseTexture, 0);
  SHADER_UNIFORM_SAMPLER2D(specularTexture, 1);
  SHADER_UNIFORM_SAMPLER2D(emissiveTexture, 2);
  SHADER_UNIFORM_SAMPLER2D(normalTexture, 3);
  SHADER_UNIFORM_SAMPLER2D(depthTexture, 4);
  SHADER_UNIFORM_SAMPLER2D(shadowMap, 5);
  SHADER_UNIFORM_SAMPLER_CUBE(shadowMapCube, 6);
  SHADER_UNIFORM_SAMPLER2D(ssaoNoiseTexture, 7);

  SHADER_UNIFORM_VEC3(lightPositionWorldspace);
  SHADER_UNIFORM_VEC3(lightDirectionWorldspace);
  SHADER_UNIFORM_INT(lightType);
  SHADER_UNIFORM_VEC3(lightColour);
  SHADER_UNIFORM_VEC3(lightAmbience);
  SHADER_UNIFORM_VEC3(lightFalloff);
  SHADER_UNIFORM_FLOAT(lightSpreadDegrees);

  SHADER_UNIFORM_MAT4(P);
  SHADER_UNIFORM_MAT4(V);
  SHADER_UNIFORM_MAT4(shadowmapDepthBiasVP);

  SHADER_UNIFORM_BOOL(useDiffuse);
  SHADER_UNIFORM_BOOL(useSpecular);
  SHADER_UNIFORM_BOOL(useShadow);
  SHADER_UNIFORM_BOOL(useSSAO);
  SHADER_UNIFORM_VEC3_ARRAY(ssaoKernel);
};

class PostProcessFrag: public FragmentShader {
public:
  PostProcessFrag(): FragmentShader("shaders/postProcess.frag") {}
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_SAMPLER2D(tex, 0);
  SHADER_UNIFORM_SAMPLER2D(depthTexture, 1);
  SHADER_UNIFORM_SAMPLER2D(pickingTexture, 2);

  SHADER_UNIFORM_BOOL(useBlur);
  SHADER_UNIFORM_BOOL(useMotionBlur);
  SHADER_UNIFORM_BOOL(shudder);
  SHADER_UNIFORM_FLOAT(currentTime);
  SHADER_UNIFORM_MAT4(newToOldMatrix);
  SHADER_UNIFORM_FLOAT(fpsCorrection);
  SHADER_UNIFORM_INT(selectedMeshId);
};

} // namespace shaders

#endif


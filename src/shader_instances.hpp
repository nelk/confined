
#ifndef SHADER_INSTANCES_HPP
#define SHADER_INSTANCES_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.hpp"

namespace shaders {

class GeomTexturesVertShader: public VertexShader {
public:
  GeomTexturesVertShader(): VertexShader("shaders/geomTexture.vert") {}

  // TODO: Create macro for shaderFieldNames.
  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_MAT4(MVP)
  SHADER_UNIFORM_MAT4(M)
  SHADER_UNIFORM_MAT4(V)
  SHADER_UNIFORM_MAT4(P)
  SHADER_UNIFORM_VEC3(halfspacePoint)
  SHADER_UNIFORM_VEC3(halfspaceNormal)
};

class GeomTexturesFragShader: public FragmentShader {
public:
  GeomTexturesFragShader(): FragmentShader("shaders/geomTexture.frag") {}

  static std::vector<const GLchar*> shaderFieldNames;

  SHADER_UNIFORM_BOOL(useDiffuseTexture)
  SHADER_UNIFORM_BOOL(useNormalTexture)
  SHADER_UNIFORM_VEC3(material_kd)
  SHADER_UNIFORM_VEC3(material_ks)
  SHADER_UNIFORM_FLOAT(material_shininess)
  SHADER_UNIFORM_VEC3(material_emissive)

  SHADER_UNIFORM_VEC3(halfspacePoint)
  SHADER_UNIFORM_VEC3(halfspaceNormal)
  SHADER_UNIFORM_BOOL(useNoPerspectiveUVs)

  SHADER_UNIFORM_INT(meshId)
};

} // namespace shaders

#endif


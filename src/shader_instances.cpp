
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader_instances.hpp"

namespace shaders {

std::vector<const GLchar*> GeomTexturesVertShader::shaderFieldNames;
std::vector<const GLchar*> GeomTexturesFragShader::shaderFieldNames;
std::vector<const GLchar*> JustTextureFrag::shaderFieldNames;
std::vector<const GLchar*> PassThroughVert::shaderFieldNames;
std::vector<const GLchar*> DepthShadowVert::shaderFieldNames;
std::vector<const GLchar*> DepthShadowFrag::shaderFieldNames;
std::vector<const GLchar*> DeferredShadingVert::shaderFieldNames;
std::vector<const GLchar*> DeferredShadingFrag::shaderFieldNames;
std::vector<const GLchar*> PostProcessFrag::shaderFieldNames;

} // namespace shaders


#ifndef SHADER_HPP
#define SHADER_HPP

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>

namespace shaders {

GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path);

#define SHADER_UNIFORM_GENERIC(name, type, cmd) void set_##name (const type &n) { \
  GLint id = shaderFieldMap[#name]; \
  if (id == -1) {                                                        \
    std::cerr << "Shader Runtime Error: No uniform field " #name " for shader " \
              << filename << std::endl; \
  }                                                                      \
  cmd;                         \
}                                                                        \
struct Field##name { \
  Field##name() { \
    shaderFieldNames.push_back(#name); \
  } \
} _field_##name; \

#define SHADER_UNIFORM_MAT4(name) SHADER_UNIFORM_GENERIC(name, glm::mat4, glUniformMatrix4fv(id, 1, GL_FALSE, &n[0][0]))
#define SHADER_UNIFORM_VEC3(name) SHADER_UNIFORM_GENERIC(name, glm::vec3, glUniform3fv(id, 1, &n[0]))
#define SHADER_UNIFORM_INT(name) SHADER_UNIFORM_GENERIC(name, int, glUniform1i(id, n))
#define SHADER_UNIFORM_BOOL(name) SHADER_UNIFORM_GENERIC(name, bool, glUniform1i(id, n))
#define SHADER_UNIFORM_FLOAT(name) SHADER_UNIFORM_GENERIC(name, float, glUniform1f(id, n))

class Shader {
public:
  Shader(const char* filename): filename(filename) {
    shaderId = loadShader(filename);
  }

  ~Shader() {
    glDeleteShader(shaderId);
  }

  static GLuint loadShader(const char* filename);

  virtual GLuint getProgramId() = 0;

  GLuint getShaderId() { return shaderId; }

protected:
  const char* filename;
  GLuint shaderId;
  std::map<std::string, GLint> shaderFieldMap;
};

class VertexShader: public Shader {
protected:
  VertexShader(const char* filename): Shader(filename) {}
};

class FragmentShader: public Shader {
protected:
  FragmentShader(const char* filename): Shader(filename) {}
};

template<class VERT, class FRAG>
class ShaderProgram: virtual public VERT, virtual public FRAG {
public:
  ShaderProgram() {
    linkProgram();
    setup();
  }

  GLuint getProgramId() {
    return programId;
  }

  void linkProgram() {
    std::cout << "Linking program" << std::endl;
    programId = glCreateProgram();
    glAttachShader(programId, VERT::getShaderId());
    glAttachShader(programId, FRAG::getShaderId());
    glLinkProgram(programId);

    // Check the program
    GLint Result = GL_FALSE;
    int infoLogLength;

    glGetProgramiv(programId, GL_LINK_STATUS, &Result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0){
      std::vector<char> ProgramErrorMessage(infoLogLength+1);
      glGetProgramInfoLog(programId, infoLogLength, NULL, &ProgramErrorMessage[0]);
      std::cerr << &ProgramErrorMessage[0] << std::endl;
    }
  }

  void setup() {
    for (const GLchar* field : VERT::shaderFieldNames) {
      GLint id = glGetUniformLocation(getProgramId(), field);
      if (id == -1) {
        std::cerr << "Vertex Shader Error: No uniform field " << field << " for shader "
                  << VERT::filename << std::endl;
        shader_success = false;
        return;
      }
      VERT::shaderFieldMap[field] = id;
    }
    for (const GLchar* field : FRAG::shaderFieldNames) {
      GLint id = glGetUniformLocation(getProgramId(), field);
      if (id == -1) {
        std::cerr << "Fragment Shader Error: No uniform field " << field << " for shader "
                  << FRAG::filename << std::endl;
        shader_success = false;
        return;
      }
      FRAG::shaderFieldMap[field] = id;
    }
    // TODO: Other setup and validation.
  }

private:
  bool shader_success;
  GLuint programId;
};

} // namespace shaders


#endif

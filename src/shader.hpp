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

//#define SHADER_MEMBERS static std::vector<const GLchar*> shaderFieldNames;

#define SHADER_UNIFORM_GENERIC(name, type, cmd) void set_##name (type n) { \
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
} _field_##name

#define SHADER_UNIFORM_MAT4(name) SHADER_UNIFORM_GENERIC(name, const glm::mat4&, glUniformMatrix4fv(id, 1, GL_FALSE, &n[0][0]))
#define SHADER_UNIFORM_VEC3(name) SHADER_UNIFORM_GENERIC(name, const glm::vec3&, glUniform3fv(id, 1, &n[0]))
#define SHADER_UNIFORM_VEC3_ARRAY(name) SHADER_UNIFORM_GENERIC(name, const glm::vec3*, glUniform3fv(id, sizeof(n), (float*)n))
#define SHADER_UNIFORM_INT(name) SHADER_UNIFORM_GENERIC(name, int, glUniform1i(id, n))
#define SHADER_UNIFORM_BOOL(name) SHADER_UNIFORM_GENERIC(name, bool, glUniform1i(id, n))
#define SHADER_UNIFORM_FLOAT(name) SHADER_UNIFORM_GENERIC(name, float, glUniform1f(id, n))
// TODO: Make this take Texture*.
#define SHADER_UNIFORM_SAMPLER2D(name, slot) SHADER_UNIFORM_GENERIC(name, GLuint, {glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D, n); glUniform1i(id, slot);})
#define SHADER_UNIFORM_SAMPLER_CUBE(name, slot) SHADER_UNIFORM_GENERIC(name, GLuint, {glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_CUBE_MAP, n); glUniform1i(id, slot);})

// TODO: Check names and locations during validation.
#define SHADER_IN_VBO(name, location, size) void vbo_##name(GLuint vboId) { \
  glEnableVertexAttribArray(location); \
  enabledVertexAttribPointers[location] = true; \
  glBindBuffer(GL_ARRAY_BUFFER, vboId); \
  glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, 0, (void*)0); \
}
#define SHADER_IN_VBO_VEC3(name, location) SHADER_IN_VBO(name, location, 3)
#define SHADER_IN_VBO_VEC2(name, location) SHADER_IN_VBO(name, location, 2)

#define SHADER_DRAW_TRIANGLE_ELEMENTS() void drawTriangleElements(GLuint elem_buf, GLuint num_triangles) { \
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buf); \
  prepareDraw(); \
  glDrawElements(GL_TRIANGLES, num_triangles, GL_UNSIGNED_SHORT, (void*)0); \
  arrayBufferSet = true; \
  cleanupDraw(); \
}
#define SHADER_DRAW_TRIANGLE_ARRAYS() void drawTriangles(GLuint num_triangles) { \
  prepareDraw(); \
  glDrawArrays(GL_TRIANGLES, 0, num_triangles); \
  cleanupDraw(); \
}


#define SHADER_OUT_COLOR_ATTACHMENT(name, slot) void attach_##name(GLuint texture) { \
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, texture, 0); \
  enabledColorAttachements[slot] = true; \
  drawBuffersSetUp = false; \
}
#define SHADER_OUT_DEPTH_ATTACHMENT(name) void attach_##name(GLuint texture) { \
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0); \
}
#define SHADER_OUT_DEPTH_CUBE_ATTACHMENT(name, slot) void attach_##name(GLuint texture) { \
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_CUBE_MAP_POSITIVE_X + slot, texture, 0); \
}

// TODO: Make typesafe VBOs wrapper.


class Shader {
public:
  Shader(const char* filename): filename(filename), shaderId(0), enabledColorAttachements(20, false), enabledVertexAttribPointers(10, false), arrayBufferSet(false), drawBuffersSetUp(false) {
  }

  ~Shader() {
    glDeleteShader(shaderId);
  }

  bool loadShader();

  virtual GLuint getProgramId() = 0;
  virtual GLuint getShaderType() = 0;
  GLuint getShaderId() { return shaderId; }

  void setupDrawBuffers() {
    if (drawBuffersSetUp) {
      return;
    }
    std::vector<GLuint> drawBuffers;
    for (uint i = 0; i < enabledColorAttachements.size(); ++i) {
      if (enabledColorAttachements[i]) {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
      }
    }
    if (drawBuffers.size() > 0) {
      glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);
    }
    drawBuffersSetUp = true;
  }

  void prepareDraw() {
    setupDrawBuffers();
  }

  void cleanupDraw() {
    unbindVertexAttribPointers();
    for (uint i = 0; i < enabledColorAttachements.size(); ++i) {
      enabledColorAttachements[i] = false;
    }
    drawBuffersSetUp = false;
  }

  void unbindVertexAttribPointers() {
    for (uint i = 0; i < enabledVertexAttribPointers.size(); ++i) {
      glDisableVertexAttribArray(i);
      enabledVertexAttribPointers[i] = false;
    }
    if (arrayBufferSet) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      arrayBufferSet = false;
    }
  }

protected:
  const char* filename;
  GLuint shaderId;
  std::map<std::string, GLint> shaderFieldMap;
  std::vector<bool> enabledColorAttachements;
  std::vector<bool> enabledVertexAttribPointers;
  bool arrayBufferSet;
  bool drawBuffersSetUp;
};

class VertexShader: public Shader {
protected:
  VertexShader(const char* filename): Shader(filename) {}
  GLuint getShaderType() { return GL_VERTEX_SHADER; }
};

class FragmentShader: public Shader {
protected:
  FragmentShader(const char* filename): Shader(filename) {}
  GLuint getShaderType() { return GL_FRAGMENT_SHADER; }
};

template<class VERT, class FRAG>
class ShaderProgram: public VERT, public FRAG {
public:
  ShaderProgram() {}

  GLuint getProgramId() {
    return programId;
  }

  bool initialize() {
    // Load shaders.
    bool loaded = VERT::loadShader();
    if (!loaded) return false;
    loaded = FRAG::loadShader();
    if (!loaded) return false;

    if (!linkProgram()) {
      return false;
    }

    std::cerr << "Validating shader fields" << std::endl;
    for (const GLchar* field : VERT::shaderFieldNames) {
      GLint id = glGetUniformLocation(getProgramId(), field);
      if (id == -1) {
        std::cerr << "Vertex Shader Error: No uniform field " << field << " for shader "
                  << VERT::filename << std::endl;
        return false;
      }
      VERT::shaderFieldMap[field] = id;
    }
    for (const GLchar* field : FRAG::shaderFieldNames) {
      GLint id = glGetUniformLocation(getProgramId(), field);
      if (id == -1) {
        std::cerr << "Fragment Shader Error: No uniform field " << field << " for shader "
                  << FRAG::filename << std::endl;
        return false;
      }
      FRAG::shaderFieldMap[field] = id;
    }

    // TODO: Other setup and validation.
    return true;
  }

protected:
  bool linkProgram() {
    std::cout << "Linking program" << std::endl;
    programId = glCreateProgram();
    glAttachShader(programId, VERT::getShaderId());
    glAttachShader(programId, FRAG::getShaderId());
    glLinkProgram(programId);

    // Check the program
    GLint result = GL_FALSE;
    int infoLogLength;

    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (result != GL_TRUE){
      std::vector<char> ProgramErrorMessage(infoLogLength+1);
      glGetProgramInfoLog(programId, infoLogLength, NULL, &ProgramErrorMessage[0]);
      std::cerr << &ProgramErrorMessage[0] << std::endl;
      return false;
    }
    std::cout << "Shader program (" << VERT::filename << ", " << FRAG::filename << ") linked with id=" << programId << std::endl;
    return true;
  }

  GLuint programId;
};

} // namespace shaders


#endif

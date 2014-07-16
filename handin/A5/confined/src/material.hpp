#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <GL/gl.h>

struct Material {
  Material(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, float shininess)
    : ka(ka), kd(kd), ks(ks), shininess(shininess), texture(0) {}

  ~Material() {
    if (texture != 0) {
      glDeleteTextures(1, &texture);
    }
  }

  void setTexture(int w, int h, void* data) {
    if (texture != 0) {
      glDeleteTextures(1, &texture);
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }

  glm::vec3 ka, kd, ks;
  float shininess;
  GLuint texture;
};


#endif

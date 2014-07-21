#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <GL/gl.h>

class Material {
public:
  Material(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, float shininess)
    : ka(ka), kd(kd), ks(ks), shininess(shininess), diffuseTexture(0), normalTexture(0) {}

  ~Material() {
    if (diffuseTexture != 0) {
      glDeleteTextures(1, &diffuseTexture);
    }
    if (normalTexture != 0) {
      glDeleteTextures(1, &normalTexture);
    }
  }

  void setDiffuseTexture(int w, int h, void* data) {
    setTexture(diffuseTexture, w, h, data);
  }

  void setNormalTexture(int w, int h, void* data) {
    setTexture(normalTexture, w, h, data);
  }

  glm::vec3 getAmbience() {
    return ka;
  }

  glm::vec3 getDiffuse() {
    return kd;
  }

  glm::vec3 getSpecular() {
    return ks;
  }

  float getShininess() {
    return shininess;
  }

  bool hasDiffuseTexture() {
    return diffuseTexture != 0;
  }

  GLuint getDiffuseTexture() {
    return diffuseTexture;
  }

  bool hasNormalTexture() {
    return normalTexture != 0;
  }

  GLuint getNormalTexture() {
    return normalTexture;
  }

  virtual void update() {}

  virtual bool isMirror() { return false; }

protected:
  void setTexture(GLuint &tex, int w, int h, void* data) {
    if (tex != 0) {
      glDeleteTextures(1, &tex);
      tex = 0;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Note: Assuming texture was BGR.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
  }

  glm::vec3 ka, kd, ks;
  float shininess;
  GLuint diffuseTexture;
  GLuint normalTexture;
};


#endif

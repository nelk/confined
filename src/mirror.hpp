#ifndef MIRROR_H
#define MIRROR_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>

#include "texture.hpp"
#include "material.hpp"

class Mirror: public Material {
public:
  Mirror(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, const glm::vec3& ke, float shininess);

  ~Mirror();

  void updateSize(int width, int height);

  bool isReflective() {
    return reflective;
  }

  void setReflective(bool r) {
    reflective = r;
  }

  bool isMirror() { return true; }

  virtual void update();

  Texture* getMirrorTexture() {
    return mirrorTexture;
  }

  GLuint getMirrorFBO() {
    return mirrorFBO;
  }

private:
  bool reflective;
  Texture* mirrorTexture;

  GLuint mirrorFBO;
  GLuint mirrorRBO;
};

#endif

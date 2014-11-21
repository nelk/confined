
#include <iostream>
#include "mirror.hpp"

#include "viewer.hpp" // For DEFAULT_*

Mirror::Mirror(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, const glm::vec3& ke, float shininess)
  : Material(ka, kd, ks, ke, shininess), reflective(true) {

  mirrorFBO = 0;
  glGenFramebuffers(1, &mirrorFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mirrorFBO);

  GLuint diffuseTextureId = 0;
  glGenTextures(1, &diffuseTextureId);
  glBindTexture(GL_TEXTURE_2D, diffuseTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // Don't bind this texture to fbo.
  diffuseTexture = new Texture(diffuseTextureId, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  GLuint mirrorTextureId = 0;
  glGenTextures(1, &mirrorTextureId);
  glBindTexture(GL_TEXTURE_2D, mirrorTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  mirrorTexture = new Texture(mirrorTextureId, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  glGenRenderbuffers(1, &mirrorRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, mirrorRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mirrorRBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->getTextureId(), 0);

  checkGLFramebuffer();
  checkGLErrors("mirror");
}

Mirror::~Mirror() {
  delete diffuseTexture;
  delete mirrorTexture;
}

void Mirror::updateSize(int width, int height) {
  glBindTexture(GL_TEXTURE_2D, diffuseTexture->getTextureId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, mirrorTexture->getTextureId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, mirrorRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
}

void Mirror::update() {
  // Swap read/write textures.
  Texture* temp = mirrorTexture;
  mirrorTexture = diffuseTexture;
  diffuseTexture = temp;

  glBindFramebuffer(GL_FRAMEBUFFER, mirrorFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->getTextureId(), 0);
}


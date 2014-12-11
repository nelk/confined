#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "texture.hpp"

class Material {
public:
  Material(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, const glm::vec3& ke, float shininess)
    : ka(ka), kd(kd), ks(ks), ke(ke), shininess(shininess), diffuseTexture(nullptr), normalTexture(nullptr) {}

  ~Material() {}

  void setDiffuseTexture(Texture* texture) {
    diffuseTexture = texture;
  }

  void setNormalTexture(Texture* texture) {
    normalTexture = texture;
  }

  glm::vec3& getAmbience() {
    return ka;
  }

  glm::vec3& getDiffuse() {
    return kd;
  }

  glm::vec3& getSpecular() {
    return ks;
  }

  glm::vec3& getEmissive() {
    return ke;
  }

  float getShininess() {
    return shininess;
  }

  bool hasDiffuseTexture() {
    return diffuseTexture != nullptr;
  }

  Texture* getDiffuseTexture() {
    return diffuseTexture;
  }

  bool hasNormalTexture() {
    return normalTexture != nullptr;
  }

  Texture* getNormalTexture() {
    return normalTexture;
  }

  virtual void update() {}

  virtual bool isMirror() { return false; }

protected:
  glm::vec3 ka, kd, ks, ke;
  float shininess;
  Texture* diffuseTexture;
  Texture* normalTexture;
};


#endif

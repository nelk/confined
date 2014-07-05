#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

struct Material {
  Material(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, float shininess)
    : ka(ka), kd(kd), ks(ks), shininess(shininess) {}

  glm::vec3 ka, kd, ks;
  float shininess;
  // TODO: Texture.
};


#endif

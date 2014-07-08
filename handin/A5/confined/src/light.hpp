#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Light {
public:
  enum LightType {
    DIRECTIONAL = 0,
    SPOT = 1,
    POINT = 2
  };

  static Light* directionalLight(const glm::vec3& colour, const glm::vec3& direction);
  static Light* spotLight(const glm::vec3& colour, const glm::vec3& position, const glm::vec3& direction, float spread);
  static Light* pointLight(const glm::vec3& colour, const glm::vec3& position);
  static Light* ambientLight(const glm::vec3& colour);

  LightType getType() {
    return type;
  }
  glm::vec3& getColour() {
    return colour;
  }
  glm::vec3& getAmbience() {
    return ambientColour;
  }
  glm::vec3& getPosition() {
    return position;
  }
  glm::vec3& getDirection() {
    return direction;
  }
  glm::vec3& getFalloff() {
    return falloff;
  }
  float& getSpread() {
    return spread;
  }
  bool isEnabled() {
    return enabled;
  }
  void setEnabled(bool e) {
    enabled = e;
  }

private:
  Light(LightType type, const glm::vec3& colour, const glm::vec3& position, const glm::vec3& direction, float spread);

  LightType type;
  glm::vec3 colour;
  glm::vec3 ambientColour;
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 falloff;
  float spread;
  bool enabled;
};

#endif

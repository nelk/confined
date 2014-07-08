
#include "light.hpp"

Light::Light(LightType type, const glm::vec3& colour, const glm::vec3& position, const glm::vec3& direction, float spread)
  : type(type), colour(colour), ambientColour(0, 0, 0), position(position), direction(direction), falloff(1, 0, 0), spread(spread), enabled(true) {}

Light* Light::directionalLight(const glm::vec3& colour, const glm::vec3& direction) {
  return new Light(DIRECTIONAL, colour, glm::vec3(0, 0, 0), direction, 0);
}

Light* Light::spotLight(const glm::vec3& colour, const glm::vec3& position, const glm::vec3& direction, float spread) {
  return new Light(SPOT, colour, position, direction, spread);
}

Light* Light::pointLight(const glm::vec3& colour, const glm::vec3& position) {
  return new Light(SPOT, colour, position, glm::vec3(0, 0, 0), 0);
}


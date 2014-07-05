#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <glm/glm.hpp>
#include "viewer.hpp"

#define SPEED 5.0f
#define MOUSE_SPEED 0.001f

class Viewer;

class Controller {
public:
  Controller(Viewer* viewer);

  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();
  void reset();
  void update();
  void setPosition(glm::vec3& p);
  void setHorizontalAngle(float a);
  void setVerticalAngle(float a);

private:
  Viewer* viewer;
  double lastTime;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  float horizontalAngle;
  float verticalAngle;
  int skipMovements;
};

#endif

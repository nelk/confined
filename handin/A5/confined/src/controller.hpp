#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <map>
#include <glm/glm.hpp>
#include "settings.hpp"
#include "viewer.hpp"

#define SPEED 5.0f
#define MOUSE_SPEED 0.001f

class Viewer;

class Controller {
public:
  Controller(Viewer* viewer, Settings* settings);

  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();
  void setPosition(glm::vec3& p);
  glm::vec3 getPosition();
  void reset();
  void update();
  void setHorizontalAngle(float a);
  void setVerticalAngle(float a);

  bool isFlashlightOn() {
    return flashlight;
  }

private:
  bool checkKeyJustPressed(int k);

  Viewer* viewer;
  Settings* settings;
  double lastTime;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  float horizontalAngle;
  float verticalAngle;
  int skipMovements;
  std::map<int, bool> keysPressed;
  bool flashlight;
};

#endif

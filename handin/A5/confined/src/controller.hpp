#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <map>
#include <glm/glm.hpp>
#include "settings.hpp"
#include "viewer.hpp"
#include "sound.hpp"

#define SPEED 8.0f
#define MOUSE_SPEED 0.001f
#define GRAVITY 1.0f

class Viewer;

class Controller {
public:
  Controller(Viewer* viewer, Settings* settings);
  ~Controller();

  glm::mat4 getViewMatrix();
  glm::mat4 getMirroredViewMatrix(const glm::vec3& mirrorPosition, const glm::vec3& mirrorNormal);
  glm::mat4 getProjectionMatrix();
  void setPosition(glm::vec3& p);
  glm::vec3 getPosition();
  void reset();
  void update();
  void setHorizontalAngle(float a);
  void setVerticalAngle(float a);

  bool isFlashlightOn() {
    return hasFlashlight && flashlight;
  }

  void setHasFlashlight(bool f) {
    hasFlashlight = f;
  }

  void setHasGun(bool f) {
    hasGun = f;
  }

  bool isClicking() {
    return clicking;
  }

private:
  bool checkKeyJustPressed(int k);

  Viewer* viewer;
  Settings* settings;
  double lastTime;
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 velocity;
  float horizontalAngle;
  float verticalAngle;
  int skipMovements;
  std::map<int, bool> keysPressed;
  bool flashlight;
  bool hasFlashlight;
  bool hasGun;
  double lastStepSoundTime;
  bool clicking;

  Sound* flashlightSound;
  Sound* gunSound;
  Sound* stepSound;
};

#endif

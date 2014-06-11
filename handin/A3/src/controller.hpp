#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include "algebra.hpp"
#include "viewer.hpp"
#include "scene.hpp"

class Viewer;
class SceneNode;

class Controller {
public:
  enum Button {
    LEFT_BUTTON = 0,
    MIDDLE_BUTTON = 1,
    RIGHT_BUTTON = 2
  };

  Controller(Viewer* v, SceneNode* translateScene, SceneNode* rotateScene);

  void press(Button button, int x, int y);
  void release(Button button, int x, int y);
  void move(int x, int y);

private:
  Viewer* viewer;
  SceneNode* translateScene;
  SceneNode* rotateScene;
  int lastX, lastY;
  double trackballDiameter;
  std::vector<bool> buttonActive;
};

class Trackball {
public:
  static Vector3D calculateRotation(double newX, double newY, double oldX, double oldY, double diameter);
  //static Matrix4x4 vAxisRotMatrix(Vector3D vec);
};

#endif


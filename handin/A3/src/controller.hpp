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

  Controller(Viewer* v, SceneNode* rootscene, SceneNode* translateScene, SceneNode* rotateScene);

  void press(Button button, int x, int y);
  void release(Button button, int x, int y);
  void move(int x, int y);

  void undo();
  void redo();

  void resetJoints();

private:
  void pick(double x, double y);
  int processHits(GLint hits, GLuint buffer[]);

  Viewer* viewer;
  SceneNode* rootScene;
  SceneNode* translateScene;
  SceneNode* rotateScene;
  int lastX, lastY;
  double trackballDiameter;
  std::vector<bool> buttonActive;
  int undoStackSize, redoStackSize;
  bool performedJointMovement;
};

class Trackball {
public:
  static Vector3D calculateRotation(double newX, double newY, double oldX, double oldY, double diameter);
  //static Matrix4x4 vAxisRotMatrix(Vector3D vec);
};

#endif


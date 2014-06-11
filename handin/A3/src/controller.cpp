
#include "controller.hpp"
#include "matrices.hpp"

#define TRACKBALL_FACTOR 50.0
#define PAN_FACTOR 0.01
#define ZOOM_FACTOR 0.01

Controller::Controller(Viewer* v, SceneNode* translateScene, SceneNode* rotateScene):
    viewer(v),
    translateScene(translateScene),
    rotateScene(rotateScene),
    trackballDiameter(trackballDiameter) {

  buttonActive.push_back(false);
  buttonActive.push_back(false);
  buttonActive.push_back(false);
}

void Controller::press(Button button, int x, int y) {
  buttonActive[button] = true;
  lastX = x;
  lastY = y;
}
void Controller::release(Button button, int x, int y) {
  buttonActive[button] = false;
  lastX = x;
  lastY = y;
}

void Controller::move(int x, int y) {
  bool needsInvalidate = false;
  switch (viewer->getMode()) {
    case Viewer::POSITION:
      if (buttonActive[LEFT_BUTTON]) {
        translateScene->preTranslate(Vector3D((x - lastX) * PAN_FACTOR, -(y - lastY) * PAN_FACTOR, 0.0));
        needsInvalidate = true;
      }

      if (buttonActive[MIDDLE_BUTTON]) {
        translateScene->preTranslate(Vector3D(0.0, 0.0, (y - lastY) * PAN_FACTOR));
        needsInvalidate = true;
      }

      if (buttonActive[RIGHT_BUTTON]) {
        const double W = viewer->get_width();
        const double H = viewer->get_height();
        const double offsetX = -W / 2.0;
        const double offsetY = -H / 2.0;
        const double diameter = std::min(W, H) / 2.0;
        Vector3D rotVec = Trackball::calculateRotation(
            x + offsetX, y + offsetY,
            lastX + offsetX, lastY + offsetY,
            diameter);

        // Use negative rotations on model. Except for y, where already inverted because of flipped screen coordinates.
        rotVec[X] = -rotVec[X];
        rotVec[Z] = -rotVec[Z];

        // Rotate this vector by current rotation matrix.
        rotVec = rotateScene->get_inverse() * rotVec;
        rotateScene->rotate('x', rotVec[X] * TRACKBALL_FACTOR);
        rotateScene->rotate('y', rotVec[Y] * TRACKBALL_FACTOR);
        rotateScene->rotate('z', rotVec[Z] * TRACKBALL_FACTOR);

        needsInvalidate = true;
      }

      break;
    case Viewer::JOINTS:
      break;
    default:
      break;
  }

  lastX = x;
  lastY = y;
  if (needsInvalidate) {
    viewer->invalidate();
  }
}


/**
 * Matrix4x4 calculateRotation(double newX, double newY,
 *                  double oldX, double oldY,
 *                  double diameter);
 *    Based off of cs488 demo trackball  implementation.
 *    Calculates a rotation vector based on mouse motion over
 *    a virtual trackball.
 *
 *    The new and old mouse positions
 *    should be in 'trackball' space. That is, they have been
 *    transformed into a coordinate system centered at the middle
 *    of the trackball. new, old, and diameter must all be specified
 *    in the same units (pixels for example).
 *
 * Parameters: newX, newY - New mouse position in trackball space.
 *                            This is the second point along direction
 *                            of rotation.
 *             oldX, oldY - Old mouse position in trackball space.
 *                            This is the first point along direction
 *                            of rotation.
 *             diameter - Diameter of the trackball. This should
 *                         be specified in the same units as new and old.
 *                         (ie, usually pixels if new and old are transformed
 *                         mouse positions)
 *             fVec - The output rotation vector. The length of the vector
 *                    is proportional to the angle of rotation.
 *
 **/
Vector3D Trackball::calculateRotation(double newX, double newY, double oldX, double oldY, double diameter) {
  float newVecX, newVecY, newVecZ,        /* Vector corresponding to new mouse location */
        oldVecX, oldVecY, oldVecZ,        /* Vector corresponding to old mouse location */
        length;

  /* Vector pointing from center of virtual trackball to
   * new mouse position
   */
  newVecX    = newX * 2.0 / diameter;
  newVecY    = newY * 2.0 / diameter;
  newVecZ    = (1.0 - newVecX * newVecX - newVecY * newVecY);

  /* If the Z component is less than 0, the mouse point
   * falls outside of the trackball which is interpreted
   * as rotation about the Z axis.
   */
  if (newVecZ < 0.0) {
    length = sqrt(1.0 - newVecZ);
    newVecZ  = 0.0;
    newVecX /= length;
    newVecY /= length;
  } else {
    newVecZ = sqrt(newVecZ);
  }

  /* Vector pointing from center of virtual trackball to
   * old mouse position
   */
  oldVecX    = oldX * 2.0 / diameter;
  oldVecY    = oldY * 2.0 / diameter;
  oldVecZ    = (1.0 - oldVecX * oldVecX - oldVecY * oldVecY);

  /* If the Z component is less than 0, the mouse point
   * falls outside of the trackball which is interpreted
   * as rotation about the Z axis.
   */
  if (oldVecZ < 0.0) {
    length = sqrt(1.0 - oldVecZ);
    oldVecZ  = 0.0;
    oldVecX /= length;
    oldVecY /= length;
  } else {
    oldVecZ = sqrt(oldVecZ);
  }

  /* Generate rotation vector by calculating cross product:
   *
   * oldVec x newVec.
   *
   * The rotation vector is the axis of rotation
   * and is non-unit length since the length of a crossproduct
   * is related to the angle between oldVec and newVec which we need
   * in order to perform the rotation.
   */
  return Vector3D(
      oldVecY * newVecZ - newVecY * oldVecZ,
      oldVecZ * newVecX - newVecZ * oldVecX,
      oldVecX * newVecY - newVecX * oldVecY);
}


/**
 * Calculate the rotation matrix for rotation about an arbitrary axis.
 * The length of vec is the amount to rotate by.
 */
/*
Matrix4x4 Trackball::vAxisRotMatrix(Vector3D vec) {
  const float fRadians = vec.length();

  // If the vector has zero length - return the identity matrix
  if (fRadians > -0.000001 && fRadians < 0.000001) {
    return Matrix4x4();
  }

  // Normalize the rotation vector now in preparation for making rotation matrix.
  vec.normalize();

  // Create the arbitrary axis rotation matrix
  double dSinAlpha = sin(fRadians);
  double dCosAlpha = cos(fRadians);
  double dT = 1 - dCosAlpha;

  return Matrix4x4((double[16]){
      dCosAlpha + vec[X]*vec[X]*dT,
      vec[X]*vec[Y]*dT + vec[Z]*dSinAlpha,
      vec[X]*vec[Z]*dT - vec[Y]*dSinAlpha,
      0,

      vec[X]*vec[Y]*dT - dSinAlpha*vec[Z],
      dCosAlpha + vec[Y]*vec[Y]*dT,
      vec[Y]*vec[Z]*dT + dSinAlpha*vec[X],
      0,

      vec[Z]*vec[X]*dT + dSinAlpha*vec[Y],
      vec[Z]*vec[Y]*dT - dSinAlpha*vec[X],
      dCosAlpha + vec[Z]*vec[Z]*dT,
      0,

      0, 0, 0, 1}).transpose();
}
*/


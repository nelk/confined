#include "a2.hpp"
#include <math.h>

// Return a matrix to represent a counterclockwise rotation of "angle"
// degrees around the axis "axis", where "axis" is one of the
// characters 'x', 'y', or 'z'.
Matrix4x4 rotation(double angle, char axis) {
  switch (axis) {
    case 'x':
      return Matrix4x4((double[16]){
        1,          0,           0, 0,
        0, cos(angle), -sin(angle), 0,
        0, sin(angle),  cos(angle), 0,
        0,          0,           0, 1
      });
    case 'y':
      return Matrix4x4((double[16]){
         cos(angle), 0, sin(angle), 0,
                  0, 1,          0, 0,
        -sin(angle), 0, cos(angle), 0,
                  0, 0,          0, 1
      });
    case 'z':
      return Matrix4x4((double[16]){
        cos(angle), -sin(angle), 0, 0,
        sin(angle),  cos(angle), 0, 0,
                 0,           0, 1, 0,
                 0,           0, 0, 1
      });
    default:
      return Matrix4x4();
  }
}

// Return a matrix to represent a displacement of the given vector.
Matrix4x4 translation(const Vector3D& displacement) {
  return Matrix4x4((double[16]){
    1, 0, 0, displacement[0],
    0, 1, 0, displacement[1],
    0, 0, 1, displacement[2],
    0, 0, 0,               1
  });
}

// Return a matrix to represent a nonuniform scale with the given factors.
Matrix4x4 scaling(const Vector3D& scale) {
  return Matrix4x4((double[16]){
    scale[0],        0,        0, 0,
           0, scale[1],        0, 0,
           0,        0, scale[2], 0,
           0,        0,        0, 1
  });
}

// Returns perspective matrix - assumes looking down -z like OpenGL.
Matrix4x4 perspectiveMatrix(double fov, double aspect, double near, double far) {
  double cotHalfAngle = 1.0/tan(fov/2);
  double planeDist = far - near;
  return Matrix4x4((double[16]){
     cotHalfAngle/aspect,            0,                       0,                       0,
                       0, cotHalfAngle,                       0,                       0,
                       0,            0, -(far + near)/planeDist, -2.0*far*near/planeDist,
                       0,            0,                      -1,                       0
  });
}


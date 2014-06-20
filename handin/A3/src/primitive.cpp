#include "primitive.hpp"
#include "algebra.hpp"
#include <iostream>
#include <GL/gl.h>

Primitive::~Primitive() {
}

Sphere::~Sphere() {
}

GLuint Sphere::sphereList;

void Sphere::init() {
  Sphere::sphereList = glGenLists(1);
  glNewList(Sphere::sphereList, GL_COMPILE);
  drawSphere(40, 40);
  glEndList();
}

// Draw sphere using quad strips in a given resolution.
// Note that slices * stacks polygons will be draw.
void Sphere::drawSphere(int slices, int stacks) {
  for (int i = 0; i <= stacks; i++) {
    double stack0 = M_PI * ((double) (i - 1) / stacks - 0.5);
    double stack1 = M_PI * ((double) i / stacks - 0.5);
    if (i == 0) {
      double temp = stack0;
      stack0 = stack1;
      stack1 = temp;
    }

    double z0 = sin(stack0);
    double zr0 = cos(stack0);

    if (i == 0) stack1 = M_PI * (-0.5 - 1.0 / stacks);
    double z1 = sin(stack1);
    double zr1 = cos(stack1);

    glBegin(GL_QUAD_STRIP);
    for (int j = 0; j <= slices; j++) {
      double s = 2 * M_PI * (double) (j - 1) / slices;
      double x = cos(s);
      double y = sin(s);

      glNormal3d(x * zr1, y * zr1, z1);
      glVertex3d(x * zr1, y * zr1, z1);

      glNormal3d(x * zr0, y * zr0, z0);
      glVertex3d(x * zr0, y * zr0, z0);
    }
    glEnd();
  }
}

void Sphere::walk_gl(bool picking) const {
  glCallList(Sphere::sphereList);
}

#include "shape.hpp"
#include "draw.hpp"

void Shape::drawLine(int pointIdx1, int pointIdx2) {
  const Point4D& p1 = points[pointIdx1];
  const Point4D& p2 = points[pointIdx2];
  //std::cout << "drawLine " << p1 << ", " << p2 << std::endl;
  draw_line(Point2D(p1[0], p1[1]), Point2D(p2[0], p2[1]));
}

void Shape::homogonize() {
  for (std::vector<Point4D>::iterator it = points.begin(); it != points.end(); it++) {
    it->homogonize();
  }
}

Cube::Cube(): Shape(8) {
  // Front face.
  points[0] = Point4D(-1, -1, 1);
  points[1] = Point4D(1, -1, 1);
  points[2] = Point4D(1, 1, 1);
  points[3] = Point4D(-1, 1, 1);

  // Back face.
  points[4] = Point4D(-1, -1, -1);
  points[5] = Point4D(-1, 1, -1);
  points[6] = Point4D(1, 1, -1);
  points[7] = Point4D(1, -1, -1);

  // Rest of line segments made in terms of these 8 points.
}

Cube::Cube(std::vector<Point4D>& points): Shape(points) {}
Cube::Cube(Cube& cube): Shape(cube.points) {}

Shape* Cube::transform(Matrix4x4& m) {
  for (std::vector<Point4D>::iterator it = points.begin(); it != points.end(); it++) {
    (*it) = m * (*it);
  }
  return this;
}

void Cube::drawOrtho() {
  // Front face.
  drawLine(0, 1);
  drawLine(1, 2);
  drawLine(2, 3);
  drawLine(3, 0);

  // Back face.
  drawLine(4, 5);
  drawLine(5, 6);
  drawLine(6, 7);
  drawLine(7, 4);

  // Left face.
  drawLine(0, 3);
  drawLine(3, 5);
  drawLine(5, 4);
  drawLine(4, 0);

  // Right face.
  drawLine(1, 7);
  drawLine(7, 6);
  drawLine(6, 2);
  drawLine(2, 1);

  // Top face.
  drawLine(2, 6);
  drawLine(6, 5);
  drawLine(5, 3);
  drawLine(3, 2);

  // Bottom face.
  drawLine(0, 4);
  drawLine(4, 7);
  drawLine(7, 1);
  drawLine(1, 0);
}


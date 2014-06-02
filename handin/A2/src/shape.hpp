#ifndef SHAPE_H
#define SHAPE_H

#include "algebra.hpp"
#include <vector>

class Shape {
public:
  Shape(int numPoints): points(numPoints) {};
  Shape(std::vector<Point4D>& points): points(points) {};
  virtual Shape* transform(Matrix4x4& m) = 0;
  virtual void drawOrtho() = 0;
  virtual void homogonize();

protected:
  void drawLine(int pointIdx1, int pointIdx2);

  std::vector<Point4D> points;
};

class Cube : public Shape {
public:
  Cube();
  Cube(std::vector<Point4D>& points);
  Cube(Cube& cube);

  Shape* transform(Matrix4x4& m);
  void drawOrtho();
};

#endif


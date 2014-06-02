#ifndef SHAPE_H
#define SHAPE_H

#include "algebra.hpp"
#include <vector>

class LineSegment4D {
public:
  LineSegment4D(): colour(0) {}
  LineSegment4D(const Point4D& p1, const Point4D& p2, const Colour& c)
    : p1(p1), p2(p2), colour(c) {}

  LineSegment4D(const LineSegment4D& l)
    : p1(l.p1), p2(l.p2), colour(l.colour) {}

  void operator=(const LineSegment4D& l) {
    p1 = l.p1;
    p2 = l.p2;
    colour = l.colour;
  }

  const Point4D& getP1() { return p1; }
  const Point4D& getP2() { return p2; }
  const Colour& getColour() { return colour; }

private:
  Point4D p1, p2;
  Colour colour;
};

class Node {
public:
  Node(): transformed(false) {}
  ~Node();

  const Matrix4x4& getTransform() { return transformation; }
  bool isTransformed() { return transformed; }
  void resetTransform();
  void setTransform(const Matrix4x4& m);

  // Helpers for manipulating this node. Simply uses the matrices defined in a2.hpp.
  void rotate(double angle, char axis);
  void translate(const Vector3D& displacement);
  void scale(const Vector3D& scale);

  void addChild(Node* n);
  void removeChild(Node* n);
  virtual std::vector<LineSegment4D> getTransformedLineSegments();

protected:
  virtual std::vector<LineSegment4D> getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform=false);

private:
  std::vector<Node*> children;
  Matrix4x4 transformation;
  bool transformed;
};

class Shape : public Node {
public:
  Shape(): colour(0) {};
  virtual std::vector<LineSegment4D> getTransformedLineSegments();
  void setColour(const Colour& c) { colour = c; }

protected:
  virtual std::vector<LineSegment4D> getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform=false);
  //void drawLine(int pointIdx1, int pointIdx2);

  virtual void getPoints(Point4D const*& points, int& len) = 0;
  virtual void getLinePointIdxs(int const*& points, int& len) = 0;

  //std::vector<Point4D> points;
  //std::vector<int> lineSegmentPointIndices;
  Colour colour;
};

class Cube : public Shape {
public:
  Cube() {}
  /*
  Cube();
  Cube(const std::vector<Point4D>& points);
  Cube(const Cube& cube);

  void drawOrtho();
  */
protected:
  void getPoints(Point4D const*& points, int& len) {
    points = this->points;
    len = NUM_POINTS;
  }

  void getLinePointIdxs(int const*& linePointIdxs, int& len) {
    linePointIdxs = this->linePointIdxs;
    len = NUM_LINE_POINTS;
  }

private:
  const static int NUM_POINTS = 8;
  const static int NUM_LINE_POINTS = 24;
  const static Point4D points[NUM_POINTS];
  const static int linePointIdxs[NUM_LINE_POINTS];
};

class Gnomon : public Shape {
public:
  Gnomon() {}

protected:
  void getPoints(Point4D const*& points, int& len) {
    points = this->points;
    len = NUM_POINTS;
  }

  void getLinePointIdxs(int const*& linePointIdxs, int& len) {
    linePointIdxs = this->linePointIdxs;
    len = NUM_LINE_POINTS;
  }

private:
  const static int NUM_POINTS = 4;
  const static int NUM_LINE_POINTS = 6;
  const static Point4D points[NUM_POINTS];
  const static int linePointIdxs[NUM_LINE_POINTS];
};

#endif


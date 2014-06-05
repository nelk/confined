#ifndef SCENE_H
#define SCENE_H

#include "algebra.hpp"
#include <vector>

// 3D line segment in homogenous coordinates.
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

// Base class for scene graph nodes.
// Can transform it and recursively get transformed line segments from its descendents.
class Node {
public:
  Node(): transformed(false) {}
  ~Node();

  // Basic matrix manipulation functions.
  const Matrix4x4& getTransform() { return transformation; }
  bool isTransformed() { return transformed; }
  void resetTransform();
  void setTransform(const Matrix4x4& m);

  // Helpers for manipulating this node.
  void preTranslate(const Vector3D& displacement);
  void postTranslate(const Vector3D& displacement);
  void preRotate(double angle, char axis);
  void postRotate(double angle, char axis);
  void preScale(const Vector3D& scale);
  void postScale(const Vector3D& scale);

  // Manipulating scene graph.
  void addChild(Node* n);
  void removeChild(Node* n);

  // Recursively gets line segments from all descendents, transformed by each matrix going down the stack.
  virtual std::vector<LineSegment4D> getTransformedLineSegments();

protected:
  // Internal overload that passes matrix stack so far.
  virtual std::vector<LineSegment4D> getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform=false);

private:
  std::vector<Node*> children;
  Matrix4x4 transformation;
  bool transformed;
};

// A scene graph node that has line segments.
class Shape : public Node {
public:
  Shape(): colour(0) {}
  virtual std::vector<LineSegment4D> getTransformedLineSegments();
  void setColour(const Colour& c) { colour = c; }

protected:
  virtual std::vector<LineSegment4D> getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform=false);

  // Template methods for derived classes to provide specific geometry.
  virtual void getPoints(Point4D const*& points, int& len) = 0;
  virtual void getLinePointIdxs(int const*& points, int& len) = 0;

  // Colour of all line segments for this shape.
  Colour colour;
};

// Provides cube geometry to shape.
class Cube : public Shape {
public:
  Cube() {}

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

// Provides gnomon (3 orthogonal axis lines) geometry to shape.
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


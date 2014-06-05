#include "scene.hpp"
#include "a2.hpp"
#include "draw.hpp"

Node::~Node() {
  for (std::vector<Node*>::iterator childIt = children.begin(); childIt != children.end(); childIt++) {
    delete *childIt;
  }
  children.clear();
}

void Node::setTransform(const Matrix4x4& m) {
  transformation = m;
  transformed = true;
}

void Node::resetTransform() {
  transformation = Matrix4x4();
  transformed = false;
}

void Node::addChild(Node* n) {
  children.push_back(n);
}
void Node::removeChild(Node* n) {
  children.erase(std::remove(children.begin(), children.end(), n), children.end());
}

void Node::translate(const Vector3D& displacement) {
  // Apply transformation to left, to maintain proper order.
  setTransform(translation(displacement) * getTransform());
}
void Node::rotate(double angle, char axis) {
  // Apply transformation to right, to maintain proper order.
  setTransform(getTransform() * rotation(angle, axis));
}
void Node::scale(const Vector3D& scale) {
  // Apply transformation to right, to maintain proper order.
  setTransform(getTransform() * scaling(scale));
}

std::vector<LineSegment4D> Node::getTransformedLineSegments() {
  Matrix4x4 ident;
  return getTransformedLineSegments(ident);
}

std::vector<LineSegment4D> Node::getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform) {
  std::vector<LineSegment4D> lines;
  Matrix4x4 sentM = m;
  if (isTransformed() && !appliedOwnTransform) {
    LOG("Transforming lines in node");
    sentM = sentM * getTransform();
  }
  for (std::vector<Node*>::const_iterator childIt = children.begin(); childIt != children.end(); childIt++) {
    std::vector<LineSegment4D> childLines = (*childIt)->getTransformedLineSegments(sentM);
    lines.insert(lines.end(), childLines.begin(), childLines.end());
  }
  return lines;
}

std::vector<LineSegment4D> Shape::getTransformedLineSegments() {
  Matrix4x4 ident;
  return getTransformedLineSegments(ident);
}

std::vector<LineSegment4D> Shape::getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform) {
  // Apply node's transform.
  Matrix4x4 sentM = m;
  if (isTransformed() && !appliedOwnTransform) {
    LOG("Transforming lines in ");
    sentM = sentM * getTransform();
  }

  // Get any lines from children.
  std::vector<LineSegment4D> lines = Node::getTransformedLineSegments(sentM, true);

  // Get own lines and apply m to them before returning.
  Point4D const* points;
  int const* linePointIdxs;
  int numPoints, numLinePointIdxs;
  getPoints(points, numPoints);
  getLinePointIdxs(linePointIdxs, numLinePointIdxs);

  for (int i = 0; i < numLinePointIdxs - 1; i += 2) {
    lines.push_back(LineSegment4D(
      sentM * points[linePointIdxs[i]],
      sentM * points[linePointIdxs[i+1]],
      colour
    ));
  }
  return lines;
}

const Point4D Cube::points[] = {
  // Front face.
  Point4D(-1, -1, 1),
  Point4D(1, -1, 1),
  Point4D(1, 1, 1),
  Point4D(-1, 1, 1),

  // Back face.
  Point4D(-1, -1, -1),
  Point4D(-1, 1, -1),
  Point4D(1, 1, -1),
  Point4D(1, -1, -1)
};

const int Cube::linePointIdxs[] = {
  // Front face.
  0, 1,
  1, 2,
  2, 3,
  3, 0,
  // Back face.
  4, 5,
  5, 6,
  6, 7,
  7, 4,
  // Connecting lines.
  3, 5,
  0, 4,
  2, 6,
  1, 7
};

const Point4D Gnomon::points[] = {
  Point4D(0.0, 0.0, 0.0),
  Point4D(0.5, 0.0, 0.0),
  Point4D(0.0, 0.5, 0.0),
  Point4D(0.0, 0.0, 0.5)
};

const int Gnomon::linePointIdxs[] = {
  0, 1,
  0, 2,
  0, 3
};



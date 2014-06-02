#include "shape.hpp"
#include "draw.hpp"


void Node::setTransform(const Matrix4x4& m) {
  transformation = m;
  transformed = true;
}

void Node::addChild(Node* n) {
  children.push_back(n);
}
void Node::removeChild(Node* n) {
  children.erase(std::remove(children.begin(), children.end(), n), children.end());
}

std::vector<LineSegment4D> Node::getTransformedLineSegments() {
  Matrix4x4 ident;
  return getTransformedLineSegments(ident);
}

std::vector<LineSegment4D> Node::getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform) {
  std::vector<LineSegment4D> lines;
  Matrix4x4 sentM = m;
  if (isTransformed() && !appliedOwnTransform) {
    sentM = getTransform() * sentM;
  }
  for (std::vector<Node*>::const_iterator childIt = children.begin(); childIt != children.end(); childIt++) {
    std::vector<LineSegment4D> childLines = (*childIt)->getTransformedLineSegments(sentM);
    lines.insert(lines.end(), childLines.begin(), childLines.end());
  }
  return lines;
}

/*
void Shape::drawLine(int pointIdx1, int pointIdx2) {
  const Point4D& p1 = points[pointIdx1];
  const Point4D& p2 = points[pointIdx2];
  //std::cout << "drawLine " << p1 << ", " << p2 << std::endl;
  draw_line(Point2D(p1[0], p1[1]), Point2D(p2[0], p2[1]));
}

Shape* Shape::transform(const Matrix4x4& m) {
  for (std::vector<Point4D>::iterator it = points.begin(); it != points.end(); it++) {
    (*it) = m * (*it);
  }
  return this;
}
*/

std::vector<LineSegment4D> Shape::getTransformedLineSegments() {
  Matrix4x4 ident;
  return getTransformedLineSegments(ident);
}

std::vector<LineSegment4D> Shape::getTransformedLineSegments(const Matrix4x4& m, bool appliedOwnTransform) {
  // Apply node's transform.
  Matrix4x4 sentM = m;
  if (isTransformed() && !appliedOwnTransform) {
    std::cout << "Transforming lines in shape" << std::endl;
    sentM = getTransform() * sentM;
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

/*
void Shape::homogonize() {
  for (std::vector<Point4D>::iterator it = points.begin(); it != points.end(); it++) {
    it->homogonize();
  }
}
*/

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
  // Connecting faces.
  3, 5,
  0, 4,
  2, 6,
  1, 7

  // Left face.
  /*
  0, 3,
  3, 5,
  5, 4,
  4, 0,
  // Right face.
  1, 7,
  7, 6,
  6, 2,
  2, 1,
  // Top face.
  2, 6,
  6, 5,
  5, 3,
  3, 2,
  // Bottom face.
  0, 4,
  4, 7,
  7, 1,
  1, 0
  */
};

/*
Cube::Cube(): Shape(8) {
  // Points.

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

  // Lines.

  // Front face.
  int l = 0;
  lineSegmentPointIndices[l++] = 0;
  lineSegmentPointIndices[l++] = 1;
  lineSegmentPointIndices[l++] = 1;
  lineSegmentPointIndices[l++] = 2;
  linesegmentPointIndices[l++] = 2;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 0;

  // Back face.
  linesegmentPointIndices[l++] = 4;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 4;

  // Left face.
  linesegmentPointIndices[l++] = 0;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 4;
  linesegmentPointIndices[l++] = 4;
  linesegmentPointIndices[l++] = 0;

  // Right face.
  linesegmentPointIndices[l++] = 1;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 2;
  linesegmentPointIndices[l++] = 2;
  linesegmentPointIndices[l++] = 1;

  // Top face.
  linesegmentPointIndices[l++] = 2;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 6;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 5;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 3;
  linesegmentPointIndices[l++] = 2;

  // Bottom face.
  linesegmentPointIndices[l++] = 0;
  linesegmentPointIndices[l++] = 4;
  linesegmentPointIndices[l++] = 4;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 7;
  linesegmentPointIndices[l++] = 1;
  linesegmentPointIndices[l++] = 1;
  linesegmentPointIndices[l++] = 0;
}

Cube::Cube(const std::vector<Point4D>& points): Shape(points) {}
Cube::Cube(const Cube& cube): Shape(cube.points) {}
*/

/*
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
*/

/*
Gnomon::Gnomon(): Shape(4) {
  points[0] = Point4D(0, 0, 0);
  points[1] = Point4D(5, 0, 0);
  points[2] = Point4D(0, 5, 0);
  points[3] = Point4D(0, 0, 5);
}

Gnomon::Gnomon(const std::vector<Point4D>& points): Shape(points) {}
Gnomon::Gnomon(const Gnomon& gnomon): Shape(gnomon.points) {}

void Gnomon::drawOrtho() {
  drawLine(0, 1);
  drawLine(0, 2);
  drawLine(0, 3);
}

*/




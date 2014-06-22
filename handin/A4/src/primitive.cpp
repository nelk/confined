#include "primitive.hpp"
#include "polyroots.hpp"

// TODO
#include <iostream>

Primitive::~Primitive() {
}

Sphere::~Sphere() {
}

std::vector<Intersection> Sphere::findIntersections(const Ray& ray) {
  return std::vector<Intersection>();
}

Cube::~Cube() {
}

std::vector<Intersection> Cube::findIntersections(const Ray& ray) {
  return std::vector<Intersection>();
}

NonhierSphere::~NonhierSphere() {
}

std::vector<Intersection> NonhierSphere::findIntersections(const Ray& ray) {

  // Will use quadratic solver.
  double A = ray.dir[X]*ray.dir[X]
           * ray.dir[Y]*ray.dir[Y]
           * ray.dir[Z]*ray.dir[Z];

  double B = 2 * (
      ray.dir[X] * (ray.pos[X] - m_pos[X])
    + ray.dir[Y] * (ray.pos[Y] - m_pos[Y])
    + ray.dir[Z] * (ray.pos[Z] - m_pos[Z])
  );

  double C = ray.pos[X]*ray.pos[X] + m_pos[X]*m_pos[X] - 2*ray.pos[X]*m_pos[X]
           + ray.pos[Y]*ray.pos[Y] + m_pos[Y]*m_pos[Y] - 2*ray.pos[Y]*m_pos[Y]
           + ray.pos[Z]*ray.pos[Z] + m_pos[Z]*m_pos[Z] - 2*ray.pos[Z]*m_pos[Z]
           - m_radius*m_radius;

  double roots[2];
  size_t num_roots = quadraticRoots(A, B, C, roots);

  std::vector<Intersection> intersections;

  //std::cout << "num roots: " << num_roots << std::endl;

  // TODO: Use intersection epsilon?
  if(num_roots == 1 && roots[0] > 0.0) {
    const double t = roots[0];
    const Point3D p = ray.pos + t*ray.dir;
    const Vector3D normal = p - m_pos;
    std::cout << "INTERSECTION " << p << std::endl;
    intersections.push_back(Intersection(
      t, normal, NULL
    ));
  }

  return intersections;
}

NonhierBox::~NonhierBox() {
}

std::vector<Intersection> NonhierBox::findIntersections(const Ray& ray) {
  return std::vector<Intersection>();
}


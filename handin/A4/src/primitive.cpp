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
           + ray.dir[Y]*ray.dir[Y]
           + ray.dir[Z]*ray.dir[Z];

  double B = 2 * (
      ray.dir[X] * (ray.pos[X] - m_pos[X])
    + ray.dir[Y] * (ray.pos[Y] - m_pos[Y])
    + ray.dir[Z] * (ray.pos[Z] - m_pos[Z])
  );

  double C = (ray.pos[X] - m_pos[X])*(ray.pos[X] - m_pos[X])
           + (ray.pos[Y] - m_pos[Y])*(ray.pos[Y] - m_pos[Y])
           + (ray.pos[Z] - m_pos[Z])*(ray.pos[Z] - m_pos[Z])
           - m_radius*m_radius;

  //std::cout << A << ", " << B << ", " << C << std::endl;
  double roots[2];
  size_t num_roots = quadraticRoots(A, B, C, roots);

  std::vector<Intersection> intersections;

  //std::cout << num_roots << " roots: " << roots[0] << ", " << roots[1] << std::endl;

  const double EPSILON = 0.0; // TODO.
  if (num_roots > 0) {
    double t = roots[0];
    if (num_roots == 2 && roots[1] > EPSILON && roots[1] < t) {
      t = roots[1];
    }
    if (t > EPSILON) {
      const Point3D p = ray.pos + t*ray.dir;
      Vector3D normal = p - m_pos;
      normal.normalize();
      //std::cout << "INTERSECTION for ray " << ray << " with params " << t << ", " << p << std::endl;
      intersections.push_back(Intersection(
        t, normal, NULL
      ));
    } else {
      //std::cout << "Intersection behind for ray " << ray << std::endl;
    }
  } else {
    //std::cout << "No Intersection for ray " << ray << std::endl;
  }

  return intersections;
}

NonhierBox::~NonhierBox() {
}

std::vector<Intersection> NonhierBox::findIntersections(const Ray& ray) {
  return std::vector<Intersection>();
}


#include "primitive.hpp"
#include "polyroots.hpp"
#include <limits>

// TODO
#include <iostream>

std::vector<Intersection> NonhierSphere::findIntersections(const Ray& ray) {

  // Will use quadratic solver.
  double A = ray.dir.length2();

  double B = 2 * ray.dir.dot(ray.pos - m_pos);

  double C = (ray.pos - m_pos).length2() - m_radius*m_radius;

  double roots[2];
  size_t num_roots = quadraticRoots(A, B, C, roots);

  std::vector<Intersection> intersections;

  const double EPSILON = 0.01;
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
        p, normal, NULL
      ));
    }
  }

  return intersections;
}

std::vector<Intersection> NonhierBox::findIntersections(const Ray& ray) {
  std::vector<Intersection> intersections;
  const double EPSILON = 0.01;

  // Derived from Kay and Kayjia's slab method for ray-box intersection.
  double tFar = std::numeric_limits<double>::max(); // Inf.
  double tNear = -tFar; // -Inf.
  Vector3D nearNormal, farNormal;
  for (int axis = 0; axis <= Z; axis++) {
    const double low = m_pos[axis];
    const double high = m_pos[axis] + m_size;
    int nearAxisSign = -1;
    if (ray.dir[axis] == 0.0) {
      if (ray.pos[axis] < low || ray.pos[axis] > high) {
        return intersections;
      }
    } else {
      double t1 = (low - ray.pos[axis]) / ray.dir[axis];
      double t2 = (high - ray.pos[axis]) / ray.dir[axis];
      if (t1 > t2) {
        double temp = t1;
        t1 = t2;
        t2 = temp;
        nearAxisSign *= -1;
      }
      if (t1 > tNear) {
        tNear = t1;
        nearNormal = Vector3D(
          axis == 0 ? nearAxisSign : 0,
          axis == 1 ? nearAxisSign : 0,
          axis == 2 ? nearAxisSign : 0
        );
      }
      if (t2 < tFar) {
        tFar = t2;
        farNormal = Vector3D(
          axis == 0 ? -nearAxisSign : 0,
          axis == 1 ? -nearAxisSign : 0,
          axis == 2 ? -nearAxisSign : 0
        );
      }
      if (tNear > tFar || tFar < 0) {
        return intersections;
      }
    }
  }

  if (tNear > EPSILON) {
    intersections.push_back(Intersection(ray.pos + tNear*ray.dir, nearNormal, NULL));
  }
  if (tFar > EPSILON) {
    intersections.push_back(Intersection(ray.pos + tFar*ray.dir, farNormal, NULL));
  }
  return intersections;
}


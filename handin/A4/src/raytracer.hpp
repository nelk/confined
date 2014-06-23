#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <iostream>
#include <list>
#include "algebra.hpp"
#include "material.hpp"
#include "light.hpp"

class Material;

enum Axis {
  X = 0,
  Y = 1,
  Z = 2,
  W = 3
};

struct Ray {
  Point3D pos;
  Vector3D dir;

  Ray(const Point3D& pos, const Vector3D& dir)
    : pos(pos), dir(dir) {}

  Ray transform(const Matrix4x4& mat) const {
    Point3D newPos = mat * this->pos;
    return Ray(newPos, mat * (this->pos + this->dir) - newPos);
  }
};

inline std::ostream& operator <<(std::ostream& os, const Ray& ray) {
  return os << "ray<" << ray.pos << " + t* " << ray.dir << ">";
}


struct Lighting {
  Colour ambient;
  std::list<Light*> lights;

  Lighting(const Colour& ambient, const std::list<Light*>& lights)
    : ambient(ambient), lights(lights) {}
};

// Warning: Normal is not always normalized.
struct Intersection {
  Point3D point;
  Vector3D normal;
  Material* material;

  Intersection(const Point3D& intersection, const Vector3D& normal, Material* material)
    : point(intersection), normal(normal), material(material) {}

  void transform(const Matrix4x4& mat) {
    point = mat * point;
    normal = mat.invert().transpose() * normal;
  }
};

struct ViewParams {
  Point3D eye;
  Vector3D view;
  Vector3D up;
  double fov; // In radians.

  ViewParams(const Point3D& eye, const Vector3D& view, const Vector3D& up, double fov)
    : eye(eye), view(view), up(up), fov(fov) {}

};

struct RayResult {
  bool hit;
  Colour colour;

  RayResult(): hit(false), colour(0.0) {}
  RayResult(Colour colour): hit(true), colour(colour) {}
};

#endif


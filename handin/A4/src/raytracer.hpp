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

struct Intersection {
  double rayParam; // intersection_point = ray.pos + rayParam * ray.dir;
  Vector3D normal;
  Material* material;

  Intersection(double rayParam, const Vector3D& normal, Material* material)
    : rayParam(rayParam), normal(normal), material(material) {}
};

struct ViewParams {
  Point3D eye;
  Vector3D view;
  Vector3D up;
  double fov;

  ViewParams(const Point3D& eye, const Vector3D& view, const Vector3D& up, double fov)
    : eye(eye), view(view), up(up), fov(fov) {}
};

#endif


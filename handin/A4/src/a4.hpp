#ifndef CS488_A4_HPP
#define CS488_A4_HPP

#include <string>
#include <vector>
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "mesh.hpp"

struct Ray {
  Point3D pos;
  Vector3D dir;

  Ray(const Point3D& pos, const Vector3D& dir)
    : pos(pos), dir(dir) {}
};

struct Lighting {
  Colour ambient;
  std::list<Light*> lights;

  Lighting(const Colour& ambient, const std::list<Light*>& lights)
    : ambient(ambient), lights(lights) {}
};

struct Intersection {
  double rayParam; // intersection_point = ray.pos + rayParam * ray.dir;
  Vector3D normal;
};

struct ViewParams {
  Point3D eye;
  Vector3D view;
  Vector3D up;
  double fov;

  ViewParams(const Point3D& eye, const Vector3D& view, const Vector3D& up, double fov)
    : eye(eye), view(view), up(up), fov(fov) {}
};

void a4_render(
  SceneNode* root, // What to render
  const std::string& filename, // Where to output the image
  int width, int height, // Image size
  const Point3D& eye, const Vector3D& view, // Viewing parameters
  const Vector3D& up, double fov, // Viewing parameters
  const Colour& ambient, const std::list<Light*>& lights // Lighting parameters
);

Colour raytrace_pixel(SceneNode* node,
  int x, int y,
  int width, int height,
  const ViewParams& viewParams,
  const Lighting& lighting);

Colour raytrace_visible(SceneNode* node, const Ray& ray, const Lighting& lighting);
Colour raytrace_shadow(SceneNode* node, const Ray& ray, const Lighting& lighting);

// Returns intersections sorted by distance.
std::vector<Intersection> ray_intersections(SceneNode* node, const Ray& ray);


#endif

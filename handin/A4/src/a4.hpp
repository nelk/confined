#ifndef CS488_A4_HPP
#define CS488_A4_HPP

#include <pthread.h>
#include <string>
#include <vector>
#include "raytracer.hpp"
#include "algebra.hpp"
#include "scene.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "image.hpp"
#include "workmanager.hpp"

class SceneNode;

struct WorkBundle {
  Image* image;
  int width, height;
  WorkManager* manager;
  SceneNode* scene;
  ViewParams* viewParams;
  Lighting* lighting;
};

void a4_render(
  SceneNode* root, // What to render
  const std::string& filename, // Where to output the image
  int width, int height, // Image size
  const Point3D& eye, const Vector3D& view, // Viewing parameters
  const Vector3D& up, double fov, // Viewing parameters
  const Colour& ambient, const std::list<Light*>& lights // Lighting parameters
);

void* do_raytrace(void* params);

Colour raytrace_pixel(SceneNode* node,
  int x, int y,
  int width, int height,
  const ViewParams& viewParams,
  const Lighting& lighting);

RayResult raytrace_visible(SceneNode* node, const Ray& ray, const Lighting& lighting, int depth=0);
Colour raytrace_shadow(SceneNode* node, const Ray& ray, const Lighting& lighting);

// 0 <= x <= 1, 0 <= y <= 1.
Colour genBackground(double x, double y);


#endif

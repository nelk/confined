#include "a4.hpp"
#include "image.hpp"
#include "matrices.hpp"
#include "algebra.hpp"
#include <algorithm>
#include <limits>
#include <iostream>

#define SHADOWS true

void a4_render(
  SceneNode* root, // What to render
  const std::string& filename, // Where to output the image
  int width, int height, // Image size
  const Point3D& eye, const Vector3D& view,
  const Vector3D& up, double fov, // Viewing parameters
  const Colour& ambient, const std::list<Light*>& lights // Lighting parameters
) {

  ViewParams viewParams(eye, view, up, fov * M_PI / 180.0);
  Lighting lighting(ambient, lights);

  Image img(width, height, 3);

  int totalRays = width * height;
  int completedRays = 0;
  int nextDecile = 1;

  std::cout << "Raytracing " << totalRays << " rays." << std::endl;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Colour colour = raytrace_pixel(root, x, y, width, height, viewParams, lighting);

      img(x, y, 0) = colour.R();
      img(x, y, 1) = colour.G();
      img(x, y, 2) = colour.B();

      completedRays++;
      if (((double)completedRays)/totalRays * 10.0 >= nextDecile) {
        std::cout << "Completed " << nextDecile << "0%" << std::endl;
        nextDecile++;
      }
    }
  }
  std::cout << "Done! Saving image..." << std::endl;
  img.savePng(filename);
  std::cout << "Saved" << std::endl;

}


Colour raytrace_pixel(SceneNode* node,
  int x, int y,
  int width, int height,
  const ViewParams& view,
  const Lighting& lighting
) {
  double d = 50.0; // TODO - Is this derived from something?
  double virtualH = 2.0 * d * tan(view.fov / 2.0);
  double virtualW = ((double)width) / height * virtualH;

  //Point3D pixel(x, y, -d);
  Point3D pixel(x, y, d);

  //Matrix4x4 translatePixelToOrigin = translation(Vector3D(-width/2, -height/d, d));
  Matrix4x4 translatePixelToOrigin = translation(Vector3D(-width/2, -height/2, 0));
  Matrix4x4 scalePixel = scaling(Vector3D(virtualW/width, -virtualH/height, 1.0));

  Vector3D w = view.view;
  //Vector3D u = view.up.cross(w);
  Vector3D u = w.cross(view.up);
  u.normalize();
  w.normalize();
  //Vector3D v = w.cross(u);
  Vector3D v = u.cross(w);

  Matrix4x4 rotatePixelToWCS = Matrix4x4((double[16]) {
    u[X], v[X], w[X], 0,
    u[Y], v[Y], w[Y], 0,
    u[Z], v[Z], w[Z], 0,
       0,    0,    0, 1
  });

  //std::cout << rotatePixelToWCS << std::endl;

  Matrix4x4 pixelEyeTranslation = translation(view.eye - Point3D());

  Point3D pixelWorld = pixelEyeTranslation * rotatePixelToWCS * scalePixel * translatePixelToOrigin * pixel;

  Vector3D rayDir = pixelWorld - view.eye;
  //Vector3D rayDir = pixel - view.eye;
  rayDir.normalize();

  Ray ray(view.eye, rayDir);
  RayResult result = raytrace_visible(node, ray, lighting);
  if (result.hit) {
    return result.colour;
  } else {
    return genBackground(((double)x)/width, ((double)y)/height);
  }
}

RayResult raytrace_visible(SceneNode* node, const Ray& ray, const Lighting& lighting) {
  std::vector<Intersection> intersections = node->findIntersections(ray);
  if (intersections.empty()) {
    return RayResult();
  }
  // TODO: add/subtract volumes?
  //std::sort(intersections.begin(), intersections.end());
  Intersection* closestIntersection = NULL;
  double closestDistance = std::numeric_limits<double>::max();
  for (std::vector<Intersection>::iterator it = intersections.begin(); it != intersections.end(); it++) {
    double distance = (it->point - ray.pos).length();
    if (closestIntersection == NULL
        || distance < closestDistance) {
      closestIntersection = &(*it);
      closestDistance = distance;
    }
  }

  // Normalize intersection normal.
  closestIntersection->normal.normalize();

  // Start with ambient light.
  // TODO: This makes it too bright...
  Colour finalColour = lighting.ambient * closestIntersection->material->ambientColour();

  // Add intensity from each light source.
  for (std::list<Light*>::const_iterator it = lighting.lights.begin(); it != lighting.lights.end(); it++) {
    Light* light = *it;
    Colour lightColour = light->colour;
    Vector3D incident = light->position - closestIntersection->point;

    double dist = incident.length();
    incident = 1.0/dist * incident; // Normalize;
    double attenuation = light->falloff[0] + light->falloff[1] * dist + light->falloff[2] * dist * dist;
    lightColour = 1.0/attenuation * lightColour;

    // Check for shadow.
    Colour shadowMultiplier(1.0);
    if (SHADOWS) {
      Ray shadowRay(closestIntersection->point, incident);
      shadowMultiplier = raytrace_shadow(node, shadowRay, lighting);
    }

    Vector3D viewerDirection = -1 * ray.dir;
    viewerDirection.normalize();
    Colour rayLightColour = closestIntersection->material->calculateLighting(incident, closestIntersection->normal, viewerDirection, lightColour);
    finalColour = finalColour + shadowMultiplier * rayLightColour;
  }

  return RayResult(finalColour);
}

Colour raytrace_shadow(SceneNode* node, const Ray& ray, const Lighting& lighting) {
  std::vector<Intersection> intersections = node->findIntersections(ray);

  if (intersections.empty()) {
    return Colour(1.0);
  } else {
    return Colour(0.0);
  }

  /*
  //const double EPSILON = 0.00001;
  const double EPSILON = 0.0;

  for (std::vector<Intersection>::const_iterator it = intersections.begin(); it != intersections.end(); it++) {
    if (it->rayParam > EPSILON) {
      return Colour(0.0);
    }
  }
  return Colour(1.0);
  */
}

Colour genBackground(double x, double y) {
  // Provided background.
  /*
  return Colour(
    y, // Red: increasing from top to bottom
    x, // Green: increasing from left to right
    ((y < 0.5 && x < 0.5) || (y >= 0.5 && x >= 0.5)) ? 1.0 : 0.0 // Blue: in lower-left and upper-right corners
  );
  */

  // Arbitrary sinusoids.
  //return Colour(sin(x)*10.0, sin(1/(y*x))/2.0, sin(x));
  //return Colour(sin(x)/x, sin(1.0/(y*x))/y, tan(x*y/5.0)*2.0) * Colour(0.5);
  return Colour(sin(1.0/(y*x))/y) * Colour(1.0, 0.2, 0.2);
  //return Colour(sin(x*50.0), sin(y*50.0), sin(1.0/(y*x))) * Colour(0.7);
  //return Colour(
    //sqrt((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5) - 0.1),
    //sqrt((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5) - 0.05),
    //sqrt((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5) - 0.2)
  //);
}



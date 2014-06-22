#include "a4.hpp"
#include "image.hpp"
#include "matrices.hpp"
#include "algebra.hpp"

enum Axis {
  X = 0,
  Y = 1,
  Z = 2,
  W = 3
};

void a4_render(
  SceneNode* root, // What to render
  const std::string& filename, // Where to output the image
  int width, int height, // Image size
  const Point3D& eye, const Vector3D& view,
  const Vector3D& up, double fov, // Viewing parameters
  const Colour& ambient, const std::list<Light*>& lights // Lighting parameters
) {
  /*
  std::cerr << "Stub: a4_render(" << root << ",\n     "
            << filename << ", " << width << ", " << height << ",\n     "
            << eye << ", " << view << ", " << up << ", " << fov << ",\n     "
            << ambient << ",\n     {";

  for (std::list<Light*>::const_iterator I = lights.begin(); I != lights.end(); ++I) {
    if (I != lights.begin()) std::cerr << ", ";
    std::cerr << **I;
  }
  std::cerr << "});" << std::endl;
  */

  ViewParams viewParams(eye, view, up, fov);
  Lighting lighting(ambient, lights);

  Image img(width, height, 3);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < height; x++) {
      Colour colour = raytrace_pixel(root, x, y, width, height, viewParams, lighting);

      img(x, y, 0) = colour.R();
      img(x, y, 1) = colour.G();
      img(x, y, 2) = colour.B();

      /*
      // Red: increasing from top to bottom
      img(x, y, 0) = (double)y / height;
      // Green: increasing from left to right
      img(x, y, 1) = (double)x / width;
      // Blue: in lower-left and upper-right corners
      img(x, y, 2) = ((y < height/2 && x < height/2)
                      || (y >= height/2 && x >= height/2)) ? 1.0 : 0.0;
      */
    }
  }
  img.savePng(filename);

}


Colour raytrace_pixel(SceneNode* node,
  int x, int y,
  int width, int height,
  const ViewParams& view,
  const Lighting& lighting
) {
  double d = 2.0; // TODO - Is this derived from something?
  double virtualH = 2.0 * d * tan(view.fov / 2.0);
  double virtualW = ((double)width) / height * virtualW;

  Point3D pixel(x, y, d);

  Matrix4x4 translatePixelToOrigin = translation(Vector3D(-width/d, -height/d, -d));
  Matrix4x4 scalePixel = scaling(Vector3D(-virtualW/width, -virtualH/height, 1.0));

  const Vector3D& w = view.view;
  Vector3D u = view.up.cross(w);
  u.normalize();
  Vector3D v = w.cross(u);

  Matrix4x4 rotatePixelToWCS = Matrix4x4((double[16]) {
    u[X], v[X], w[X], 0,
    u[Y], v[Y], w[Y], 0,
    u[Z], v[Z], w[Z], 0,
       0,    0,    0, 1
  });

  Matrix4x4 pixelEyeTranslation = translation(view.eye - Point3D());

  Point3D pixelWorld = pixelEyeTranslation * rotatePixelToWCS * scalePixel * translatePixelToOrigin * pixel;

  Ray ray(view.eye, pixelWorld - view.eye);
  return raytrace_visible(node, ray, lighting);
}

Colour raytrace_visible(SceneNode* node, const Ray& ray, const Lighting& lighting) {
  return Colour(0.5);
}

Colour raytrace_shadow(SceneNode* node, const Ray& ray, const Lighting& lighting) {
  return Colour(0.5);
}

// Returns intersections sorted by distance.
std::vector<Intersection> ray_intersections(SceneNode* node, const Ray& ray) {
  std::vector<Intersection> intersections;
  return intersections;
}



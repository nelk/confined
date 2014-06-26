#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include <vector>
#include "algebra.hpp"
#include "raytracer.hpp"

class Primitive {
public:
  virtual ~Primitive() {}

  virtual RayResult* findIntersections(const Ray& ray) = 0;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(pos), m_radius(radius) {
  }
  virtual ~NonhierSphere() {}

  virtual RayResult* findIntersections(const Ray& ray);

private:
  Point3D m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size) {
  }

  virtual ~NonhierBox() {}

  virtual RayResult* findIntersections(const Ray& ray);

private:
  Point3D m_pos;
  double m_size;
};

class Sphere : public NonhierSphere {
public:
  Sphere(): NonhierSphere(Point3D(), 1.0) {}
  virtual ~Sphere() {}
};

class Cube : public NonhierBox {
public:
  Cube(): NonhierBox(Point3D(-0.5, -0.5, -0.5), 1.0) {}
  virtual ~Cube() {}
};


#endif

#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include <vector>
#include "algebra.hpp"
#include "raytracer.hpp"

class Primitive {
public:
  virtual ~Primitive();

  virtual std::vector<Intersection> findIntersections(const Ray& ray) = 0;
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();

  virtual std::vector<Intersection> findIntersections(const Ray& ray);
};

class Cube : public Primitive {
public:
  virtual ~Cube();

  virtual std::vector<Intersection> findIntersections(const Ray& ray);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(pos), m_radius(radius) {
  }
  virtual ~NonhierSphere();

  virtual std::vector<Intersection> findIntersections(const Ray& ray);

private:
  Point3D m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size) {
  }

  virtual ~NonhierBox();

  virtual std::vector<Intersection> findIntersections(const Ray& ray);

private:
  Point3D m_pos;
  double m_size;
};

#endif

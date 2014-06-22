#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include "algebra.hpp"
#include "raytracer.hpp"

struct Ray;

class Material {
public:
  virtual ~Material();
  virtual void apply_gl() const = 0;

  virtual Colour rayColour(const Ray& ray) = 0;

protected:
  Material() {
  }
};

class PhongMaterial : public Material {
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess);
  virtual ~PhongMaterial();

  virtual void apply_gl() const;

  virtual Colour rayColour(const Ray& ray);

private:
  Colour m_kd;
  Colour m_ks;

  double m_shininess;
};


#endif

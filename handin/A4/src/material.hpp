#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include "algebra.hpp"

class Material {
public:
  virtual ~Material();
  virtual void apply_gl() const = 0;

  virtual double reflectance() const = 0;
  virtual Colour ambientColour() const = 0;
  virtual Colour calculateLighting(const Vector3D& incident, const Vector3D& normal, const Vector3D& reflected, const Vector3D& viewer, const Colour& intensity) const = 0;

protected:
  Material() {
  }
};

class PhongMaterial : public Material {
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess, double reflectance=0.0);
  virtual ~PhongMaterial();

  virtual void apply_gl() const;

  virtual double reflectance() const;
  virtual Colour ambientColour() const;
  virtual Colour calculateLighting(const Vector3D& incident, const Vector3D& normal, const Vector3D& reflected, const Vector3D& viewer, const Colour& intensity) const;

private:
  Colour m_kd;
  Colour m_ks;

  double m_shininess;
  double m_reflectance;
};


#endif

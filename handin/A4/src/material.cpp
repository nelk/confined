#include "material.hpp"

#define BLINN_PHONG

Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess, double reflectance)
  : m_kd(kd), m_ks(ks), m_shininess(shininess), m_reflectance(reflectance) {
}

PhongMaterial::~PhongMaterial() {
}

void PhongMaterial::apply_gl() const {
  // Perform OpenGL calls necessary to set up this material.
}

Colour PhongMaterial::ambientColour() const {
  return m_kd;
}

double PhongMaterial::reflectance() const {
  return m_reflectance;
}

Colour PhongMaterial::calculateLighting(const Vector3D& incident, const Vector3D& normal, const Vector3D& reflected, const Vector3D& viewer, const Colour& intensity) const {
  double incidentDotNormal = std::max(0.0, incident.dot(normal));

  Colour diffuse = m_kd * incidentDotNormal * intensity;

#ifdef BLINN_PHONG
  // Blinn-Phong.
  Vector3D h = viewer + incident;
  h.normalize();
  Colour specular = m_ks * pow(std::max(0.0, h.dot(normal)), m_shininess) * intensity;
#else
  // Phong.
  Colour specular = m_ks * pow(reflected.dot(viewer), m_shininess) * intensity;
#endif

  //std::cout << "incident " << incident << ", normal " << normal << ", viewer " << viewer << ", intensity " << intensity << ", result=" << diffuse+specular << std::endl;

  return diffuse + specular;
}


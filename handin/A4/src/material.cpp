#include "material.hpp"

Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess)
  : m_kd(kd), m_ks(ks), m_shininess(shininess) {
}

PhongMaterial::~PhongMaterial() {
}

void PhongMaterial::apply_gl() const {
  // Perform OpenGL calls necessary to set up this material.
}

Colour PhongMaterial::calculateLighting(Vector3D incident, Vector3D normal, Colour intensity) {
  // TODO: Specular.
  return m_kd * (incident.dot(normal)) * intensity;
}


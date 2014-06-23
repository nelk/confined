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

Colour PhongMaterial::ambientColour() const {
  return m_kd;
}

Colour PhongMaterial::calculateLighting(Vector3D incident, Vector3D normal, Vector3D viewer, Colour intensity) {
  double incidentDotNormal = incident.dot(normal);
  Colour diffuse = m_kd * (incidentDotNormal) * intensity;
  // Phong.
  //Vector3D reflected = incident - 2 * incident.dot(normal) * normal;
  //Colour specular = m_ks * pow(reflected.dot(viewer), m_shininess) * intensity;

  // Blinn-Phong.
  Vector3D h = viewer + incident;
  h.normalize();
  Colour specular = m_ks * pow(h.dot(normal), m_shininess) * intensity;
  return diffuse + specular;
}


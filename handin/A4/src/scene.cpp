#include "scene.hpp"
#include <iostream>
#include "matrices.hpp"

SceneNode::SceneNode(const std::string& name)
  : m_name(name) {
}

SceneNode::~SceneNode() {
}

void SceneNode::rotate(char axis, double angleDegrees) {
  double angle = angleDegrees * M_PI / 180.0;
  m_trans = m_trans * rotation(axis, angle);
  m_invtrans = m_trans.invert();
}

void SceneNode::scale(const Vector3D& amount) {
  m_trans = m_trans * scaling(amount);
  m_invtrans = m_trans.invert();
}

void SceneNode::translate(const Vector3D& amount) {
  m_trans = m_trans * translation(amount);
  m_invtrans = m_trans.invert();
}

bool SceneNode::is_joint() const {
  return false;
}

RayResult* SceneNode::findIntersections(const Ray& ray) {
  Ray transformedRay = ray.transform(get_inverse());
  RayResult* result = new RayResult(std::vector<Intersection>(), 0);
  for(std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    RayResult* childResult = (*it)->findIntersections(transformedRay);
    result->merge(*childResult);
    delete childResult;
  }
  transformIntersectionsUp(result->intersections);
  return result;
}

void SceneNode::transformIntersectionsUp(std::vector<Intersection>& intersections) {
  for(std::vector<Intersection>::iterator it = intersections.begin(); it != intersections.end(); it++) {
    it->transform(get_transform());
  }
}

JointNode::JointNode(const std::string& name)
  : SceneNode(name) {
}

JointNode::~JointNode() {
}

bool JointNode::is_joint() const {
  return true;
}

void JointNode::set_joint_x(double min, double init, double max) {
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max) {
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name), m_primitive(primitive) {
}

GeometryNode::~GeometryNode() {
}

RayResult* GeometryNode::findIntersections(const Ray& ray) {
  Ray transformedRay = ray.transform(get_inverse());
  RayResult* result = m_primitive->findIntersections(transformedRay);
  for (std::vector<Intersection>::iterator it = result->intersections.begin(); it != result->intersections.end(); it++) {
    it->material = m_material;
  }
  transformIntersectionsUp(result->intersections);

  RayResult* childResult = SceneNode::findIntersections(ray);
  result->merge(*childResult);
  delete childResult;

  return result;
}


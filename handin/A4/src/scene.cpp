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
 

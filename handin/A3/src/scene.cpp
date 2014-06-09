#include "scene.hpp"
#include "matrices.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name)
  : m_name(name)
{
}

SceneNode::~SceneNode() {
}

void SceneNode::walk_gl(bool picking) const
{
  push_transform_gl();

  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->walk_gl(picking);
  }

  pop_transform_gl();
}

void SceneNode::rotate(char axis, double angleDegrees)
{
  //std::cerr << "Rotate " << m_name << " around " << axis << " by " << angleDegrees << std::endl;

  double angle = angleDegrees * M_PI / 180.0;
  m_trans = m_trans * rotation(axis, angle);
}

void SceneNode::scale(const Vector3D& amount)
{
  //std::cerr << "Scale " << m_name << " by " << amount << std::endl;

  m_trans = m_trans * scaling(amount);
}

void SceneNode::translate(const Vector3D& amount)
{
  //std::cerr << "Translate " << m_name << " by " << amount << std::endl;

  m_trans = m_trans * translation(amount);
}

void SceneNode::preRotate(char axis, double angleDegrees) {
  double angle = angleDegrees * M_PI / 180.0;
  m_trans = rotation(axis, angle) * m_trans;
}
void SceneNode::preScale(const Vector3D& amount) {
  m_trans = scaling(amount) * m_trans;
}
void SceneNode::preTranslate(const Vector3D& amount) {
  m_trans = translation(amount) * m_trans;
}

bool SceneNode::is_joint() const
{
  return false;
}

JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
}

JointNode::~JointNode()
{
}

void JointNode::walk_gl(bool picking) const
{
  SceneNode::walk_gl();
  // Fill me in
}

bool JointNode::is_joint() const
{
  return true;
}

void JointNode::set_joint_x(double min, double init, double max)
{
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max)
{
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name),
    m_primitive(primitive)
{
}

GeometryNode::~GeometryNode()
{
}

void GeometryNode::walk_gl(bool picking) const {
  SceneNode::walk_gl(picking);

  push_transform_gl();
  get_material()->apply_gl();

  m_primitive->walk_gl(picking);

  pop_transform_gl();
}


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
  jointRotation[X] = 0.0;
  jointRotation[Y] = 0.0;
  jointRotation[Z] = 0.0;
}

JointNode::~JointNode()
{
}

void JointNode::setJointRange(Axis axis, double min, double init, double max) {
  jointRanges[axis].min = min;
  jointRanges[axis].init = init;
  jointRanges[axis].max = max;
  jointRotation[axis] = init;
}

void JointNode::rotateJoint(Axis axis, double delta) {
  const JointRange& range = jointRanges[axis];
  double newRot = jointRotation[axis] + delta;

  if (newRot < range.min) {
    newRot = range.min;
  } else if (newRot > range.max) {
    newRot = range.max;
  }
  jointRotation[axis] = newRot;
}

void JointNode::walk_gl(bool picking) const {
  glPushMatrix();

  glRotated(jointRotation[X], 1.0, 0.0, 0.0);
  glRotated(jointRotation[Y], 0.0, 1.0, 0.0);
  glRotated(jointRotation[Z], 0.0, 0.0, 1.0);

  SceneNode::walk_gl();

  glPopMatrix();
}

bool JointNode::is_joint() const
{
  return true;
}


GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name),
    m_material(NULL),
    m_primitive(primitive)
{
}

GeometryNode::~GeometryNode()
{
}

void GeometryNode::walk_gl(bool picking) const {
  SceneNode::walk_gl(picking);

  push_transform_gl();
  const Material* material = get_material();
  if (material != NULL) {
    material->apply_gl();
  }

  m_primitive->walk_gl(picking);

  pop_transform_gl();
}


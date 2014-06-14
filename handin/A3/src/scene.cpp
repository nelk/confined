#include "scene.hpp"
#include "matrices.hpp"
#include <iostream>

int SceneNode::nextId = 1;

SceneNode::SceneNode(const std::string& name)
    : m_name(name), picked(false) {
  m_id = nextId++;
}

SceneNode::~SceneNode() {
}

bool SceneNode::togglePick(int id, bool parent_is_joint) {
  if (m_id == id) {
    if (parent_is_joint) {
      picked = !picked;
      return true;
    } else {
      // Parent not joint, don't pick anything.
      return false;
    }
  }
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    if ((*it)->togglePick(id, is_joint())) {
      return true;
    }
  }
  return false;
}

void SceneNode::moveJoints(double primaryDelta, double secondaryDelta) {
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->moveJoints(primaryDelta, secondaryDelta);
  }
}

void SceneNode::resetJoints() {
  picked = false;
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->resetJoints();
  }
}

void SceneNode::saveJointUndoState() {
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->saveJointUndoState();
  }
}

void SceneNode::undoJoints() {
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->undoJoints();
  }
}

void SceneNode::redoJoints() {
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->redoJoints();
  }
}

void SceneNode::walk_gl(bool picking) const {
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
  m_invtrans = m_trans.invert();
}

void SceneNode::scale(const Vector3D& amount)
{
  //std::cerr << "Scale " << m_name << " by " << amount << std::endl;

  m_trans = m_trans * scaling(amount);
  m_invtrans = m_trans.invert();
}

void SceneNode::translate(const Vector3D& amount)
{
  //std::cerr << "Translate " << m_name << " by " << amount << std::endl;

  m_trans = m_trans * translation(amount);
  m_invtrans = m_trans.invert();
}

void SceneNode::preRotate(char axis, double angleDegrees) {
  double angle = angleDegrees * M_PI / 180.0;
  m_trans = rotation(axis, angle) * m_trans;
  m_invtrans = m_trans.invert();
}
void SceneNode::preScale(const Vector3D& amount) {
  m_trans = scaling(amount) * m_trans;
  m_invtrans = m_trans.invert();
}
void SceneNode::preTranslate(const Vector3D& amount) {
  m_trans = translation(amount) * m_trans;
  m_invtrans = m_trans.invert();
}

bool SceneNode::is_joint() const
{
  return false;
}


JointNode::JointNode(const std::string& name)
  : SceneNode(name), jointRotation(NUM_AXES, 0.0)
{
  for (int a = 0; a < NUM_AXES; a++) {
    jointRanges[a].min = 0;
    jointRanges[a].init = 0;
    jointRanges[a].max = 0;
  }
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

void JointNode::resetJoints() {
  jointRotation[X] = jointRanges[X].init;
  jointRotation[Y] = jointRanges[Y].init;
  jointRotation[Z] = jointRanges[Z].init;
  undoStack.clear();
  undoStack.push_back(jointRotation);
  redoStack.clear();
  SceneNode::resetJoints();
}

void JointNode::saveJointUndoState() {
  SceneNode::saveJointUndoState();
  redoStack.clear();
  undoStack.push_back(jointRotation);
}

void JointNode::undoJoints() {
  SceneNode::undoJoints();
  // Don't remove last item.
  if (undoStack.size() <= 1) {
    return;
  }
  redoStack.push_back(jointRotation);
  undoStack.pop_back();
  jointRotation = undoStack.back();
}

void JointNode::redoJoints() {
  SceneNode::redoJoints();
  if (redoStack.empty()) {
    return;
  }
  jointRotation = redoStack.back();
  undoStack.push_back(jointRotation);
  redoStack.pop_back();
}


void JointNode::moveJoints(double primaryDelta, double secondaryDelta) {
  bool childPicked = false;
  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    if ((*it)->isPicked()) {
      childPicked = true;
    }
    (*it)->moveJoints(primaryDelta, secondaryDelta);
  }
  if (childPicked) {
    rotateJoint(X, primaryDelta);
    rotateJoint(Y, secondaryDelta);
  }
}

void JointNode::walk_gl(bool picking) const {
  push_transform_gl();

  glRotated(jointRotation[X], 1.0, 0.0, 0.0);
  glRotated(jointRotation[Y], 0.0, 1.0, 0.0);
  glRotated(jointRotation[Z], 0.0, 0.0, 1.0);

  for (std::list<SceneNode*>::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->walk_gl(picking);
  }

  pop_transform_gl();
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

bool GeometryNode::pickHighlight = false;
PhongMaterial GeometryNode::highlightMaterial(Colour(1.0, 1.0, 1.0), Colour(0.2, 0.2, 0.2), 15.0);
PhongMaterial GeometryNode::defaultMaterial(Colour(0.5, 0.5, 0.5), Colour(0.1, 0.1, 0.1), 10.0);

void GeometryNode::walk_gl(bool picking) const {
  SceneNode::walk_gl(picking);

  push_transform_gl();
  if (!picking) {
    const Material* material = get_material();
    if (picked && pickHighlight) {
      highlightMaterial.apply_gl();
    } else if (material != NULL) {
      material->apply_gl();
    } else {
      defaultMaterial.apply_gl();
    }
  }

  if (picking) {
    glPushName(m_id);
  }
  m_primitive->walk_gl(picking);
  if (picking) {
    glPopName();
  }

  pop_transform_gl();
}


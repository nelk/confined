#ifndef SCENE_HPP
#define SCENE_HPP

#include <list>
#include "algebra.hpp"
#include "primitive.hpp"
#include "material.hpp"

enum Axis {
  X = 0,
  Y = 1,
  Z = 2,
  NUM_AXES = 3
};

class SceneNode {
public:
  SceneNode(const std::string& name);
  virtual ~SceneNode();

  virtual void walk_gl(bool picking = false) const;

  const Matrix4x4& get_transform() const { return m_trans; }
  const Matrix4x4& get_inverse() const { return m_invtrans; }

  void set_transform(const Matrix4x4& m)
  {
    m_trans = m;
    m_invtrans = m.invert();
  }

  void set_transform(const Matrix4x4& m, const Matrix4x4& i)
  {
    m_trans = m;
    m_invtrans = i;
  }

  void push_transform_gl() const {
    glPushMatrix();
    Matrix4x4 columnMajorTrans = m_trans.transpose();
    //std::cout << "Pushing " << columnMajorTrans << std::endl;
    glMultMatrixd(columnMajorTrans.begin());
  }

  void pop_transform_gl() const {
    //std::cout << "Popping" << std::endl;
    glPopMatrix();
  }

  void add_child(SceneNode* child)
  {
    m_children.push_back(child);
  }

  void remove_child(SceneNode* child)
  {
    m_children.remove(child);
  }

  // Callbacks to be implemented.
  // These will be called from Lua.
  // Note that these are post-multiplied.
  void rotate(char axis, double angle);
  void scale(const Vector3D& amount);
  void translate(const Vector3D& amount);

  void preRotate(char axis, double angle);
  void preScale(const Vector3D& amount);
  void preTranslate(const Vector3D& amount);

  // Returns true if and only if this node is a JointNode
  virtual bool is_joint() const;

  bool togglePick(int id);

protected:
  static int nextId;

  // Useful for picking.
  int m_id;
  std::string m_name;

  bool picked;

  // Transformations
  Matrix4x4 m_trans;
  Matrix4x4 m_invtrans;

  // Hierarchy
  typedef std::list<SceneNode*> ChildList;
  ChildList m_children;
};

class JointNode : public SceneNode {
public:
  JointNode(const std::string& name);
  virtual ~JointNode();

  virtual void walk_gl(bool picking = false) const;

  virtual bool is_joint() const;

  void set_joint_x(double min, double init, double max) {
    setJointRange(X, min, init, max);
  }
  void set_joint_y(double min, double init, double max) {
    setJointRange(Y, min, init, max);
  }

  // Set range and set current rotation to init.
  void setJointRange(Axis axis, double min, double init, double max);
  void rotateJoint(Axis axis, double delta);
  void resetJoint();

  struct JointRange {
    double min, init, max;
  };

protected:

  JointRange jointRanges[NUM_AXES];
  double jointRotation[NUM_AXES];
};

class GeometryNode : public SceneNode {
public:
  GeometryNode(const std::string& name,
               Primitive* primitive);
  virtual ~GeometryNode();

  virtual void walk_gl(bool picking = false) const;

  const Material* get_material() const {
    return m_material;
  }
  Material* get_material() {
    return m_material;
  }

  void set_material(Material* material)
  {
    m_material = material;
  }

  static void togglePickHighlight() {
    pickHighlight = !pickHighlight;
  }

protected:
  static bool pickHighlight;
  static PhongMaterial highlightMaterial;
  static PhongMaterial defaultMaterial;

  Material* m_material;
  Primitive* m_primitive;
};

#endif

#ifndef CS488_MESH_HPP
#define CS488_MESH_HPP

#include <vector>
#include <iosfwd>
#include "primitive.hpp"
#include "algebra.hpp"
#include "raytracer.hpp"
#include "scene.hpp"

#define DRAW_BOUNDING_BOXES false

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh(const std::vector<Point3D>& verts,
       const std::vector< std::vector<int> >& faces,
       int depth=0);

  virtual ~Mesh();

  virtual RayResult* findIntersections(const Ray& ray);

  typedef std::vector<int> Face;

  struct FaceComparator {
    FaceComparator(Axis axis, Mesh* mesh): axis(axis), mesh(mesh) {}

    bool operator()(const Face& a, const Face& b) {
      return mesh->m_verts[a[0]][axis] < mesh->m_verts[b[0]][axis];
    }

    Axis axis;
    Mesh* mesh;
  };

private:
  std::vector<Point3D> m_verts;
  std::vector<Face> m_faces;
  SceneNode* m_bound;
  SceneNode* m_descendents;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};


#endif

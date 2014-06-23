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
       const std::vector< std::vector<int> >& faces);

  virtual ~Mesh();

  typedef std::vector<int> Face;

  virtual std::vector<Intersection> findIntersections(const Ray& ray);

private:
  std::vector<Point3D> m_verts;
  std::vector<Face> m_faces;
  GeometryNode* m_bound;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};

#endif

#include "mesh.hpp"
#include <iostream>

Mesh::Mesh(const std::vector<Point3D>& verts,
           const std::vector< std::vector<int> >& faces)
  : m_verts(verts), m_faces(faces) {
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh) {
  std::cerr << "mesh({";
  for (std::vector<Point3D>::const_iterator I = mesh.m_verts.begin(); I != mesh.m_verts.end(); ++I) {
    if (I != mesh.m_verts.begin()) std::cerr << ",\n      ";
    std::cerr << *I;
  }
  std::cerr << "},\n\n     {";

  for (std::vector<Mesh::Face>::const_iterator I = mesh.m_faces.begin(); I != mesh.m_faces.end(); ++I) {
    if (I != mesh.m_faces.begin()) std::cerr << ",\n      ";
    std::cerr << "[";
    for (Mesh::Face::const_iterator J = I->begin(); J != I->end(); ++J) {
      if (J != I->begin()) std::cerr << ", ";
      std::cerr << *J;
    }
    std::cerr << "]";
  }
  std::cerr << "});" << std::endl;
  return out;
}


std::vector<Intersection> Mesh::findIntersections(const Ray& ray) {
  std::vector<Intersection> intersections;

  // Convext angle tests.
  // TODO: Ray casting algorithm if we have convex faces.
  /*
  const double EPSILON = 0.0;
  for (std::vector<Face>::const_iterator it = m_faces.begin(); it != m_faces.end(); it++) {
    const Face& face = *it;
    if (face.size() < 3) continue;
    Vector3D normal;
    Vector3D lastV = m_verts[face[face.size() - 1]] - m_verts[face[0]];
    Vector3D currentV = m_verts[face[0]] - m_verts[face[1]];
    normal = (-1 * lastV).cross(currentV);
    normal.normalize();

    double t = -(ray.pos[X] + ray.pos[Y] + ray.pos[Z])/(normal[X]*ray.dir[X] + normal[Y]*ray.dir[Y] + normal[Z]*ray.dir[Z]);
    if (t < EPSILON) {
      //break; // TODO
      continue; // Pointing away from face.
    }
    Point3D p = ray.pos + t*ray.dir;

    for (unsigned int i = 0; i < face.size(); i++) {
      if (i > 0) {
        currentV = m_verts[face[i]] - m_verts[face[i == face.size() - 1 ? 0 : i + 1]];
      }

      double thedot = lastV.dot(p - m_verts[face[i]]);
      if (lastV.dot(currentV) < thedot) {
        //break; // TODO
        continue; // Point outside.
      }

      lastV = currentV;
    }

    intersections.push_back(Intersection(t, normal, NULL));
    //break; // TODO
  }
  */

  // Reinier van Vliet and Remco Lam angle sums algorithm.
  const double EPSILON = 0.0000001;
  for (std::vector<Face>::const_iterator it = m_faces.begin(); it != m_faces.end(); it++) {
    const Face& face = *it;
    if (face.size() < 3) {
      std::cout << "Warn: Face with less than 3 verts found." << std::endl;
      continue;
    }

    Vector3D v1 = m_verts[face[1]] - m_verts[face[0]];
    Vector3D v2 = m_verts[face[1]] - m_verts[face[2]];
    //Vector3D normal = v1.cross(v2);
    Vector3D normal = v2.cross(v1);
    normal.normalize();

    //double t = (ray.pos[X] + ray.pos[Y] + ray.pos[Z])/(normal[X]*ray.dir[X] + normal[Y]*ray.dir[Y] + normal[Z]*ray.dir[Z]);
    double t = -(ray.pos - m_verts[face[0]]).dot(normal) / ray.dir.dot(normal);
    if (t < EPSILON) {
      //std::cout << "Jump ship! t=" << t << std::endl;
      continue; // Pointing away from face.
    }
    Point3D q = ray.pos + t*ray.dir;

    double anglesum = 0.0;
    for (unsigned int i = 0; i < face.size(); i++) {
      Vector3D p1 = m_verts[face[i]] - q;
      Vector3D p2 = m_verts[face[(i+1)%face.size()]] - q;

      double m1 = p1.length();
      double m2 = p2.length();
      if (m1*m2 <= EPSILON) {
        anglesum = M_PI*2; // We are on a node, consider this inside.
      } else {
        anglesum += acos(p1.dot(p2) / (m1*m2));
      }
      if (anglesum >= M_PI*2 - EPSILON && anglesum <= M_PI*2 + EPSILON) {
        intersections.push_back(Intersection(t, normal, NULL));
        //std::cout << "INTERSECTION " << t << std::endl;
      }
    }
    //std::cout << "Exitted with anglesum=" << anglesum << std::endl;
  }

  return intersections;
}


#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "material.hpp"

class Mesh {
public:
  enum BufferIndex {
    VERTEX_BUF,
    UV_BUF,
    NORMAL_BUF,
    ELEMENT_BUF,
    NUM_BUFS
  };

  Mesh(std::vector<glm::vec3>& vertices,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned short>& triangles,
    Material* material
  );
  ~Mesh();

  Material* getMaterial() {
    return material;
  }
  void renderGLVertsOnly();
  void renderGL();

  int getNumIndices() {
    return numIndices;
  }

private:
  GLuint buffers[NUM_BUFS];
  int numIndices;
  Material* material;
};

std::vector<Mesh*> loadScene(const char* fileName);

#endif

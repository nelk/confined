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
    VERTEX_BUF = 0,
    UV_BUF = 1,
    NORMAL_BUF = 2,
    TANGENT_BUF = 3,
    BITANGENT_BUF = 3,
    ELEMENT_BUF = 4,
    NUM_BUFS = 5
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

  glm::mat4& getModelMatrix() {
    return modelMatrix;
  }

private:
  GLuint buffers[NUM_BUFS];
  int numIndices;
  Material* material;
  glm::mat4 modelMatrix;
};

std::vector<Mesh*> loadScene(const char* fileName, bool invertNormals = false);

#endif

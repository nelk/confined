
#include "mesh.hpp"
#include <glm/glm.hpp>
#include <FreeImage.h>
#include <iostream>

Mesh::Mesh(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned short>& indices,
    Material* material): material(material) {

  // Load scene data into VBOs.
  glGenBuffers(NUM_BUFS, buffers);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[UV_BUF]);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUF]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

  numIndices = indices.size();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ELEMENT_BUF]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

  std::cout << "Created Mesh with " << numIndices << " indices" << std::endl;
}

Mesh::~Mesh() {
  glDeleteBuffers(NUM_BUFS, buffers);
}


// Bind only vertices and index buffer.
void Mesh::renderGLVertsOnly() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);

  glVertexAttribPointer(
    0,  // The attribute we want to configure
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // Index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ELEMENT_BUF]);

  glDrawElements(
    GL_TRIANGLES,      // mode
    numIndices,        // count
    GL_UNSIGNED_SHORT, // type
    (void*)0           // element array buffer offset
  );

  glDisableVertexAttribArray(0);
}

void Mesh::renderGL() {
  // 1st attribute buffer - vertices.
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);
  glVertexAttribPointer(
    0,                  // attribute
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // 2nd attribute buffer - UVs.
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[UV_BUF]);
  glVertexAttribPointer(
    1,                                // attribute
    2,                                // size
    GL_FLOAT,                         // type
    GL_FALSE,                         // normalized?
    0,                                // stride
    (void*)0                          // array buffer offset
  );

  // 3rd attribute buffer - normals.
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUF]);
  glVertexAttribPointer(
    2,                                // attribute
    3,                                // size
    GL_FLOAT,                         // type
    GL_FALSE,                         // normalized?
    0,                                // stride
    (void*)0                          // array buffer offset
  );

  // Index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ELEMENT_BUF]);

  // TODO: Bind material properties.

  // Draw the triangles for render pass.
  glDrawElements(
    GL_TRIANGLES,      // mode
    numIndices,        // count
    GL_UNSIGNED_SHORT, // type
    (void*)0           // element array buffer offset
  );

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void MessageFunction(FREE_IMAGE_FORMAT fif, const char *msg) {
  std::cerr << (int)fif << ": " << msg << std::endl;
}

std::vector<Mesh*> loadScene(const char* fileName) {

  FreeImage_SetOutputMessage(MessageFunction); 

  std::vector<Mesh*> meshes;
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(fileName, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
  if (!scene) {
    std::cerr << importer.GetErrorString() << std::endl;
    return meshes;
  }

  // Load materials.
  Material* materials[scene->mNumMaterials];
  for (unsigned int matId = 0; matId < scene->mNumMaterials; matId++) {
    const aiMaterial* m = scene->mMaterials[matId];
    aiColor3D ka(0, 0, 0);
    aiColor3D kd(0, 0, 0);
    aiColor3D ks(0, 0, 0);
    float shininess = 0.0;
    m->Get(AI_MATKEY_COLOR_AMBIENT, ka);
    m->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
    m->Get(AI_MATKEY_COLOR_SPECULAR, ks);
    m->Get(AI_MATKEY_SHININESS, shininess);
    //AI_MATKEY_COLOR_REFLECTIVE

    materials[matId] = new Material(
      glm::vec3(ka.r, ka.g, ka.b),
      glm::vec3(kd.r, kd.g, kd.b),
      glm::vec3(ks.r, ks.g, ks.b),
      shininess);

    int num_textures = m->GetTextureCount(aiTextureType_DIFFUSE);
    if (num_textures >= 1) {
      aiString texFileName;
      if (m->GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == AI_SUCCESS) {
        std::string prefixedTexFileName = "models/" + std::string(texFileName.C_Str());
        FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(prefixedTexFileName.c_str(), 0), prefixedTexFileName.c_str());
        FIBITMAP *pImage = FreeImage_ConvertTo32Bits(bitmap);

        const int texWidth = FreeImage_GetWidth(bitmap);
        const int texHeight = FreeImage_GetHeight(bitmap);

        /*
        BYTE raw[texWidth*texHeight];
        FreeImage_ConvertToRawBits(raw, pImage, texWidth*texHeight, 32, 1, 1, 1, false);

        materials[matId]->setTexture(texWidth, texHeight, (void*) raw);
        */

        materials[matId]->setTexture(texWidth, texHeight, (void*) FreeImage_GetBits(pImage));

        FreeImage_Unload(pImage);
        FreeImage_Unload(bitmap);
      }
    }
  }

  // Load meshes.
  for (unsigned int meshId = 0; meshId < scene->mNumMeshes; meshId++) {
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    const aiMesh* mesh = scene->mMeshes[meshId];
    Material* material = NULL;

    unsigned int materialIndex = mesh->mMaterialIndex;
    if (materialIndex < scene->mNumMaterials) {
      material = materials[materialIndex];
    }

    // Vertex positions.
    vertices.reserve(mesh->mNumVertices);
    for(unsigned int i=0; i<mesh->mNumVertices; i++){
      aiVector3D pos = mesh->mVertices[i];
      vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }

    // Vertex texture coordinates.
    if (mesh->HasTextureCoords(0)) {
      uvs.reserve(mesh->mNumVertices);
      for(unsigned int i=0; i<mesh->mNumVertices; i++){
        aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
        uvs.push_back(glm::vec2(UVW.x, UVW.y));
      }
    }

    // Vertex normals.
    if (mesh->HasNormals()) {
      normals.reserve(mesh->mNumVertices);
      for(unsigned int i=0; i<mesh->mNumVertices; i++){
        aiVector3D n = mesh->mNormals[i];
        normals.push_back(glm::vec3(n.x, n.y, n.z));
      }
    }

    // Face indices.
    indices.reserve(3*mesh->mNumFaces);
    for (unsigned int i=0; i<mesh->mNumFaces; i++){
      // Only supporting triangles here.
      indices.push_back(mesh->mFaces[i].mIndices[0]);
      indices.push_back(mesh->mFaces[i].mIndices[1]);
      indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    meshes.push_back(new Mesh(vertices, uvs, normals, indices, material));
  }

  // TODO: Don't leak materials.
  return meshes;
}



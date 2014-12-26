
#include "mesh.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <list>

#include "mirror.hpp"
#include "texture.hpp"
#include "shader.hpp"

uint32_t Mesh::meshIdCounter = 1;

Mesh::Mesh(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals,
    std::vector<unsigned short>& indices,
    Material* material): name(""), material(material) {

  meshId = meshIdCounter++;

  modelMatrix = glm::mat4(1.0);

  for (int i = 0; i < NUM_BUFS; i++) {
    buffers[i] = 0;
  }

  // Load scene data into VBOs.
  glGenBuffers(NUM_BUFS, buffers);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[VERTEX_BUF]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[UV_BUF]);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_BUF]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

  // Construct tangents.
  std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0, 0, 0));
  std::vector<glm::vec3> bitangents(vertices.size(), glm::vec3(0, 0, 0));
  // Only do tangents if there are enough UVs.
  //std::cerr << "#UVs: " << uvs.size() << ", #Vertices: " << vertices.size() << std::endl;
  if (uvs.size() >= vertices.size()) {
    // Go through each triangular face and add tangents.
    for (unsigned int face = 0; face*3 + 2 < indices.size(); face++) {
      int p[] = {
        indices[face*3],
        indices[face*3+1],
        indices[face*3+2]
      };

      // Edges of the triangle - position delta.
      glm::vec3 deltaPos1 = vertices[p[1]] - vertices[p[0]];
      glm::vec3 deltaPos2 = vertices[p[2]] - vertices[p[0]];

      // UV delta
      glm::vec2 deltaUV1 = uvs[p[1]] - uvs[p[0]];
      glm::vec2 deltaUV2 = uvs[p[2]] - uvs[p[0]];

      /*
      std::cerr << "indices " << p[0] << ", " << p[1] << ", " << p[2] << std::endl;
      std::cerr << "position "
        << "(" << vertices[p[0]][0] << ", " << vertices[p[0]][1] << ", " << vertices[p[0]][2] << "), "
        << "(" << vertices[p[1]][0] << ", " << vertices[p[1]][1] << ", " << vertices[p[1]][2] << "), "
        << "(" << vertices[p[2]][0] << ", " << vertices[p[2]][1] << ", " << vertices[p[2]][2] << ")"
        << std::endl;
      std::cerr << "deltaPos1 "
        << "(" << deltaPos1[0] << ", " << deltaPos1[1] << ", " << deltaPos1[2] << ")"
        << std::endl;
      std::cerr << "deltaPos2 "
        << "(" << deltaPos2[0] << ", " << deltaPos2[1] << ", " << deltaPos2[2] << ")"
        << std::endl;
      std::cerr << "uvs "
        << "(" << uvs[p[0]][0] << ", " << uvs[p[0]][1] << "), "
        << "(" << uvs[p[1]][0] << ", " << uvs[p[1]][1] << "), "
        << "(" << uvs[p[2]][0] << ", " << uvs[p[2]][1] << ")"
        << std::endl;
      std::cerr << "deltaUV1 " << deltaUV1[0] << ", " << deltaUV1[1] << std::endl;
      std::cerr << "deltaUV2 " << deltaUV2[0] << ", " << deltaUV2[1] << std::endl;
      */

      float oneOverR = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
      if (oneOverR == 0) {
        static bool nanErrorOutput = false;
        if (!nanErrorOutput) {
          nanErrorOutput = true;
          std::cerr << "Error: NaN tangents computed!" << std::endl;
        }
        continue;
      }
      float r = 1.0f / oneOverR;
      glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
      glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
      //std::cerr << "tangent " << tangent[0] << ", " << tangent[1] << ", " << tangent[2] << std::endl;

      for (unsigned int v = 0; v < 3; v++) {
        tangents[p[v]] += tangent;
        bitangents[p[v]] += bitangent;
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers[TANGENT_BUF]);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[BITANGENT_BUF]);
    glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);
  }


  numIndices = indices.size();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[ELEMENT_BUF]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

  // For mirrors.
  for (unsigned int i = 0; i < 4 && i < vertices.size(); i++) {
    firstFourVertices[i] = vertices[i];
  }
  firstNormal = normals[0];
}

Mesh::~Mesh() {
  glDeleteBuffers(NUM_BUFS, buffers);
}

void Mesh::setUVs(std::vector<glm::vec2>& uvs) {
  // TODO: Update Tangents and Bitangents!
  glBindBuffer(GL_ARRAY_BUFFER, buffers[UV_BUF]);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
}

bool loadTexture(aiTextureType aiType, const aiMaterial* m, Material *material) {
  aiString texFileName;
  aiReturn result = m->GetTexture(aiType, 0, &texFileName);
  std::string prefixedTexFileName;

  if (result == AI_SUCCESS) {
    prefixedTexFileName = "models/" + std::string(texFileName.C_Str());
  } else if (aiType == aiTextureType_HEIGHT && m->GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == AI_SUCCESS) {
    std::string originalName(texFileName.C_Str());
    int lastPeriod = originalName.find_last_of('.');
    if (lastPeriod == (int)std::string::npos) {
      return false;
    }
    prefixedTexFileName = "models/" + originalName.substr(0, lastPeriod) + "_normal" + originalName.substr(lastPeriod);
  } else {
    return false;
  }

  Texture* texture = Texture::loadOrGet(prefixedTexFileName, aiType == aiTextureType_DIFFUSE);

  if (aiType == aiTextureType_DIFFUSE) {
    material->setDiffuseTexture(texture);
  } else if (aiType == aiTextureType_HEIGHT) {
    material->setNormalTexture(texture);
  }

  return true;
}

std::vector<Mesh*> loadScene(std::string fileName, bool invertNormals) {

  std::vector<Mesh*> meshes;
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(fileName.c_str(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
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
    aiColor3D ke(0, 0, 0);
    float shininess = 0.0;
    m->Get(AI_MATKEY_COLOR_AMBIENT, ka);
    m->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
    m->Get(AI_MATKEY_COLOR_SPECULAR, ks);
    m->Get(AI_MATKEY_COLOR_EMISSIVE, ke);
    m->Get(AI_MATKEY_SHININESS, shininess);
    //AI_MATKEY_COLOR_REFLECTIVE

    aiString materialName;
    m->Get(AI_MATKEY_NAME, materialName);

    std::string materialNameString(materialName.C_Str());
    //std::cerr << "Loading material " << materialNameString << std::endl;

    if (materialNameString.substr(0, 6) == "Mirror") {
      std::cerr << "Creating mirror" << std::endl;
      materials[matId] = new Mirror(
        glm::vec3(ka.r, ka.g, ka.b),
        glm::vec3(kd.r, kd.g, kd.b),
        glm::vec3(ks.r, ks.g, ks.b),
        glm::vec3(ke.r, ke.g, ke.b),
        shininess);
    } else {
      materials[matId] = new Material(
        glm::vec3(ka.r, ka.g, ka.b),
        glm::vec3(kd.r, kd.g, kd.b),
        glm::vec3(ks.r, ks.g, ks.b),
        glm::vec3(ke.r, ke.g, ke.b),
        shininess);
    }

    loadTexture(aiTextureType_DIFFUSE, m, materials[matId]);
    loadTexture(aiTextureType_HEIGHT, m, materials[matId]); // Normal Map.
    // NOTE: Must use "bump" in .mtl file, or have name.png and name_normal.png in same directory.
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
        if (invertNormals) {
          normals.push_back(-glm::vec3(n.x, n.y, n.z));
        } else {
          normals.push_back(glm::vec3(n.x, n.y, n.z));
        }
      }
    }

    // Face indices.
    indices.reserve(3*mesh->mNumFaces);
    for (unsigned int i=0; i<mesh->mNumFaces; i++){
      if (mesh->mFaces[i].mNumIndices != 3) {
        std::cerr << "Warning! Face found with " << mesh->mFaces[i].mNumIndices << " indices!" << std::endl;
      }
      // Only supporting triangles here.
      indices.push_back(mesh->mFaces[i].mIndices[0]);
      indices.push_back(mesh->mFaces[i].mIndices[1]);
      indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    meshes.push_back(new Mesh(vertices, uvs, normals, indices, material));
  }

  // Name meshes by going down hierarchy.
  std::list<aiNode*> nodeQueue;
  nodeQueue.push_front(scene->mRootNode);
  while (!nodeQueue.empty()) {
    aiNode* node = nodeQueue.front();
    nodeQueue.pop_front();
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      nodeQueue.push_back(node->mChildren[i]);
    }
    for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++) {
      unsigned int meshId = node->mMeshes[meshIndex];
      meshes[meshId]->setName(std::string(node->mName.C_Str()));
    }
  }


  // Prune "hidden" meshes.
  for (std::vector<Mesh*>::iterator it = meshes.begin(); it != meshes.end(); it++) {
    Mesh* mesh = *it;
    if (mesh->getName().substr(0, 6) == "Hidden") {
      it = meshes.erase(it);
      delete mesh;
    }
  }

  std::cout << "Loaded " << meshes.size() << " meshes." << std::endl;
  for (unsigned int i = 0; i < meshes.size(); i++) {
    std::cout << meshes[i]->getName() << ": " << meshes[i]->getNumIndices() << " indices" << std::endl;
  }

  //std::cout << scene->mNumAnimations << " animations" << std::endl;

  // TODO: Don't leak materials.
  return meshes;
}



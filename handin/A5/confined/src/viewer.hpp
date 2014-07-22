#ifndef VIEWER_H
#define VIEWER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "controller.hpp"
#include "mesh.hpp"
#include "light.hpp"

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 768
#define SSAO_NOISE_TEXTURE_WIDTH 4
#define NOISE_SIZE (SSAO_NOISE_TEXTURE_WIDTH*SSAO_NOISE_TEXTURE_WIDTH)

class Controller;
class Mirror;

class Viewer {
public:
  Viewer();
  ~Viewer();

  bool initGL();
  void run();

  /**
   * Render scene with deferred pipeline.
   * Set renderTarget=0 to render to screen.
   */
  void renderScene(GLuint renderTargetFBO, std::vector<Mesh*>& thisFrameMeshes, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraPosition, bool postProcess, double currentTime, double deltaTime, const glm::vec3& halfspacePosition, const glm::vec3& halfspaceNormal);

  void bindRenderTarget(GLuint renderTargetFBO);

  GLFWwindow* getWindow() {
    return window;
  }
  int getWidth() {
    return width;
  }
  int getHeight() {
    return height;
  }
  Controller* getController() {
    return controller;
  }

  void updateSize(int width, int height);
  void drawTextureWithQuadProgram(GLuint tex);
  void drawQuad();

private:
  int width, height;
  GLFWwindow* window;
  Settings* settings;
  Controller* controller;
  std::vector<Mesh*> meshes;
  Mesh* pointLightMesh;
  std::vector<std::vector<Mesh*> > characterMeshes; // TODO: Make MeshAnimation type or something.
  std::vector<Light*> lights;

  glm::mat4 shadowmapBiasMatrix;
  glm::vec3 ssaoKernel[4];
  glm::vec3 ssaoNoise[NOISE_SIZE];

  GLuint programId;
  GLuint depthProgramId;
  GLuint quadProgramId;
  GLuint postProcessProgramId;

  // Deferred Shading objects.
  GLuint geomTexturesProgramId;
  GLuint deferredShadingProgramId;
  GLuint deferredDiffuseTexture;
  GLuint deferredSpecularTexture;
  GLuint deferredEmissiveTexture;
  GLuint deferredNormalTexture;
  GLuint deferredDepthTexture;

  GLuint vertexArrayId;
  GLuint deferredShadingFramebuffer;
  GLuint shadowMapFramebuffer;
  GLuint shadowCubeMapFramebuffer;
  GLuint accumRenderFramebuffer;
  GLuint quadVertexBuffer;
  GLuint depthRenderBuffers[2]; // TODO: Remove second one.

  GLuint shadowmapDepthTexture;
  GLuint shadowmapCubeDepthTexture;
  GLuint ssaoNoiseTexture;
  GLuint accumRenderTexture;
};

bool checkGLFramebuffer();
bool checkGLErrors(std::string msg);

#endif

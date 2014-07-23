#ifndef VIEWER_H
#define VIEWER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "controller.hpp"
#include "mesh.hpp"
#include "light.hpp"
#include "sound.hpp"

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

  bool initialize();
  void run();

  /**
   * Render scene with deferred pipeline.
   * Set renderTarget=0 to render to screen.
   */
  void renderScene(GLuint renderTargetFBO, std::vector<Mesh*>& thisFrameMeshes, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraPosition, bool postProcess, double currentTime, double deltaTime, const glm::vec3& halfspacePosition, const glm::vec3& halfspaceNormal, bool doPicking);

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
  Sound* thunderSound;
  Sound* backgroundMusic;
  Sound* getItemSound;
  double lastThunderPlay;

  std::vector<Mesh*> meshes;
  Mesh* pointLightMesh;
  std::vector<std::vector<Mesh*> > characterMeshes; // TODO: Make MeshAnimation type or something.
  std::vector<Mesh*> flashlightMeshes;
  std::vector<Mesh*> gunMeshes;

  std::vector<Light*> lights;
  Light* lightningLight;
  Light* gunLight;

  uint16_t lastPickedMesh;

  glm::mat4 shadowmapBiasMatrix;
  glm::vec3 ssaoKernel[4];
  glm::vec3 ssaoNoise[NOISE_SIZE];

  GLuint programId;
  GLuint depthProgramId;
  GLuint quadProgramId;
  GLuint postProcessProgramId;
  GLuint geomTexturesProgramId;
  GLuint deferredShadingProgramId;

  // Deferred Shading textures.
  GLuint deferredDiffuseTexture;
  GLuint deferredSpecularTexture;
  GLuint deferredEmissiveTexture;
  GLuint deferredNormalTexture;
  GLuint deferredDepthTexture;

  // Other textures.
  GLuint shadowmapDepthTexture;
  GLuint shadowmapCubeDepthTexture;
  GLuint ssaoNoiseTexture;
  GLuint accumRenderTexture;
  GLuint pickingTexture;

  GLuint vertexArrayId;
  GLuint deferredShadingFramebuffer;
  GLuint shadowMapFramebuffer;
  GLuint shadowCubeMapFramebuffer;
  GLuint accumRenderFramebuffer;
  GLuint quadVertexBuffer;
  GLuint depthRenderBuffers[2]; // TODO: Remove second one.

};

bool checkGLFramebuffer();
bool checkGLErrors(std::string msg);

#endif

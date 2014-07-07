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

class Controller;

class Viewer {
public:
  Viewer();
  ~Viewer();

  bool initGL();
  void run();

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
  Controller* controller;
  std::vector<Mesh*> meshes;
  std::vector<Light*> lights;

  GLuint programId;
  GLuint depthProgramId;
  GLuint quadProgramId;

  // Deferred Shading objects.
  GLuint geomTexturesProgramId;
  GLuint deferredShadingProgramId;
  GLuint deferredDiffuseTexture;
  GLuint deferredNormalTexture;
  GLuint deferredDepthTexture;

  GLuint vertexArrayId;
  GLuint deferredShadingFramebuffer;
  GLuint shadowMapFramebuffer;
  GLuint quadVertexBuffer;

  //GLuint texture;
  GLuint shadowmapDepthTexture;
};

#endif

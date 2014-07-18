#include <cstdlib>
#include <vector>
#include <iostream>
#include <ctime>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "mesh.hpp"

#include "viewer.hpp"
#include "controller.hpp"

#define MIN_REQUIRED_COLOUR_ATTACHMENTS 2
#define RENDER_DEBUG_IMAGES true
#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048
#define MAX_FPS 60
#define FPS_SAMPLE_RATE 20
#define SSAO_NOISE_TEXTURE_WIDTH 4

void window_size_callback(GLFWwindow* window, int width, int height) {
  Viewer* viewer = (Viewer*)glfwGetWindowUserPointer(window);
  viewer->updateSize(width, height);
  viewer->getController()->reset();
}

void window_focus_callback(GLFWwindow* window, int focussed) {
  if (focussed == GL_TRUE) {
    Viewer* viewer = (Viewer*)glfwGetWindowUserPointer(window);
    viewer->getController()->reset();
  }
}

bool checkGLErrors(std::string msg) {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cerr << "OpenGL error " << msg << ": " << error << " - " << glewGetErrorString(error) << std::endl;
    return false;
  }
  return true;
}

bool checkGLFramebuffer() {
  GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer check failed! " << frameBufferStatus << " = " << glewGetErrorString(frameBufferStatus) << std::endl;
    return false;
  }
  return true;
}

Viewer::Viewer(): width(DEFAULT_WIDTH), height(DEFAULT_HEIGHT) {

  controller = new Controller(this);

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(width, height, "Confined", NULL, NULL);

  if( window == NULL ){
    std::cerr << "Failed to open GLFW window. This application requires OpenGL 3.3 support." << std::endl;
    glfwTerminate();
    return;
  }

  // Setup callbacks for window.
  glfwSetWindowUserPointer(window, (void*)this);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetWindowFocusCallback(window, window_focus_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // Ensure we can capture the escape key.
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}

void Viewer::updateSize(int width, int height) {
  this->width = width;
  this->height = height;

  glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredSpecularTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredEmissiveTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
}

bool Viewer::initGL() {
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return false;
  }

  // Ignore invalid enum error from glew call.
  glGetError();

  GLint maxAttachments;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
  if (maxAttachments < MIN_REQUIRED_COLOUR_ATTACHMENTS) {
    std::cerr << "Only " << maxAttachments << " supported FBO Colour Attachments, but this program requires " << MIN_REQUIRED_COLOUR_ATTACHMENTS << std::endl;
  }

  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one.
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  glGenVertexArrays(1, &vertexArrayId);

  //meshes = loadScene("models/monkeybox.obj");
  meshes = loadScene("models/shadowhouse.obj");
  std::vector<Mesh*> pointLightMeshes = loadScene("models/sphere.obj", false);
  if (pointLightMeshes.size() == 1) {
    pointLightMesh = pointLightMeshes[0];
    pointLightMesh->getModelMatrix() = glm::scale(glm::mat4(1.0), glm::vec3(0.1, 0.1, 0.1));
  } else {
    std::cerr << "Loading sphere mesh resulted in not 1 meshes!!" << std::endl;
    exit(1);
  }
  glm::vec3 startPosition(0, 2, -10);
  controller->setHorizontalAngle(0);

  controller->setPosition(startPosition);

  // Framebuffer for deferred shading.
  deferredShadingFramebuffer = 0;
  glGenFramebuffers(1, &deferredShadingFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);

  // Deferred rendering texture targets.
  glGenTextures(1, &deferredDiffuseTexture);
  glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glGenTextures(1, &deferredSpecularTexture);
  glBindTexture(GL_TEXTURE_2D, deferredSpecularTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, 0);

  glGenTextures(1, &deferredEmissiveTexture);
  glBindTexture(GL_TEXTURE_2D, deferredEmissiveTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glGenTextures(1, &deferredNormalTexture);
  glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glGenTextures(1, &deferredDepthTexture);
  glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  // This is necessary.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferredDepthTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredDiffuseTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredSpecularTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferredEmissiveTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, deferredNormalTexture, 0);

  if (!checkGLFramebuffer()) return false;

  // Framebuffer for accumulating rendering.
  accumRenderFramebuffer = 0;
  glGenFramebuffers(1, &accumRenderFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, accumRenderFramebuffer);

  // Main texture for accumulating deferred shading light passes.
  glGenTextures(1, &accumRenderTexture);
  glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumRenderTexture, 0);

  // Depth render buffer.
  glGenRenderbuffers(1, &depthRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

  if (!checkGLFramebuffer()) return false;


  // SSAO Noise texture.
  glGenTextures(1, &ssaoNoiseTexture);
  glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SSAO_NOISE_TEXTURE_WIDTH, SSAO_NOISE_TEXTURE_WIDTH, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  // Shadow mapping setup.
  shadowMapFramebuffer = 0;
  glGenFramebuffers(1, &shadowMapFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);

  glGenTextures(1, &shadowmapDepthTexture);
  glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmapDepthTexture, 0);

  if (!checkGLFramebuffer()) return false;


  // Shadow cube map for point lights.
  shadowCubeMapFramebuffer = 0;
  glGenFramebuffers(1, &shadowCubeMapFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeMapFramebuffer);

  glGenTextures(1, &shadowmapCubeDepthTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmapCubeDepthTexture);
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0); // TODO: GL_DEPTH_COMPONENT16/32?
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  for (int i = 0; i < 6; i++) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, shadowmapCubeDepthTexture, 0);
  }

  if (!checkGLFramebuffer()) return false;


  // Quad for drawing textures.
  static const GLfloat quadVBuffer[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
  };

  glGenBuffers(1, &quadVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVBuffer), quadVBuffer, GL_STATIC_DRAW);

  // Compile GLSL programs.
  quadProgramId = loadShaders( "shaders/passthrough.vert", "shaders/justTexture.frag" );

  depthProgramId = loadShaders("shaders/depthShadow.vert", "shaders/depthShadow.frag" );

  geomTexturesProgramId = loadShaders("shaders/geomTextures.vert", "shaders/geomTextures.frag");

  deferredShadingProgramId = loadShaders("shaders/deferredShading.vert", "shaders/deferredShading.frag");

  postProcessProgramId = loadShaders("shaders/passthrough.vert", "shaders/postProcess.frag");

  if (quadProgramId == 0 || depthProgramId == 0 || geomTexturesProgramId == 0 || deferredShadingProgramId == 0 || postProcessProgramId == 0) {
    return false;
  }


  // Scene-specific setup:

  // Flash light.
  lights.push_back(Light::spotLight(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -1.0), 15.0));
  lights.back()->getFalloff() = glm::vec3(1.0, 0.02, 0.001);
  lights.back()->getAmbience() = glm::vec3(0.04, 0.04, 0.04);
  //lights.back()->setEnabled(false);

  // Monkeybox.
  /*
  lights.push_back(Light::pointLight(glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.0, 3.0, 0.0)));
  lights.back()->getAmbience() = glm::vec3(0.4, 0.4, 0.4);
  */

  // "Sun"
  lights.push_back(Light::directionalLight(glm::vec3(0.7, 0.7, 0.7), glm::vec3(0.1, -1.0, 0.0)));
  //lights.back()->getAmbience() = glm::vec3(0.1, 0.1, 0.1);
  lights.back()->setEnabled(false);

  // Lamp
  lights.push_back(Light::pointLight(glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.0, 3.0, 0.0)));
  lights.back()->setEnabled(false);

  return true;
}


void Viewer::drawQuad() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
  glVertexAttribPointer(
    0,                  // attribute 0.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  glDrawArrays(GL_TRIANGLES, 0, 2*3);
  glDisableVertexAttribArray(0);
}


void Viewer::run() {
  // Handle for MVP uniform (shadow depth pass).
  GLuint depthMatrixId = glGetUniformLocation(depthProgramId, "depthMVP");

  GLuint texId = glGetUniformLocation(quadProgramId, "texture");

  // Handles for MVP matrix uniforms (render pass).
  GLuint geomMVPId = glGetUniformLocation(geomTexturesProgramId, "MVP");
  GLuint geomViewMatrixId = glGetUniformLocation(geomTexturesProgramId, "V");
  GLuint geomModelMatrixId = glGetUniformLocation(geomTexturesProgramId, "M");
  GLuint geomProjectionMatrixId = glGetUniformLocation(geomTexturesProgramId, "P");

  GLuint deferredViewMatrixId = glGetUniformLocation(deferredShadingProgramId, "V");
  GLuint deferredProjectionMatrixId = glGetUniformLocation(deferredShadingProgramId, "P");
  GLuint lightPosId = glGetUniformLocation(deferredShadingProgramId, "lightPositionWorldspace");
  GLuint lightDirId = glGetUniformLocation(deferredShadingProgramId, "lightDirectionWorldspace");
  GLuint lightTypeId = glGetUniformLocation(deferredShadingProgramId, "lightType");
  GLuint lightColourId = glGetUniformLocation(deferredShadingProgramId, "lightColour");
  GLuint lightAmbienceId = glGetUniformLocation(deferredShadingProgramId, "lightAmbience");
  GLuint lightFalloffId = glGetUniformLocation(deferredShadingProgramId, "lightFalloff");
  GLuint lightSpreadDegreesId = glGetUniformLocation(deferredShadingProgramId, "lightSpreadDegrees");
  GLuint cameraPositionId = glGetUniformLocation(deferredShadingProgramId, "cameraPositionWorldspace");

  GLuint depthBiasId = glGetUniformLocation(deferredShadingProgramId, "depthBiasVP");

  // Deferred shading textures.
  GLuint deferredDiffuseTextureId = glGetUniformLocation(deferredShadingProgramId, "diffuseTexture");
  GLuint deferredSpecularTextureId = glGetUniformLocation(deferredShadingProgramId, "specularTexture");
  GLuint deferredEmissiveTextureId = glGetUniformLocation(deferredShadingProgramId, "emissiveTexture");
  GLuint deferredNormalTextureId = glGetUniformLocation(deferredShadingProgramId, "normalTexture");
  GLuint deferredDepthTextureId = glGetUniformLocation(deferredShadingProgramId, "depthTexture");
  GLuint shadowmapId = glGetUniformLocation(deferredShadingProgramId, "shadowMap");
  GLuint shadowmapCubeId = glGetUniformLocation(deferredShadingProgramId, "shadowMapCube");

  // SSAO.
  GLuint useSSAOId = glGetUniformLocation(deferredShadingProgramId, "useSSAO");
  GLuint ssaoKernelId = glGetUniformLocation(deferredShadingProgramId, "ssaoKernel");
  GLuint ssaoNoiseId = glGetUniformLocation(deferredShadingProgramId, "ssaoNoiseTexture");

  // Handles for material properties (render pass).
  //GLuint material_ka = glGetUniformLocation(geomTexturesProgramId, "material_ka");
  GLuint geomMaterialKdId = glGetUniformLocation(geomTexturesProgramId, "material_kd");
  GLuint geomMaterialKsId = glGetUniformLocation(geomTexturesProgramId, "material_ks");
  GLuint geomMaterialShininessId = glGetUniformLocation(geomTexturesProgramId, "material_shininess");
  GLuint geomMaterialEmissiveId = glGetUniformLocation(geomTexturesProgramId, "material_emissive");

  GLuint geomDiffuseTexId = glGetUniformLocation(geomTexturesProgramId, "diffuseTexture");
  GLuint geomUseDiffuseTextureId = glGetUniformLocation(geomTexturesProgramId, "useDiffuseTexture");
  GLuint geomNormalTexId = glGetUniformLocation(geomTexturesProgramId, "normalTexture");
  GLuint geomUseNormalTextureId = glGetUniformLocation(geomTexturesProgramId, "useNormalTexture");

  GLuint postProcessTexId = glGetUniformLocation(postProcessProgramId, "texture");

  // Set up SSAO data.
  srand(1);
  glm::vec3 ssaoKernel[4];
  for (int i = 0; i < 4; i++) {
    ssaoKernel[i] = glm::vec3(
      (float)(rand() / (float)(RAND_MAX) * 2.0f - 1.0f),
      (float)(rand() / (float)(RAND_MAX) * 2.0f - 1.0f),
      (float)(rand() / (float)(RAND_MAX)));
    ssaoKernel[i] = glm::normalize(ssaoKernel[i]);

    float scale = (float) i / 4.0f;
    ssaoKernel[i] *= 0.9f * scale * scale;
  }
  int noiseSize = SSAO_NOISE_TEXTURE_WIDTH * SSAO_NOISE_TEXTURE_WIDTH;
  glm::vec3 ssaoNoise[noiseSize];
  for (int i = 0; i < noiseSize; i++) {
    ssaoNoise[i] = glm::vec3(
      (float)(rand() / (float)(RAND_MAX)),
      (float)(rand() / (float)(RAND_MAX)),
      0.0f);
    ssaoNoise[i] = glm::normalize(ssaoNoise[i]);
  }
  glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
  // Required or data won't show up!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SSAO_NOISE_TEXTURE_WIDTH, SSAO_NOISE_TEXTURE_WIDTH, 0, GL_RGB, GL_FLOAT, ssaoNoise);

  double lightTime = 0.0;
  controller->reset();

  glBindVertexArray(vertexArrayId);

  double lastTime = glfwGetTime();
  long fpsDisplayCounter = 0;
  double lastFPSTime = lastTime;

  do {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // TODO: Make config class and hook up controller to flip options like this.
    bool useSSAO = false; //(int) std::floor(currentTime) % 2 == 0;
    bool doPostProcessing = useSSAO;


    // ======= Deferred rendering stage 1: Render geometry into textures. ===========

    glUseProgram(geomTexturesProgramId);

    // Render to framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Bind textures to fbo as multiple render target.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deferredDepthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredDiffuseTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredSpecularTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferredEmissiveTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, deferredNormalTexture, 0);

    // Set to render both colour attachments.
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    GLenum drawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute the MVP matrix from keyboard and mouse input
    controller->update();
    const glm::mat4& projectionMatrix = controller->getProjectionMatrix();
    const glm::mat4& viewMatrix = controller->getViewMatrix();
    const glm::vec3& cameraPosition = controller->getPosition();

    // Send MVP transformations to currently bound shader.
    glUniformMatrix4fv(geomViewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(geomProjectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);

    for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
      Mesh* mesh = *it;

      glm::mat4 modelMatrix = mesh->getModelMatrix();
      glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

      glUniformMatrix4fv(geomMVPId, 1, GL_FALSE, &MVP[0][0]);
      glUniformMatrix4fv(geomModelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);

      Material* material = mesh->getMaterial();
      if (material != NULL) {
        //glUniform3f(material_ka, material->ka.x, material->ka.y, material->ka.z);
        glUniform3f(geomMaterialKdId, material->getDiffuse().x, material->getDiffuse().y, material->getDiffuse().z);
        glUniform3f(geomMaterialKsId, material->getSpecular().x, material->getSpecular().y, material->getSpecular().z);
        glUniform1f(geomMaterialShininessId, material->getShininess());
        glUniform3f(geomMaterialEmissiveId, 0, 0, 0);

        // Bind diffuse texture if it exists.
        glUniform1i(geomUseDiffuseTextureId, material->hasDiffuseTexture());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->getDiffuseTexture());
        glUniform1i(geomDiffuseTexId, 0);

        // Bind normal texture if it exists.
        glUniform1i(geomUseNormalTextureId, material->hasNormalTexture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material->getNormalTexture());
        glUniform1i(geomNormalTexId, 1);
      }
      mesh->renderGL();
    }

    // Render point lights as spheres.
    glUniform1i(geomUseDiffuseTextureId, false);
    glUniform1i(geomUseNormalTextureId, false);
    for (std::vector<Light*>::const_iterator lightIt = lights.begin(); lightIt != lights.end(); lightIt++) {
      Light *light = *lightIt;
      if (!light->isEnabled() || light->getType() != Light::POINT) continue;

      // Set model matrix to move model to point light's position.
      glm::mat4 sphereModelMatrix = glm::translate(glm::mat4(1.0), light->getPosition()) * pointLightMesh->getModelMatrix();
      glm::mat4 MVP = projectionMatrix * viewMatrix * sphereModelMatrix;
      glUniformMatrix4fv(geomModelMatrixId, 1, GL_FALSE, &sphereModelMatrix[0][0]);
      glUniformMatrix4fv(geomMVPId, 1, GL_FALSE, &MVP[0][0]);

      // Use light's diffuse as emissive material.
      glUniform3f(geomMaterialKdId, 0, 0, 0);
      glUniform3f(geomMaterialKsId, 0, 0, 0);
      glm::vec3 emissiveLight = light->getColour();
      emissiveLight *= 5.0;
      glUniform3f(geomMaterialEmissiveId, emissiveLight.x, emissiveLight.y, emissiveLight.z);

      pointLightMesh->renderGL();
    }


    // ======= Shadow map and blend deferred shading for each light ======================

    // Clear target first.
    glViewport(0, 0, width, height);

    if (doPostProcessing) {
      // Render to texture.
      glBindFramebuffer(GL_FRAMEBUFFER, accumRenderFramebuffer);
      glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumRenderTexture, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
    } else {
      // Render to screen.
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDrawBuffer(GL_FRONT_LEFT);
    }

    if (useSSAO) {
      glClearColor(0.0f, 0.4f, 0.0f, 0.0f);
    } else {
      glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    }
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool firstBlendPass = true;
    lightTime += 3.0 * deltaTime;

    // Moving lights.

    // Flashlight.
    lights[0]->getPosition() = cameraPosition + glm::vec3(0, -0.6f, 0);
    // TODO: Why backwards about x?
    lights[0]->getDirection() = glm::vec3(glm::inverse(viewMatrix) * glm::vec4(0, 0, -1, 0));
    //lights[0]->setEnabled(false);

    // Monkey box.
    //lights[1]->getPosition() = glm::vec3(std::cos(lightTime)/2.0, 3.0, std::sin(lightTime)/2.0); // Point.


    for (std::vector<Light*>::const_iterator it = lights.begin(); it != lights.end(); it++) {
      Light* light = *it;
      if (!light->isEnabled()) continue;

      // ======= Shadow mapping =========
      glUseProgram(depthProgramId);
      if (light->getType() == Light::POINT) {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeMapFramebuffer);
      } else {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);
      }

      glDrawBuffer(GL_NONE); // No colour output.
      glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
      glEnable(GL_DEPTH_TEST);

      glm::vec3 lightPos = light->getPosition();
      glm::vec3 lightDir = light->getDirection();

      glm::mat4 depthVP;

      // Loop for all shadow maps that have to be generated for this light.
      for (int shadowMapFace = 0; shadowMapFace < 6; shadowMapFace++) {

        // Bind shadow map texture.
        if (light->getType() == Light::POINT) {
          // Shadow cube map.
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + shadowMapFace, shadowmapCubeDepthTexture, 0);
        } else {
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmapDepthTexture, 0);
        }

        glClear(GL_DEPTH_BUFFER_BIT);

        // Compute the MVP matrix from the light's point of view.
        glm::mat4 depthProjectionMatrix;
        glm::mat4 depthViewMatrix;
        glm::vec3 lightDirectionWorldspace;
        switch (light->getType()) {
          case Light::DIRECTIONAL:
            // TODO: Need to adjust for scene size...
            //depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
            depthProjectionMatrix = glm::ortho<float>(-40, 40, -40, 40, -10, 100);
            depthViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), lightDir, glm::vec3(0, 1, 0));
            break;
          case Light::SPOT:
            depthProjectionMatrix = glm::perspective(light->getSpread() + 15.0f, 1.0f, 2.0f, 100.0f);
            depthViewMatrix = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0, 1, 0));
            break;
          case Light::POINT:
            depthProjectionMatrix = glm::perspective(90.0f, 1.0f, 1.0f, 500.0f);
            switch (shadowMapFace) {
              case 0: // +X
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0),glm::vec3(0, -1, 0));
                break;
              case 1: // -X
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0),glm::vec3(0, -1, 0));
                break;
              case 2:// +Y
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0),glm::vec3(0, 0, 1));
                break;
              case 3: // -Y
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0),glm::vec3(0, 0, -1));
                break;
              case 4:// +Z
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
                break;
              case 5: // -Z
                depthViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
                break;
            }
            break;
        }
        glm::mat4 depthModelMatrix = glm::mat4(1.0);
        depthVP = depthProjectionMatrix * depthViewMatrix;
        glm::mat4 depthMVP = depthVP * depthModelMatrix;

        glUniformMatrix4fv(depthMatrixId, 1, GL_FALSE, &depthMVP[0][0]);

        for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
          (*it)->renderGLVertsOnly();
        }

        if (light->getType() != Light::POINT) break;
      }

      // TODO: Debug angles/perspectives of point light - seems a little off for some directions.
      // TODO: Fix shadow stripes!
      if (light->getType() == Light::POINT) {
        // TODO: Figure out why we need to shift by -1 here.
        depthVP = glm::translate(glm::mat4(1.0), glm::vec3(-lightPos.x - 1, -lightPos.y - 1, -lightPos.z - 1));
      }

      // ======= Deferred rendering stage 2: Deferred rendering using textures. ===========

      glUseProgram(deferredShadingProgramId);

      glViewport(0, 0, width, height);
      if (doPostProcessing) {
        // Set output to accumulation texture.
        glBindFramebuffer(GL_FRAMEBUFFER, accumRenderFramebuffer);
        glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumRenderTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
      } else {
        // Render to screen.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_FRONT_LEFT);
      }

      glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

      //glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
      //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Blend in order to accumulate lights.
      glEnablei(GL_BLEND, 0);
      glDisable(GL_DEPTH_TEST);
      if (firstBlendPass) {
        firstBlendPass = false;
        glBlendFunc(GL_ONE, GL_ZERO);
      } else {
        glBlendFunc(GL_ONE, GL_ONE);
      }

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
      glUniform1i(deferredDiffuseTextureId, 0);

      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, deferredSpecularTexture);
      glUniform1i(deferredSpecularTextureId, 1);

      glActiveTexture(GL_TEXTURE0 + 2);
      glBindTexture(GL_TEXTURE_2D, deferredEmissiveTexture);
      glUniform1i(deferredEmissiveTextureId, 2);

      glActiveTexture(GL_TEXTURE0 + 3);
      glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
      glUniform1i(deferredNormalTextureId, 3);

      glActiveTexture(GL_TEXTURE0 + 4);
      glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
      glUniform1i(deferredDepthTextureId, 4);

      glActiveTexture(GL_TEXTURE0 + 5);
      glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
      glUniform1i(shadowmapId, 5);

      /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      // Don't know if this is necessary - doesn't seem to do anything.
      //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
      */

      glActiveTexture(GL_TEXTURE0 + 6);
      glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmapCubeDepthTexture);
      glUniform1i(shadowmapCubeId, 6);

      // TODO: Do we need these?
      /*
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      */

      glActiveTexture(GL_TEXTURE0 + 7);
      glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
      glUniform1i(ssaoNoiseId, 7);

      /*
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
      */

      glm::mat4 biasMatrix(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
      );
      glm::mat4 depthBiasVP = biasMatrix * depthVP;

      glUniformMatrix4fv(deferredViewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
      glUniformMatrix4fv(deferredProjectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
      glUniformMatrix4fv(depthBiasId, 1, GL_FALSE, &depthBiasVP[0][0]);
      glUniform3f(cameraPositionId, cameraPosition.x, cameraPosition.y, cameraPosition.z);
      glUniform3f(lightDirId, lightDir.x, lightDir.y, lightDir.z);
      glUniform3f(lightPosId, lightPos.x, lightPos.y, lightPos.z);

      glUniform1i(useSSAOId, useSSAO);
      glUniform3fv(ssaoKernelId, sizeof(ssaoKernel), (float*)ssaoKernel);

      glm::vec3 lightColour = light->getColour();
      glm::vec3 lightAmbience = light->getAmbience();
      glm::vec3 lightFalloff = light->getFalloff();
      glUniform1i(lightTypeId, (int) light->getType());
      glUniform3f(lightColourId, lightColour.x, lightColour.y, lightColour.z);
      glUniform3f(lightAmbienceId, lightAmbience.x, lightAmbience.y, lightAmbience.z);
      glUniform3f(lightFalloffId, lightFalloff.x, lightFalloff.y, lightFalloff.z);
      glUniform1f(lightSpreadDegreesId, light->getSpread());

      drawQuad();

      glDisablei(GL_BLEND, 0);

    }

    // --------- Final pass - post-processing and rendering to screen ---------------

    if (doPostProcessing) {
      glUseProgram(postProcessProgramId);

      // Draw to screen.
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      glDrawBuffer(GL_FRONT_LEFT);
      glDisable(GL_DEPTH_TEST);

      glActiveTexture(GL_TEXTURE0);
      glUniform1i(postProcessTexId, 0);
      glViewport(0, 0, width, height);
      glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
      drawQuad();
    }



    // ============ Debug Rendering =============
    if (RENDER_DEBUG_IMAGES) {

      glUseProgram(quadProgramId);

      // Render to the screen
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDrawBuffer(GL_FRONT_LEFT);

      // Must be disabled to draw overtop.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      glDisable(GL_DEPTH_TEST);
      glActiveTexture(GL_TEXTURE0);
      glUniform1i(texId, 0);

      // Draw diffuse ----------------
      glViewport(0, 0, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
      drawQuad();

      // Draw normals ----------------
      glViewport(height/4, 0, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
      drawQuad();

      // Draw depth ----------------
      glViewport(height/2, 0, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
      drawQuad();

      // Draw noise --------------------
      glViewport(height * 3 / 4, 0, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
      drawQuad();

      // Draw shadowmap ----------------
      glViewport(0, height*3/4, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
      drawQuad();

      // Draw cube shadowmap ----------
      //glViewport(height/4, height*3/4, height/4, height/4);
      //glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmapCubeDepthTexture);
      //drawQuad();
    }
    // =========== End Debug =================

    // Swap buffers
    glfwSwapBuffers(window);

    fpsDisplayCounter++;
    if (fpsDisplayCounter % FPS_SAMPLE_RATE == 0) {
      double fpsDeltaTime = float(currentTime - lastFPSTime);
      lastFPSTime = currentTime;
      std::cout << FPS_SAMPLE_RATE / fpsDeltaTime << "FPS" << std::endl;
    }
    //timespec ts;
    //ts.tv_sec = 0;
    //ts.tv_nsec = 30*1000;
    //nanosleep(&ts, NULL);

    checkGLErrors("loop");

    glfwPollEvents();
  } // Check if the ESC key was pressed or the window was closed
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
      glfwWindowShouldClose(window) == 0 );
}

Viewer::~Viewer() {
  delete controller;
  controller = NULL;

  glDeleteProgram(depthProgramId);
  glDeleteProgram(quadProgramId);
  glDeleteProgram(geomTexturesProgramId);
  glDeleteProgram(deferredShadingProgramId);

  glDeleteFramebuffers(1, &deferredShadingFramebuffer);
  glDeleteFramebuffers(1, &shadowMapFramebuffer);
  glDeleteFramebuffers(1, &shadowCubeMapFramebuffer);

  glDeleteTextures(1, &shadowmapDepthTexture);
  glDeleteTextures(1, &deferredDiffuseTexture);
  glDeleteTextures(1, &deferredSpecularTexture);
  glDeleteTextures(1, &deferredEmissiveTexture);
  glDeleteTextures(1, &deferredNormalTexture);
  glDeleteTextures(1, &deferredDepthTexture);
  glDeleteTextures(1, &ssaoNoiseTexture);
  glDeleteTextures(1, &accumRenderTexture);
  //glDeleteTextures(1, &texture);

  glDeleteBuffers(1, &quadVertexBuffer);
  glDeleteVertexArrays(1, &vertexArrayId);
  glDeleteRenderbuffers(1, &depthRenderBuffer);

  for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
    delete *it;
  }
  meshes.clear();

  for (std::vector<Light*>::const_iterator it = lights.begin(); it != lights.end(); it++) {
    delete *it;
  }
  lights.clear();

  // Cleans up and closes window.
  glfwTerminate();
}


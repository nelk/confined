#include <cstdlib>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "mirror.hpp"
#include "sound.hpp"

#include "viewer.hpp"
#include "controller.hpp"

#define MIN_REQUIRED_COLOUR_ATTACHMENTS 6
#define RENDER_DEBUG_IMAGES false
#define RENDER_LIGHTS_AS_SPHERES false
#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048
#define TARGET_FPS 60
#define TARGET_FRAME_DELTA 0.01666667
#define FPS_SAMPLE_RATE 20

void window_size_callback(GLFWwindow* window, int width, int height) {
  Viewer* viewer = (Viewer*)glfwGetWindowUserPointer(window);
  viewer->updateSize(width, height);
  viewer->getController()->reset();
}

void window_focus_callback(GLFWwindow* window, int focused) {
  if (focused == GL_TRUE) {
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

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(width, height, "Confined", NULL, NULL);

  if (window == NULL) {
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

  glBindTexture(GL_TEXTURE_2D, pickingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_INT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  for (int i = 0; i < 2; i++) {
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffers[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  }

  // Update Mirror textures, which are of screen size.
  for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
    Material *material = (*it)->getMaterial();
    if (!material->isMirror()) continue;
    Mirror* mirror = static_cast<Mirror*>(material);
    mirror->updateSize(width, height);
  }
}

bool Viewer::initializeSound() {
  // Initialize OpenAL.
  if (!Sound::initialize()) {
    std::cerr << "Couldn't initialize OpenAL" << std::endl;
    return false;
  }

  thunderSound = Sound::load("sound/thunder_mono.wav");
  if (thunderSound == NULL) {
    std::cerr << "Couldn't load sound/thunder_mono.wav" << std::endl;
    return false;
  }
  thunderSound->setPosition(glm::vec3(0, 1, 0));
  thunderSound->setVelocity(glm::vec3(0, 0, 0));
  thunderSound->setDirection(glm::vec3(0, 0, -1));
  thunderSound->setGain(1.0); // TODO, increase volume.

  backgroundMusic = Sound::load("sound/creep.wav");
  if (backgroundMusic == NULL) {
    std::cerr << "Couldn't load sound/creep.wav" << std::endl;
    return false;
  }
  backgroundMusic->setGain(0.6);

  getItemSound = Sound::load("sound/getitem.wav");
  if (getItemSound == NULL) {
    std::cerr << "Couldn't load sound/getitem.wav" << std::endl;
    return false;
  }
  return true;
}

bool Viewer::initializeShaders() {
  // Compile GLSL programs.
  if (!geomTexturesProgram.initialize()) return false;
  if (!quadProgram.initialize()) return false;
  if (!deferredShadingProgram.initialize()) return false;
  if (!deferredShadingProgram.initialize()) return false;
  if (!depthProgram.initialize()) return false;
  if (!postProcessProgram.initialize()) return false;
  return true;
}

bool Viewer::initialize() {
  if (!initializeSound()) {
    return false;
  }

  settings = new Settings();
  controller = new Controller(this, settings);
  startCharAnimTime = 0;

  // Initial settings (all start on).
  settings->set(Settings::SSAO, false);
  settings->set(Settings::BLUR, false);
  settings->set(Settings::HIGHLIGHT_PICK, false);

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

  if (!initializeShaders()) {
    return false;
  }

  // Initialize textures.
  Texture::initialize();

  glGenVertexArrays(1, &vertexArrayId);
  glBindVertexArray(vertexArrayId);

  meshes = loadScene("models/shadowhouse_large.obj", false);
  std::vector<Mesh*> pointLightMeshes = loadScene("models/sphere.obj", false);
  for (int i = 0; i < 20; i++) {
    std::stringstream fname;
    fname << "models/minecraft_rigs/steve_animate_";
    fname << std::setfill('0') << std::setw(6) << i << ".obj";
    characterMeshes.push_back(loadScene(fname.str()));
  }

  flashlightMeshes = loadScene("models/flashlight.obj");
  for (std::vector<Mesh*>::iterator it = flashlightMeshes.begin(); it != flashlightMeshes.end(); it++) {
    Mesh* mesh = *it;
    mesh->getModelMatrix() = glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(21, 2, -11)), 180.0f, glm::vec3(0, 1, 0));
  }
  meshes.insert(meshes.end(), flashlightMeshes.begin(), flashlightMeshes.end());

  gunMeshes = loadScene("models/gun.obj");
  for (std::vector<Mesh*>::iterator it = gunMeshes.begin(); it != gunMeshes.end(); it++) {
    Mesh* mesh = *it;
    mesh->getModelMatrix() = glm::translate(glm::mat4(1.0), glm::vec3(-22, 0.3, -23));
  }
  meshes.insert(meshes.end(), gunMeshes.begin(), gunMeshes.end());

  if (pointLightMeshes.size() == 1) {
    pointLightMesh = pointLightMeshes[0];
    pointLightMesh->getModelMatrix() = glm::scale(glm::mat4(1.0), glm::vec3(0.1, 0.1, 0.1));
  } else {
    std::cerr << "Loading sphere mesh resulted in not 1 meshes!!" << std::endl;
    exit(1);
  }

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

  // Picking texture.
  glGenTextures(1, &pickingTexture);
  glBindTexture(GL_TEXTURE_2D, pickingTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_INT, 0);

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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, pickingTexture, 0);
  // Note: Adding here? Make sure to add to glDrawBuffers.

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
  glGenRenderbuffers(2, &depthRenderBuffers[0]);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffers[0]);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffers[0]);

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


  // Set up constant data.
  shadowmapBiasMatrix = glm::mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
  );

  // Set up SSAO data.
  srand(1);
  for (int i = 0; i < 4; i++) {
    ssaoKernel[i] = glm::vec3(
      (float)(rand() / (float)(RAND_MAX) * 2.0f - 1.0f),
      (float)(rand() / (float)(RAND_MAX) * 2.0f - 1.0f),
      (float)(rand() / (float)(RAND_MAX)));
    ssaoKernel[i] = glm::normalize(ssaoKernel[i]);

    float scale = (float) i / 4.0f;
    ssaoKernel[i] *= 0.9f * scale * scale;
  }
  for (int i = 0; i < NOISE_SIZE; i++) {
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


  // Scene-specific setup:
  glm::vec3 startPosition(0, 3.5, -40);
  controller->setHorizontalAngle(0);
  controller->setPosition(startPosition);
  controller->setHasFlashlight(false);
  controller->setHasGun(false);

  // Flashlight.
  lights.push_back(Light::spotLight(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -1.0), 18.0));
  //lights.push_back(Light::pointLight(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0, 3, 0)));
  lights.back()->getFalloff() = glm::vec3(1.0, 0.01, 0.0005);
  lights.back()->getAmbience() = glm::vec3(0.1, 0.1, 0.1);

  // "Sun"
  lights.push_back(Light::directionalLight(glm::vec3(0.7, 0.7, 0.7), glm::vec3(0.1, -1.0, 0.0)));
  //lights.back()->getAmbience() = glm::vec3(0.1, 0.1, 0.1);
  lights.back()->setEnabled(false);

  // Lamp
  moveLamp = Light::pointLight(glm::vec3(0.8, 0.8, 0.8), glm::vec3(0.0, 6.0, 0.0));
  moveLamp->getFalloff() = glm::vec3(1.0, 0.01, 0.01);
  lights.push_back(moveLamp);
  moveLamp->setEnabled(false);

  lightningLight = Light::pointLight(glm::vec3(1, 1, 1), glm::vec3(-70, 20, 30));
    //Light::directionalLight(glm::vec3(1, 1, 1), glm::vec3(1, -1, 0));
  lightningLight->setEnabled(false);
  lights.push_back(lightningLight);

  gunLight = Light::pointLight(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0));
  gunLight->setEnabled(false);
  lights.push_back(gunLight);

  // Test spotlight.
  //lights.push_back(Light::spotLight(glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 1.0, -1.0), glm::vec3(0.0, 0.0, 1.0), 15.0));

  // Specific stuff for meshes.
  for (unsigned int meshId = 0; meshId < meshes.size(); meshId++) {
    Mesh* mesh = meshes[meshId];
    if (mesh->getName().substr(0, 11) == "CandleFlame") {
      glm::vec3 candleColour = glm::vec3(0.8, 0.555, 0);
      mesh->getMaterial()->getEmissive() = glm::vec3(1, 1, 1);
      glm::vec3* vecs = mesh->getFirstFourVertices();
      lights.push_back(Light::pointLight(candleColour, vecs[0]));
      lights.back()->getAmbience() = glm::vec3(0.03, 0.03, 0.03);
      lights.back()->getFalloff() = glm::vec3(1.0, 0.002, 0.008);
    } else if (mesh->getName().substr(0, 9) == "Lightbulb") {
      mesh->getMaterial()->getEmissive() = glm::vec3(1, 1, 1);
    }
  }

  return true;
}

void Viewer::drawQuad() {
  quadProgram.vbo_vertexPositionModelspace(quadVertexBuffer);
  quadProgram.drawTriangles(2*3);
}

void Viewer::bindRenderTarget(GLuint renderTargetFBO) {

  checkGLErrors("bindRenderTarget start");
  // Render to target.
  if (renderTargetFBO == 0) {
    // Render to screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_FRONT_LEFT);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, renderTargetFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
  }
  checkGLErrors("bindRenderTarget end");
}

void Viewer::renderMesh(Mesh* mesh, bool onlyVerts) {
  geomTexturesProgram.vbo_vertexPositionModelspace(mesh->getBuffer(Mesh::VERTEX_BUF));
  if (!onlyVerts) {
    geomTexturesProgram.vbo_vertexUV(mesh->getBuffer(Mesh::UV_BUF));
    geomTexturesProgram.vbo_vertexNormalModelspace(mesh->getBuffer(Mesh::NORMAL_BUF));
    geomTexturesProgram.vbo_vertexTangentModelspace(mesh->getBuffer(Mesh::TANGENT_BUF));
    geomTexturesProgram.vbo_vertexBitangentModelspace(mesh->getBuffer(Mesh::BITANGENT_BUF));
  }
  geomTexturesProgram.drawTriangleElements(mesh->getBuffer(Mesh::ELEMENT_BUF), mesh->getNumIndices());
}

void Viewer::renderScene(GLuint renderTargetFBO, std::vector<Mesh*>& thisFrameMeshes, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraPosition, bool postProcess, double currentTime, double deltaTime, const glm::vec3& halfspacePosition, const glm::vec3& halfspaceNormal, bool doPicking) {

  static glm::mat4 lastVP = projectionMatrix * viewMatrix;

  // ======= Deferred rendering stage 1: Render geometry into textures. ===========

  // TODO: Make using program implicit.
  glUseProgram(geomTexturesProgram.getProgramId());

  // Render to framebuffer.
  glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);

  // Bind textures to fbo as multiple render target.
  geomTexturesProgram.attach_gl_FragDepth(deferredDepthTexture);
  geomTexturesProgram.attach_outDiffuse(deferredDiffuseTexture);
  geomTexturesProgram.attach_outSpecular(deferredSpecularTexture);
  geomTexturesProgram.attach_outEmissive(deferredEmissiveTexture);
  geomTexturesProgram.attach_outNormal(deferredNormalTexture);
  geomTexturesProgram.attach_outPicking(pickingTexture);

  // Setup attachments here so we can clear all of them.
  geomTexturesProgram.shaders::FragmentShader::setupDrawBuffers();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  if (lightningLight->isEnabled()) {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
  } else {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4 VP = projectionMatrix * viewMatrix;

  // Send MVP transformations to currently bound shader.
  geomTexturesProgram.set_V(viewMatrix);
  //geomTexturesProgram.set_P(projectionMatrix);
  // TODO: Find a better solution for this.
  geomTexturesProgram.shaders::GeomTexturesVertShader::set_halfspacePoint(halfspacePosition);
  geomTexturesProgram.shaders::GeomTexturesVertShader::set_halfspaceNormal(halfspaceNormal);


  for (std::vector<Mesh*>::const_iterator it = thisFrameMeshes.begin(); it != thisFrameMeshes.end(); it++) {
    Mesh* mesh = *it;

    glm::mat4 modelMatrix = mesh->getModelMatrix();
    glm::mat4 MVP = VP * modelMatrix;

    geomTexturesProgram.set_M(modelMatrix);
    geomTexturesProgram.set_MVP(MVP);

    // Bind mesh id for picking.
    geomTexturesProgram.set_meshId(mesh->getId());

    Material* material = mesh->getMaterial();
    if (material != NULL) {
      //geomTexturesProgram.set_material_ka(material_ka);
      geomTexturesProgram.set_material_kd(material->getDiffuse());
      geomTexturesProgram.set_material_ks(material->getSpecular());
      geomTexturesProgram.set_material_shininess(material->getShininess());
      geomTexturesProgram.set_material_emissive(material->getEmissive());

      // Bind diffuse texture if it exists.
      if (material->hasDiffuseTexture() && settings->isSet(Settings::TEXTURE_MAP)) {
        geomTexturesProgram.set_useDiffuseTexture(true);
        geomTexturesProgram.set_diffuseTexture(material->getDiffuseTexture()->getTextureId());
      } else {
        geomTexturesProgram.set_useDiffuseTexture(false);
      }

      // Avoid hardware perspective divide if pre-divided for mirrors.
      geomTexturesProgram.set_useNoPerspectiveUVs(material->isMirror() && settings->isSet(Settings::MIRRORS));

      // Bind normal texture if it exists.
      if (material->hasNormalTexture() && settings->isSet(Settings::NORMAL_MAP)) {
        geomTexturesProgram.set_useNormalTexture(true);
        geomTexturesProgram.set_normalTexture(material->getNormalTexture()->getTextureId());
      } else {
        geomTexturesProgram.set_useNormalTexture(false);
      }
    }

    renderMesh(mesh);
  }

  // Render point lights as spheres.
  if (RENDER_LIGHTS_AS_SPHERES) {
    geomTexturesProgram.set_useDiffuseTexture(false);
    geomTexturesProgram.set_useNormalTexture(false);
    for (std::vector<Light*>::const_iterator lightIt = lights.begin(); lightIt != lights.end(); lightIt++) {
      Light *light = *lightIt;
      if (!light->isEnabled() || (light->getType() != Light::POINT && light->getType() != Light::SPOT)) continue;

      // Set model matrix to move model to point light's position.
      glm::mat4 sphereModelMatrix = glm::translate(glm::mat4(1.0), light->getPosition()) * pointLightMesh->getModelMatrix();
      glm::mat4 MVP = VP * sphereModelMatrix;
      geomTexturesProgram.set_M(sphereModelMatrix);
      geomTexturesProgram.set_MVP(MVP);

      // Use light's diffuse as emissive material.
      geomTexturesProgram.set_material_kd(glm::vec3(0));
      geomTexturesProgram.set_material_ks(glm::vec3(0));
      glm::vec3 emissiveLight = light->getColour();
      emissiveLight *= 5.0;
      geomTexturesProgram.set_material_emissive(emissiveLight);

      renderMesh(pointLightMesh);
    }
  }

  // Picking - just get id of middle pixel!
  lastPickedMesh = 0;
  if (doPicking) {
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    glBindTexture(GL_TEXTURE_2D, pickingTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);
    glReadPixels(width/2, height/2, 1, 1, GL_RED, GL_UNSIGNED_INT, &lastPickedMesh);
  }


  // ======= Shadow map and blend deferred shading for each light ======================

  // Clear target first.
  if (postProcess) {
    // Render to texture.
    glBindFramebuffer(GL_FRAMEBUFFER, accumRenderFramebuffer);
    glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumRenderTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffers[0]);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
  } else {
    bindRenderTarget(renderTargetFBO);
  }

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  bool firstBlendPass = true;

  for (std::vector<Light*>::const_iterator it = lights.begin(); it != lights.end(); it++) {
    Light* light = *it;
    if (!light->isEnabled()) continue;

    glm::vec3 lightPos = light->getPosition();
    glm::vec3 lightDir = light->getDirection();
    glm::mat4 shadowmapDepthBiasVP;

    // ======= Shadow mapping =========
    if (settings->isSet(Settings::SHADOW_MAP)) {
      glUseProgram(depthProgram.getProgramId());
      if (light->getType() == Light::POINT) {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowCubeMapFramebuffer);
      } else {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);
      }

      glDrawBuffer(GL_NONE); // No colour output.
      glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
      glEnable(GL_DEPTH_TEST);

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
        depthVP = depthProjectionMatrix * depthViewMatrix;

        for (std::vector<Mesh*>::const_iterator it = thisFrameMeshes.begin(); it != thisFrameMeshes.end(); it++) {
          glm::mat4 depthMVP = depthVP * (*it)->getModelMatrix();
          depthProgram.set_depthMVP(depthMVP);

          renderMesh(*it, true);
        }

        if (light->getType() != Light::POINT) break;
      }

      if (light->getType() == Light::POINT) {
        // TODO: Figure out why we need to shift by -1 here.
        depthVP = glm::translate(glm::mat4(1.0), glm::vec3(-lightPos.x - 1, -lightPos.y - 1, -lightPos.z - 1));
      }

      shadowmapDepthBiasVP = shadowmapBiasMatrix * depthVP;
    }

    // ======= Deferred rendering stage 2: Deferred rendering using textures. ===========

    glUseProgram(deferredShadingProgram.getProgramId());

    if (postProcess) {
      // Set output to accumulation texture.
      glBindFramebuffer(GL_FRAMEBUFFER, accumRenderFramebuffer);
      glBindTexture(GL_TEXTURE_2D, accumRenderTexture);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumRenderTexture, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffers[0]);
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
    } else {
      bindRenderTarget(renderTargetFBO);
    }

    glViewport(0, 0, width, height);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Blend in order to accumulate lights.
    glEnablei(GL_BLEND, 0);
    glDisable(GL_DEPTH_TEST);
    if (firstBlendPass) {
      firstBlendPass = false;
      glBlendFunc(GL_ONE, GL_ZERO);
    } else {
      glBlendFunc(GL_ONE, GL_ONE);
    }

    deferredShadingProgram.set_diffuseTexture(deferredDiffuseTexture);
    deferredShadingProgram.set_specularTexture(deferredSpecularTexture);
    deferredShadingProgram.set_emissiveTexture(deferredEmissiveTexture);
    deferredShadingProgram.set_normalTexture(deferredNormalTexture);
    deferredShadingProgram.set_depthTexture(deferredDepthTexture);
    deferredShadingProgram.set_shadowMap(shadowmapDepthTexture);

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

    deferredShadingProgram.set_shadowMapCube(shadowmapCubeDepthTexture);

    // TODO: Do we need these?
    /*
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    */

    deferredShadingProgram.set_ssaoNoiseTexture(ssaoNoiseTexture);

    /*
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    */

    deferredShadingProgram.set_V(viewMatrix);
    deferredShadingProgram.set_P(projectionMatrix);
    deferredShadingProgram.set_shadowmapDepthBiasVP(shadowmapDepthBiasVP);
    deferredShadingProgram.set_lightDirectionWorldspace(lightDir);
    deferredShadingProgram.set_lightPositionWorldspace(lightPos);

    deferredShadingProgram.set_useDiffuse(settings->isSet(Settings::LIGHT_DIFFUSE));
    deferredShadingProgram.set_useSpecular(settings->isSet(Settings::LIGHT_SPECULAR));
    deferredShadingProgram.set_useShadow(settings->isSet(Settings::SHADOW_MAP));
    deferredShadingProgram.set_useSSAO(settings->isSet(Settings::SSAO));
    deferredShadingProgram.set_ssaoKernel(ssaoKernel);

    deferredShadingProgram.set_lightType((int) light->getType());
    deferredShadingProgram.set_lightColour(light->getColour());
    deferredShadingProgram.set_lightAmbience(light->getAmbience());
    deferredShadingProgram.set_lightFalloff(light->getFalloff());
    deferredShadingProgram.set_lightSpreadDegrees(light->getSpread());

    drawQuad();

    glDisablei(GL_BLEND, 0);
  }

  // --------- Final pass - post-processing and rendering to screen ---------------

  if (postProcess) {
    glUseProgram(postProcessProgram.getProgramId());

    glViewport(0, 0, width, height);
    // TODO: Clear?

    bindRenderTarget(renderTargetFBO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glDisable(GL_DEPTH_TEST);

    postProcessProgram.set_tex(accumRenderTexture);
    postProcessProgram.set_depthTexture(deferredDepthTexture);
    postProcessProgram.set_pickingTexture(pickingTexture);

    // Settings.
    postProcessProgram.set_useBlur(settings->isSet(Settings::BLUR));
    postProcessProgram.set_useMotionBlur(settings->isSet(Settings::MOTION_BLUR));
    postProcessProgram.set_shudder(currentTime - startShudderTime < 0.4);
    postProcessProgram.set_currentTime(currentTime);

    glm::mat4 newToOldMatrix = lastVP * glm::inverse(VP);
    postProcessProgram.set_newToOldMatrix(newToOldMatrix);
    postProcessProgram.set_fpsCorrection(TARGET_FRAME_DELTA / deltaTime);
    if (settings->isSet(Settings::HIGHLIGHT_PICK)) {
      postProcessProgram.set_selectedMeshId(lastPickedMesh);
    } else {
      postProcessProgram.set_selectedMeshId(0);
    }

    drawQuad();

    lastVP = VP; // Note: Only storing if doing post-processing. Gets around mirror rendering messing with this.
  }
}

void Viewer::run() {
  backgroundMusic->loop();
  controller->reset();

  glBindVertexArray(vertexArrayId);

  double lastTime = glfwGetTime();
  long fpsDisplayCounter = 0;
  double lastFPSTime = lastTime;
  lastThunderPlay = 0;

  do {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    bool doPostProcessing = settings->isSet(Settings::BLUR) || settings->isSet(Settings::MOTION_BLUR);

    // Get camera info from keyboard and mouse input.
    controller->update();
    const glm::mat4& projectionMatrix = controller->getProjectionMatrix();
    const glm::mat4& viewMatrix = controller->getViewMatrix();
    const glm::vec3& cameraPosition = controller->getPosition();

    // Thunder loop.
    static int tl = 0;
    static float tls[] = {8.0, 8.1, 8.8, 9.1, 9.8, 9.9};
    if (tl < 6 && currentTime - lastThunderPlay > tls[tl]) {
      tl++;
      lightningLight->setEnabled(!lightningLight->isEnabled());
    }
    if (currentTime - lastThunderPlay > 10.0) {
      thunderSound->play();
      lastThunderPlay = currentTime;
      tl = 0;
    }


    if (controller->isJumping()) {
      startCharAnimTime = currentTime;
    }

    // Find all meshes to render this frame.
    std::vector<Mesh*> thisFrameMeshes(meshes);
    int characterAnimId = 0;
    if (currentTime < startCharAnimTime + 1.0) {
      characterAnimId = static_cast<int>(round((currentTime - startCharAnimTime) * 20.0f)) % 20;
    }
    thisFrameMeshes.insert(thisFrameMeshes.begin(), characterMeshes[characterAnimId].begin(), characterMeshes[characterAnimId].end());

    // Move character.
    glm::mat4 characterModelMatrix = glm::inverse(viewMatrix);
      //glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0, 8, 0)), (float)currentTime*8.0f, glm::vec3(0, 1, 0));

    for (std::vector<Mesh*>::iterator it = characterMeshes[characterAnimId].begin(); it != characterMeshes[characterAnimId].end(); it++) {
      (*it)->getModelMatrix() = characterModelMatrix;
    }


    // Moving lights.

    // Flashlight.
    lights[0]->getPosition() = cameraPosition + glm::vec3(0, -01.6f, 0);
    lights[0]->setEnabled(controller->isFlashlightOn());
    // TODO: Why backwards about x?
    lights[0]->getDirection() = glm::vec3(glm::inverse(viewMatrix) * glm::vec4(0, 0, -1, 0));

    moveLamp->getPosition() = glm::vec3(17 + 5.0*std::cos(3.0*currentTime), 4.0, 19.6 + 5.0*std::sin(3.0*currentTime)); // Point.

    // Gun light.
    gunLight->getPosition() = cameraPosition;
    if (controller->isShooting()) {
      startShudderTime = currentTime;
      gunLight->setEnabled(true);
      moveLamp->setEnabled(!moveLamp->isEnabled());
    } else {
      gunLight->setEnabled(false);
    }

    if (settings->isSet(Settings::MIRRORS)) {
      // Note that this technique won't generally work for multiple mirrors without cube maps, because mirror view is only rendered one direction.
      // Can probably get this to work with two mirrors facing each other.
      for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
        Mesh* mesh = *it;
        Material* material = mesh->getMaterial();
        if (material == NULL || !material->isMirror()) continue;

        Mirror* mirror = static_cast<Mirror*>(material);

        const glm::vec3 *mirrorQuad = mesh->getFirstFourVertices();
        const glm::vec3 mirrorVertex = mirrorQuad[0];
        const glm::vec3 mirrorNormal = mesh->getFirstNormal();
        const glm::mat4 mirroredViewMatrix = controller->getMirroredViewMatrix(mirrorVertex, mirrorNormal);

        // TODO: This is a hack - make it work generally by scaling using original UVs...
        const glm::mat4 uvVP = projectionMatrix * mirroredViewMatrix * mesh->getModelMatrix();
        std::vector<glm::vec2> newUVs;
        for (int i = 0; i < 4; i++) {
          glm::vec4 projUV = uvVP * glm::vec4(mirrorQuad[i], 1);
          projUV = projUV / projUV[3];
          // Important Note: These are now pre-computed as perspective-corrected, so hardware perspective correction needs to be turned off when interpolating these!
          newUVs.push_back(glm::vec2(projUV[0] * 0.5f + 0.5f, projUV[1] * 0.5f + 0.5f));
        }
        mesh->setUVs(newUVs);

        renderScene(mirror->getMirrorFBO(), thisFrameMeshes, mirroredViewMatrix, projectionMatrix, cameraPosition, false, currentTime, deltaTime, mirrorVertex, mirrorNormal, false);

        mirror->update();
      }
    }

    // Main render of scene.
    renderScene(0, thisFrameMeshes, viewMatrix, projectionMatrix, cameraPosition, doPostProcessing, currentTime, deltaTime, glm::vec3(0), glm::vec3(0), true);


    // Picking up items.
    if (controller->isSelecting()) {
      if (lastPickedMesh != 0) {
        bool gotFlashlight = false;
        for (std::vector<Mesh*>::iterator it = flashlightMeshes.begin(); it != flashlightMeshes.end(); it++) {
          Mesh* mesh = *it;
          if (mesh->getId() == lastPickedMesh) {
            gotFlashlight = true;
            break;
          }
        }
        if (gotFlashlight) {
          controller->setHasFlashlight(true);
          getItemSound->play();
          std::vector<Mesh*>::iterator newEnd = meshes.end();
          for (std::vector<Mesh*>::iterator it = flashlightMeshes.begin(); it != flashlightMeshes.end(); it++) {
            newEnd = std::remove(meshes.begin(), newEnd, *it);
          }
          meshes.erase(newEnd, meshes.end());
        }


        bool gotGun = false;
        for (std::vector<Mesh*>::iterator it = gunMeshes.begin(); it != gunMeshes.end(); it++) {
          Mesh* mesh = *it;
          if (mesh->getId() == lastPickedMesh) {
            gotGun = true;
            break;
          }
        }
        if (gotGun) {
          controller->setHasGun(true);
          getItemSound->play();
          std::vector<Mesh*>::iterator newEnd = meshes.end();
          for (std::vector<Mesh*>::iterator it = gunMeshes.begin(); it != gunMeshes.end(); it++) {
            newEnd = std::remove(meshes.begin(), newEnd, *it);
          }
          meshes.erase(newEnd, meshes.end());
        }
      }
    }


    // ============ Debug Rendering =============
    if (RENDER_DEBUG_IMAGES) {
      glUseProgram(quadProgram.getProgramId());

      // Render to the screen
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDrawBuffer(GL_FRONT_LEFT);

      // Must be disabled to draw overtop.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      glDisable(GL_DEPTH_TEST);

      /*
      // Debug draw mirrors.
      std::vector<Mesh*> allMirrors;
      for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
        Material* m = (*it)->getMaterial();
        if (m != NULL && m->isMirror()) {
          allMirrors.push_back(*it);
        }
      }

      // Draw mirror 1 ----------------
      if (allMirrors.size() >= 1) {
        glViewport(0, 0, width/4, height/4);
        glBindTexture(GL_TEXTURE_2D, static_cast<Mirror*>(allMirrors[0]->getMaterial())->getMirrorTexture()->getTextureId());
        drawQuad();
      }

      // Draw mirror 2 ----------------
      if (allMirrors.size() >= 2) {
        glViewport(width/4, 0, width/4, height/4);
        glBindTexture(GL_TEXTURE_2D, static_cast<Mirror*>(allMirrors[1]->getMaterial())->getMirrorTexture()->getTextureId());
        drawQuad();
      }
      */

      // Draw diffuse ----------------
      glViewport(0, 0, height/4, height/4);
      quadProgram.set_texture(deferredDiffuseTexture);
      drawQuad();

      // Draw normals ----------------
      glViewport(height/4, 0, height/4, height/4);
      quadProgram.set_texture(deferredNormalTexture);
      drawQuad();

      // Draw depth ----------------
      glViewport(height/2, 0, height/4, height/4);
      quadProgram.set_texture(deferredDepthTexture);
      drawQuad();

      // Draw noise --------------------
      glViewport(height * 3 / 4, 0, height/4, height/4);
      quadProgram.set_texture(ssaoNoiseTexture);
      drawQuad();

      // Draw shadowmap ----------------
      glViewport(0, height*3/4, height/4, height/4);
      quadProgram.set_texture(shadowmapDepthTexture);
      drawQuad();

      // Draw cube shadowmap ----------
      //glViewport(height/4, height*3/4, height/4, height/4);
      //glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmapCubeDepthTexture);
      //drawQuad();

      // Draw picking texture ------------
      //glViewport(0, 0, width/4, height/4);
      //glBindTexture(GL_TEXTURE_2D, pickingTexture);
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

  delete settings;
  settings = NULL;

  delete thunderSound;
  thunderSound = NULL;

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
  glDeleteTextures(1, &pickingTexture);

  glDeleteBuffers(1, &quadVertexBuffer);
  glDeleteVertexArrays(1, &vertexArrayId);
  glDeleteRenderbuffers(2, &depthRenderBuffers[0]);

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

void Viewer::takeScreenshot() {
  unsigned char* pixels = new unsigned char[3*width*height];
  uint32_t* picks = new uint32_t[width*height];

  const std::string names[] = {"diffuse", "specular", "emissive", "normal",  "picking", "depth", "accum"};
  const GLuint texes[] = {deferredDiffuseTexture, deferredSpecularTexture, deferredEmissiveTexture, deferredNormalTexture, pickingTexture, deferredDepthTexture, accumRenderTexture};

  for (int i = 0; i < 7; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    glBindTexture(GL_TEXTURE_2D, texes[i]);
    if (i == 5) {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texes[i], 0);
    } else {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texes[i], 0);
    }


    if (i == 4) {
      glReadPixels(0, 0, width, height, GL_RED, GL_UNSIGNED_INT, picks);
      for (int p = 0; p < width*height; p++) {
        uint16_t pick = picks[p];
        pick *= 1000;
        pixels[3*p] = static_cast<unsigned char>(pick & 0xff);
        pixels[3*p+1] = static_cast<unsigned char>(pick & 0xff);
        pixels[3*p+2] = static_cast<unsigned char>(pick & 0xff);
      }
    } else if (i == 5) {
      glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, picks);
      for (int p = 0; p < width*height; p++) {
        uint32_t pick = picks[p];
        unsigned char fpix = static_cast<unsigned char>(static_cast<float>(pick - 4190000000)/90000000.0f*256.0f);
        pixels[3*p] = fpix;
        pixels[3*p+1] = fpix;
        pixels[3*p+2] = fpix;
      }
    } else {
      glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    }

    Texture::saveTextureToFile(pixels, width, height, "screenshots/screen_" + names[i] + ".png");
  }

  delete pixels;
  delete picks;
  std::cout << "Screenshots saved!" << std::endl;
}


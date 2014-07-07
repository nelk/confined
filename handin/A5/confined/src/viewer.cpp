#include <stdlib.h>
#include <vector>
#include <iostream>
#include <ctime>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "mesh.hpp"

#include "viewer.hpp"
#include "controller.hpp"

#define MIN_REQUIRED_COLOUR_ATTACHMENTS 2
#define RENDER_DEBUG_IMAGES true
#define SHADOWMAP_WIDTH 2048
#define SHADOWMAP_HEIGHT 2048

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

bool checkGLErrors() {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cerr << "OpenGL error! " << error << " - " << glewGetErrorString(error) << std::endl;
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
  window = glfwCreateWindow(width, height, "Shadow House", NULL, NULL);

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

  glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
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

  meshes = loadScene("models/test1.obj");
  glm::vec3 startPosition(0, 0, -10);
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, deferredDepthTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredDiffuseTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredNormalTexture, 0);

  if (!checkGLFramebuffer()) return false;


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
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmapDepthTexture, 0);

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

  if (quadProgramId == 0 || depthProgramId == 0 || geomTexturesProgramId == 0 || deferredShadingProgramId == 0) {
    return false;
  }

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

  // TODO: Texture sampler handlers.
  //GLuint textureId  = glGetUniformLocation(deferredShadingProgramId, "thetexture");
  GLuint texId = glGetUniformLocation(quadProgramId, "texture");

  // Handles for MVP matrix uniforms (render pass).
  GLuint matrixId = glGetUniformLocation(geomTexturesProgramId, "MVP");
  GLuint geomViewMatrixId = glGetUniformLocation(geomTexturesProgramId, "V");
  GLuint geomModelMatrixId = glGetUniformLocation(geomTexturesProgramId, "M");

  GLuint deferredViewMatrixId = glGetUniformLocation(deferredShadingProgramId, "V");
  GLuint deferredModelMatrixId = glGetUniformLocation(deferredShadingProgramId, "M");
  GLuint deferredProjectionMatrixId = glGetUniformLocation(deferredShadingProgramId, "P");
  GLuint lightPosId = glGetUniformLocation(deferredShadingProgramId, "lightPositionWorldspace");
  GLuint lightDirId = glGetUniformLocation(deferredShadingProgramId, "lightDirectionWorldspace");
  GLuint cameraPositionId = glGetUniformLocation(deferredShadingProgramId, "cameraPositionWorldspace");

  GLuint depthBiasId = glGetUniformLocation(deferredShadingProgramId, "depthBiasVP");

  // Deferred shading textures.
  GLuint deferredDiffuseTextureId = glGetUniformLocation(deferredShadingProgramId, "diffuseTexture");
  GLuint deferredNormalTextureId = glGetUniformLocation(deferredShadingProgramId, "normalTexture");
  GLuint deferredDepthTextureId = glGetUniformLocation(deferredShadingProgramId, "depthTexture");
  GLuint shadowmapId = glGetUniformLocation(deferredShadingProgramId, "shadowMap");

  // Handles for material properties (render pass).
  //GLuint material_ka = glGetUniformLocation(geomTexturesProgramId, "material_ka");
  GLuint material_kd = glGetUniformLocation(geomTexturesProgramId, "material_kd");
  //GLuint material_ks = glGetUniformLocation(geomTexturesProgramId, "material_ks");
  //GLuint material_shininess = glGetUniformLocation(geomTexturesProgramId, "material_shininess");

  // Direction light is from center. It points towards center.
  glm::vec3 lightDir = glm::vec3(0, 1, 0.01);
  double lightTime = 0.0;

  controller->reset();

  glBindVertexArray(vertexArrayId);

  do {
    // Enable this for render pass.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    // ======= Deferred rendering stage 1: Render geometry into textures. ===========

    glUseProgram(geomTexturesProgramId);

    // Render to framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Bind textures to fbo as multiple render target.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, deferredDepthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredDiffuseTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredNormalTexture, 0);

    // Set to render both colour attachments.
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFramebuffer);
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute the MVP matrix from keyboard and mouse input
    controller->update();
    const glm::mat4& projectionMatrix = controller->getProjectionMatrix();
    const glm::mat4& viewMatrix = controller->getViewMatrix();
    const glm::vec3& cameraPosition = controller->getPosition();
    glm::mat4 modelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

    // Send MVP transformations to currently bound shader.
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(geomModelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(geomViewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);

    for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
      Material* material = (*it)->getMaterial();
      if (material != NULL) {
        //glUniform3f(material_ka, material->ka.x, material->ka.y, material->ka.z);
        glUniform3f(material_kd, material->kd.x, material->kd.y, material->kd.z);
        //glUniform3f(material_ks, material->ks.x, material->ks.y, material->ks.z);
        //glUniform1f(material_shininess, material->shininess);
      }
      (*it)->renderGL();
    }

    // TODO: Textures.
    // Bind our texture in Texture Unit 0
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, texture);
    //glUniform1i(textureId, 0);

    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);

    // ======= Shadow mapping for each light =========
    glUseProgram(depthProgramId);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);
    glDrawBuffer(GL_NONE); // No colour output.
    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Bind texture.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmapDepthTexture, 0);

    // Light operations.
    lightTime += 0.02;
    lightDir = vec3(0, std::sin(lightTime), std::cos(lightTime));

    // Compute the MVP matrix from the light's point of view.
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
    glm::mat4 depthViewMatrix = glm::lookAt(lightDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0);
    glm::mat4 depthVP = depthProjectionMatrix * depthViewMatrix;
    glm::mat4 depthMVP = depthVP * depthModelMatrix;

    glUniformMatrix4fv(depthMatrixId, 1, GL_FALSE, &depthMVP[0][0]);

    for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
      (*it)->renderGLVertsOnly();
    }

    // ======= Deferred rendering stage 2: Deferred rendering using textures. ===========

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glUseProgram(deferredShadingProgramId);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render to screen.
    glDrawBuffer(GL_FRONT_LEFT);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
    glUniform1i(deferredDiffuseTextureId, 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
    glUniform1i(deferredNormalTextureId, 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
    glUniform1i(deferredDepthTextureId, 2);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
    glUniform1i(shadowmapId, 3);

    glm::mat4 biasMatrix(
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 0.5, 0.0,
      0.5, 0.5, 0.5, 1.0
    );
    glm::mat4 depthBiasVP = biasMatrix * depthVP;

    glUniformMatrix4fv(deferredModelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(deferredViewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(deferredProjectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(depthBiasId, 1, GL_FALSE, &depthBiasVP[0][0]);
    glUniform3f(cameraPositionId, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform3f(lightDirId, lightDir.x, lightDir.y, lightDir.z);
    glUniform3f(lightPosId, lightDir.x, lightDir.y, lightDir.z); // TODO.

    drawQuad();


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
      glUniform1i(texId, 0); // 0 - base image, 2 - normal map, 4 - shadow map.

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

      // Draw shadowmap ----------------
      glViewport(0, height*3/4, height/4, height/4);
      glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
      drawQuad();
    }
    // =========== End Debug =================

    // Swap buffers
    glfwSwapBuffers(window);

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 30*1000;
    nanosleep(&ts, NULL);

    checkGLErrors();

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

  glDeleteTextures(1, &shadowmapDepthTexture);
  glDeleteTextures(1, &deferredDiffuseTexture);
  glDeleteTextures(1, &deferredNormalTexture);
  glDeleteTextures(1, &deferredDepthTexture);
  //glDeleteTextures(1, &texture);

  glDeleteBuffers(1, &quadVertexBuffer);
  glDeleteVertexArrays(1, &vertexArrayId);

  for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
    delete *it;
  }
  meshes.clear();

  // Cleans up and closes window.
  glfwTerminate();
}


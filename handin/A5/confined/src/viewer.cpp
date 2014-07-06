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

void checkGLErrors() {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::cerr << "OpenGL error! " << error << " - " << glewGetErrorString(error) << std::endl;
  }
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

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

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one.
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  glGenVertexArrays(1, &vertexArrayId);

  meshes = loadScene("models/test1.obj");
  glm::vec3 startPosition(0, 0, -10);
  controller->setHorizontalAngle(0);

  controller->setPosition(startPosition);

  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  framebuffer = 0;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  // Depth texture.
  // TODO: constantify shadow depth map dimensions.
  glGenTextures(1, &shadowmapDepthTexture);
  glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowmapDepthTexture, 0);

  // No color output in the bound framebuffer, only depth.
  // TODO: Do this before shadowmapping.
  //glDrawBuffer(GL_NONE);

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, 0);

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

  // Always check that our framebuffer is ok
  GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer check failed! " << frameBufferStatus << " = " << glewGetErrorString(frameBufferStatus) << std::endl;
    return false;
  }

  // Quad's FBO for debugging visualization.
  static const GLfloat debugQuadVBuffer[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
  };

  glGenBuffers(1, &quad_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(debugQuadVBuffer), debugQuadVBuffer, GL_STATIC_DRAW);

  // Compile our GLSL programs.
  quad_programId = loadShaders( "shaders/passthrough.vert", "shaders/justTexture.frag" );

  depthProgramId = loadShaders("shaders/depthShadow.vert", "shaders/depthShadow.frag" );

  programId = loadShaders( "shaders/shader.vert", "shaders/shader.frag" );

  geomTexturesProgramId = loadShaders("shaders/geomTextures.vert", "shaders/geomTextures.frag");
  deferredShadingProgramId = 1; // TODO.
  //deferredShadingProgramId = loadShaders("shaders/deferredShading.vert", "shaders/deferredShading.frag");

  if (quad_programId == 0 || depthProgramId == 0 || programId == 0 || geomTexturesProgramId == 0 || deferredShadingProgramId == 0) {
    return false;
  }

  return true;
}

void Viewer::run() {
  // Handle for MVP uniform (shadow depth pass).
  //GLuint depthMatrixId = glGetUniformLocation(depthProgramId, "depthMVP");

  // Texture sampler handlers.
  //GLuint textureId  = glGetUniformLocation(programId, "thetexture");
  GLuint texId = glGetUniformLocation(quad_programId, "texture");

  // Handles for MVP matrix uniforms (render pass).
  GLuint matrixId = glGetUniformLocation(geomTexturesProgramId, "MVP");
  GLuint viewMatrixId = glGetUniformLocation(geomTexturesProgramId, "V");
  GLuint modelMatrixId = glGetUniformLocation(geomTexturesProgramId, "M");
  //GLuint depthBiasId = glGetUniformLocation(geomTexturesProgramId, "depthBiasMVP");
  //GLuint shadowMapId = glGetUniformLocation(programId, "shadowMap");

  // Handles for material properties (render pass).
  //GLuint material_ka = glGetUniformLocation(programId, "material_ka");
  GLuint material_kd = glGetUniformLocation(geomTexturesProgramId, "material_kd");
  //GLuint material_ks = glGetUniformLocation(programId, "material_ks");
  //GLuint material_shininess = glGetUniformLocation(programId, "material_shininess");

  glm::vec3 lightDir = glm::vec3(0, 0, 0);
  double lightTime = 0.0;

  controller->reset();

  glBindVertexArray(vertexArrayId);

  do {
    // Enable this for render pass.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    // ======= Deferred rendering stage 1: Render geometry into textures. ===========

    // Render to framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    // Bind textures to fbo as multiple render target.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, deferredDepthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredDiffuseTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredNormalTexture, 0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(geomTexturesProgramId);

    // Compute the MVP matrix from keyboard and mouse input
    controller->update();
    const glm::mat4& ProjectionMatrix = controller->getProjectionMatrix();
    const glm::mat4& ViewMatrix = controller->getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    //glm::mat4 biasMatrix(
      //0.5, 0.0, 0.0, 0.0,
      //0.0, 0.5, 0.0, 0.0,
      //0.0, 0.0, 0.5, 0.0,
      //0.5, 0.5, 0.5, 1.0
    //);

    // TODO: Bind in deferred pass.
    //glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

    // Send MVP transformations to currently bound shader.
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &ViewMatrix[0][0]);
    //glUniformMatrix4fv(depthBiasId, 1, GL_FALSE, &depthBiasMVP[0][0]);

    //glUniform3f(lightDirId, lightDir.x, lightDir.y, lightDir.z);

    // Set to render both colour attachments.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

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


    // ============ Debug Rendering =============
    // Render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(quad_programId);
    glDrawBuffer(GL_FRONT_LEFT);
    // TODO: Temp
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1st attribute buffer - vertices.
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glVertexAttribPointer(
      0,                  // attribute 0.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );

    // Must be disabled to draw overtop.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texId, 0); // 0 - base image, 2 - normal map, 4 - shadow map.

    // Draw diffuse ----------------
    glViewport(0, height/2, width/2, height/2);
    glBindTexture(GL_TEXTURE_2D, deferredDiffuseTexture);
    glDrawArrays(GL_TRIANGLES, 0, 2*3);

    // Draw normals ----------------
    glViewport(width/2, height/2, width/2, height/2);
    glBindTexture(GL_TEXTURE_2D, deferredNormalTexture);
    glDrawArrays(GL_TRIANGLES, 0, 2*3);

    // Draw depth ----------------
    glViewport(0, 0, width/2, height/2);
    glBindTexture(GL_TEXTURE_2D, deferredDepthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 2*3);

    glDisableVertexAttribArray(0);

    // =========== End Debug =================

    /*
    // ---- Render the shadowmap for debugging. -----
    // Render on a corner of the window.
    glViewport(0, 0, height/4, height/4);
    glUseProgram(quad_programId);

    // Bind texture in Texture Unit 0.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowmapDepthTexture);
    glUniform1i(texId, 0);

    // 1st attribute buffer - vertices.
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glVertexAttribPointer(
      0,                  // attribute 0.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );

    // Must be disabled to draw overtop.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    // Draw the triangles.
    glDrawArrays(GL_TRIANGLES, 0, 2*3);
    glDisableVertexAttribArray(0);
    */

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

  // Cleanup VBO and shader
  glDeleteProgram(programId);
  glDeleteProgram(depthProgramId);
  glDeleteProgram(quad_programId);
  glDeleteProgram(geomTexturesProgramId);
  glDeleteProgram(deferredShadingProgramId);
  //glDeleteTextures(1, &texture);

  glDeleteFramebuffers(1, &framebuffer);
  glDeleteTextures(1, &shadowmapDepthTexture);
  glDeleteTextures(1, &deferredDiffuseTexture);
  glDeleteTextures(1, &deferredNormalTexture);
  glDeleteTextures(1, &deferredDepthTexture);
  glDeleteBuffers(1, &quad_vertexbuffer);
  glDeleteVertexArrays(1, &vertexArrayId);

  for (std::vector<Mesh*>::const_iterator it = meshes.begin(); it != meshes.end(); it++) {
    delete *it;
  }
  meshes.clear();

  // Cleans up and closes window.
  glfwTerminate();
}


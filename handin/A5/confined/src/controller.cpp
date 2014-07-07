
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "controller.hpp"

Controller::Controller(Viewer* viewer)
  : viewer(viewer), lastTime(0), position(0, 0, 0), horizontalAngle(0.86f), verticalAngle(0), skipMovements(2) {}

void Controller::reset() {
  lastTime = glfwGetTime();
  glfwSetCursorPos(viewer->getWindow(), viewer->getWidth()/2, viewer->getHeight()/2);
  skipMovements = 2;
}

void Controller::setPosition(glm::vec3& p) {
  position = p;
}

glm::vec3 Controller::getPosition() {
  return position;
}

void Controller::setHorizontalAngle(float a) {
  horizontalAngle = a;
}
void Controller::setVerticalAngle(float a) {
  verticalAngle = a;
}

glm::mat4 Controller::getViewMatrix(){
  return viewMatrix;
}
glm::mat4 Controller::getProjectionMatrix(){
  return projectionMatrix;
}

void Controller::update() {
  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  GLFWwindow* window = viewer->getWindow();
  int width = viewer->getWidth();
  int height = viewer->getHeight();

  // Get mouse position.
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  // Reset mouse position for next frame.
  glfwSetCursorPos(window, width/2, height/2);

  if (skipMovements > 0) {
    skipMovements--;
    return;
  }

  // Compute new orientation
  float delta_x = width/2 - xpos;
  float delta_y = height/2 - ypos;
  horizontalAngle += MOUSE_SPEED * delta_x;
  verticalAngle   += MOUSE_SPEED * delta_y;

  if (verticalAngle > M_PI/2) {
    verticalAngle = M_PI/2;
  } else if (verticalAngle < -M_PI/2) {
    verticalAngle = -M_PI/2;
  }

  // Compute 3 direction vectors from spherical coordinates.
  glm::vec3 direction(
    cos(verticalAngle) * sin(horizontalAngle),
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
  );

  glm::vec3 right = glm::vec3(
    sin(horizontalAngle - M_PI/2.0),
    0,
    cos(horizontalAngle - M_PI/2.0)
  );

  glm::vec3 up = glm::cross(right, direction);

  // Keyboard movement.
  // Forwards.
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS
      || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    position += direction * deltaTime * SPEED;
  }
  // Backwards.
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS
      || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    position -= direction * deltaTime * SPEED;
  }
  // Right.
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS
      || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    position += right * deltaTime * SPEED;
  }
  // Left.
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS
      || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    position -= right * deltaTime * SPEED;
  }
  // Up.
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    position += up * deltaTime * SPEED;
  }
  // Down.
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    position -= up * deltaTime * SPEED;
  }

  // Projection matrix: 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units.
  projectionMatrix = glm::perspective(45.0f, width/(float)height, 0.1f, 100.0f);
  // Camera matrix
  viewMatrix = glm::lookAt(
    position,           // Camera position.
    position+direction, // Lookat position.
    up                  // Up.
  );

  lastTime = currentTime;
}


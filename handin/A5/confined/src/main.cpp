
#include <iostream>
#include "viewer.hpp"

int main(int argc, char* argv[]) {
  // Initialise GLFW
  if(!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  Viewer viewer;
  bool result = viewer.initGL();
  if (!result) {
    exit(1);
  }
  viewer.run();

  return 0;
}


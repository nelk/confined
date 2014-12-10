#include <iostream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <stdlib.h>
#include <string.h>

#include "shader.hpp"

namespace shaders {

bool Shader::loadShader() {
  shaderId = glCreateShader(getShaderType());
  std::ifstream fileStream(filename, std::ios::in);
  std::stringstream shaderStream;
  if (fileStream.is_open()) {
    std::string line = "";
    while (getline(fileStream, line)) {
      shaderStream << line << "\n";
    }
    fileStream.close();
  } else {
    std::cerr << "Could not open " << filename << std::endl;
    return false;
  }

  GLint result = GL_FALSE;
  int infoLogLength;

  // Compile Shader.
  std::cout << "Compiling shader: " << filename << std::endl;
  const std::string shaderSource = shaderStream.str();
  const char* shaderSourceCStr = shaderSource.c_str();
  glShaderSource(shaderId, 1, &shaderSourceCStr, NULL);
  glCompileShader(shaderId);

  // Check Shader.
  std::cout << "Checking shader: " << filename << std::endl;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (result != GL_TRUE) {
    std::vector<char> errorMessage(infoLogLength + 1);
    glGetShaderInfoLog(shaderId, infoLogLength, NULL, &errorMessage[0]);
    std::cerr << "Shader error: " << &errorMessage[0] << std::endl;
    return false;
  }

  return true;
}


// TODO: delete this.
GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open()){
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }else{
    std::cerr << "Could not open " << vertex_file_path << std::endl;
    return 0;
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int infoLogLength;

  // Compile Vertex Shader.
  std::cout << "Compiling shader: " << vertex_file_path << std::endl;
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if ( infoLogLength > 0 ){
    std::vector<char> VertexShaderErrorMessage(infoLogLength+1);
    glGetShaderInfoLog(VertexShaderID, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
    std::cerr << &VertexShaderErrorMessage[0] << std::endl;
  }

  // Compile Fragment Shader
  std::cout << "Compiling shader: " << fragment_file_path << std::endl;
  char const* FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if ( infoLogLength > 0 ){
    std::vector<char> FragmentShaderErrorMessage(infoLogLength+1);
    glGetShaderInfoLog(FragmentShaderID, infoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
  }

  // Link the program
  std::cout << "Linking program" << std::endl;
  GLuint programId = glCreateProgram();
  glAttachShader(programId, VertexShaderID);
  glAttachShader(programId, FragmentShaderID);
  glLinkProgram(programId);

  // Check the program
  glGetProgramiv(programId, GL_LINK_STATUS, &Result);
  glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0){
    std::vector<char> ProgramErrorMessage(infoLogLength+1);
    glGetProgramInfoLog(programId, infoLogLength, NULL, &ProgramErrorMessage[0]);
    std::cerr << &ProgramErrorMessage[0] << std::endl;
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return programId;
}

} // namespace shaders.


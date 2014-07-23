
#include <FreeImage.h>
#include <iostream>
#include "texture.hpp"

std::map<std::string, Texture*> Texture::loadedTextures;

static bool fiError = false;

void fiMessageFunction(FREE_IMAGE_FORMAT fif, const char *msg) {
  std::cerr << (int)fif << ": " << msg << std::endl;
  fiError = true;
}

void Texture::initialize() {
  FreeImage_SetOutputMessage(fiMessageFunction);
}

Texture* Texture::loadOrGet(std::string fname, bool useMipmaps) {
  if (loadedTextures.find(fname) != loadedTextures.end()) {
    return loadedTextures[fname];
  }

  FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(fname.c_str(), 0), fname.c_str());
  FIBITMAP *pImage = FreeImage_ConvertTo24Bits(bitmap);

  const int texWidth = FreeImage_GetWidth(bitmap);
  const int texHeight = FreeImage_GetHeight(bitmap);

  /*
  //unsigned char raw[32*texWidth*texHeight];
  BYTE* raw = (BYTE*)malloc(32*texWidth*texHeight*sizeof(BYTE));
  FreeImage_ConvertToRawBits(raw, pImage, texWidth*texHeight, 32, 1, 1, 1, false);
  */

  Texture* texture = new Texture(fname, texWidth, texHeight, (void*) FreeImage_GetBits(pImage), useMipmaps);

  loadedTextures[fname] = texture;

  //free(raw);
  FreeImage_Unload(pImage);
  FreeImage_Unload(bitmap);

  if (fiError) {
    fiError = false;
    return 0;
  }

  std::cout << "Loaded Texture " << fname << std::endl;
  return texture;
}

void Texture::freeLoadedTextures() {
  for (std::map<std::string, Texture*>::iterator it = loadedTextures.begin(); it != loadedTextures.end(); it++) {
    delete it->second;
  }
  loadedTextures.clear();
}

Texture::Texture(std::string fname, int width, int height, void* data, bool useMipmaps): name(fname), width(width), height(height) {
  texId = 0;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Note: Assuming texture was BGR.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
  if (useMipmaps) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
}

Texture::Texture(GLuint texId, int width, int height): texId(texId), name(""), width(width), height(height) {}

Texture::~Texture() {
  glDeleteTextures(1, &texId);
}


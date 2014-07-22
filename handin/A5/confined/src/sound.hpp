#ifndef SOUND_H
#define SOUND_H

#include <iostream>
#include <vector>
#include <AL/alut.h>
#include <glm/glm.hpp>

class Sound {
public:
  static bool initialize();
  static void deinitialize();

  static Sound* load(std::string fname);

  static void setListenerPosition(const glm::vec3& p);
  static void setListenerVelocity(const glm::vec3& v);
  static void setListenerOrientation(const glm::vec3& at, const glm::vec3& up);
  static void checkErrors();

  ~Sound();

  void play();
  void pause();
  void rewind();
  void stop();

  void setPosition(const glm::vec3& p);
  void setVelocity(const glm::vec3& v);
  void setDirection(const glm::vec3& d);

private:
  Sound(std::string name, ALuint buffer, ALuint source);

  ALuint buffer;
  ALuint source;
  std::string name;
};

#endif

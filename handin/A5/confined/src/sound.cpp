
#include "sound.hpp"
#include <AL/al.h>

bool Sound::initialize() {
  ALCcontext* context;
  ALCdevice* device;

  const ALCchar* deviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  std::cout << "OpenAL using device: " << deviceName << std::endl;
  device = alcOpenDevice(deviceName);
  if (device == NULL) {
    std::cerr << "aclOpenDevice gave NULL device" << std::endl;
    return false;
  }

  context = alcCreateContext(device, NULL);
  alcMakeContextCurrent(context);

  int error = alGetError();
  if (error != 0) {
    std::cerr << "aclCreateContext gave error: " << error << std::endl;
    return false;
  }

  if (alutInitWithoutContext(NULL, NULL) != AL_TRUE) {
    std::cerr << "alutInitWithoutContext failed" << std::endl;
    return false;
  }

  return true;
  /*
  alutInit(0, NULL);  // Init openAL.
  alGetError();       // Clear Error Code.
  */
}

Sound* Sound::load(std::string fname) {
  int error;
  ALuint buffer;
  ALuint source;

  buffer = alutCreateBufferFromFile(fname.c_str());
  if ((error = alutGetError()) != ALUT_ERROR_NO_ERROR ) {
    std::cerr << "alutCreateBufferFromFile " << fname << ", buffer=" << buffer << ": " << error << std::endl;
    return NULL;
  }

  // Generate source.
  alGenSources(1, &source);
  if ((error = alGetError()) != AL_NO_ERROR) {
    std::cerr << "alGenSources: " << error << std::endl;
    alDeleteBuffers(1, &buffer);
    return NULL;
  }

  alSourcei(source, AL_BUFFER, buffer);
  if ((error = alGetError()) != AL_NO_ERROR) {
    std::cerr << "alSourcei: " << error << std::endl;
    return NULL;
  }

  alSourcei(source, AL_REFERENCE_DISTANCE, 1.0);
  alSourcei(source, AL_MAX_DISTANCE, 2.0);

  std::cout << "Sound '" << fname << "' loaded." << std::endl;
  return new Sound(fname, buffer, source);
}


void Sound::deinitialize() {
  alutExit();
}

void Sound::setListenerPosition(const glm::vec3& p) {
  alListenerfv(AL_POSITION, &p[0]);
}

void Sound::setListenerVelocity(const glm::vec3& v) {
  alListenerfv(AL_VELOCITY, &v[0]);
}

void Sound::setListenerOrientation(const glm::vec3& at, const glm::vec3& up) {
  ALfloat orientation[6] = {
    at[0], at[1], at[2],
    up[0], up[1], up[2]
  };
  alListenerfv(AL_ORIENTATION, orientation);
}

void Sound::checkErrors() {
  int error;
  if ((error = alGetError()) != AL_NO_ERROR) {
    std::cerr << "AL ERROR: " << error << std::endl;
  }
}

Sound::Sound(std::string name, ALuint buffer, ALuint source): buffer(buffer), source(source), name(name) {}

Sound::~Sound() {
  alDeleteBuffers(1, &buffer);
  alDeleteSources(1, &source);
}


void Sound::setPosition(const glm::vec3& p) {
  alSourcefv(source, AL_POSITION, &p[0]);
}

void Sound::setVelocity(const glm::vec3& v) {
  alSourcefv(source, AL_VELOCITY, &v[0]);
}

void Sound::setDirection(const glm::vec3& d) {
  alSourcefv(source, AL_DIRECTION, &d[0]);
}

void Sound::play() {
  //std::cout << "Playing " << name << std::endl;
  alSourcePlay(source);
}

void Sound::pause() {
  alSourcePause(source);
}

void Sound::stop() {
  alSourceStop(source);
}

void Sound::rewind() {
  alSourceRewind(source);
}





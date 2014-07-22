#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings {
public:
  enum SettingsEnum {
    LIGHT_DIFFUSE,
    LIGHT_SPECULAR,
    SHADOW_MAP,
    NORMAL_MAP,
    TEXTURE_MAP,
    SSAO,
    BLUR,
    MOTION_BLUR,
    MIRRORS,
    NUM_SETTINGS
  };

  Settings() {
    for (int i = 0; i < NUM_SETTINGS; i++) {
      settings[i] = true;
    }
  }

  bool isSet(SettingsEnum e) {
    return settings[(int)e];
  }

  void set(SettingsEnum e, bool s) {
    settings[(int)e] = s;
  }

  void toggle(SettingsEnum e) {
    settings[(int)e] = !settings[(int)e];
  }

  static const std::string settingNames[Settings::NUM_SETTINGS];

private:
  bool settings[NUM_SETTINGS];
};

#endif

#include "PreferencesManager.h"
#include <Arduino.h>

PreferencesManager::PreferencesManager() {
  preferences.begin("myPreferences", false);
}

void PreferencesManager::setPreference(const char* key, int value) {
  Serial.println(String(key) + String(value));
  preferences.putInt(key, value);
}

int PreferencesManager::getPreference(const char* key) {
  return preferences.getInt(key, 0);
}

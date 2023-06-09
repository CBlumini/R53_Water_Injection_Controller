#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>

class PreferencesManager {
  public:
    PreferencesManager();
    void setPreference(const char* key, int value);
    int getPreference(const char* key);

  private:
    Preferences preferences;
};

#endif

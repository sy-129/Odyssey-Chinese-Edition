#pragma once
#import <Foundation/Foundation.h>
#include "Settings.hpp"
namespace ts {
Settings loadSettings();
void saveSettings(const Settings&);
NSDictionary<NSString*,NSNumber*>* settingsDictionary(const Settings&);
Settings settingsFromDictionary(NSDictionary<NSString*,NSNumber*>*);
}

/*
 * AndroidUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGLEXAMPLES_ANDROID_UTILS_H
#define LLGLEXAMPLES_ANDROID_UTILS_H


#include <android/native_activity.h>


// Utility function to hide the system UI (status bar, navigation bar) on Android devices.
void Android_HideSystemUI(ANativeActivity* activity);


#endif


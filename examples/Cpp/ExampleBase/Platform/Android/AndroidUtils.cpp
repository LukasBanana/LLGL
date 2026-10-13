/*
 * AndroidUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidUtils.h"
#include <jni.h>


void Android_HideSystemUI(ANativeActivity* activity)
{
    JavaVM* jvm = activity->vm;
    JNIEnv* env = nullptr;
    
    jint res = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (res == JNI_EDETACHED && jvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        return;

    jobject self = activity->clazz;

    // Get Window object
    jclass clsActivity = env->GetObjectClass(self);
    jmethodID midGetWindow = env->GetMethodID(clsActivity, "getWindow", "()Landroid/view/Window;");

    if (jobject window = env->CallObjectMethod(self, midGetWindow))
    {
        jclass clsWindow = env->GetObjectClass(window);
        jmethodID midGetDecorView = env->GetMethodID(clsWindow, "getDecorView", "()Landroid/view/View;");

        if (jobject decorView = env->CallObjectMethod(window, midGetDecorView))
        {
            jclass clsView = env->GetObjectClass(decorView);
            
            // Try modern WindowInsetsController API (API 30+)
            jmethodID midGetWindowInsetsController = env->GetMethodID(clsView, "getWindowInsetsController", "()Landroid/view/WindowInsetsController;");

            if (jobject insetsController = env->CallObjectMethod(decorView, midGetWindowInsetsController))
            {
                jclass clsInsetsController = env->GetObjectClass(insetsController);
                
                // NOTE: Inner classes use '$' instead of '.' in JNI descriptors!
                if (jclass clsInsetsType = env->FindClass("android/view/WindowInsets$Type"))
                {
                    // systemBars() is a static method returning int
                    jmethodID midTypeSystemBars = env->GetStaticMethodID(clsInsetsType, "systemBars", "()I");
                    jint systemBarsTypes = env->CallStaticIntMethod(clsInsetsType, midTypeSystemBars);

                    // Set system bars behavior: BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE = 2
                    jmethodID midSetSystemBarsBehavior = env->GetMethodID(clsInsetsController, "setSystemBarsBehavior", "(I)V");
                    env->CallVoidMethod(insetsController, midSetSystemBarsBehavior, 2);

                    // Hide the bars
                    jmethodID midHide = env->GetMethodID(clsInsetsController, "hide", "(I)V");
                    env->CallVoidMethod(insetsController, midHide, systemBarsTypes);
                }
                else
                {
                    // Clear the pending ClassNotFoundException if running on a lower API level
                    env->ExceptionClear();
                }
            } 
            else
            {
                // Fallback or explicit check if insetsController was null (API < 30)
                jint flags = 0x00000200 | // SYSTEM_UI_FLAG_LAYOUT_STABLE
                             0x00000400 | // SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                             0x00000100 | // SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                             0x00000002 | // SYSTEM_UI_FLAG_HIDE_NAVIGATION
                             0x00000004 | // SYSTEM_UI_FLAG_FULLSCREEN
                             0x00001000;  // SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                             
                jmethodID midSetSystemUiVisibility = env->GetMethodID(clsView, "setSystemUiVisibility", "(I)V");
                env->CallVoidMethod(decorView, midSetSystemUiVisibility, flags);
            }
        }
    }
}


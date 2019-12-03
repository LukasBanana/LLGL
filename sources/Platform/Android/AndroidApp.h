/*
 * AndroidApp.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_APP_H
#define LLGL_ANDROID_APP_H


#include <android/native_activity.h>


namespace LLGL
{


// Singleton class to store and access the primary native objects for an Android app.
class AndroidApp
{

    public:
    
        AndroidApp(const AndroidApp&) = delete;
        AndroidApp& operator = (const AndroidApp&) = delete;

        static AndroidApp& Get();
        
        void Initialize();

    private:
    
        AndroidApp() = default;
        
    private:

        ANativeActivity*    activity_   = nullptr;
        ALooper*            looper_     = nullptr;
        AInputQueue*        inputQueue_ = nullptr;
        ANativeWindow*      window_     = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================

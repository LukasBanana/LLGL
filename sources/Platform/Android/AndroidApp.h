/*
 * AndroidApp.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_APP_H
#define LLGL_ANDROID_APP_H


#include <LLGL/Types.h>
#include <android_native_app_glue.h>


namespace LLGL
{


// Singleton class to store and access the primary native objects for an Android app.
class AndroidApp
{

    public:
    
        AndroidApp(const AndroidApp&) = delete;
        AndroidApp& operator = (const AndroidApp&) = delete;

        static AndroidApp& Get();
        
        // Returns the size of the content rect of the specified Android app state.
        static Extent2D GetContentRectSize(android_app* appState);

        // Initializes the Android app state. This should be called once when the device is created.
        void Initialize(android_app* state);

        // Returns the android_app instance provided by the "native app glue" entry point.
        inline android_app* GetState() const
        {
            return state_;
        }
        
    private:
    
        AndroidApp() = default;
        
    private:

        android_app* state_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================

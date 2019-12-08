/*
 * AndroidApp.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_APP_H
#define LLGL_ANDROID_APP_H


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

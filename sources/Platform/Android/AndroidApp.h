/*
 * AndroidApp.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_APP_H
#define LLGL_ANDROID_APP_H


#include <LLGL/Types.h>
#include <LLGL/Platform/Android/AndroidContext.h>
#include <android_native_app_glue.h>
#include <cstddef>


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

        /*
        Initializes the Android objects LLGL was given. This should be called once when the device is
        created. The app state is optional and only carried for LLGL's own windowing; where it is given,
        this waits for the native window and installs the default input handler.
        */
        void Initialize(const AndroidContext& context, android_app* state);

        // Returns the native objects the application supplied.
        inline const AndroidContext& GetContext() const
        {
            return context_;
        }

        // Returns the android_app instance provided by the "native app glue" entry point.
        inline android_app* GetState() const
        {
            return state_;
        }
        
    private:
    
        AndroidApp() = default;
        
    private:

        AndroidContext  context_;
        android_app*    state_      = nullptr;

};


/*
Interprets the platform context of a RenderSystemDescriptor or XRSystemDescriptor: identified by
its declared size, it is either the android_app state of a NativeActivity ("native app glue")
application, or an AndroidContext supplied by an application that has none. Returns false if the
size matches neither structure. Defined inline so the XR frontend can share it without linking
against internal symbols of the core library.
*/
inline bool AndroidInterpretPlatformContext(AndroidContext& outContext, android_app*& outAppState, void* platformContext, std::size_t platformContextSize)
{
    outContext  = AndroidContext{};
    outAppState = nullptr;

    if (platformContext == nullptr)
        return true;

    if (platformContextSize == sizeof(android_app))
    {
        android_app* appState = static_cast<android_app*>(platformContext);
        outAppState = appState;
        if (appState->activity != nullptr)
        {
            outContext.applicationVM        = appState->activity->vm;
            outContext.applicationActivity  = appState->activity->clazz;
            outContext.assetManager         = appState->activity->assetManager;
        }
        return true;
    }

    if (platformContextSize == sizeof(AndroidContext))
    {
        outContext = *static_cast<const AndroidContext*>(platformContext);
        return true;
    }

    return false;
}


} // /namespace LLGL


#endif



// ================================================================================

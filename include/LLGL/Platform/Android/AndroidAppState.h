/*
 * AndroidAppState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_APP_STATE_H
#define LLGL_ANDROID_APP_STATE_H


#include <android/native_activity.h>


namespace LLGL
{


/**
\brief Android specific application state structure.
\remarks This is used to pass the app's entry point arguments to the render system.
\remarks This structure is derived from design of the \c android_app structure for native android apps.
\see RenderSystemDescriptor::android
\see https://developer.android.com/ndk/samples/sample_na#mac
*/
struct AndroidAppState
{
    //! The \c ANativeActivity object instance that this app is running in.
    ANativeActivity*    activity   = nullptr;
    
    //! The \c ALooper associated with the app's thread.
    ALooper*            looper      = nullptr;
    
    //! Optional pointer to the input queue from which the app will receive user input events.
    AInputQueue*        inputQueue  = nullptr;
    
    //! The window surface that the app can draw in.
    ANativeWindow*      window      = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================

/*
 * AndroidContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_CONTEXT_H
#define LLGL_ANDROID_CONTEXT_H


#include <LLGL/Platform/Platform.h>

#if defined LLGL_OS_ANDROID

#include <android/asset_manager.h>


namespace LLGL
{


/**
\brief Native Android objects LLGL needs from the host application.
\remarks LLGL names the individual objects rather than taking the NDK's \c android_app structure,
because that structure is only ever produced by \c ANativeActivity_onCreate - the entry point of a
NativeActivity-based application. An application whose Activity is written in Java, as with SDL,
never has one, but can readily supply everything here.
\remarks An application entered through \c android_main(android_app*) fills this in from its app
state; see the LLGL examples. Bringing up a render system or XR system requires nothing beyond
this structure.
\see RenderSystemDescriptor::androidContext
\see XRSystemDescriptor::androidContext
*/
struct AndroidContext
{
    /**
    \brief Java VM the application is running under, i.e. a \c JavaVM*. \b Required.
    \remarks Declared \c void* so this header does not pull in JNI. The OpenXR structures this is
    forwarded to declare it the same way.
    \remarks From \c android_main: \c app->activity->vm.
    */
    void*           applicationVM       = nullptr;

    /**
    \brief Android Activity instance, i.e. a JNI \c jobject. \b Required.
    \remarks The reference must stay valid for as long as the system it was passed to, so a caller
    holding only a JNI local reference must promote it to a global one.
    \remarks From \c android_main: \c app->activity->clazz.
    */
    void*           applicationActivity = nullptr;

    /**
    \brief Asset manager used to read files bundled in the APK. Optional.
    \remarks Only required if resources are loaded by file path, such as a shader specified with
    ShaderDescriptor::source referring to a file rather than in-memory code.
    \remarks From \c android_main: \c app->activity->assetManager.
    */
    AAssetManager*  assetManager        = nullptr;
};


} // /namespace LLGL

#endif // /LLGL_OS_ANDROID


#endif



// ================================================================================

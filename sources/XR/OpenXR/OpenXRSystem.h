/*
 * OpenXRSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_SYSTEM_H
#define LLGL_OPENXR_SYSTEM_H


#include <LLGL/XRSystem.h>
#include <LLGL/Report.h>
#include <LLGL/Container/DynamicVector.h>
#include <LLGL/Container/SmallVector.h>

#include "OpenXRPlatform.h"

#include <memory>
#include <vector>


namespace LLGL
{

namespace OpenXR
{


class GraphicsBinding;
class OpenXRSession;

class OpenXRSystem final : public XRSystem
{

    public:

        OpenXRSystem();
        ~OpenXRSystem();

        //! Brings up the OpenXR runtime per the descriptor. Returns false on failure with details in \c report.
        bool Startup(const XRSystemDescriptor& desc, Report* report);

    public:

        const char*                                 GetRuntimeName() const override;
        ArrayView<XRViewConfigurationView>          GetViewConfigurations() const override;
        const Report*                               GetReport() const override;

        RenderSystemPtr                             CreateRenderSystem(const XRRenderSystemDescriptor& renderSystemDesc, Report* report) override;
        XRSession*                                  CreateSession(const XRSessionDescriptor& sessionDesc, RenderSystem& renderSystem) override;
        void                                        Release(XRSession& session) override;

        bool                                        PollEvents() override;
        bool                                        GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

    private:

        bool LoadGraphicsBindings(Report* report);
        bool CreateInstance(const XRSystemDescriptor& desc, Report* report);
        bool QuerySystem(const XRSystemDescriptor& desc, Report* report);
        bool QueryViewConfigurations(Report* report);

        GraphicsBinding* FindBindingByName(const char* name) const;
        XrEnvironmentBlendMode SelectEnvironmentBlendMode(XREnvironmentBlendMode preferred) const;

    private:

        XrInstance                                                  instance_                   = XR_NULL_HANDLE;
        XrSystemId                                                  systemId_                   = XR_NULL_SYSTEM_ID;
        XrViewConfigurationType                                     viewConfigurationType_      = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        XRFormFactor                                                formFactor_                 = XRFormFactor::HeadMountedDisplay;
        XRViewConfiguration                                         viewConfiguration_          = XRViewConfiguration::Stereo;
        bool                                                        depthSubmissionEnabled_     = false;

    #ifdef LLGL_OS_ANDROID
        // Cached Android app context for the duration of the XR session lifecycle.
        android_app*                                                androidApp_                 = nullptr;
    #endif

        char                                                        runtimeName_[XR_MAX_RUNTIME_NAME_SIZE] = { 0 };

        DynamicVector<XRViewConfigurationView>                      viewConfigViews_;
        SmallVector<XrEnvironmentBlendMode>                         supportedBlendModes_;
        std::vector<std::unique_ptr<GraphicsBinding>>               bindings_;

        GraphicsBinding*                                            activeBinding_              = nullptr;
        Report                                                      report_;

        std::unique_ptr<OpenXRSession>                              activeSession_;

};


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================

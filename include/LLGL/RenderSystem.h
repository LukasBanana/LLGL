/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_H__
#define __LLGL_RENDER_SYSTEM_H__


#include "Export.h"

#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


//! Render system interface.
class LLGL_EXPORT RenderSystem
{

    public:

        /* ----- Render system ----- */

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator = (const RenderSystem&) = delete;

        virtual ~RenderSystem();

        /**
        \brief Returns the list of all available render system modules for the current platform
        (e.g. on Windows this might be { "OpenGL", "Direct3D12" }, but on MacOS it might be only { "OpenGL" }).
        */
        static std::vector<std::string> FindModules();

        /**
        \brief Loads a new render system from the specified module.
        \param[in] moduleName Specifies the name from which the new render system is to be loaded.
        This denotes a dynamic library (*.dll-files on Windows, *.so-files on Unix systems).
        If compiled in debug mode, the postfix "D" is appended to the module name.
        Moreover, the platform dependent file extension is always added automatically
        as well as the prefix "LLGL_", i.e. a module name "OpenGL" will be
        translated to "LLGL_OpenGLD.dll", if compiled on Windows in Debug mode.
        \remarks Usually the return type is a std::unique_ptr, but LLGL needs to keep track
        of the existance of this render system because only a single instance can be loaded at a time.
        So a std::weak_ptr is stored internally to check if it has been expired
        (see http://en.cppreference.com/w/cpp/memory/weak_ptr/expired),
        and this type can only refer to a std::shared_ptr.
        \throws std::runtime_exception If loading the render system from the specified module failed.
        \throws std::runtime_exception If there is already a loaded instance of a render system
        (make sure there are no more shared pointer references to the previous render system!)
        */
        static std::shared_ptr<RenderSystem> Load(const std::string& moduleName);

        //! Returns the name of this render system.
        inline const std::string& GetName() const
        {
            return name_;
        }

        //! Returns a descriptive version string of this render system (e.g. "OpenGL 4.5").
        virtual std::string GetVersion() const = 0;

    protected:

        RenderSystem() = default;

    private:

        std::string name_;

};


} // /namespace LLGL


#endif



// ================================================================================

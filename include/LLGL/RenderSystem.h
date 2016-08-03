/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_H__
#define __LLGL_RENDER_SYSTEM_H__


#include "Export.h"
#include "RenderContext.h"
#include "RenderSystemFlags.h"

#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "IndexBuffer.h"
#include "IndexFormat.h"

#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


//! Render system interface.
class LLGL_EXPORT RenderSystem
{

    public:

        /* ----- Common ----- */

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

        /* ----- Render context ----- */

        /**
        \brief Creates a new render context and returns the raw pointer.
        \remarks Rhe render system takes the ownership of this object. All render contexts are deleted in the destructor of this render system.
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) = 0;

        /**
        \brief Makes the specified render context to the current one.
        \param[in] renderContext Specifies the new current render context. If this is null, no render context is active.
        \return True on success, otherwise false.
        \remarks Never draw anything, while no render context is active!
        */
        bool MakeCurrent(RenderContext* renderContext);

        //! Returns the current render context. This may also be null.
        inline RenderContext* GetCurrentContext() const
        {
            return currentContext_;
        }

        /* ----- Hardware buffers ------ */

        virtual VertexBuffer* CreateVertexBuffer() = 0;
        virtual IndexBuffer* CreateIndexBuffer() = 0;
        /*virtual ConstantBuffer* CreateConstantBuffer(const ConstantBufferDescriptor& desc) = 0;
        virtual StorageBuffer* CreateStorageBuffer(const StorageBufferDescriptor& desc) = 0;*/

        virtual void WriteVertexBuffer(
            VertexBuffer& vertexBuffer,
            const void* data,
            std::size_t dataSize,
            const BufferUsage usage,
            const VertexFormat& vertexFormat
        ) = 0;

        virtual void WriteIndexBuffer(
            IndexBuffer& indexBuffer,
            const void* data,
            std::size_t dataSize,
            const BufferUsage usage,
            const IndexFormat& indexFormat
        ) = 0;

    protected:

        RenderSystem() = default;

        /**
        \brief Callback when a new render context is about to be made the current one.
        \remarks At this point, "GetCurrentContext" returns still the previous render context.
        */
        virtual bool OnMakeCurrent(RenderContext* renderContext);

    private:

        std::string     name_;

        RenderContext*  currentContext_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================

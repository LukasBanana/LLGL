/*
 * ShaderDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class ShaderDescriptor
    {
        public ShaderDescriptor() { }

        public ShaderDescriptor(
            ShaderType                  type,
            string                      sourceText,
            ShaderSourceType            sourceType,
            string                      entryPoint  = null,
            string                      profile     = null,
            ShaderMacro[]               defines     = null,
            ShaderCompileFlags          flags       = 0,
            string                      name        = null,
            VertexShaderAttributes      vertex      = null,
            FragmentShaderAttributes    fragment    = null,
            ComputeShaderAttributes     compute     = null)
        {
            if (sourceType == ShaderSourceType.BinaryBuffer)
            {
                throw new ArgumentException("Cannot set shader source as buffer when provided with string", "sourceType");
            }
            DebugName   = name;
            Type        = type;
            SourceText  = sourceText;
            SourceType  = sourceType;
            EntryPoint  = entryPoint;
            Profile     = profile;
            Defines     = defines;
            Flags       = flags;
            if (vertex != null)
            {
                Vertex = vertex;
            }
            if (fragment != null)
            {
                Fragment = fragment;
            }
            if (compute != null)
            {
                Compute = compute;
            }
        }

        public ShaderDescriptor(
            ShaderType                  type,
            byte[]                      sourceData,
            string                      entryPoint  = null,
            string                      profile     = null,
            ShaderMacro[]               defines     = null,
            ShaderCompileFlags          flags       = 0,
            string                      name        = null,
            VertexShaderAttributes      vertex      = null,
            FragmentShaderAttributes    fragment    = null,
            ComputeShaderAttributes     compute     = null)
        {
            Type        = type;
            SourceData  = sourceData;
            SourceType  = ShaderSourceType.BinaryBuffer;
            EntryPoint  = entryPoint;
            Profile     = profile;
            Defines     = defines;
            Flags       = flags;
            DebugName   = name;
            if (vertex != null)
            {
                Vertex = vertex;
            }
            if (fragment != null)
            {
                Fragment = fragment;
            }
            if (compute != null)
            {
                Compute = compute;
            }
        }

        public AnsiString DebugName { get; set; }

        public ShaderType Type { get; set; } = ShaderType.Undefined;

        private string sourceText;
        private byte[] sourceData;

        public string SourceText
        {
            get
            {
                return sourceText;
            }
            set
            {
                sourceText = value;
                sourceData = Encoding.ASCII.GetBytes(sourceText + "\0");
            }
        }

        public byte[] SourceData
        {
            get
            {
                return sourceData;
            }
            set
            {
                sourceData = value;
                sourceText = "";
            }
        }

        public ShaderSourceType SourceType { get; set; } = ShaderSourceType.CodeFile;

        private string entryPoint;
        private byte[] entryPointAscii;
        public string EntryPoint
        {
            get
            {
                return entryPoint;
            }
            set
            {
                entryPoint = value;
                entryPointAscii = Encoding.ASCII.GetBytes(entryPoint + "\0");
            }
        }

        private string profile;
        private byte[] profileAscii;
        public string Profile
        {
            get
            {
                return profile;
            }
            set
            {
                profile = value;
                profileAscii = Encoding.ASCII.GetBytes(profile + "\0");
            }
        }

        private ShaderMacro[] defines;
        private NativeLLGL.ShaderMacro[] definesNative;
        public ShaderMacro[] Defines
        {
            get
            {
                return defines;
            }
            set
            {
                if (value != null)
                {
                    defines = value;
                    definesNative = new NativeLLGL.ShaderMacro[defines.Length];
                    for (int definesIndex = 0; definesIndex < defines.Length; ++definesIndex)
                    {
                        if (defines[definesIndex] != null)
                        {
                            definesNative[definesIndex] = defines[definesIndex].Native;
                        }
                    }
                }
            }
        }

        public ShaderCompileFlags Flags { get; set; } = 0;
        [Obsolete("ShaderDescriptor.Name is deprecated since 0.04b; Use ShaderDescriptor.DebugName instead.")]
        public string Name
        {
            get
            {
                return DebugName;
            }
            set
            {
                DebugName = value;
            }
        }
        public VertexShaderAttributes Vertex { get; set; } = new VertexShaderAttributes();
        public FragmentShaderAttributes Fragment { get; set; } = new FragmentShaderAttributes();
        public ComputeShaderAttributes Compute { get; set; } = new ComputeShaderAttributes();

        internal NativeLLGL.ShaderDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.ShaderDescriptor();
                unsafe
                {
                    fixed (byte* debugNamePtr = DebugName.Ascii)
                    {
                        native.debugName = debugNamePtr;
                    }
                    native.type = Type;
                    if (sourceData != null)
                    {
                        fixed (byte* sourcePtr = sourceData)
                        {
                            native.source = sourcePtr;
                        }
                        native.sourceSize = (IntPtr)sourceData.Length;
                    }
                    native.sourceType = SourceType;
                    if (entryPoint != null)
                    {
                        fixed (byte* entryPointPtr = entryPointAscii)
                        {
                            native.entryPoint = entryPointPtr;
                        }
                    }
                    fixed (byte* profilePtr = profileAscii)
                    {
                        native.profile = profilePtr;
                    }
                    if (defines != null && defines.Length > 0)
                    {
                        fixed (NativeLLGL.ShaderMacro* definesPtr = definesNative)
                        {
                            native.defines = definesPtr;
                        }
                    }
                    native.flags = (int)Flags;
                    if (Vertex != null)
                    {
                        native.vertex = Vertex.Native;
                    }
                    if (Fragment != null)
                    {
                        native.fragment = Fragment.Native;
                    }
                    if (Compute != null)
                    {
                        native.compute = Compute.Native;
                    }
                }
                return native;
            }
        }
    }
}




// ================================================================================

/*
 * Shader.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class Shader : RenderSystemChild
    {
        internal NativeLLGL.Shader Native { get; private set; }

        internal override NativeLLGL.RenderSystemChild NativeChild
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.RenderSystemChild() { ptr = Native.ptr };
                }
            }
        }

        internal Shader(NativeLLGL.Shader native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~Shader()
        {
            NativeLLGL.ReleaseShader(Native);
        }

        public ShaderType Type
        {
            get
            {
                return NativeLLGL.GetShaderType(Native);
            }
        }

        public Report Report
        {
            get
            {
                unsafe
                {
                    NativeLLGL.Report nativeReport = NativeLLGL.GetShaderReport(Native);
                    if (nativeReport.ptr != null)
                    {
                        return new Report(nativeReport);
                    }
                    return null;
                }
            }
        }

        public ShaderReflection Reflect()
        {
            var nativeReflection = new NativeLLGL.ShaderReflection();
            NativeLLGL.ReflectShader(Native, ref nativeReflection);
            return new ShaderReflection(nativeReflection);
        }

    }
}




// ================================================================================

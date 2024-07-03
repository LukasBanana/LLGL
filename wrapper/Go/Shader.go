/*
 * Shader.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type Shader interface {
	RenderSystemChild
	Reflect() (ShaderReflection, bool)
	GetReport() Report
	GetType() ShaderType
}

type shaderImpl struct {
	native C.LLGLShader
}

func (self shaderImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self shaderImpl) Reflect() (ShaderReflection, bool) {
	var reflection ShaderReflection
	var nativeReflection C.LLGLShaderReflection
	if C.llglReflectShader(self.native, &nativeReflection) {
		convertShaderReflectionOutput(&reflection, &nativeReflection)
		return reflection, true
	}
	return reflection, false
}

func (self shaderImpl) GetReport() Report {
	return reportImpl{ C.llglGetShaderReport(self.native) }
}

func (self shaderImpl) GetType() ShaderType {
	return ShaderType(C.llglGetShaderType(self.native))
}



// ================================================================================

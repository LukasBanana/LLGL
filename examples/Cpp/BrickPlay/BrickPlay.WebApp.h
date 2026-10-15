/*
 * BrickPlay.WebApp.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_WEBAPP_H
#define LLGL_EXAMPLE_BRICKPLAY_WEBAPP_H


#include <LLGL/LLGL.h>
#include <functional>

#if LLGL_OS_WASM
#include <emscripten.h>
#endif


// This is where most of the Emscripten/JavaScript glue code lives

#if LLGL_OS_WASM

using FileUploadCallback = std::function<void(const char* filename, LLGL::StringView content)>;
using SubmitFormCallback = std::function<void(LLGL::StringView title)>;
using CanceledCallback = std::function<void()>;

struct WasmContext
{
    FileUploadCallback  fileUploadCallback  = nullptr;
    SubmitFormCallback  submitFormCallback  = nullptr;
    CanceledCallback    canceledCallback    = nullptr;
};

static WasmContext g_wasmContext;

extern "C"
{
    void BrickPlay_PromptFileDownload(const char* content, int contentLen, const char* filename, int filenameLen);
    void BrickPlay_PromptFileUpload();
    void BrickPlay_PromptSubmitForm();
    void BrickPlay_UploadToDatabase(const char* title, int titleLen, const char* content, int contentLen);
}

// Prompts the user to download a text file to their device.
static void PromptFileDownload(LLGL::StringView filename, LLGL::StringView content)
{
    BrickPlay_PromptFileDownload(content.data(), content.size(), filename.data(), filename.size());
}

// Extern "C" function that JavaScript can invoke once the file is read
extern "C" EMSCRIPTEN_KEEPALIVE
void OnFileLoadedFromJS(const char* filenamePtr, const char* contentPtr, int contentLength)
{
    if (g_wasmContext.fileUploadCallback)
    {
        g_wasmContext.fileUploadCallback(filenamePtr, LLGL::StringView{ contentPtr, static_cast<std::size_t>(contentLength) });
        g_wasmContext.fileUploadCallback = nullptr; // Reset callback
    }
}

// Prompts the user to upload a text file from their device.
static void PromptFileUpload(const FileUploadCallback& fileUploadCallback)
{
    g_wasmContext.fileUploadCallback = fileUploadCallback;
    BrickPlay_PromptFileUpload();
}

extern "C" EMSCRIPTEN_KEEPALIVE
void OnFormSubmitted(const char* name)
{
    if (g_wasmContext.submitFormCallback)
        g_wasmContext.submitFormCallback(LLGL::StringView{ name });

    // Reset callback
    g_wasmContext.submitFormCallback = nullptr;
    g_wasmContext.canceledCallback = nullptr;
}

extern "C" EMSCRIPTEN_KEEPALIVE
void OnFormCancelled()
{
    if (g_wasmContext.canceledCallback)
        g_wasmContext.canceledCallback();

    // Reset callbacks
    g_wasmContext.submitFormCallback = nullptr;
    g_wasmContext.canceledCallback = nullptr;
}

// Call this from C++ whenever you want to display the form overlay
static void PromptSubmitForm(const SubmitFormCallback& submitFormCallback, const CanceledCallback& canceledCallback)
{
    g_wasmContext.submitFormCallback = submitFormCallback;
    g_wasmContext.canceledCallback = canceledCallback;
    BrickPlay_PromptSubmitForm();
}

static void UploadToDatabase(LLGL::StringView title, LLGL::StringView content)
{
    BrickPlay_UploadToDatabase(title.data(), title.size(), content.data(), content.size());
}

#endif // /LLGL_OS_WASM


#endif // /LLGL_EXAMPLE_BRICKPLAY_WEBAPP_H



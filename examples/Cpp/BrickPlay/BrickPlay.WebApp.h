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

// Prompts the user to download a text file to their device.
static void PromptFileDownload(LLGL::StringView filename, LLGL::StringView content)
{
    EM_ASM(
        {
            // Convert C++ string pointers to JS strings/typed arrays
            var contentStr = UTF8ArrayToString(HEAPU8, $0, $1);
            var filenameStr = UTF8ArrayToString(HEAPU8, $2, $3);

            // Create a blob containing the text data
            var blob = new Blob([contentStr], { type: 'text/plain;charset=utf-8' });

            // Create an invisible anchor element to trigger the download
            var link = document.createElement('a');
            link.href = URL.createObjectURL(blob);
            link.download = filenameStr;

            // Append, trigger click, and cleanup
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);

            // Revoke the object URL to free memory
            setTimeout(function() {
                URL.revokeObjectURL(link.href);
            }, 1000);
        },
        content.data(), content.size(), filename.data(), filename.size()
    );
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

    EM_ASM(
        {
            // Create an invisible file input element
            var input = document.createElement('input');
            input.type = 'file';
            input.accept = '.txt,text/plain'; // Adjust extension/MIME filters as needed

            // iOS Fix: Must hide visually and append to document BODY
            input.style.display = 'none';
            document.body.appendChild(input);

            // Handle file selection
            input.onchange = function(e) {
                var file = e.target.files[0];

                // Cleanup from DOM on change or cancel
                if (input.parentNode)
                    input.parentNode.removeChild(input);

                if (!file)
                    return;

                var reader = new FileReader();
                reader.onload = function(evt) {
                    var arrayBuffer = evt.target.result;
                    var uint8Array = new Uint8Array(arrayBuffer);

                    // Allocate temporary WebAssembly memory for the buffer
                    var ptr = _malloc(uint8Array.length);
                    HEAPU8.set(uint8Array, ptr);

                    // Pass null-terminated filename
                    var filenameLength = lengthBytesUTF8(file.name) + 1;
                    var filenamePtr = _malloc(filenameLength);
                    stringToUTF8(file.name, filenamePtr, filenameLength);

                    // Call exported C++ function
                    _OnFileLoadedFromJS(filenamePtr, ptr, uint8Array.length);

                    // Free allocated C++ heap memory
                    _free(ptr);
                    _free(filenamePtr);
                };

                reader.readAsArrayBuffer(file);
            };

            // 3. Trigger file open dialog
            input.click();
        }
    );
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

    EM_ASM(
        {
            const overlay = document.getElementById('emscripten-form-overlay');
            if (overlay) {
                overlay.classList.remove('hidden');
                document.getElementById('es-name-input').focus();
            }
        }
    );
}

static void UploadToDatabase(LLGL::StringView title, LLGL::StringView content)
{
    EM_ASM_INT(
        {
            // Convert WebAssembly memory views to JavaScript strings and Uint8Arrays
            const title = UTF8ToString($0, $1);
            const contentBytes = new Uint8Array(Module.HEAPU8.buffer, $2, $3);

            (async () => {
                try {
                    // 1. Compress content using browser-native Gzip compression
                    const stream = new Blob([contentBytes]).stream().pipeThrough(new CompressionStream('gzip'));
                    const compressedBlob = await new Response(stream).blob();
                    const compressedBuffer = new Uint8Array(await compressedBlob.arrayBuffer());

                    // 2. Convert Uint8Array to PostgreSQL hex format for BYTEA columns
                    let hexString = '\\\\x';
                    for (let i = 0; i < compressedBuffer.length; i++) {
                        hexString += compressedBuffer[i].toString(16).padStart(2, '0');
                    }

                    // 3. Send to Supabase via REST API (using 'anon' key)
                    const SUPABASE_URL = 'https://ydbljrnnedmofzqkuyeo.supabase.co';
                    const SUPABASE_ANON_KEY = 'sb_publishable_rE6ZlsOGjgpdCYWKZNq0-g_UFR1KWW6';

                    const response = await fetch(`${SUPABASE_URL}/rest/v1/worlds`, {
                        method: 'POST',
                            headers: {
                            'Content-Type': 'application/json',
                            'apikey': SUPABASE_ANON_KEY,
                            'Authorization': `Bearer ${SUPABASE_ANON_KEY}`,
                            'Prefer': 'return=minimal'
                        },
                        body: JSON.stringify({
                            title: title,
                            world_data: hexString
                        })
                    });

                    if (!response.ok) {
                        const errText = await response.text();
                        console.error("Failed to upload '${title}' to Supabase:", errText);
                    } else {
                        console.log("Successfully uploaded '${title}' to Supabase!");
                    }
                } catch (err) {
                    console.error("Exception while uploading '${title}' to Supabase:", err);
                }
            })();
        },
        title.data(), title.size(), content.data(), content.size()
    );
}

#endif // /LLGL_OS_WASM


#endif // /LLGL_EXAMPLE_BRICKPLAY_WEBAPP_H



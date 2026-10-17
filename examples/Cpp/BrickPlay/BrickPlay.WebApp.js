/*
 * BrickPlay.WebApp.js
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

// `addToLibrary` requires Emscripten 3.1.4x or later.
// Use `mergeInto(LibraryManager.library, {...})` for older versions
addToLibrary({
    BrickPlay_PromptFileDownload__deps: ['$UTF8ArrayToString'],
    BrickPlay_PromptFileDownload: function(contentPtr, contentLen, filenamePtr, filenameLen) {
        // Convert C++ string pointers to JS strings/typed arrays
        var contentStr = UTF8ArrayToString(HEAPU8, contentPtr, contentLen);
        var filenameStr = UTF8ArrayToString(HEAPU8, filenamePtr, filenameLen);

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
        setTimeout(function() { URL.revokeObjectURL(link.href); }, 1000);
    },

    BrickPlay_PromptFileUpload__deps: ['malloc', 'free', '$lengthBytesUTF8', '$stringToUTF8'],
    BrickPlay_PromptFileUpload: function() {
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
    },

    BrickPlay_PromptSubmitForm: function() {
        if (window.showSubmitForm) {
            window.showSubmitForm();
        }
    },

    BrickPlay_UploadToDatabase__deps: ['$UTF8ToString'],
    BrickPlay_UploadToDatabase: function(titlePtr, titleLen, contentPtr, contentLen) {
        // Convert WebAssembly memory views to JavaScript strings and Uint8Arrays
        const title = UTF8ToString(titlePtr, titleLen);
        const contentBytes = new Uint8Array(Module.HEAPU8.buffer, contentPtr, contentLen);
        
        (async () => {
            try {
                // 1. Compress content using browser-native Gzip compression
                const stream = new Blob([contentBytes]).stream().pipeThrough(new CompressionStream('gzip'));
                const compressedBlob = await new Response(stream).blob();
                const compressedBuffer = new Uint8Array(await compressedBlob.arrayBuffer());

                // 2. Convert Uint8Array to PostgreSQL hex format for BYTEA columns
                let hexString = '\\x';
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
                    console.error(`Failed to upload '${title}' to Supabase:`, errText);
                } else {
                    console.log(`Successfully uploaded '${title}' to Supabase!`);
                }
            } catch (err) {
                console.error(`Exception while uploading '${title}' to Supabase:`, err);
            }
        })();
    },

    // Run this once when your application initializes
    BrickPlay_InitIDBFS: function() {
        FS.mkdir('/savefiles'); // Create a directory for your persistent files
        FS.mount(IDBFS, {}, '/savefiles');

        // Synchronize data from IndexedDB into the memory file system (load phase)
        FS.syncfs(true, function (err) {
            if (err) {
                console.error("Error loading from IndexedDB:", err);
            } else {
                console.log("IndexedDB loaded successfully into memory.");
                _OnIDBFSLoaded(); // Notify C++ that IDBFS has been loaded
            }
        });
    },

    BrickPlay_SyncIDBFS: function() {
        // Synchronize data from the memory file system back to IndexedDB (save phase)
        FS.syncfs(false, function (err) {
            if (err) {
                console.error("Error saving to IndexedDB:", err);
            }
        });
    }
});


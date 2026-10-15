document.addEventListener('DOMContentLoaded', () => {
    const overlay = document.getElementById('emscripten-form-overlay');
    const form = document.getElementById('es-user-form');
    const nameInput = document.getElementById('es-name-input');
    const cancelBtn = document.getElementById('es-cancel-btn');
    const closeBtn = document.getElementById('es-close-btn');
    const submitBtn = form.querySelector('.emscripten-btn-submit');

    function updateSubmitButton() {
        submitBtn.disabled = nameInput.value.trim().length === 0;
    }

    function hideForm() {
        overlay.classList.add('hidden');
        nameInput.value = '';
        updateSubmitButton();
    }

    nameInput.addEventListener('input', updateSubmitButton);
    updateSubmitButton();

    // Called from C++, e.g. EM_ASM(showSubmitForm()); or emscripten_run_script("showSubmitForm()");
    window.showSubmitForm = () => {
        overlay.classList.remove('hidden');
        nameInput.focus();
    };

    form.addEventListener('submit', (e) => {
        e.preventDefault();
        const nameVal = nameInput.value;
        if (nameVal.trim().length === 0) {
            updateSubmitButton();
            return;
        }

        hideForm();

        // Pass data back to C++ if Module is loaded and functions are exported
        if (typeof Module !== 'undefined' && Module._OnFormSubmitted) {
            const bufferBytes = lengthBytesUTF8(nameVal) + 1;
            const bufferPtr = _malloc(bufferBytes);
            stringToUTF8(nameVal, bufferPtr, bufferBytes);
            Module._OnFormSubmitted(bufferPtr);
            _free(bufferPtr);
        }
    });

    const triggerCancel = () => {
        hideForm();
        if (typeof Module !== 'undefined' && Module._OnFormCancelled) {
            Module._OnFormCancelled();
        }
    };

    cancelBtn.addEventListener('click', triggerCancel);
    closeBtn.addEventListener('click', triggerCancel);

    // Close if clicking the dark backdrop outside the modal card
    overlay.addEventListener('click', (e) => {
        if (e.target === overlay) {
            triggerCancel();
        }
    });

    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && !overlay.classList.contains('hidden')) {
            triggerCancel();
        }
    });
});
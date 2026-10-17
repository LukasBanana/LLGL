document.addEventListener('DOMContentLoaded', () => {
    const overlay = document.getElementById('emscripten-form-overlay');
    const form = document.getElementById('es-user-form');
    const nameInput = document.getElementById('es-name-input');
    const cancelBtn = document.getElementById('es-cancel-btn');
    const closeBtn = document.getElementById('es-close-btn');
    const submitBtn = form.querySelector('.emscripten-btn-submit');

    const cooldownMsg = document.getElementById('es-cooldown-msg');
    const COOLDOWN_MS = 120 * 1000;
    const STORAGE_KEY = 'brickplay.lastSubmitTime';
    let lastSubmitTime = -Infinity;
    let cooldownTimer = null;

    try {
        const stored = Number(localStorage.getItem(STORAGE_KEY));
        if (Number.isFinite(stored)) {
            lastSubmitTime = stored;
        }
    } catch (e) { /* storage unavailable, e.g. blocked by privacy settings */ }

    function remainingSeconds() {
        return Math.min(COOLDOWN_MS / 1000, Math.max(0, Math.ceil((lastSubmitTime + COOLDOWN_MS - Date.now()) / 1000)));
    }

    function updateSubmitButton() {
        submitBtn.disabled = nameInput.value.trim().length === 0 || remainingSeconds() > 0;
    }

    function stopCooldownTimer() {
        clearInterval(cooldownTimer);
        cooldownTimer = null;
    }

    function updateCooldown() {
        const secs = remainingSeconds();
        if (secs > 0) {
            cooldownMsg.textContent = `Please wait ${secs} second${secs === 1 ? '' : 's'} before submitting another creation.`;
            cooldownMsg.hidden = false;
        } else {
            cooldownMsg.hidden = true;
            stopCooldownTimer();
        }
        updateSubmitButton();
    }

    function startCooldownTimer() {
        stopCooldownTimer();
        updateCooldown();
        if (remainingSeconds() > 0) {
            cooldownTimer = setInterval(updateCooldown, 1000);
        }
    }

    function hideForm() {
        stopCooldownTimer();
        overlay.classList.add('hidden');
        nameInput.value = '';
        updateSubmitButton();
    }

    nameInput.addEventListener('input', updateSubmitButton);
    updateSubmitButton();

    // Called from C++, e.g. EM_ASM(showSubmitForm()); or emscripten_run_script("showSubmitForm()");
    window.showSubmitForm = () => {
        overlay.classList.remove('hidden');
        startCooldownTimer();
        nameInput.focus();
    };

    form.addEventListener('submit', (e) => {
        e.preventDefault();
        const nameVal = nameInput.value;
        if (nameVal.trim().length === 0 || remainingSeconds() > 0) {
            updateSubmitButton();
            return;
        }

        lastSubmitTime = Date.now();
        try {
            localStorage.setItem(STORAGE_KEY, String(lastSubmitTime));
        } catch (e) { /* storage unavailable */ }
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
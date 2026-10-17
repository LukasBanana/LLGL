document.addEventListener('DOMContentLoaded', () => {
    const overlay = document.getElementById('emscripten-form-overlay');
    const form = document.getElementById('es-user-form');
    const nameInput = document.getElementById('es-name-input');
    const cancelBtn = document.getElementById('es-cancel-btn');
    const closeBtn = document.getElementById('es-close-btn');
    const submitBtn = form.querySelector('.emscripten-btn-submit');

    const cooldownMsg = document.getElementById('es-cooldown-msg');

    // English is the default; its static text also lives in index.html as a fallback.
    // Values are trusted static HTML (applied via innerHTML), never user input.
    const TRANSLATIONS = {
        en: {
            heading: 'Would you like to share your creation?',
            intro: 'Sharing is optional. If you submit, your creation title and 3D world will be stored ' +
                'in a private database and reviewed by the creator of <a href="https://github.com/LukasBanana/LLGL">LLGL</a> ' +
                'to help improve this open-source library. Your contribution may be included in a curated collection in the future.',
            privacy: 'Please <b>don\'t include personal or identifying information</b> in your title or build!',
            titleLabel: 'Title:',
            placeholder: 'Give your creation a title...',
            cancel: 'Cancel',
            submit: 'Submit',
            cooldown: (s) => `Please wait ${s} second${s === 1 ? '' : 's'} before submitting another creation.`,
        },
        de: {
            heading: 'M\u00F6chtest du deine Kreation teilen?',
            intro: 'Das Teilen ist freiwillig. Wenn du sie einreichst, werden der Titel und die 3D-Welt deiner Kreation ' +
                'in einer privaten Datenbank gespeichert und vom Entwickler von <a href="https://github.com/LukasBanana/LLGL">LLGL</a> ' +
                'durchgesehen, um diese Open-Source-Bibliothek zu verbessern. Dein Beitrag kann in Zukunft in eine kuratierte Sammlung aufgenommen werden.',
            privacy: 'Bitte gib in Titel und Bauwerk <b>keine pers\u00F6nlichen oder identifizierenden Informationen</b> an!',
            titleLabel: 'Titel:',
            placeholder: 'Gib deiner Kreation einen Titel...',
            cancel: 'Abbrechen',
            submit: 'Einreichen',
            cooldown: (s) => `Bitte warte noch ${s} Sekunde${s === 1 ? '' : 'n'}, bevor du eine weitere Kreation einreichst.`,
        },
        fr: {
            heading: 'Souhaitez-vous partager votre cr\u00E9ation ?',
            intro: 'Le partage est facultatif. Si vous soumettez votre cr\u00E9ation, son titre et son monde 3D seront stock\u00E9s ' +
                'dans une base de donn\u00E9es priv\u00E9e et examin\u00E9s par le cr\u00E9ateur de <a href="https://github.com/LukasBanana/LLGL">LLGL</a> ' +
                'afin d\'aider \u00E0 am\u00E9liorer cette biblioth\u00E8que open source. Votre contribution pourra \u00EAtre int\u00E9gr\u00E9e \u00E0 une collection s\u00E9lectionn\u00E9e \u00E0 l\'avenir.',
            privacy: 'Merci de <b>ne pas inclure d\'informations personnelles ou identifiantes</b> dans votre titre ou votre construction !',
            titleLabel: 'Titre :',
            placeholder: 'Donnez un titre \u00E0 votre cr\u00E9ation...',
            cancel: 'Annuler',
            submit: 'Soumettre',
            cooldown: (s) => `Veuillez patienter ${s} seconde${s === 1 ? '' : 's'} avant de soumettre une autre cr\u00E9ation.`,
        },
    };

    function detectLanguage() {
        for (const tag of (navigator.languages || [navigator.language])) {
            const base = String(tag).toLowerCase().split('-')[0];
            if (TRANSLATIONS[base]) {
                return base;
            }
        }
        return 'en';
    }

    const lang = detectLanguage();
    const strings = TRANSLATIONS[lang];
    document.documentElement.lang = lang;

    document.querySelectorAll('[data-i18n]').forEach((el) => {
        el.innerHTML = strings[el.dataset.i18n];
    });
    document.querySelectorAll('[data-i18n-placeholder]').forEach((el) => {
        el.placeholder = strings[el.dataset.i18nPlaceholder];
    });
    document.querySelectorAll('[data-i18n-title]').forEach((el) => {
        el.title = strings[el.dataset.i18nTitle];
    });

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
            cooldownMsg.textContent = strings.cooldown(secs);
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
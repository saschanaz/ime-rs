#![allow(clippy::missing_safety_doc)]

use composition_processor::{
    CandidateMode, CompositionProcessorEngine, KeystrokeCategory, KeystrokeFunction,
};
use globals::{
    SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
};
use itf_components::compartment::Compartment;
use windows::Win32::UI::{
    Input::KeyboardAndMouse::{GetKeyboardState, MapVirtualKeyW, ToUnicode},
    TextServices::{
        ITfThreadMgr, GUID_COMPARTMENT_EMPTYCONTEXT, GUID_COMPARTMENT_KEYBOARD_DISABLED,
        GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
    },
    WindowsAndMessaging::MAPVK_VK_TO_VSC,
};

// This copies CSampleIME::_IsKeyboardDisabled from the original demo.
// This theoretically makes sense, but a confirmation is needed whether a real
// world situation exists where this becomes true.
fn is_keyboard_disabled(thread_mgr: ITfThreadMgr, tf_client_id: u32) -> bool {
    let document_mgr = unsafe { thread_mgr.GetFocus() };
    if document_mgr.is_err() {
        // if there is no focus document manager object, the keyboard
        // is disabled.
        return true;
    }

    let document_mgr = document_mgr.unwrap();
    if unsafe { document_mgr.GetTop() }.is_err() {
        // if there is no context object, the keyboard is disabled.
        return true;
    }

    let keyborard_disabled = Compartment::new(
        thread_mgr.clone(),
        tf_client_id,
        GUID_COMPARTMENT_KEYBOARD_DISABLED,
    );
    if let Ok(disabled) = keyborard_disabled.get_bool() {
        if disabled {
            return true;
        }
    }

    let empty_context = Compartment::new(thread_mgr, tf_client_id, GUID_COMPARTMENT_EMPTYCONTEXT);
    if let Ok(empty) = empty_context.get_bool() {
        if empty {
            return true;
        }
    }

    false
}

fn convert_vkey(code: u32) -> u16 {
    let scan_code = unsafe { MapVirtualKeyW(code, MAPVK_VK_TO_VSC) };

    let mut keyboard_state = [0u8; 256];
    if !unsafe { GetKeyboardState(&mut keyboard_state) }.as_bool() {
        return 0;
    }

    let mut wch = [0u16; 1];
    if unsafe { ToUnicode(code, scan_code, &keyboard_state, &mut wch, 0) } == 1 {
        return wch[0];
    }

    0
}

fn has_wide_version(ch: char) -> bool {
    (' '..='~').contains(&ch)
}

pub fn _is_key_eaten(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    engine: &CompositionProcessorEngine,
    composing: bool,
    candidate_mode: CandidateMode,
    code: u32,
) -> (bool, char, KeystrokeCategory, KeystrokeFunction) {
    // if the keyboard is disabled, we don't eat keys.
    if is_keyboard_disabled(thread_mgr.clone(), tf_client_id) {
        return (
            false,
            '\0',
            KeystrokeCategory::None,
            KeystrokeFunction::None,
        );
    }

    let is_open = Compartment::read_bool(
        thread_mgr.clone(),
        tf_client_id,
        GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
    );

    let is_double_single_byte = Compartment::read_bool(
        thread_mgr.clone(),
        tf_client_id,
        SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE,
    );

    let is_punctuation = Compartment::read_bool(
        thread_mgr,
        tf_client_id,
        SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
    );

    // if the keyboard is closed, we don't eat keys
    if !is_open && !is_double_single_byte && !is_punctuation {
        return (
            false,
            '\0',
            KeystrokeCategory::None,
            KeystrokeFunction::None,
        );
    }

    //
    // Map virtual key to character code
    //
    let ch = unsafe { char::from_u32_unchecked(convert_vkey(code) as u32) };
    let mut keystroke_state = (KeystrokeCategory::None, KeystrokeFunction::None);

    if is_open {
        //
        // The candidate or phrase list handles the keys through ITfKeyEventSink.
        //
        // eat only keys that CKeyHandlerEditSession can handles.
        //
        let (needed, category, function) =
            engine.test_virtual_key(code as u16, ch, composing, candidate_mode);
        if needed {
            return (true, ch, category, function);
        }
        keystroke_state = (category, function);
    }

    //
    // Punctuation
    //
    if is_punctuation
        && (candidate_mode == CandidateMode::None)
        && engine.punctuation_mapper().has_alternative_punctuation(ch)
    {
        return (
            true,
            ch,
            KeystrokeCategory::Composing,
            KeystrokeFunction::Punctuation,
        );
    }

    //
    // Double/Single byte
    //
    if is_double_single_byte && has_wide_version(ch) && candidate_mode == CandidateMode::None {
        return (
            true,
            ch,
            KeystrokeCategory::Composing,
            KeystrokeFunction::DoubleSingleByte,
        );
    }

    (false, ch, keystroke_state.0, keystroke_state.1)
}

#[export_name = "key_event_sink_is_key_eaten"]
pub unsafe extern "C" fn is_key_eaten(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    engine: &CompositionProcessorEngine,
    composing: bool,
    candidate_mode: CandidateMode,
    code: u32,
    ch: *mut u16,
    category: *mut KeystrokeCategory,
    function: *mut KeystrokeFunction,
) -> bool {
    let (eaten, _ch, _category, _function) = _is_key_eaten(
        thread_mgr,
        tf_client_id,
        engine,
        composing,
        candidate_mode,
        code,
    );
    *ch = _ch as _;
    *category = _category;
    *function = _function;
    eaten
}

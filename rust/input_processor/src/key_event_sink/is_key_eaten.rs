use itf_components::compartment::Compartment;
use windows::Win32::UI::{
    Input::KeyboardAndMouse::{GetKeyboardState, MapVirtualKeyW, ToUnicode},
    TextServices::{
        ITfThreadMgr, GUID_COMPARTMENT_EMPTYCONTEXT, GUID_COMPARTMENT_KEYBOARD_DISABLED,
    },
    WindowsAndMessaging::MAPVK_VK_TO_VSC,
};

// This copies CSampleIME::_IsKeyboardDisabled from the original demo.
// This theoretically makes sense, but a confirmation is needed whether a real
// world situation exists where this becomes true.
#[no_mangle]
pub extern "C" fn is_keyboard_disabled(thread_mgr: ITfThreadMgr, tf_client_id: u32) -> bool {
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

#[no_mangle]
pub extern "C" fn convert_vkey(code: u32) -> u16 {
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

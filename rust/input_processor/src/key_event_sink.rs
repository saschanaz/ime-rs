use windows::{
    core::Interface,
    Win32::UI::TextServices::{ITfKeyEventSink, ITfKeystrokeMgr, ITfThreadMgr},
};

pub mod is_key_eaten;

// TODO: Port other functions...
// Several of them use _InvokeKeyHandler which has complex dependencies.
// For now port things that don't call it first e.g. OnPreservedKey.

fn _init_key_event_sink(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    sink: ITfKeyEventSink,
) -> windows::core::Result<()> {
    let keystroke_mgr: ITfKeystrokeMgr = thread_mgr.cast()?;

    unsafe { keystroke_mgr.AdviseKeyEventSink(tf_client_id, sink, true) }?;

    Ok(())
}

#[no_mangle]
pub unsafe extern "C" fn init_key_event_sink(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    sink: ITfKeyEventSink,
) -> bool {
    _init_key_event_sink(thread_mgr, tf_client_id, sink).is_ok()
}

fn _uninit_key_event_sink(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
) -> windows::core::Result<()> {
    let keystroke_mgr: ITfKeystrokeMgr = thread_mgr.cast()?;

    unsafe { keystroke_mgr.UnadviseKeyEventSink(tf_client_id) }?;

    Ok(())
}

#[no_mangle]
pub unsafe extern "C" fn uninit_key_event_sink(thread_mgr: ITfThreadMgr, tf_client_id: u32) {
    _uninit_key_event_sink(thread_mgr, tf_client_id).ok();
}

use composition_processor::{CandidateMode, CompositionProcessorEngine};
use windows::{
    core::Interface,
    Win32::{
        Foundation::{LPARAM, WPARAM},
        UI::TextServices::{ITfKeyEventSink, ITfKeystrokeMgr, ITfThreadMgr},
    },
};

use self::is_key_eaten::_is_key_eaten;

pub mod is_key_eaten;

// TODO: Port other functions...
// Several of them use _InvokeKeyHandler which has complex dependencies.
// For now port things that don't call it first e.g. OnPreservedKey.

#[export_name = "key_event_sink_on_key_up"]
pub unsafe extern "C" fn on_key_up(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    engine: &mut CompositionProcessorEngine,
    composing: bool,
    candidate_mode: CandidateMode,
    wparam: WPARAM,
    lparam: LPARAM,
) -> bool {
    engine.modifiers_mut().update(wparam, lparam);

    _is_key_eaten(
        thread_mgr,
        tf_client_id,
        engine,
        composing,
        candidate_mode,
        wparam.0 as u32,
    )
    .0
}

fn _init_key_event_sink(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    sink: ITfKeyEventSink,
) -> windows::core::Result<()> {
    let keystroke_mgr: ITfKeystrokeMgr = thread_mgr.cast()?;

    unsafe { keystroke_mgr.AdviseKeyEventSink(tf_client_id, sink, true) }?;

    Ok(())
}

#[export_name = "key_event_sink_init_key_event_sink"]
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

#[export_name = "key_event_sink_uninit_key_event_sink"]
pub unsafe extern "C" fn uninit_key_event_sink(thread_mgr: ITfThreadMgr, tf_client_id: u32) {
    _uninit_key_event_sink(thread_mgr, tf_client_id).ok();
}

#![allow(clippy::missing_safety_doc)]

use std::ffi::c_void;

use windows::{
    core::{HSTRING, PCWSTR},
    Win32::{
        Graphics::Gdi::{
            CreateFontW, GetDC, GetDeviceCaps, FONT_CHARSET, FONT_CLIP_PRECISION,
            FONT_OUTPUT_PRECISION, FONT_QUALITY, FW_MEDIUM, HFONT, LOGFONTW, LOGPIXELSY,
        },
        System::WindowsProgramming::MulDiv,
        UI::WindowsAndMessaging::{
            SystemParametersInfoW, SPI_GETICONTITLELOGFONT, SYSTEM_PARAMETERS_INFO_UPDATE_FLAGS,
        },
    },
};

use crate::resources::DEFAULT_FONT;

#[no_mangle]
pub static mut DEFAULT_FONT_HANDLE: HFONT = HFONT(std::ptr::null_mut());

pub unsafe fn set_default_candidate_text_font() {
    // Candidate Text Font
    if !DEFAULT_FONT_HANDLE.0.is_null() {
        return;
    }

    DEFAULT_FONT_HANDLE = CreateFontW(
        -MulDiv(10, GetDeviceCaps(Some(GetDC(None)), LOGPIXELSY), 72),
        0,
        0,
        0,
        FW_MEDIUM.0 as i32,
        0,
        0,
        0,
        FONT_CHARSET(0),
        FONT_OUTPUT_PRECISION(0),
        FONT_CLIP_PRECISION(0),
        FONT_QUALITY(0),
        0,
        PCWSTR(HSTRING::from(DEFAULT_FONT).as_ptr()),
    );
    if DEFAULT_FONT_HANDLE.0.is_null() {
        // Fall back to the default GUI font on failure.
        // This might also fail, but then we don't have any better fallback.
        let mut lf = LOGFONTW::default();
        SystemParametersInfoW(
            SPI_GETICONTITLELOGFONT,
            core::mem::size_of::<LOGFONTW>() as _,
            Some(&mut lf as *mut LOGFONTW as *mut c_void),
            SYSTEM_PARAMETERS_INFO_UPDATE_FLAGS(0),
        )
        .ok();
        DEFAULT_FONT_HANDLE = CreateFontW(
            -MulDiv(10, GetDeviceCaps(Some(GetDC(None)), LOGPIXELSY), 72),
            0,
            0,
            0,
            FW_MEDIUM.0 as i32,
            0,
            0,
            0,
            FONT_CHARSET(0),
            FONT_OUTPUT_PRECISION(0),
            FONT_CLIP_PRECISION(0),
            FONT_QUALITY(0),
            0,
            PCWSTR(lf.lfFaceName.as_ptr()),
        );
    }
}

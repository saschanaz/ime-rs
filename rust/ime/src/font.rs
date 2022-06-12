#![allow(clippy::missing_safety_doc)]

use std::ffi::c_void;

use windows::{
    core::PCWSTR,
    Win32::{
        Foundation::HWND,
        Graphics::Gdi::{
            CreateFontW, GetDC, GetDeviceCaps, FONT_CLIP_PRECISION, FONT_OUTPUT_PRECISION,
            FONT_PITCH_AND_FAMILY, FONT_QUALITY, FW_MEDIUM, HFONT, LOGFONTW, LOGPIXELSY,
        },
        System::WindowsProgramming::MulDiv,
        UI::WindowsAndMessaging::{
            SystemParametersInfoW, SPI_GETICONTITLELOGFONT, SYSTEM_PARAMETERS_INFO_UPDATE_FLAGS,
        },
    },
};

use crate::resources::DEFAULT_FONT;

#[no_mangle]
pub static mut DEFAULT_FONT_HANDLE: HFONT = HFONT(0);

pub unsafe fn set_default_candidate_text_font() {
    // Candidate Text Font
    if DEFAULT_FONT_HANDLE.0 != 0 {
        return;
    }

    DEFAULT_FONT_HANDLE = CreateFontW(
        -MulDiv(10, GetDeviceCaps(GetDC(HWND(0)), LOGPIXELSY), 72),
        0,
        0,
        0,
        FW_MEDIUM as i32,
        0,
        0,
        0,
        0,
        FONT_OUTPUT_PRECISION(0),
        FONT_CLIP_PRECISION(0),
        FONT_QUALITY(0),
        FONT_PITCH_AND_FAMILY(0),
        DEFAULT_FONT,
    );
    if DEFAULT_FONT_HANDLE.0 == 0 {
        let mut lf = LOGFONTW::default();
        SystemParametersInfoW(
            SPI_GETICONTITLELOGFONT,
            core::mem::size_of::<LOGFONTW>() as _,
            &mut lf as *mut LOGFONTW as *mut c_void,
            SYSTEM_PARAMETERS_INFO_UPDATE_FLAGS(0),
        );
        // Fall back to the default GUI font on failure.
        DEFAULT_FONT_HANDLE = CreateFontW(
            -MulDiv(10, GetDeviceCaps(GetDC(HWND(0)), LOGPIXELSY), 72),
            0,
            0,
            0,
            FW_MEDIUM as i32,
            0,
            0,
            0,
            0,
            FONT_OUTPUT_PRECISION(0),
            FONT_CLIP_PRECISION(0),
            FONT_QUALITY(0),
            FONT_PITCH_AND_FAMILY(0),
            PCWSTR(lf.lfFaceName.as_ptr()),
        );
    }
}

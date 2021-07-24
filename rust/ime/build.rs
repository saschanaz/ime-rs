fn main() {
    windows::build! {
        Windows::Win32::Foundation::{HINSTANCE, MAX_PATH, PWSTR},
        Windows::Win32::System::Com::CoCreateInstance,
        Windows::Win32::System::LibraryLoader::GetModuleFileNameW,
        Windows::Win32::System::SystemServices::{LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED},
        Windows::Win32::UI::TextServices::{
            CLSID_TF_CategoryMgr,
            CLSID_TF_InputProcessorProfiles,
            ITfCategoryMgr,
            ITfInputProcessorProfileMgr,
            GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
            GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
            GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
            GUID_TFCAT_TIPCAP_SECUREMODE,
            GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
            GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
            GUID_TFCAT_TIP_KEYBOARD,
            // GUID_TFCAT_TIPCAP_COMLESS doesn't exist
            // https://github.com/microsoft/win32metadata/issues/575
            HKL,
        },
    };
}

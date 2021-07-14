use globals::SAMPLEIME_CLSID;
use windows::create_instance;
use windows::{self, Guid};
use winreg::{enums::HKEY_CLASSES_ROOT, RegKey};

use crate::bindings::{
    Windows::Win32::Foundation::{HINSTANCE, MAX_PATH, PWSTR},
    Windows::Win32::System::LibraryLoader::GetModuleFileNameW,
    Windows::Win32::UI::TextServices::{
        CLSID_TF_CategoryMgr,
        ITfCategoryMgr,
        GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
        GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
        GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
        GUID_TFCAT_TIPCAP_SECUREMODE,
        GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
        GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
        GUID_TFCAT_TIP_KEYBOARD,
        // GUID_TFCAT_TIPCAP_COMLESS doesn't exist
        // https://github.com/microsoft/win32metadata/issues/575
    },
};

const TEXTSERVICE_DESC: &str = "Sample Rust IME";

static SUPPORT_CATEGORIES: [Guid; 8] = [
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
    GUID_TFCAT_TIPCAP_SECUREMODE,
    // GUID_TFCAT_TIPCAP_COMLESS doesn't exist
    // https://github.com/microsoft/win32metadata/issues/575
    Guid::from_values(
        0x64215d9,
        0x75bc,
        0x11d7,
        [0xa6, 0xef, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c],
    ),
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
];

pub fn register_categories() -> Result<(), windows::Error> {
    let category_manager: ITfCategoryMgr = create_instance(&CLSID_TF_CategoryMgr)?;

    for guid in SUPPORT_CATEGORIES {
        unsafe {
            category_manager.RegisterCategory(&SAMPLEIME_CLSID, &guid, &SAMPLEIME_CLSID)?;
        }
    }

    Ok(())
}

pub fn unregister_categories() -> Result<(), windows::Error> {
    let category_manager: ITfCategoryMgr = create_instance(&CLSID_TF_CategoryMgr)?;

    for guid in SUPPORT_CATEGORIES {
        unsafe {
            category_manager.UnregisterCategory(&SAMPLEIME_CLSID, &guid, &SAMPLEIME_CLSID)?;
        }
    }

    Ok(())
}

fn get_ime_key() -> String {
    format!("CLSID\\{{{:?}}}", SAMPLEIME_CLSID)
}

pub fn register_server(dll_instance_handle: HINSTANCE) -> Result<(), std::io::Error> {
    let module_file_name = unsafe {
        let mut file_name = [0u16; MAX_PATH as usize];
        GetModuleFileNameW(dll_instance_handle, PWSTR(file_name.as_mut_ptr()), MAX_PATH);
        String::from_utf16(&file_name).unwrap()
    };

    let ime_key = get_ime_key();
    let (key, _) = RegKey::predef(HKEY_CLASSES_ROOT).create_subkey(ime_key)?;
    key.set_value("", &TEXTSERVICE_DESC)?;

    let (inproc_key, _) = key.create_subkey("InProcServer32")?;
    inproc_key.set_value("", &module_file_name)?;
    inproc_key.set_value("ThreadingModel", &"Apartment")?;

    Ok(())
}

#[no_mangle]
pub extern "C" fn unregister_server() {
    let ime_key = get_ime_key();
    RegKey::predef(HKEY_CLASSES_ROOT)
        .delete_subkey_all(ime_key)
        .ok();
}

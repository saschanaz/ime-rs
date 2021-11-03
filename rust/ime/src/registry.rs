use globals::{SAMPLEIME_CLSID, SAMPLEIME_GUID_PROFILE};
use windows::runtime::GUID;
use winreg::{enums::HKEY_CLASSES_ROOT, RegKey};

use crate::com::create_instance_inproc;
use windows::Win32::{
    Foundation::{HINSTANCE, MAX_PATH, PWSTR},
    System::LibraryLoader::GetModuleFileNameW,
    System::SystemServices::{LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED},
    UI::TextServices::{
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

const TEXTSERVICE_DESC: &str = "Sample Rust IME";
// MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
const TEXTSERVICE_LANGID: u16 = (SUBLANG_CHINESE_SIMPLIFIED << 10 | LANG_CHINESE) as u16;
// #define TEXTSERVICE_ICON_INDEX   -IDIS_SAMPLEIME
const TEXTSERVICE_ICON_INDEX: u32 = -12i32 as u32;

fn get_module_file_name(dll_instance_handle: HINSTANCE) -> String {
    unsafe {
        let mut file_name = [0u16; MAX_PATH as usize];
        GetModuleFileNameW(dll_instance_handle, PWSTR(file_name.as_mut_ptr()), MAX_PATH);
        String::from_utf16(&file_name).unwrap()
    }
}

pub fn register_profile(dll_instance_handle: HINSTANCE) -> windows::runtime::Result<()> {
    let profile_manager: ITfInputProcessorProfileMgr =
        create_instance_inproc(&CLSID_TF_InputProcessorProfiles)?;

    let mut icon_file_name: Vec<u16> = get_module_file_name(dll_instance_handle)
        .encode_utf16()
        .collect();

    let mut description: Vec<u16> = TEXTSERVICE_DESC.encode_utf16().collect();

    unsafe {
        profile_manager.RegisterProfile(
            &SAMPLEIME_CLSID,
            TEXTSERVICE_LANGID,
            &SAMPLEIME_GUID_PROFILE,
            PWSTR(description.as_mut_ptr()),
            TEXTSERVICE_DESC.len() as u32,
            PWSTR(icon_file_name.as_mut_ptr()),
            icon_file_name.len() as u32,
            TEXTSERVICE_ICON_INDEX as u32,
            HKL::default(),
            0,
            true,
            0,
        )?;
    }

    Ok(())
}

pub fn unregister_profile() -> Result<(), windows::runtime::Error> {
    let profile_manager: ITfInputProcessorProfileMgr =
        create_instance_inproc(&CLSID_TF_InputProcessorProfiles)?;

    unsafe {
        profile_manager.UnregisterProfile(
            &SAMPLEIME_CLSID,
            TEXTSERVICE_LANGID,
            &SAMPLEIME_GUID_PROFILE,
            0,
        )?;
    }

    Ok(())
}

static SUPPORT_CATEGORIES: [GUID; 8] = [
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
    GUID_TFCAT_TIPCAP_SECUREMODE,
    // GUID_TFCAT_TIPCAP_COMLESS doesn't exist
    // https://github.com/microsoft/win32metadata/issues/575
    GUID::from_values(
        0x64215d9,
        0x75bc,
        0x11d7,
        [0xa6, 0xef, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c],
    ),
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
];

pub fn register_categories() -> windows::runtime::Result<()> {
    let category_manager: ITfCategoryMgr = create_instance_inproc(&CLSID_TF_CategoryMgr)?;

    for guid in SUPPORT_CATEGORIES {
        unsafe {
            category_manager.RegisterCategory(&SAMPLEIME_CLSID, &guid, &SAMPLEIME_CLSID)?;
        }
    }

    Ok(())
}

pub fn unregister_categories() -> windows::runtime::Result<()> {
    let category_manager: ITfCategoryMgr = create_instance_inproc(&CLSID_TF_CategoryMgr)?;

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
    let module_file_name = get_module_file_name(dll_instance_handle);

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

use globals::SAMPLEIME_CLSID;
use winreg::{enums::HKEY_CLASSES_ROOT, RegKey};

const TEXTSERVICE_DESC: &str = "Sample Rust IME";

fn get_ime_key() -> String {
    format!("CLSID\\{{{:?}}}", SAMPLEIME_CLSID)
}

pub fn register_server(module_file_name: &str) -> Result<(), std::io::Error> {
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

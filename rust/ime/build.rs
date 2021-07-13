fn main() {
    windows::build! {
        Windows::Win32::Foundation::{HINSTANCE, MAX_PATH, PWSTR},
        Windows::Win32::System::LibraryLoader::GetModuleFileNameW
    };
}

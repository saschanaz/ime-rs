fn main() {
    windows::build!(
      Windows::Win32::Foundation::{HINSTANCE, LPARAM, MAX_PATH, PWSTR, WPARAM},
      Windows::Win32::System::LibraryLoader::GetModuleFileNameW,
      Windows::Win32::UI::KeyboardAndMouseInput::GetKeyState,
      Windows::Win32::UI::WindowsAndMessaging::{VK_CONTROL, VK_MENU, VK_SHIFT}
    );
}

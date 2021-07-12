fn main() {
    windows::build! {
      Windows::Win32::Foundation::{HINSTANCE, LPARAM, MAX_PATH, PWSTR, WPARAM},
      Windows::Win32::System::LibraryLoader::GetModuleFileNameW,
      Windows::Win32::UI::KeyboardAndMouseInput::GetKeyState,
      Windows::Win32::UI::WindowsAndMessaging::{
        VK_CONTROL, VK_MENU, VK_SHIFT,
        VK_SPACE, VK_RETURN, VK_UP, VK_DOWN, VK_PRIOR, VK_NEXT, VK_HOME, VK_END,
        VK_LEFT, VK_RIGHT, VK_ESCAPE, VK_BACK
      }
    };
}

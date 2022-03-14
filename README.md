# ime-rs

This is a porting project for Microsoft Chinese (Simplified, China) IME sample from C++ to Rust.

## Build

* You need to be on 64bit Windows. (Solely because I don't want to complicate the Visual Studio configuration and the build script)
* Install the latest [Visual Studio](https://visualstudio.microsoft.com/downloads/), [Rust](https://www.rust-lang.org/), and [PowerShell](https://www.microsoft.com/store/productId/9MZ1SNWT0N5D).
* Copy-paste `cpp/SampleIME/Dictionary/SampleIMESimplifiedQuanPinUtf8.txt` into `cpp/x64/Debug`.
* Run and choose the x64/Debug configuration, as any other configurations are not supported.

## Deploy

`regsvr32 cpp/x64/Debug/SampleIME.dll` on an admin privilieged terminal.

## Features

These features are all from the original sample and being ported.

* Pinyin input with conversion candidate dictionary: Enabled by <kbd>Shift</kbd> or the language bar button (英/中).
  * Supports wildcard (`*` and `?`), for example, `n*o` shows 你好 and 您好.
* Wide character support: enabled by <kbd>Shift+Spacebar</kbd>. Note that spacebar keyup should happen first or it instead toggles pinyin mode.
  * Converts alphabets to ａｌｐｈａｂｅｔｓ.
  * Converts certain punctuations to their wide versions, e.g. `<>` to `＜＞`.
* Alternative punctuation support: enabled by <kbd>Control+.</kbd>.
  * Produces different characters from punctuation keys, e.g. `$` for `￥`.
  * Some keys just map to their wide versions, e.g. `()` for `（）`.
  * Quotes (`''` and `""`) and brackets (`<>`) automatically change based on the previous input:
    1. `‘` appears first and then `’`. Toggled whenever the key input happens.
    1. `“` appears first and then `”`. Toggled whenever the key input happens.
    1. Opening brackets increments a nested count and closing brackets decrements it. `《》` appears while the count is 0, otherwise `〈〉`.
  * When enabled together with wide character mode, this one wins.

### Removed features

Not all features are alive, because Windows API support has changed since the initial release of the sample.

* Multiple language bar buttons. Originally the sample wanted to add wide character mode and alternative punctuation mode buttons in addition to the pinyin button, but [Windows 8+ allows only a single button](https://docs.microsoft.com/en-us/windows/win32/api/ctfutb/nf-ctfutb-itflangbaritem-getinfo#parameters).
* 24px icon in secure mode. The original demo used 24px icon if the IME was activated with `TF_TMAE_SECUREMODE` flag, but:

  1. It's not clear what "secure mode" means here.
      * A comment in the demo claimed it was about UAC, but the flag never turns on regardless of the privilege, at least on Windows 11 21H2.
  1. It needlessly complicates the code by passing `isSecureMode` only for that specific "feature".
  1. Even if the "secure mode" was a real thing, it doesn't make sense to use higher resolution based on that.

Below is the original README of this sample.

# Input Method Editor (IME) sample

This sample shows how to create an Input Method Editor (IME) that works in Windows Store apps and Windows 8.1 desktop apps.

The sample IME has the following features:

- Uses the Text Services Framework (TSF)
- Runs in base trust
- Compatible with Windows Store apps
- Compatible with Systray and desktop
- Interacts with touch keyboard
- Integrates with Search contract
- Interacts with light-dismiss

The IME sample uses the following code to obtain the parent window:

`pView->GetWnd(&parentWndHandle);`

This implementation works only for Windows Store apps that use the built-in edit controls. This implementation won't work if the app uses the custom edit control from the subset of Text Service Framework (TSF) APIs available in the Windows Runtime. To ensure that the IME gets the proper parent window, so that the owned window is set correctly and works for desktop and Windows Store apps, replace the previous code with the following code:

` if (FAILED(pView->GetWnd(&parentWndHandle)) || (parentWndHandle == nullptr)) { parentWndHandle = GetFocus(); }`

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[Guidelines and checklist for IME development](http://go.microsoft.com/fwlink/p/?linkid=262401)

## Related technologies

[Text Services Framework](http://go.microsoft.com/fwlink/p/?linkid=262402)

## Operating system requirements

Client

Windows 8.1

Server

Windows Server 2012 R2

## Build the sample

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled SampleIME.sln.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

## Run the sample

If you build the IME sample by using Visual Studio 2013, create an installation experience for the IME by using a third-party installer that supports Windows 8.1, like InstallShield from Flexera Software.

The following steps show how to use InstallShield to create a setup project for your IME DLL.

- Install Visual Studio 2013.
- Start Visual Studio 2013.
- On the **File** menu, point to **New** and select **Project**. The **New Project** dialog opens.
- In the left pane, navigate to **Templates \> Other Project Types \> Setup and Deployment**, click **Enable InstallShield Limited Edition**, and click **OK**. Follow the installation instructions.
- Restart Visual Studio 2013.
- Open the IME solution (.sln) file.
- Press F6 to build the solution.
- In Solution Explorer, right-click the solution, point to **Add**, and select **New Project**. The **Add New Project** dialog opens.
- In the left tree view control, navigate to **Templates \> Other Project Types \> InstallShield Limited Edition**.
- In the center window, click **InstallShield Limited Edition Project**.
- In the **Name** text box, type "SetupIME" and click **OK**.
- In the **Project Assistant** dialog, click **Application Information**.
- Fill in your company name and the other fields.
- Click **Application Files**.
- In the left pane, right-click the **[INSTALLDIR]** folder, and select **New Folder**. Name the folder "Plugins".
- Click **Add Files**. Navigate to SampleIME.dll, which is in the C++\\Debug folder, and add it to the **Plugins** folder. Repeat this step for the IME dictionary, which is in the C++\\SampleIME\\Dictionary folder.
- Right-click the IME DLL and select **Properties**. The **Properties** dialog opens.
- In the **Properties** dialog, click the **COM & .NET Settings** tab.
- Under **Registration Type**, select **Self-registration** and click **OK**.
- Build the solution. The IME DLL is built, and InstallShield creates a setup.exe file that enables users to install your IME on Windows 8.1. The setup.exe file is located in the SetupIME\\SetupIME\\Express\\DVD-5\\DiskImages\\DISK1 folder.
